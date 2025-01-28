#include "DynamicRequestHandler.hpp"
#include "Router.hpp"
#include <regex>

namespace Softadastra
{
    void Router::add_route(http::verb method, const std::string &route, std::shared_ptr<IRequestHandler> handler)
    {
        routes_[{method, route}] = std::move(handler);
    }

    bool Router::handle_request(const http::request<http::string_body> &req, http::response<http::string_body> &res)
    {
        // Vérifie l'environnement, et n'affiche des logs d'info que si ce n'est pas en production
        bool is_production = std::getenv("ENV") && std::string(std::getenv("ENV")) == "production";

        if (!is_production)
        {
            // Log de la méthode de la requête et du chemin (affiché en dev)
            spdlog::info("Received {} request for path '{}'", req.method_string(), req.target());
        }

        // Si la méthode est OPTIONS, on gère les en-têtes CORS
        if (req.method() == http::verb::options)
        {
            if (!is_production)
            {
                spdlog::info("Handling OPTIONS request for path '{}'", req.target());
            }

            res.result(http::status::no_content);                                                               // Réponse vide pour OPTIONS
            res.set(http::field::access_control_allow_origin, "*");                                             // Permet toutes les origines
            res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD"); // Autorise ces méthodes
            res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");                  // Autorise le Content-Type et Authorization
            return true;                                                                                        // Requête OPTIONS traitée
        }

        // Recherche de la route exacte pour la méthode et le chemin spécifié
        RouteKey key = {req.method(), std::string(req.target())};
        auto it = routes_.find(key);

        if (it != routes_.end())
        {
            // La route exacte existe, on l'appelle
            if (!is_production)
            {
                spdlog::info("Exact match found for method '{}' and path '{}'", req.method_string(), req.target());
            }
            it->second->handle_request(req, res);
            return true;
        }

        // Si la méthode n'est pas autorisée (si elle n'est pas dans GET, POST, PUT, DELETE, PATCH)
        if (req.method() != http::verb::get && req.method() != http::verb::post && req.method() != http::verb::put &&
            req.method() != http::verb::delete_ && req.method() != http::verb::patch && req.method() != http::verb::head)
        {
            spdlog::warn("Method '{}' is not allowed for path '{}'", req.method_string(), req.target());
            res.result(http::status::method_not_allowed); // 405 Method Not Allowed
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "Method Not Allowed"}}.dump();
            return false; // Retourne false si la méthode n'est pas supportée
        }

        // Recherche de routes dynamiques
        if (!is_production)
        {
            spdlog::info("Exact match not found, trying dynamic routes...");
        }

        for (auto &[route_key, handler] : routes_)
        {
            if (route_key.first == req.method() && matches_dynamic_route(route_key.second, std::string(req.target()), handler, res, req))
            {
                return true;
            }
        }

        // Si aucune route n'a été trouvée, renvoyer 404 Not Found
        spdlog::warn("Route not found for method '{}' and path '{}'", req.method_string(), req.target());
        res.result(http::status::not_found); // 404 Not Found
        res.set(http::field::content_type, "application/json");
        res.body() = json{{"message", "Route not found"}}.dump();
        return false; // Aucun handler trouvé, route non trouvée
    }

    bool Router::matches_dynamic_route(const std::string &route_pattern, const std::string &path,
                                       std::shared_ptr<IRequestHandler> handler, http::response<http::string_body> &res,
                                       const http::request<http::string_body> &req)
    {
        // Vérifie l'environnement pour réduire les logs en production
        bool is_production = std::getenv("ENV") && std::string(std::getenv("ENV")) == "production";

        if (!is_production)
        {
            // Log de l'expression régulière générée
            std::string regex_pattern = convert_route_to_regex(route_pattern);
            spdlog::info("Converted route pattern: {}", regex_pattern);

            // Log avant la correspondance avec l'URL
            spdlog::info("Trying to match path '{}' with regex pattern '{}'", path, regex_pattern);
        }

        boost::regex dynamic_route(convert_route_to_regex(route_pattern));
        boost::smatch match;

        // Si le chemin correspond au modèle de la route
        if (boost::regex_match(path, match, dynamic_route))
        {
            if (!is_production)
            {
                spdlog::info("Path '{}' matches route pattern '{}'", path, route_pattern);
            }

            std::unordered_map<std::string, std::string> params;
            size_t param_count = 0;

            // Extraction des paramètres dynamiques
            for (size_t start = 0; (start = route_pattern.find('{', start)) != std::string::npos;)
            {
                size_t end = route_pattern.find('}', start);
                if (end != std::string::npos)
                {
                    std::string param_name = route_pattern.substr(start + 1, end - start - 1);

                    // Log pour chaque paramètre extrait
                    if (!is_production)
                    {
                        spdlog::info("Extracting parameter '{}', matched value: {}", param_name, match[param_count + 1].str());
                    }

                    if (param_count < match.size() - 1)
                    {
                        params[param_name] = match[param_count + 1].str(); // Associer à la valeur capturée
                        param_count++;
                    }
                    start = end + 1;
                }
            }

            // Log des paramètres extraits
            if (!is_production)
            {
                spdlog::info("Extracted parameters: {}", map_to_string(params));
            }

            // Valider les paramètres extraits
            if (!validate_parameters(params, res))
            {
                // Si la validation échoue, renvoyer la réponse d'erreur 400 immédiatement
                spdlog::warn("Parameter validation failed for path '{}'", path);
                return false;
            }

            // Si les paramètres sont valides, on les passe au handler
            auto dynamic_handler = std::dynamic_pointer_cast<DynamicRequestHandler>(handler);
            if (dynamic_handler)
            {
                dynamic_handler->set_params(params);       // Passer les paramètres au handler
                dynamic_handler->handle_request(req, res); // Exécuter la requête avec le handler
                return true;
            }
            else
            {
                spdlog::warn("Handler is not of type DynamicRequestHandler.");
                res.result(http::status::internal_server_error); // 500 Internal Server Error
                res.set(http::field::content_type, "application/json");
                res.body() = json{{"message", "Internal Server Error: Handler type mismatch"}}.dump();
                return false;
            }
        }

        // Log si l'URL ne correspond pas à la route
        if (!is_production)
        {
            spdlog::info("Path '{}' does not match route pattern '{}'", path, route_pattern);
        }
        return false;
    }

    std::string Router::map_to_string(const std::unordered_map<std::string, std::string> &map)
    {
        std::ostringstream oss;
        oss << "{ ";
        for (const auto &[key, value] : map)
        {
            oss << key << ": " << value << ", ";
        }
        std::string result = oss.str();
        if (!map.empty())
        {
            result.pop_back(); // Supprime la dernière virgule
            result.pop_back(); // Supprime l'espace
        }
        result += " }";
        return result;
    }

    // Fonction de sanitisation des paramètres (enlevant les balises HTML)
    std::string Router::sanitize_input(const std::string &input)
    {
        std::string sanitized = input;
        // Supprimer les balises HTML (pour éviter XSS)
        sanitized = std::regex_replace(sanitized, std::regex("<[^>]*>"), "");
        return sanitized;
    }

    bool Router::validate_parameters(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res)
    {
        for (const auto &[key, value] : params)
        {
            // Sanitiser les valeurs
            std::string sanitized_value = sanitize_input(value);

            // Valider 'id' (doit être un entier positif)
            if (key == "id" && !std::regex_match(sanitized_value, std::regex("^[0-9]+$")))
            {
                spdlog::warn("Invalid 'id' parameter: {}", sanitized_value);
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = json{{"message", "Invalid 'id' parameter. Must be a positive integer."}}.dump();
                return false;
            }

            // Valider 'slug' (doit être alphanumérique, avec des tirets et tirets bas)
            if (key == "slug" && !std::regex_match(sanitized_value, std::regex("^[a-zA-Z0-9_-]+$")))
            {
                spdlog::warn("Invalid 'slug' parameter: {}", sanitized_value);
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = json{{"message", "Invalid 'slug' parameter. Must be alphanumeric, with dashes or underscores."}}.dump();
                return false;
            }
        }

        return true;
    }

    std::string Router::convert_route_to_regex(const std::string &route_pattern)
    {
        std::string regex = "^";
        bool inside_placeholder = false;
        std::string param_name;

        for (size_t i = 0; i < route_pattern.size(); ++i)
        {
            char c = route_pattern[i];
            if (c == '{')
            {
                inside_placeholder = true;
                param_name.clear();
                regex += "("; // Début du paramètre dynamique
            }
            else if (c == '}')
            {
                inside_placeholder = false;
                regex += "[^/]+)"; // Capture une partie de l'URL jusqu'au prochain "/"
            }
            else
            {
                if (inside_placeholder)
                {
                    param_name += c; // Construire le nom du paramètre
                }
                else
                {
                    if (c == '/')
                    {
                        regex += "\\/";
                    }
                    else
                    {
                        regex += c;
                    }
                }
            }
        }
        regex += "$"; // Fin de l'URL
        return regex;
    }

}
