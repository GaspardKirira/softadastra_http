#include "HTTPServer.hpp"
#include "ThreadPool.hpp"
#include <memory> // Pour std::shared_ptr

namespace Softadastra
{

    HTTPServer::HTTPServer(Config &config)
        : config_(config),
          io_context_(std::make_unique<net::io_context>()),
          acceptor_(std::make_unique<tcp::acceptor>(*io_context_,
                                                    tcp::endpoint(tcp::v6(), static_cast<unsigned short>(config_.getServerPort())))), // Écoute sur IPv6 et IPv4
          router_(),
          route_configurator_(std::make_unique<RouteConfigurator>(router_)) // Passez une référence à router_
    {
    }

    void HTTPServer::run()
    {
        route_configurator_->configure_routes();

        spdlog::info("Softadastra/master server is running at http://[::1]:{} using {} threads", config_.getServerPort(), NUMBER_OF_THREADS);
        spdlog::info("Waiting for incoming connections...");

        // Démarre l'acceptation asynchrone
        start_accept();

        // Fait tourner le contexte Asio dans plusieurs threads
        std::vector<std::thread> threads;
        for (std::size_t i = 0; i < NUMBER_OF_THREADS; ++i)
        {
            threads.emplace_back([this]
                                 { io_context_->run(); });
        }

        // Attendre que tous les threads se terminent
        for (auto &thread : threads)
        {
            thread.join();
        }
    }

    void HTTPServer::start_accept()
    {
        auto socket = std::make_shared<tcp::socket>(*io_context_);
        acceptor_->async_accept(*socket, [this, socket](boost::system::error_code ec)
                                {
                                if (!ec)
                                {
                                    spdlog::info("Client connected!");
                                    handle_client(socket, router_);
                                }
                                else
                                {
                                    spdlog::error("Error accepting connection: {}", ec.message());
                                }

                                // Continue à accepter d'autres connexions
                                start_accept(); });
    }

    void HTTPServer::handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router)
    {
        auto session = std::make_shared<Session>(std::move(*socket_ptr), router);
        session->run();
    }

}
