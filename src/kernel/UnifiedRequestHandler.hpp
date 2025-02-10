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

                // Extraire et traiter les paramètres de la requête (exemple avec id et slug)
                std::unordered_map<std::string, std::string> params = extract_params(std::string(req.target()));
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

    private:
        std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler_;

        // Fonction pour extraire les paramètres de la requête
        std::unordered_map<std::string, std::string> extract_params(const std::string &target)
        {
            std::unordered_map<std::string, std::string> params;

            // Exemple de simple extraction de paramètres (vous pouvez l'adapter à vos besoins)
            std::regex regex(R"(\?([^&=]+)=([^&=]+))");
            std::smatch matches;
            std::string::const_iterator search_start(target.cbegin());
            while (std::regex_search(search_start, target.cend(), matches, regex))
            {
                params[matches[1]] = matches[2];
                search_start = matches.suffix().first;
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

}
