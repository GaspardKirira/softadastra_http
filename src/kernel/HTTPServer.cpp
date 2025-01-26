#include "HTTPServer.hpp"

namespace Softadastra
{

    Softadastra::HTTPServer::HTTPServer(Config &config)
        : config_(config),
          io_context_(std::make_unique<net::io_context>()),
          acceptor_(std::make_unique<tcp::acceptor>(*io_context_,
                                                    tcp::endpoint(tcp::v4(), static_cast<unsigned short>(config_.getServerPort())))),
          router_(),                                                        // Initialisez le Router ici
          route_configurator_(std::make_unique<RouteConfigurator>(router_)) // Passez une référence à router_
    {
    }

    void Softadastra::HTTPServer::run()
    {
        route_configurator_->configure_routes(); // Configure les routes via RouteConfigurator

        spdlog::info("Server started on port {}", config_.getServerPort());
        spdlog::info("Waiting for incoming connections...");

        while (true)
        {
            tcp::socket socket{*io_context_};
            acceptor_->accept(socket);
            spdlog::info("Client connected!");
            std::thread{&HTTPServer::handle_client, this, std::move(socket), std::ref(router_)}.detach();
        }
    }

    void Softadastra::HTTPServer::handle_client(tcp::socket socket, Router &router)
    {
        Session(std::move(socket), router).run();
    }

}
