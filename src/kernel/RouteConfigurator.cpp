#include "../ProductController.hpp"
#include "../UserController.hpp"
#include "../HomeController.hpp"
#include <memory>

#include "RouteConfigurator.hpp"

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
