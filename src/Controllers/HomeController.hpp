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
        using Controller::Controller;

        void configure(Router &routes) override
        {
            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<SimpleRequestHandler>(
                        [](const http::request<http::string_body> &,
                           http::response<http::string_body> &res)
                        {
                            Response::success_response(res, "Welcome to the Softadastra HTTP Server");
                        })));

            routes.add_route(
                http::verb::get, "/peers",
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
                                                 peers.insert(peer_address);

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
                                                 for (const auto &peer : peers)
                                                 {
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
