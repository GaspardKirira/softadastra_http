#include "RouteConfigurator.hpp"
#include "../Controllers/ProductController.hpp"
#include "../Controllers/UserController.hpp"
#include "../Controllers/HomeController.hpp"
#include <memory>

namespace Softadastra
{
    RouteConfigurator::RouteConfigurator(Router &router, Config &config)
        : router_(router), config_(config)
    {
    }

    void RouteConfigurator::configure_routes()
    {
        Config &config = Config::getInstance();
        config.loadConfig();

        auto homeController = std::make_unique<HomeController>(config);
        homeController->configure(router_);

        auto productController = std::make_unique<ProductController>(config);
        productController->configure(router_);

        auto userController = std::make_shared<UserController>(config); // Utilisation d'un shared_ptr
        userController->configure(router_);
    }

}
