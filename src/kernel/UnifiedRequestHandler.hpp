#include "DynamicRequestHandler.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <regex>
#include "Response.hpp"

using json = nlohmann::json;
namespace Softadastra
{
    class UnifiedRequestHandler : public IRequestHandler
    {
    public:
        UnifiedRequestHandler(
            std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler)
            : handler_(std::move(handler)) {}

        void handle_request(const http::request<http::string_body> &req, http::response<http::string_body> &res) override
        {
            // Vérification de la méthode de la requête
            if (req.method() == http::verb::get)
            {
                spdlog::info("Handling GET request for path: {}", req.target());

                // Extraire et traiter les paramètres de la requête et les segments dynamiques
                std::unordered_map<std::string, std::string> params = extract_dynamic_params_public(std::string(req.target()));
                if (!validate_params(params, res))
                {
                    return; // Si validation échoue, on arrête l'exécution
                }

                // Traiter la requête avec les paramètres validés
                handler_(req, res);
            }
            else
            {
                // Traiter d'autres méthodes HTTP comme POST
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

                // Passer la requête et le corps JSON au handler
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

        // Fonction statique pour extraire les paramètres dynamiques des segments de l'URL
        static std::unordered_map<std::string, std::string> extract_dynamic_params(const std::string &target)
        {
            std::unordered_map<std::string, std::string> params;

            // Exemple d'URL avec segments dynamiques : /user/{id}/profile
            // La regex va capturer {id} et l'associer à un paramètre
            std::regex regex(R"((\/\w+)(\/\{(\w+)\}))");
            std::smatch matches;
            std::string::const_iterator search_start(target.cbegin());

            while (std::regex_search(search_start, target.cend(), matches, regex))
            {
                // match[1] : correspond au segment de l'URL (ex : "/user")
                // match[2] : correspond au segment dynamique (ex : "{id}")
                // match[3] : correspond au nom du paramètre dynamique (ex : "id")
                params[matches[3]] = matches[2]; // On associe le nom du paramètre dynamique à sa valeur
                search_start = matches.suffix().first;
            }

            // Exemple de gestion des paramètres de requête (ex: ?id=123&slug=test)
            std::regex query_regex(R"(\?([^&=]+)=([^&=]+))");
            std::string::const_iterator query_start(target.cbegin());
            while (std::regex_search(query_start, target.cend(), matches, query_regex))
            {
                params[matches[1]] = matches[2]; // Associer le nom et la valeur du paramètre de requête
                query_start = matches.suffix().first;
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
