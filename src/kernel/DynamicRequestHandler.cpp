#include "DynamicRequestHandler.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <regex>
#include "Response.hpp"

using json = nlohmann::json;

namespace Softadastra
{

    DynamicRequestHandler::DynamicRequestHandler(
        std::function<void(const std::unordered_map<std::string, std::string> &,
                           http::response<http::string_body> &)>
            handler)
        : params_(), handler_(std::move(handler))
    {
    }

    void DynamicRequestHandler::handle_request(const http::request<http::string_body> &req,
                                               http::response<http::string_body> &res)
    {
        // Vérifier la méthode de la requête
        if (req.method() == http::verb::get)
        {
            // Si c'est une méthode GET, on passe les paramètres extraits de l'URL
            spdlog::info("Handling GET request with parameters...");

            auto id_it = params_.find("id");
            if (id_it != params_.end())
            {
                spdlog::info("Parameter 'id' found: {}", id_it->second);
            }
            else
            {
                spdlog::warn("Parameter 'id' not found.");
            }

            // Appeler le handler avec les paramètres extraits de l'URL
            handler_(params_, res);
        }
        else
        {
            // Pour les autres méthodes HTTP (POST, PUT, etc.), on parse le corps de la requête
            const std::string &body = req.body();

            // Si le corps est vide, on renvoie une erreur
            if (body.empty())
            {
                Response::error_response(res, http::status::bad_request, "Empty request body.");
                return;
            }

            json request_json;
            try
            {
                // Tenter de parser le corps de la requête en JSON
                request_json = json::parse(body);
            }
            catch (const std::exception &e)
            {
                // Si le corps n'est pas valide, renvoyer une erreur
                Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                return;
            }

            // Ajouter les paramètres du corps dans le handler
            handler_({{"body", body}}, res);
        }
    }

    void DynamicRequestHandler::set_params(const std::unordered_map<std::string, std::string> &params,
                                           http::response<http::string_body> &res)
    {
        // Vérifier que les paramètres 'id' et 'slug' existent
        if (params.find("id") == params.end() || params.find("slug") == params.end())
        {
            // Si un paramètre est manquant, retourner une erreur
            Response::error_response(res, http::status::bad_request, "Missing required parameters: 'id' and/or 'slug'");
            return;
        }

        // Validation des paramètres
        for (const auto &param : params)
        {
            const std::string &key = param.first;
            const std::string &value = param.second;

            if (key == "id")
            {
                // Valider que 'id' est un entier positif
                if (!std::regex_match(value, std::regex("^[0-9]+$")))
                {
                    Response::error_response(res, http::status::bad_request, "Invalid 'id' parameter. Must be a positive integer.");
                    return; // Arrête l'exécution après la réponse
                }
            }
            else if (key == "slug")
            {
                // Valider que 'slug' est alphanumérique avec tirets ou underscores
                if (!std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                {
                    Response::error_response(res, http::status::bad_request, "Invalid 'slug' parameter. Must be alphanumeric.");
                    return; // Arrête l'exécution après la réponse
                }
            }
        }

        // Si la validation passe, on assigne les paramètres
        params_ = params;
    }

}
