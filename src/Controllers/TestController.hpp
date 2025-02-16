#ifndef TESTCONTROLLER_HPP
#define TESTCONTROLLER_HPP

#include "Controller.hpp"
#include <spdlog/spdlog.h>

namespace Softadastra
{
    class TestController : public Controller
    {
    public:
        using Controller::Controller;

        void configure(Router &router) override
        {
            add_route(router, http::verb::get, "/test",
                      [this](const http::request<http::string_body> &,
                             http::response<http::string_body> &res)
                      {
                          Response::success_response(res, "Hello from test");
                      });

            add_route(router, http::verb::put, "/update_user/{id}",
                      [this](const http::request<http::string_body> &req, http::response<http::string_body> &res)
                      {
                          std::unordered_map<std::string, std::string> params = UnifiedRequestHandler::extract_dynamic_params_public(std::string(req.target()));
                          if (params.find("id") == params.end())
                          {
                              Response::error_response(res, http::status::bad_request, "Missing 'id' parameter.");
                              return;
                          }

                          std::string id_str = params["id"];
                          int id;
                          try
                          {
                              id = std::stoi(id_str);
                          }
                          catch (const std::exception &)
                          {
                              Response::error_response(res, http::status::bad_request, "Invalid 'id' parameter. Must be an integer.");
                              return;
                          }

                          // Valider le corps de la requête (JSON attendu)
                          if (req.body().empty())
                          {
                              Response::error_response(res, http::status::bad_request, "Empty request body.");
                              return;
                          }

                          json request_json;
                          try
                          {
                              request_json = json::parse(req.body());
                          }
                          catch (const std::exception &)
                          {
                              Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                              return;
                          }

                          // Vérification du champ 'username'
                          if (request_json.find("username") == request_json.end())
                          {
                              Response::error_response(res, http::status::bad_request, "Le champ 'username' est manquant.");
                              return;
                          }

                          std::string username = request_json["username"];
                          spdlog::info("Updating user {} with username: {}", id, username);

                          Response::success_response(res, "Request received successfully with data.");
                      });
        }
    };
} // namespace Softadastra

#endif