#ifndef HOMEROUTES_HPP
#define HOMEROUTES_HPP

#include "RouteConfigurator.hpp"
#include "SimpleRequestHandler.hpp"
#include "DynamicRequestHandler.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "IRoutes.hpp"

using json = nlohmann::json;

namespace Softadastra
{
    class HomeRoutes : public IRoutes
    {
    public:
        void configure(Router &routes) override
        {
            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<SimpleRequestHandler>(
                        [](const http::request<http::string_body> &,
                           http::response<http::string_body> &res)
                        {
                            res.result(http::status::ok);
                            res.set(http::field::content_type, "application/json");
                            res.body() = json{{"message", "Bienvenue sur notre backend web"}}.dump();
                        })));

            routes.add_route(http::verb::post, "/create",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<SimpleRequestHandler>(
                                     [](const http::request<http::string_body> &req,
                                        http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             auto json_data = json::parse(req.body());

                                             if (json_data.find("name") == json_data.end())
                                             {
                                                 res.result(http::status::bad_request);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = json{{"message", "Missing 'name' field"}}.dump();
                                                 return;
                                             }

                                             std::string name = json_data["name"];

                                             res.result(http::status::ok);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", std::string("Hello, ") + name}}.dump();
                                         }
                                         catch (const std::exception &e)
                                         {
                                             res.result(http::status::internal_server_error);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", std::string("Error processing the request") + e.what()}}.dump();
                                         }
                                     })));
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
