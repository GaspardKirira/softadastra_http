#include "ThreadPool.hpp"
#include "HTTPServer.hpp"
#include <memory>
#include <thread>
#include <vector>
#include <system_error>
#include <boost/system/error_code.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>
#include "setup_routes.hpp"

namespace Softadastra
{
    HTTPServer::HTTPServer(Config &config)
        : config_(config),
          io_context_(std::make_shared<net::io_context>()),
          acceptor_(nullptr),
          router_(),
          route_configurator_(std::make_unique<RouteConfigurator>(router_)),
          request_thread_pool_(NUMBER_OF_THREADS),
          io_threads_()
    {
        try
        {
            // Validation du port
            int newPort = config_.getServerPort();
            if (newPort < 1024 || newPort > 65535)
            {
                spdlog::error("Invalid port number: {}. Must be between 1024 and 65535.", newPort);
                throw std::invalid_argument("Port number out of range (1024-65535)");
            }

            // Configuration de l'acceptor avec gestion d'erreurs détaillées
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
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error initializing server: {}", e.what());
            throw;
        }
    }

    void HTTPServer::run()
    {
        try
        {
            route_configurator_->configure_routes();

            spdlog::info("Softadastra/master server is running at http://127.0.0.1:{} using {} threads", config_.getServerPort(), NUMBER_OF_THREADS);
            spdlog::info("Waiting for incoming connections...");

            // Démarrer l'acceptation des connexions (avant de lancer les threads)
            start_accept();

            for (std::size_t i = 0; i < NUMBER_OF_THREADS; ++i)
            {
                io_threads_.emplace_back([this, i]()
                                         {
                    try
                    {
                        io_context_->run();
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Error in io_context (thread {}): {}", i, e.what());
                    }
                    spdlog::info("Thread {} finished.", i); });
            }

            // Attendre que tous les threads aient terminé
            for (auto &t : io_threads_)
            {
                if (t.joinable())
                    t.join();
            }
            spdlog::info("All io_context threads finished.");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in HTTPServer::run(): {}", e.what());
        }
    }

    void HTTPServer::start_accept()
    {
        auto socket = std::make_shared<tcp::socket>(*io_context_);

        try
        {
            acceptor_->async_accept(*socket, [this, socket](boost::system::error_code ec)
                                    {
                                        if (!ec)
                                        {
                                            spdlog::info("Client connected from: {}", socket->remote_endpoint().address().to_string());
                                            request_thread_pool_.enqueue([this, socket]()
                                                                         {
                                                             try
                                                             {
                                                                 handle_client(socket, router_);
                                                             }
                                                             catch (const std::exception &e)
                                                             {
                                                                 spdlog::error("Error handling client: {}", e.what());
                                                                 close_socket(socket);
                                                             } });
                                        }
                                        else
                                        {
                                            spdlog::error("Error accepting connection from client: {} (Error code: {})", ec.message(), ec.value());
                                        }

                                        start_accept(); // Continue accepting new connections
                                    });
        }
        catch (const std::exception &e)
        {
            spdlog::error("Exception during async_accept: {}", e.what());
            acceptor_->close();
        }
    }

    void HTTPServer::close_socket(std::shared_ptr<tcp::socket> socket)
    {
        boost::system::error_code ec;
        socket->shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != boost::system::error_code{})
        {
            spdlog::error("Failed to shutdown socket: {} (Error code: {})", ec.message(), ec.value());
        }
        socket->close(ec);
        if (ec && ec != boost::system::error_code{})
        {
            spdlog::error("Failed to close socket: {} (Error code: {})", ec.message(), ec.value());
        }
    }

    void HTTPServer::handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router)
    {
        try
        {
            spdlog::info("Starting client session for: {}", socket_ptr->remote_endpoint().address().to_string());
            auto session = std::make_shared<Session>(std::move(*socket_ptr), router);
            session->run();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in client session for client {}: {}", socket_ptr->remote_endpoint().address().to_string(), e.what());
            socket_ptr->close(); // Attempt to close the socket if error occurs
            spdlog::error("Session handler failed for client {} with exception: {}", socket_ptr->remote_endpoint().address().to_string(), e.what());
        }
    }
} // namespace Softadastra
