#include "RouteConfigurator.hpp"
#include "../Controllers/ProductController.hpp"
#include "../Controllers/UserController.hpp"
#include "../Controllers/HomeController.hpp"

namespace Softadastra
{
    RouteConfigurator::RouteConfigurator(Router &router)
        : router_(router)
    {
    }

    void RouteConfigurator::configure_routes()
    {
        HomeController homeController;
        homeController.configure(router_);

        ProductController productController;
        productController.configure(router_);

        UserController userController;
        userController.configure(router_);
    }
}
