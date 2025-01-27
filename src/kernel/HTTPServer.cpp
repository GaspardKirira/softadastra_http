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
          router_(),                                                        
          route_configurator_(std::make_unique<RouteConfigurator>(router_)) // Passez une référence à router_
    {
    }

    void HTTPServer::run()
    {
        route_configurator_->configure_routes(); // Configure les routes via RouteConfigurator

        // spdlog::info("Server started on port {}", config_.getServerPort());
        spdlog::info("Softadastra/master server is running at http://127.0.0.1:{} using {} threads", config_.getServerPort(), NUMBER_OF_THREADS);
        spdlog::info("Waiting for incoming connections...");

        // Crée un ThreadPool avec un nombre de threads fixés pour traiter les connexions
        ThreadPool pool(NUMBER_OF_THREADS); // Nombre de threads dans le pool

        while (true)
        {
            tcp::socket socket{*io_context_};
            acceptor_->accept(socket);
            spdlog::info("Client connected!");

            // Capture le socket dans un shared_ptr et le passe au ThreadPool
            auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));

            // Enqueue la tâche dans le pool de threads
            if (!pool.enqueue([this, socket_ptr, &router = router_]()
                              { handle_client(socket_ptr, router); }))
            {
                // Si la tâche a été rejetée, envoyer une réponse HTTP 503 (Service Unavailable)
                http::response<http::string_body> res;
                res.result(http::status::service_unavailable);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Server is overloaded, try again later."})";

                http::write(*socket_ptr, res);
                socket_ptr->shutdown(tcp::socket::shutdown_both);
            }
        }
    }

    // Modifier la signature de handle_client pour accepter un shared_ptr
    void HTTPServer::handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router)
    {
        Session(std::move(*socket_ptr), router).run();
    }

}
