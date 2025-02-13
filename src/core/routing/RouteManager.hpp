#pragma once
#include "HTTPServer.hpp"
#include <functional>
#include <vector>

namespace Softadastra
{
    // Déclaration du gestionnaire de routes
    class RouteManager
    {
    public:
        using RouteHandler = std::function<void(Router &router)>;

        // Ajoute une route à la configuration
        void add_route(RouteHandler handler)
        {
            route_handlers_.push_back(handler);
        }

        // Configure toutes les routes en appelant chaque gestionnaire
        void setup_routes(Router &router)
        {
            for (auto &handler : route_handlers_)
            {
                handler(router); // Applique chaque gestionnaire de route
            }
        }

    private:
        std::vector<RouteHandler> route_handlers_; // Liste des gestionnaires de route
    };
}
