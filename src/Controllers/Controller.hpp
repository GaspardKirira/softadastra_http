#ifndef IROUTES_HPP
#define IROUTES_HPP

#include "../config/RouteConfigurator.hpp"
#include "../kernel/SimpleRequestHandler.hpp"
#include "../kernel/DynamicRequestHandler.hpp"
#include "../kernel/Router.hpp"

namespace Softadastra
{
    class Controller
    {
    public:
        virtual ~Controller() = default;
        virtual void configure(Router &routes) = 0;
    };
} // namespace Softadastra

#endif // IROUTES_HPP
