#ifndef PRODUCTROUTES_HPP
#define PRODUCTROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include "Controller.hpp"

namespace Softadastra
{

    class ProductController : public Controller
    {
    public:
        void configure(Router &router) override
        {
            router.add_route(http::verb::get, "/products/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [](const std::unordered_map<std::string, std::string> &params,
                                        http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::string product_id = params.at("id");
                                             res.result(http::status::ok);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", "Product details for id: " + product_id}}.dump();
                                         }
                                         catch (const std::out_of_range &e)
                                         {
                                             throw std::invalid_argument("Missing required parameter: 'id'");
                                         }
                                     })));

            router.add_route(http::verb::get, "/products/{slug}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [](const std::unordered_map<std::string, std::string> &params,
                                        http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::string product_slug = params.at("slug");
                                             res.result(http::status::ok);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", "Product details for slug: " + product_slug}}.dump();
                                         }
                                         catch (const std::out_of_range &e)
                                         {
                                             throw std::invalid_argument("Missing required parameter: 'slug'");
                                         }
                                     })));

            router.add_route(http::verb::get, "/products/{id}/{slug}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [](const std::unordered_map<std::string, std::string> &params,
                                        http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::string product_id = params.at("id");
                                             std::string product_slug = params.at("slug");

                                             res.result(http::status::ok);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", "Product details for id: " + product_id + " and slug: " + product_slug}}.dump();
                                         }
                                         catch (const std::out_of_range &e)
                                         {
                                             throw std::invalid_argument("Missing required parameters: 'id' and/or 'slug'");
                                         }
                                     })));
        }
    };

}

#endif