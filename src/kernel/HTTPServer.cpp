#include "HTTPServer.hpp"
#include "ThreadPool.hpp"
#include <memory> // Pour std::shared_ptr

namespace Softadastra
{

    HTTPServer::HTTPServer(Config &config)
        : config_(config),
          io_context_(std::make_unique<net::io_context>()),
          acceptor_(std::make_unique<tcp::acceptor>(*io_context_,
                                                    tcp::endpoint(tcp::v4(), static_cast<unsigned short>(config_.getServerPort())))),
          router_(),                                                        // Initialisez le Router ici
          route_configurator_(std::make_unique<RouteConfigurator>(router_)) // Passez une référence à router_
    {
    }

    void HTTPServer::run()
    {
        route_configurator_->configure_routes(); // Configure les routes via RouteConfigurator

        spdlog::info("Server started on port {}", config_.getServerPort());
        spdlog::info("Waiting for incoming connections...");

        // Crée un ThreadPool avec un nombre de threads fixés pour traiter les connexions
        ThreadPool pool(6); // Nombre de threads dans le pool

        while (true)
        {
            tcp::socket socket{*io_context_};
            acceptor_->accept(socket);
            spdlog::info("Client connected!");

            // Capture le socket dans un shared_ptr et le passe au ThreadPool
            auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));

            // Enqueue la tâche dans le pool de threads, et passer le shared_ptr
            pool.enqueue([this, socket_ptr, &router = router_]()
                         { handle_client(socket_ptr, router); });
        }
    }

    // Modifier la signature de handle_client pour accepter un shared_ptr
    void HTTPServer::handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router)
    {
        Session(std::move(*socket_ptr), router).run();
    }

}
