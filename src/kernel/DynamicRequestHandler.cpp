#include "DynamicRequestHandler.hpp"
#include <spdlog/spdlog.h> 
#include <regex>          

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

        handler_(params_, res);
    }

    void DynamicRequestHandler::set_params(
        const std::unordered_map<std::string, std::string> &params)
    {
        spdlog::info("Setting parameters in DynamicRequestHandler...");

        // Vérification de tous les paramètres fournis
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
                    throw std::invalid_argument("Invalid parameter value for 'id'. Must be a positive integer.");
                }
            }
            else if (key == "slug")
            {
                // Valider que 'slug' ne contient pas de caractères spéciaux
                if (!std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                {
                    spdlog::warn("Invalid 'slug' parameter: {}", value);
                    throw std::invalid_argument("Invalid parameter value for 'slug'. Must be alphanumeric.");
                }
            }
        }

        params_ = params; // Mise à jour des paramètres
        spdlog::info("Parameters set successfully.");
    }

}
