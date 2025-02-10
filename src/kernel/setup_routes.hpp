// routes.hpp
#pragma once
#include "HTTPServer.hpp"
#include <memory>
#include <functional>
#include <unordered_map>

namespace Softadastra
{
    // Fonction générique pour ajouter une route
    template <typename RequestHandler>
    void add_route(Router &router, http::verb method, const std::string &path, RequestHandler &&handler)
    {
        router.add_route(method, path, std::static_pointer_cast<IRequestHandler>(std::make_shared<SimpleRequestHandler>(std::forward<RequestHandler>(handler))));
    }

    void setup_routes(Router &router)
    {
        add_route(router, http::verb::get, "/",
                  [](const http::request<http::string_body> &, http::response<http::string_body> &res)
                  {
                      Response::success_response(res, "Hello world");
                  });

        add_route(router, http::verb::get, "/health",
                  [](const http::request<http::string_body> &, http::response<http::string_body> &res)
                  {
                      Response::success_response(res, "Server is healthy");
                  });

        add_route(router, http::verb::post, "/create",
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
                  });
    }
} // namespace Softadastra
