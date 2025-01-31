#include "RouteConfigurator.hpp"
#include "../Controllers/ProductController.hpp"
#include "../Controllers/UserController.hpp"
#include "../Controllers/HomeController.hpp"

namespace Softadastra
{
    RouteConfigurator::RouteConfigurator(Router &router, Config &config)
        : router_(router), config_(config)
    {
    }

    void RouteConfigurator::configure_routes()
    {
        // Créer des contrôleurs en tant qu'objets dynamiques (avec des pointeurs intelligents)
        auto homeController = std::make_unique<HomeController>(config_);
        homeController->configure(router_);

        auto productController = std::make_unique<ProductController>(config_);
        productController->configure(router_);

        auto userController = std::make_unique<UserController>(config_);
        userController->configure(router_);

        // Les objets contrôleurs sont maintenant persistants jusqu'à la fin de cette méthode
    }
}
