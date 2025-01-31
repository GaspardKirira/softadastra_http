#include "ThreadPool.hpp"
#include "HTTPServer.hpp"
#include <memory>
#include <thread>
#include <vector>
#include <system_error>
#include <boost/system/error_code.hpp>

namespace Softadastra
{

    HTTPServer::HTTPServer(Config &config)
        : config_(config),
          io_context_(std::make_shared<net::io_context>()),
          acceptor_(nullptr),
          router_(),
          route_configurator_(std::make_unique<RouteConfigurator>(router_, config_)),
          request_thread_pool_(NUMBER_OF_THREADS),
          io_threads_()
    {
        try
        {
            spdlog::info("Initializing server on port {}", config_.getServerPort());

            int newPort = config_.getServerPort();
            // if (newPort == 8080)
            //     newPort = 9090;

            if (newPort < 1024 || newPort > 65535)
            {
                spdlog::error("Invalid port number: {}. Must be between 1024 and 65535.", newPort);
                throw std::invalid_argument("Port number out of range (1024-65535)");
            }

            tcp::endpoint endpoint(boost::asio::ip::address_v4::any(), static_cast<unsigned short>(newPort));
            acceptor_ = std::make_unique<tcp::acceptor>(*io_context_);

            boost::system::error_code ec;
            acceptor_->open(endpoint.protocol(), ec);
            if (ec)
            {
                spdlog::error("Failed to open acceptor socket: {} (Error code: {})", ec.message(), ec.value());
                throw std::system_error(ec, "Could not open the acceptor socket");
            }

            acceptor_->set_option(boost::asio::socket_base::reuse_address(true), ec);
            if (ec)
            {
                spdlog::error("Failed to set socket option: {} (Error code: {})", ec.message(), ec.value());
                throw std::system_error(ec, "Could not set socket option");
            }

            acceptor_->bind(endpoint, ec);
            if (ec)
            {
                spdlog::error("Failed to bind to the server port: {} (Error code: {})", ec.message(), ec.value());
                throw std::system_error(ec, "Could not bind to the server port");
            }

            acceptor_->listen(boost::asio::socket_base::max_connections, ec);
            if (ec)
            {
                spdlog::error("Failed to listen on the server port: {} (Error code: {})", ec.message(), ec.value());
                throw std::system_error(ec, "Could not listen on the server port");
            }

            spdlog::info("Server started successfully on port {}", newPort);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error initializing HTTPServer: {}", e.what());
            throw;
        }
    }

    void HTTPServer::run()
    {
        route_configurator_->configure_routes();

        spdlog::info("Softadastra/master server is running at http://127.0.0.1:{} using {} threads", config_.getServerPort(), NUMBER_OF_THREADS);
        spdlog::info("Waiting for incoming connections...");

        start_accept();

        // On ajoute des threads pour io_context_
        for (std::size_t i = 0; i < NUMBER_OF_THREADS; ++i)
        {
            io_threads_.emplace_back([this, i]()
                                     {
            try
            {
                spdlog::info("I/O Thread {} started.", i);
                io_context_->run();
            }
            catch (const std::exception &e)
            {
                spdlog::error("Error in io_context (thread {}): {}", i, e.what());
            } });
        }

        // Attendre que les threads terminent proprement
        for (auto &t : io_threads_)
        {
            if (t.joinable())
                t.join();
        }
    }

    void HTTPServer::start_accept()
    {
        auto socket = std::make_shared<tcp::socket>(*io_context_);

        try
        {
            spdlog::info("Starting async_accept...");
            acceptor_->async_accept(*socket, [this, socket](boost::system::error_code ec)
                                    {
            if (!ec)
            {
                spdlog::info("Client connected!");
                request_thread_pool_.enqueue([this, socket]()
                {
                    try
                    {
                        spdlog::info("Handling new client session...");
                        handle_client(socket, router_);
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Error handling client: {}", e.what());
                    }
                });
            }
            else
            {
                spdlog::error("Error accepting connection: {} (Error code: {})", ec.message(), ec.value());
            }

            start_accept(); });
        }
        catch (const std::exception &e)
        {
            spdlog::error("Exception during async_accept: {}", e.what());
        }
    }

    void HTTPServer::handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router)
    {
        try
        {
            spdlog::info("Starting client session...");
            auto session = std::make_shared<Session>(std::move(*socket_ptr), router);
            session->run();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in client session: {}", e.what());
        }
    }

}
