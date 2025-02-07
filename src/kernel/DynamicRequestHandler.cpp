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
            // Log de la méthode et des paramètres extraits
            spdlog::info("Handling GET request for path: {}", req.target());

            // Vérifier si les paramètres dynamiques sont présents
            auto id_it = params_.find("id");
            if (id_it != params_.end())
            {
                spdlog::info("Parameter 'id' found: {}", id_it->second);
            }

            // Traiter la requête
            handler_(params_, res);
        }
        else
        {
            // Traiter d'autres méthodes HTTP
            const std::string &body = req.body();
            if (body.empty())
            {
                Response::error_response(res, http::status::bad_request, "Empty request body.");
                return;
            }

            json request_json;
            try
            {
                request_json = json::parse(body);
            }
            catch (const std::exception &e)
            {
                Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                return;
            }

            handler_({{"body", body}}, res);
        }
    }

    void DynamicRequestHandler::set_params(
        const std::unordered_map<std::string, std::string> &params,
        http::response<http::string_body> &res)
    {
        spdlog::info("Setting parameters in DynamicRequestHandler...");

        // Vérification des paramètres
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
                    return;
                }
            }
            else if (key == "slug")
            {
                if (!std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                {
                    Response::error_response(res, http::status::bad_request, "Invalid 'slug' parameter. Must be alphanumeric.");
                    return;
                }
            }
        }

        // Si la validation passe, on assigne les paramètres
        params_ = params;
    }

}
