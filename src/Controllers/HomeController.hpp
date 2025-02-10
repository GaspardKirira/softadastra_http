#ifndef HOMEROUTES_HPP
#define HOMEROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <ctime>
#include <unordered_set>
#include <sstream>
#include "Controller.hpp"
#include "../kernel/UnifiedRequestHandler.hpp"

namespace Softadastra
{
    class HomeController : public Controller
    {
    public:
        using Controller::Controller;

        void configure(Router &routes) override
        {
            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<UnifiedRequestHandler>(
                        [this](const http::request<http::string_body> &req [[maybe_unused]],
                               http::response<http::string_body> &res [[maybe_unused]])
                        {
                            // Ici, la logique pour gérer la requête GET
                            Response::success_response(res, "Hello world");
                        })));

            routes.add_route(http::verb::post, "/create",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<UnifiedRequestHandler>(
                                     [this](const http::request<http::string_body> &req,
                                            http::response<http::string_body> &res)
                                     {
                                         // Vérifie si le corps de la requête n'est pas vide
                                         if (req.body().empty())
                                         {
                                             Response::error_response(res, http::status::bad_request, "Empty request body.");
                                             return;
                                         }

                                         // Parse le corps de la requête en JSON
                                         json json_data;
                                         try
                                         {
                                             json_data = json::parse(req.body());
                                         }
                                         catch (const std::exception &e)
                                         {
                                             // Si l'extraction JSON échoue, renvoyer une erreur
                                             Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                                             return;
                                         }

                                         // À ce stade, json_data contient les données JSON du corps de la requête
                                         // Tu peux maintenant extraire des valeurs du JSON et les utiliser
                                         if (json_data.contains("username") && json_data["username"].is_string())
                                         {
                                             std::string username = json_data["username"];
                                             spdlog::info("Received username: {}", username);
                                         }
                                         else
                                         {
                                             Response::error_response(res, http::status::bad_request, "Missing or invalid 'username' field.");
                                             return;
                                         }

                                         if (json_data.contains("email") && json_data["email"].is_string())
                                         {
                                             std::string email = json_data["email"];
                                             spdlog::info("Received email: {}", email);
                                         }
                                         else
                                         {
                                             Response::error_response(res, http::status::bad_request, "Missing or invalid 'email' field.");
                                             return;
                                         }

                                         // Traite les autres données nécessaires
                                         // Par exemple, si tu veux enregistrer un nouvel utilisateur dans une base de données ou effectuer d'autres actions.
                                         Response::success_response(res, "User created successfully");
                                     })));

            routes.add_route(http::verb::get, "/users/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<UnifiedRequestHandler>(
                                     [this](const http::request<http::string_body> &req [[maybe_unused]],
                                            http::response<http::string_body> &res [[maybe_unused]])
                                     {
                                         // Cette route peut être implémentée plus tard
                                     })));
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
