#ifndef HOMEROUTES_HPP
#define HOMEROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <ctime>
#include "Controller.hpp"

using json = nlohmann::json;

namespace Softadastra
{
    class HomeController : public Controller
    {
    public:
        void configure(Router &routes) override
        {
            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<SimpleRequestHandler>(
                        [](const http::request<http::string_body> &req,
                           http::response<http::string_body> &res)
                        {
                            // Définir la version de HTTP de la réponse
                            res.version(req.version());

                            // Définir le statut de la réponse à 200 OK
                            res.result(http::status::ok);

                            // Définir l'en-tête Content-Type en application/json
                            res.set(http::field::content_type, "application/json");

                            // Ajouter l'en-tête Server
                            res.set(http::field::server, "Softadastra/master");

                            // Générer la date actuelle au format HTTP (RFC 1123)
                            auto now = std::chrono::system_clock::now();
                            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
                            std::tm tm = *std::gmtime(&now_time_t);

                            std::ostringstream oss;
                            oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
                            std::string date = oss.str();

                            // Ajouter l'en-tête Date avec la date dynamique
                            res.set(http::field::date, date);

                            // Remplir le corps de la réponse avec un message JSON
                            res.body() = json{{"message", "Welcome to the Softadastra HTTP Server"}}.dump();
                        })));

            routes.add_route(http::verb::post, "/create",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<SimpleRequestHandler>(
                                     [](const http::request<http::string_body> &req,
                                        http::response<http::string_body> &res)
                                     {
                                         // Vérification si la méthode n'est pas POST
                                         if (req.method() != http::verb::post)
                                         {
                                             // Retourner une réponse '405 Method Not Allowed' si la méthode n'est pas POST
                                             res.result(http::status::method_not_allowed);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", "Method not allowed, use POST."}}.dump();
                                             return;
                                         }

                                         // Vérification si la requête n'a pas de corps (pas de paramètres)
                                         if (req.body().empty())
                                         {
                                             res.result(http::status::bad_request);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"message", "Missing request body."}}.dump();
                                             return;
                                         }

                                         try
                                         {
                                             // Tentative de parsing du corps de la requête en JSON
                                             auto json_data = json::parse(req.body());

                                             // Vérification si le champ 'name' est présent
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
