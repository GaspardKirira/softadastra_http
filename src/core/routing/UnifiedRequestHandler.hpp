#ifndef UNIFIEDREQUESTHANDLER_HPP
#define UNIFIEDREQUESTHANDLER_HPP

#include "DynamicRequestHandler.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <regex>
#include "http/Response.hpp"

using json = nlohmann::json;
namespace Softadastra
{
    class UnifiedRequestHandler : public IRequestHandler
    {
    public:
        UnifiedRequestHandler(
            std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler)
            : handler_(std::move(handler)) {}

        ~UnifiedRequestHandler() {}

        void handle_request(const http::request<http::string_body> &req, http::response<http::string_body> &res) override
        {
            // Vérification de la méthode de la requête
            if (req.method() == http::verb::get)
            {
                // Log de la méthode et des paramètres extraits
                spdlog::info("Handling GET request for path: {}", req.target());

                // Extraire les paramètres dynamiques de l'URL
                std::unordered_map<std::string, std::string> params = extract_dynamic_params_public(std::string(req.target()));

                // Vérifier si les paramètres dynamiques sont présents
                auto id_it = params.find("id");
                if (id_it != params.end())
                {
                    spdlog::info("Parameter 'id' found: {}", id_it->second);
                }

                // Traiter la requête en passant les paramètres extraits au gestionnaire
                handler_(req, res);
            }
            else
            {
                // Pour les autres méthodes HTTP, traiter le corps de la requête
                const std::string &body = req.body();
                if (body.empty())
                {
                    Response::error_response(res, http::status::bad_request, "Empty request body.");
                    return;
                }

                json request_json;
                try
                {
                    // Parser le corps JSON de la requête
                    request_json = json::parse(body);
                }
                catch (const std::exception &e)
                {
                    Response::error_response(res, http::status::bad_request, "Invalid JSON body.");
                    return;
                }

                // Extraire les paramètres dynamiques de l'URL
                std::unordered_map<std::string, std::string> params = extract_dynamic_params_public(std::string(req.target()));

                // Ajouter les paramètres du corps de la requête au dictionnaire de paramètres
                for (auto it = request_json.begin(); it != request_json.end(); ++it)
                {
                    params[it.key()] = it.value();
                }

                // Passer tous les paramètres à handler_ pour traitement
                handler_(req, res);
            }
        }

        // Méthode publique pour appeler extract_dynamic_params statique
        static std::unordered_map<std::string, std::string> extract_dynamic_params_public(const std::string &target)
        {
            return extract_dynamic_params(target); // Appel à la méthode statique privée
        }

    private:
        std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler_;

        static std::unordered_map<std::string, std::string> extract_dynamic_params(const std::string &target)
        {
            std::unordered_map<std::string, std::string> params;

            // Regex qui capture un segment après /update_user/ suivi de l'id
            std::regex regex(R"(^/update_user/(\d+)$)");
            std::smatch matches;

            if (std::regex_match(target, matches, regex))
            {
                // Assigner le paramètre "id" extrait
                params["id"] = matches[1].str(); // matches[1] contient l'id
            }

            return params;
        }

        // Fonction pour valider les paramètres extraits
        bool validate_params(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res)
        {
            spdlog::info("Validating parameters...");

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
                        return false;
                    }
                }
                else if (key == "slug")
                {
                    if (!std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                    {
                        Response::error_response(res, http::status::bad_request, "Invalid 'slug' parameter. Must be alphanumeric.");
                        return false;
                    }
                }
            }
            return true;
        }
    };

} // namespace Softadastra

#endif