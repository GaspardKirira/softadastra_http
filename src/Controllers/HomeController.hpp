#ifndef HOMEROUTES_HPP
#define HOMEROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <ctime>
#include <unordered_set>
#include "Controller.hpp"

using json = nlohmann::json;
std::unordered_set<std::string> peers;

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

            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<SimpleRequestHandler>(
                        [](const http::request<http::string_body> &,
                           http::response<http::string_body> &res)
                        {
                            res.result(http::status::ok);
                            res.set(http::field::content_type, "application/json");
                            res.body() = json{{"peers", peers}}.dump();
                        })));

            routes.add_route(http::verb::post, "/connect",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<SimpleRequestHandler>(
                                     [](const http::request<http::string_body> &req,
                                        http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             json request_json = json::parse(req.body());
                                             if (request_json.contains("peer"))
                                             {
                                                 std::string peer_address = request_json["peer"];
                                                 peers.insert(peer_address); // Enregistre le nouveau nœud

                                                 // Répond avec la liste des pairs connus
                                                 res.result(http::status::ok);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = json{{"message", "Peer ajouté"}, {"peers", peers}}.dump();
                                             }
                                             else
                                             {
                                                 res.result(http::status::bad_request);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = json{{"error", "Champ 'peer' manquant"}}.dump();
                                             }
                                         }
                                         catch (const std::exception &e)
                                         {
                                             res.result(http::status::internal_server_error);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"error", "Erreur lors de la lecture de la requête"}}.dump();
                                         }
                                     })));

            routes.add_route(http::verb::post, "/broadcast",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<SimpleRequestHandler>(
                                     [](const http::request<http::string_body> &req,
                                        http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             json request_json = json::parse(req.body());
                                             if (request_json.contains("message"))
                                             {
                                                 std::string message = request_json["message"];
                                                 // Diffuser le message à tous les pairs
                                                 for (const auto &peer : peers)
                                                 {
                                                     // Envoyer une requête POST à chaque pair
                                                     // (Implémentez cette fonctionnalité en utilisant Boost.Beast ou une autre bibliothèque HTTP)
                                                     std::cout << "Diffusion à " << peer << ": " << message << std::endl;
                                                 }

                                                 res.result(http::status::ok);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = json{{"message", "Message diffusé à tous les pairs"}}.dump();
                                             }
                                             else
                                             {
                                                 res.result(http::status::bad_request);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = json{{"error", "Champ 'message' manquant"}}.dump();
                                             }
                                         }
                                         catch (const std::exception &e)
                                         {
                                             res.result(http::status::internal_server_error);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = json{{"error", "Erreur lors de la lecture de la requête"}}.dump();
                                         }
                                     })));
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
