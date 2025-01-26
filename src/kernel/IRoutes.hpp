#ifndef IROUTES_HPP
#define IROUTES_HPP

#include "Router.hpp"

namespace Softadastra
{
    class IRoutes
    {
    public:
        virtual ~IRoutes() = default;
        virtual void configure(Router &routes) = 0;
    };
} // namespace Softadastra

#endif // IROUTES_HPP
