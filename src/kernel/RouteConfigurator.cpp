#include "ProductRoutes.hpp"
#include "UserRoutes.hpp"
#include "HomeRoutes.hpp"
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
        HomeRoutes homeRoutes;
        homeRoutes.configure(router_);

        ProductRoutes productRoutes;
        productRoutes.configure(router_);

        UserRoutes userRoutes;
        userRoutes.configure(router_);
    }
}
