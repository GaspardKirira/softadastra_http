#ifndef IROUTES_HPP
#define IROUTES_HPP

#include "routing/Router.hpp"
#include "config/Config.hpp"
#include "http/Response.hpp"
#include "routing/IRequestHandler.hpp"
#include "routing/UnifiedRequestHandler.hpp"

namespace Softadastra
{
    class Controller
    {
    public:
        explicit Controller(Config &config) : config_(config) {}
        virtual ~Controller() = default;
        virtual void configure(Router &router) = 0;

    protected:
        Config &config_;
        template <typename Handler>
        void add_route(Router &router, http::verb method, const std::string &path, Handler handler)
        {
            router.add_route(
                method, path,
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<UnifiedRequestHandler>(handler)));
        }
    };

} // namespace Softadastra

#endif // IROUTES_HPP