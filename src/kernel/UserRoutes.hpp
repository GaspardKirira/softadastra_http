#pragma once
#include "RouteManager.hpp"
#include "HTTPServer.hpp"
#include <functional>

namespace Softadastra
{
    // Définition des routes utilisateur dans cette fonction
    void setup_user_routes(RouteManager &route_manager)
    {
        route_manager.add_route([](Router &router)
                                {
            // Route GET /
            add_route(router, http::verb::get, "/user",
                      [](const http::request<http::string_body> &, http::response<http::string_body> &res)
                      {
                          Response::success_response(res, "User route");
                      }); });

        route_manager.add_route([](Router &router)
                                {
            // Route POST /user/create
            add_route(router, http::verb::post, "/user/create",
                      [](const http::request<http::string_body> &req,
                         http::response<http::string_body> &res)
                      {
                          try
                          {
                              json request_json;
                              try
                              {
                                  request_json = json::parse(req.body());
                              }
                              catch (const std::exception &e)
                              {
                                  Response::error_response(res, http::status::bad_request, "Invalid JSON body");
                                  return;
                              }

                              if (request_json.find("firstname") == request_json.end())
                              {
                                  Response::error_response(res, http::status::bad_request, "Missing 'firstname' field");
                                  return;
                              }

                              Response::create_response(res, http::status::created, "User created successfully");
                          }
                          catch (const nlohmann::json::exception &e)
                          {
                              Response::error_response(res, http::status::bad_request, "Invalid JSON format");
                          }
                          catch (const std::exception &e)
                          {
                              Response::error_response(res, http::status::internal_server_error, e.what());
                          }
                      }); });
    }
}
