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
            // Route statique de la page d'accueil
            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<UnifiedRequestHandler>(
                        [this](const http::request<http::string_body> &req [[maybe_unused]],
                               http::response<http::string_body> &res [[maybe_unused]])
                        {
                            Response::success_response(res, "Hello world");
                        })));

            // Route pour créer un utilisateur
            routes.add_route(http::verb::post, "/create",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<UnifiedRequestHandler>(
                                     [this](const http::request<http::string_body> &req,
                                            http::response<http::string_body> &res)
                                     {
                                         if (req.body().empty())
                                         {
                                             Response::error_response(res, http::status::bad_request, "Empty request body.");
                                             return;
                                         }

                                         json json_data;
                                         try
                                         {
                                             json_data = json::parse(req.body());
                                         }
                                         catch (const std::exception &e)
                                         {
                                             Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                                             return;
                                         }

                                         // Validation et extraction des données JSON
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

                                         // Enregistrement ou autre traitement des données

                                         Response::success_response(res, "User created successfully");
                                     })));

            // Route dynamique pour accéder à un utilisateur par ID
            routes.add_route(http::verb::get, "/users/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [this](const std::unordered_map<std::string, std::string> &params,
                                            http::response<http::string_body> &res)
                                     {
                                         if (params.find("id") == params.end())
                                         {
                                             Response::error_response(res, http::status::bad_request, "Missing 'id' parameter.");
                                             return;
                                         }

                                         std::string user_id = params.at("id");
                                         spdlog::info("Fetching user with ID: {}", user_id);

                                         // Logique de récupération de l'utilisateur par ID
                                         // Pour l'instant, on renvoie une réponse fictive
                                         Response::success_response(res, "User details for ID: " + user_id);
                                     })));
            routes.add_route(http::verb::put, "/users/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<UnifiedRequestHandler>(
                                     [this](const http::request<http::string_body> &req,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             // Extraire les paramètres dynamiques (y compris l'ID) de l'URL
                                             std::unordered_map<std::string, std::string> params = UnifiedRequestHandler::extract_dynamic_params_public(std::string(req.target()));

                                             // Vérifier que l'ID est bien présent
                                             if (params.find("id") == params.end())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Missing 'id' parameter.");
                                                 return;
                                             }

                                             // Extraire et valider l'ID
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

                                             // Vérifier si le corps de la requête est vide
                                             if (req.body().empty())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Empty request body.");
                                                 return;
                                             }

                                             // Parser le corps JSON
                                             json request_json;
                                             try
                                             {
                                                 request_json = json::parse(req.body());
                                             }
                                             catch (const std::exception &e)
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                                                 return;
                                             }

                                             // Vérification du champ 'username' dans le corps JSON
                                             if (request_json.find("username") == request_json.end())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Le champ 'username' est manquant.");
                                                 return;
                                             }

                                             // Extraire et utiliser les données (par exemple, mettre à jour l'utilisateur)
                                             std::string username = request_json["username"];
                                             spdlog::info("Updating user {} with username: {}", id, username);

                                             // Vous pouvez ajouter des opérations supplémentaires ici pour mettre à jour l'utilisateur en base de données

                                             // Répondre avec succès
                                             json response_json;
                                             response_json["id"] = id;
                                             response_json["username"] = username;
                                             Softadastra::Response::json_response(res, response_json);
                                         }
                                         catch (const std::exception &e)
                                         {
                                             // Gérer les erreurs internes
                                             Softadastra::Response::error_response(res, http::status::internal_server_error, e.what());
                                         }
                                     })));

            routes.add_route(http::verb::put, "/test",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<UnifiedRequestHandler>(
                                     [this](const http::request<http::string_body> &req,
                                            http::response<http::string_body> &res)
                                     {
                                         if (req.body().empty())
                                         {
                                             Response::error_response(res, http::status::bad_request, "Empty request body.");
                                             return;
                                         }

                                         // Afficher ou utiliser le contenu du corps
                                         spdlog::info("Received data: {}", req.body());

                                         Response::success_response(res, "Request received successfully with data.");
                                     })));

            routes.add_route(http::verb::put, "/test/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<UnifiedRequestHandler>(
                                     [this](const http::request<http::string_body> &req,
                                            http::response<http::string_body> &res)
                                     {
                                         // Extraction de l'ID
                                         std::unordered_map<std::string, std::string> params = UnifiedRequestHandler::extract_dynamic_params_public(std::string(req.target()));

                                         std::string id_str = params["id"];
                                         int id;
                                         try
                                         {
                                             // Convertir l'ID en entier
                                             id = std::stoi(id_str);
                                         }
                                         catch (const std::exception &)
                                         {
                                             Response::error_response(res, http::status::bad_request, "Invalid 'id' parameter. Must be an integer.");
                                             return;
                                         }

                                         // Vérification du corps de la requête
                                         if (req.body().empty())
                                         {
                                             Response::error_response(res, http::status::bad_request, "Empty request body.");
                                             return;
                                         }

                                         // Analyse du corps JSON
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

                                         // Traitement des données
                                         if (request_json.find("test_data") == request_json.end())
                                         {
                                             Response::error_response(res, http::status::bad_request, "'test_data' is missing.");
                                             return;
                                         }

                                         std::string test_data = request_json["test_data"];

                                         // Réponse
                                         json response_json;
                                         response_json["id"] = id;
                                         response_json["test_data"] = test_data;
                                         Response::json_response(res, response_json);
                                     })));
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
