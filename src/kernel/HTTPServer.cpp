#include "ThreadPool.hpp"
#include "HTTPServer.hpp"
#include <memory>
#include <thread>
#include <vector>
#include <system_error>
#include <boost/system/error_code.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>

namespace Softadastra
{
    HTTPServer::HTTPServer(Config &config)
        : config_(config),
          io_context_(std::make_shared<net::io_context>()),
          acceptor_(nullptr),
          router_(),
          route_configurator_(std::make_unique<RouteConfigurator>(router_, config_)),
          request_thread_pool_(NUMBER_OF_THREADS),
          io_threads_(),
          ssl_context_(ssl::context::sslv23) // Crée un contexte SSL/TLS
    {
        try
        {
            spdlog::info("Loading SSL certificate from certs/server-cert.pem");
            ssl_context_.use_certificate_chain_file("../certs/server-cert.pem");

            spdlog::info("Loading SSL private key from certs/server-key.pem");
            ssl_context_.use_private_key_file("../certs/server-key.pem", boost::asio::ssl::context::pem);

            ssl_context_.set_verify_mode(boost::asio::ssl::verify_none);

            ssl_context_.set_options(ssl::context::default_workarounds |
                                     ssl::context::no_sslv2 |
                                     ssl::context::single_dh_use);

            // Vérification SSL
            ssl_context_.set_verify_callback([](bool preverified, boost::asio::ssl::verify_context &)
                                             {
                if (!preverified)
                {
                    spdlog::error("SSL verification failed.");
                    // Loguer le message d'erreur de vérification du certificat
                    unsigned long errors = ERR_get_error();
                    if (errors)
                    {
                        char err_buff[256];
                        ERR_error_string_n(errors, err_buff, sizeof(err_buff));
                        spdlog::error("SSL verification error: {}", err_buff);
                    }
                }
                return preverified; });

            spdlog::info("SSL/TLS context initialized successfully.");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error initializing SSL context: {}", e.what());
            throw;
        }

        int newPort = config_.getServerPort();
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
    }

    void HTTPServer::run()
    {
        route_configurator_->configure_routes();

        spdlog::info("Softadastra/master server is running at https://127.0.0.1:{}", config_.getServerPort());
        spdlog::info("Waiting for incoming connections...");

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
                } });
        }

        for (auto &t : io_threads_)
        {
            if (t.joinable())
                t.join();
        }
    }

    void HTTPServer::start_accept()
    {
        auto socket = std::make_shared<ssl::stream<tcp::socket>>(*io_context_, ssl_context_); 

        try
        {
            acceptor_->async_accept(socket->lowest_layer(), [this, socket](boost::system::error_code ec)
                                    {
            if (!ec)
            {
                spdlog::info("Client connected!");
                // Logge la tentative de handshake SSL
                socket->async_handshake(ssl::stream_base::server, 
                    [this, socket](boost::system::error_code ec)
                    {
                        if (!ec)
                        {
                            spdlog::info("SSL handshake successful.");
                            // Une fois le handshake SSL/TLS réussi, traitement la requête
                            request_thread_pool_.enqueue([this, socket]()
                            {
                                try
                                {
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
                            spdlog::error("SSL handshake failed: {}", ec.message());
                            // Logge l'erreur SSL spécifique
                            spdlog::error("SSL handshake failed with error code: {}", ec.value());
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

    void HTTPServer::handle_client(std::shared_ptr<ssl::stream<tcp::socket>> socket_ptr, Router &router)
    {
        try
        {
            auto session = std::make_shared<Session>(std::move(*socket_ptr), router);
            session->run();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in client session: {}", e.what());
        }
    }

} // namespace Softadastra
