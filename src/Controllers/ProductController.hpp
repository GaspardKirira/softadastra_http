#ifndef PRODUCTROUTES_HPP
#define PRODUCTROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include "Controller.hpp"
#include <spdlog/spdlog.h>

namespace Softadastra
{

    class ProductController : public Controller
    {
    public:
        using Controller::Controller;

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
                                             Softadastra::Response::success_response(res, "Product details for id: " + product_id);
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
                                             std::string slug = params.at("slug");
                                             Softadastra::Response::success_response(res, "Product details for slug: " + slug);
                                         }
                                         catch (const std::out_of_range &e)
                                         {
                                             throw std::invalid_argument("Missing required parameter: 'slug'");
                                         }
                                     })));

            router.add_route(http::verb::get, "/products/{id}/{slug}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [this](const std::unordered_map<std::string, std::string> &params,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::string id = params.at("id");
                                             std::string slug = params.at("slug");

                                             Softadastra::Response::success_response(res, "Product details for id: " + id + " and slug: " + slug);
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