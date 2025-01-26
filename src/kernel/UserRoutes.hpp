#ifndef USERROUTES_HPP
#define USERROUTES_HPP

#include "RouteConfigurator.hpp"
#include "DynamicRequestHandler.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include "IRoutes.hpp"

namespace Softadastra
{
    class UserRoutes: public IRoutes
    {
    public:
        void configure(Softadastra::Router &router) override
        {
            router.add_route(http::verb::get, "/users/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [](const std::unordered_map<std::string, std::string> &params,
                                        http::response<http::string_body> &res)
                                     {
                                         std::string user_id = params.at("id");
                                         res.result(http::status::ok);
                                         res.set(http::field::content_type, "application/json");
                                         res.body() = json{{"message", "User details for id: " + user_id}}.dump();
                                     })));
        }
    };

}

#endif // USERROUTES_HPP
