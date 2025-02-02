#include "DynamicRequestHandler.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace Softadastra
{

    DynamicRequestHandler::DynamicRequestHandler(
        std::function<void(const std::unordered_map<std::string, std::string> &,
                           http::response<http::string_body> &)>
            handler)
        : params_(), handler_(std::move(handler))
    {
        spdlog::info("DynamicRequestHandler initialized.");
    }

    void DynamicRequestHandler::handle_request(const http::request<http::string_body> &,
                                               http::response<http::string_body> &res)
    {
        spdlog::info("Handling request with parameters...");

        auto id_it = params_.find("id");
        if (id_it != params_.end())
        {
            spdlog::info("Parameter 'id' found: {}", id_it->second);
        }
        else
        {
            spdlog::warn("Parameter 'id' not found.");
        }

        handler_(params_, res); // Appelle le handler avec les paramètres et la réponse
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
                    spdlog::warn("Invalid 'id' parameter: {}", value);
                    res.result(http::status::bad_request); // Réponse HTTP 400
                    res.set(http::field::content_type, "application/json");
                    res.body() = json{{"message", "Invalid 'id' parameter. Must be a positive integer."}}.dump();
                    return; // Arrête l'exécution après la réponse
                }
            }
            else if (key == "slug")
            {
                if (!std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                {
                    spdlog::warn("Invalid 'slug' parameter: {}", value);
                    res.result(http::status::bad_request); // Réponse HTTP 400
                    res.set(http::field::content_type, "application/json");
                    res.body() = json{{"message", "Invalid 'slug' parameter. Must be alphanumeric."}}.dump();
                    return; // Arrête l'exécution après la réponse
                }
            }
        }

        // Si la validation passe, on assigne les paramètres
        params_ = params;
        spdlog::info("Parameters set successfully.");
    }

}
