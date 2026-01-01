#ifndef ROUTECONFIGURATOR_HPP
#define ROUTECONFIGURATOR_HPP

#include "routing/Router.hpp"
#include "routing/SimpleRequestHandler.hpp"
#include "routing/DynamicRequestHandler.hpp"
#include "routing/IRequestHandler.hpp"
#include "Config.hpp"
#include <unordered_map>
#include <string>
#include <memory>

namespace Softadastra
{
    class RouteConfigurator
    {
    public:
        explicit RouteConfigurator(Router &router);
        void configure_routes();

    private:
        Router &router_;
    };
}

#endif // ROUTECONFIGURATOR_HPP
