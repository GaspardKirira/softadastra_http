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
#include <boost/filesystem.hpp>
#include <openssl/err.h>

namespace Softadastra
{

    // Constructeur du serveur HTTP avec SSL
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
<<<<<<< HEAD
        try
        {
            // Vérification des chemins des certificats SSL
            const std::string cert_path = "../certs/server-cert.pem";
            const std::string key_path = "../certs/server-key.pem";

            if (!boost::filesystem::exists(cert_path) || !boost::filesystem::exists(key_path))
            {
                spdlog::error("Certificate or private key file not found! Path: cert: {}, key: {}", cert_path, key_path);
                throw std::runtime_error("Certificate or private key file not found!");
            }

            // Ajout de logs détaillés pour chaque fichier
            spdlog::info("Loading SSL certificate from: {}", cert_path);
            ssl_context_.use_certificate_chain_file(cert_path);
            try
            {
                // Chargement du certificat
                spdlog::info("Loading SSL certificate from: {}", cert_path);
                ssl_context_.use_certificate_chain_file(cert_path);

                // Vérification de la validité du certificat après chargement
                spdlog::info("SSL certificate loaded successfully from: {}", cert_path);
            }
            catch (const boost::system::system_error &e)
            {
                spdlog::error("Failed to load SSL certificate from: {}. Error: {}", cert_path, e.what());
                throw std::runtime_error("Failed to load SSL certificate.");
            }

            try
            {
                // Chargement de la clé privée
                spdlog::info("Loading SSL private key from: {}", key_path);
                ssl_context_.use_private_key_file(key_path, boost::asio::ssl::context::pem);

                // Vérification de la clé privée après chargement
                spdlog::info("SSL private key loaded successfully from: {}", key_path);
            }
            catch (const boost::system::system_error &e)
            {
                spdlog::error("Failed to load SSL private key from: {}. Error: {}", key_path, e.what());
                throw std::runtime_error("Failed to load SSL private key.");
            }

            // Configuration des options SSL
            ssl_context_.set_verify_mode(boost::asio::ssl::verify_none); // Désactivation de la vérification des certificats des clients

            // Désactiver SSLv2 et forcer TLSv1.2
            ssl_context_.set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::no_tlsv1 |
                boost::asio::ssl::context::no_tlsv1_1 |
                boost::asio::ssl::context::single_dh_use);

            // Fonction de callback pour la vérification du certificat
            ssl_context_.set_verify_callback([](bool preverified, boost::asio::ssl::verify_context &verify_context)
                                             {
                                                 if (!preverified)
                                                 {
                                                     spdlog::warn("SSL verification was not preverified (but is disabled).");

                                                     unsigned long errors = ERR_get_error();
                                                     if (errors)
                                                     {
                                                         char err_buff[256];
                                                         ERR_error_string_n(errors, err_buff, sizeof(err_buff));
                                                         spdlog::warn("SSL verification error (though disabled): {}", err_buff);
                                                     }

                                                     // Information supplémentaire sur le certificat échoué
                                                     X509 *cert = X509_STORE_CTX_get_current_cert(verify_context.native_handle());
                                                     if (cert)
                                                     {
                                                         char *line = nullptr;
                                                         BIO *bio = BIO_new(BIO_s_mem());
                                                         X509_print(bio, cert);
                                                         long len = BIO_get_mem_data(bio, &line);
                                                         std::string cert_info(line, len);
                                                         BIO_free(bio);
                                                         spdlog::warn("Failed certificate info (though disabled):\n{}", cert_info);
                                                     }
                                                 }
                                                 return true; // Toujours retourner true pour accepter la connexion
                                             });

            spdlog::info("SSL/TLS context initialized successfully (client certificate verification disabled).");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error initializing SSL context: {}", e.what());
            throw;
        }

        // Validation du port
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
        try
        {
            route_configurator_->configure_routes();
            spdlog::info("Routes configured successfully.");

            // Informations initiales sur le serveur
            spdlog::info("Softadastra/master server is running at https://127.0.0.1:{}", config_.getServerPort());
            spdlog::info("Softadastra/master server is running at http://127.0.0.1:{}", config_.getServerPort());
            spdlog::info("Waiting for incoming connections...");

            start_accept();
            spdlog::info("Started accepting connections.");

            for (std::size_t i = 0; i < NUMBER_OF_THREADS; ++i)
            {
                io_threads_.emplace_back([this, i]()
                                         {
                                         try
                                         {
                                             spdlog::info("Thread {} started running io_context.", i);
                                             io_context_->run();  // Lancer l'io_context dans ce thread
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
            spdlog::info("All io_context threads finished.");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in HTTPServer::run(): {}", e.what());
        }
    }
    void HTTPServer::start_accept()
    {
        auto socket = std::make_shared<ssl::stream<tcp::socket>>(*io_context_, ssl_context_);
        auto socket = std::make_shared<tcp::socket>(*io_context_);

        try
        {
            acceptor_->async_accept(socket->lowest_layer(), [this, socket](boost::system::error_code ec)
                                    {
                                        if (!ec)
                                        {
                                            spdlog::info("Client connected from: {}", socket->lowest_layer().remote_endpoint().address().to_string());

                                            // Désactiver temporairement les tickets de session
                                            // Note: Ceci est juste un test; ajustez selon vos besoins réels
                                            // SSL_CTX_set_session_cache_mode(ssl_context_.native_handle(), SSL_SESS_CACHE_OFF);

                                            socket->async_handshake(ssl::stream_base::server,
                                                                    [this, socket](boost::system::error_code ec)
                                                                    {
                                                                        if (!ec)
                                                                        {
                                                                            spdlog::info("SSL handshake successful.");
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
                                                                            log_ssl_error(ec, socket);
                                                                            close_socket(socket);
                                                                        }
                                                                    });
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

    // Log détaillé pour les erreurs de handshake SSL
    void HTTPServer::log_ssl_error(const boost::system::error_code &ec, std::shared_ptr<ssl::stream<tcp::socket>>)
    {
        spdlog::error("SSL handshake failed with error code {}: {}", ec.value(), ec.message());

        if (ec == boost::asio::error::eof)
        {
            spdlog::error("SSL handshake failed due to unexpected EOF.");
        }
        else if (ec == boost::asio::error::connection_reset)
        {
            spdlog::error("SSL handshake failed due to connection reset by peer.");
        }
        else
        {
            spdlog::error("SSL handshake failed with unknown error.");
        }

        // Log des erreurs OpenSSL supplémentaires
        unsigned long ssl_error = ERR_get_error();
        while (ssl_error != 0)
        {
            char err_buff[256];
            ERR_error_string_n(ssl_error, err_buff, sizeof(err_buff));
            spdlog::error("OpenSSL Error: {}", err_buff);
            ssl_error = ERR_get_error();
            spdlog::error("Error in client session for client {}: {}", socket_ptr->remote_endpoint().address().to_string(), e.what());
            socket_ptr->close();
        }
    }

    void HTTPServer::close_socket(std::shared_ptr<ssl::stream<tcp::socket>> socket)
    {
        boost::system::error_code ec;
        socket->lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != boost::system::error_code{})
        {
            spdlog::error("Failed to shutdown socket: {} (Error code: {})", ec.message(), ec.value());
        }
        socket->lowest_layer().close(ec);
        if (ec && ec != boost::system::error_code{})
        {
            spdlog::error("Failed to close socket: {} (Error code: {})", ec.message(), ec.value());
        }
    }

    void HTTPServer::handle_client(std::shared_ptr<ssl::stream<tcp::socket>> socket_ptr, Router &router)
    {
        try
        {
            spdlog::info("Starting client session for: {}", socket_ptr->lowest_layer().remote_endpoint().address().to_string());
            auto session = std::make_shared<Session>(std::move(*socket_ptr), router);
            session->run();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in client session for client {}: {}", socket_ptr->lowest_layer().remote_endpoint().address().to_string(), e.what());
            socket_ptr->lowest_layer().close(); // Attempt to close the socket if error occurs
            spdlog::error("Session handler failed for client {} with exception: {}", socket_ptr->lowest_layer().remote_endpoint().address().to_string(), e.what());
        }
    }

} // namespace Softadastra
