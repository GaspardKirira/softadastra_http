#include "DynamicRequestHandler.hpp"
#include "Router.hpp"
#include "Response.hpp"
#include <regex>

namespace Softadastra
{
    void Router::add_route(http::verb method, const std::string &route, std::shared_ptr<IRequestHandler> handler)
    {
        // Ajouter la route au routeur
        routes_[{method, route}] = std::move(handler);

        // Ajouter le motif de la route à la liste des motifs
        route_patterns_.push_back(route); // Ajout automatique de la route au vecteur
    }

    bool Router::handle_request(const http::request<http::string_body> &req, http::response<http::string_body> &res)
    {
        bool is_production = std::getenv("ENV") && std::string(std::getenv("ENV")) == "production";

        if (!is_production)
        {
            spdlog::info("Received {} request for path '{}'", req.method_string(), req.target());
        }

        // Gestion des OPTIONS (pour CORS)
        if (req.method() == http::verb::options)
        {
            if (!is_production)
            {
                spdlog::info("Handling OPTIONS request for path '{}'", req.target());
            }

            res.result(http::status::no_content);
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
            res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
            return true;
        }

        // Vérifier si la méthode HTTP est valide (si elle est dans la liste des méthodes autorisées)
        if (req.method() != http::verb::get && req.method() != http::verb::post && req.method() != http::verb::put &&
            req.method() != http::verb::delete_ && req.method() != http::verb::patch && req.method() != http::verb::head)
        {
            spdlog::warn("Method '{}' is not allowed for path '{}'", req.method_string(), req.target());
            res.result(http::status::method_not_allowed); // Code 405 pour méthode non autorisée
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "Method Not Allowed"}}.dump();
            return false; // Retourner immédiatement après avoir géré l'erreur
        }

        // Vérifier si la route existe pour la méthode HTTP donnée
        bool route_exists = false;
        bool method_allowed = false;

        // Rechercher dans les routes existantes
        for (const auto &[route_key, handler] : routes_)
        {
            if (route_key.second == std::string(req.target()))
            {
                route_exists = true;
                if (route_key.first == req.method())
                {
                    method_allowed = true;
                    handler->handle_request(req, res);
                    return true;
                }
            }
        }

        // Vérifier les routes dynamiques
        bool matched = false;
        for (auto &[route_key, handler] : routes_)
        {
            if (route_key.first == req.method() && matches_dynamic_route(route_key.second, std::string(req.target()), handler, res, req))
            {
                matched = true;
                break;
            }
        }

        // Si une route dynamique a été trouvée, la traiter
        if (matched)
        {
            return true;
        }

        // Si la route existe mais que la méthode n'est pas autorisée
        if (route_exists && !method_allowed)
        {
            spdlog::warn("Method '{}' is not allowed for path '{}'", req.method_string(), req.target());
            res.result(http::status::method_not_allowed); // Code 405 pour méthode non autorisée
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "Method Not Allowed"}}.dump();
            return false; // Retourner immédiatement après avoir géré l'erreur
        }

        // Si aucune route ne correspond, retourner "Route not found"
        spdlog::warn("Route not found for method '{}' and path '{}'", req.method_string(), req.target());
        res.result(http::status::not_found); // Code 404 pour route non trouvée
        res.set(http::field::content_type, "application/json");
        res.body() = json{{"message", "Route not found"}}.dump(); // Message pour "Route not found"
        return false;                                             // Retourner immédiatement après avoir géré l'erreur
    }

    bool Router::matches_dynamic_route(const std::string &route_pattern, const std::string &path,
                                       std::shared_ptr<IRequestHandler> handler, http::response<http::string_body> &res,
                                       const http::request<http::string_body> &req)
    {
        boost::regex dynamic_route(convert_route_to_regex(route_pattern));
        boost::smatch match;

        if (boost::regex_match(path, match, dynamic_route))
        {
            std::unordered_map<std::string, std::string> params;
            size_t param_count = 0;

            // Extraire les paramètres dynamiques
            for (size_t start = 0; (start = route_pattern.find('{', start)) != std::string::npos;)
            {
                size_t end = route_pattern.find('}', start);
                if (end != std::string::npos)
                {
                    std::string param_name = route_pattern.substr(start + 1, end - start - 1);
                    if (param_count < match.size() - 1)
                    {
                        params[param_name] = match[param_count + 1].str();
                        param_count++;
                    }
                    start = end + 1;
                }
            }

            spdlog::info("Extracted parameters: {}", map_to_string(params));

            // Assurez-vous que le gestionnaire est bien de type DynamicRequestHandler
            auto dynamic_handler = std::dynamic_pointer_cast<DynamicRequestHandler>(handler);
            if (dynamic_handler)
            {
                // Passer les paramètres extraits au gestionnaire
                dynamic_handler->set_params(params, res);
                if (res.result() != http::status::ok)
                {
                    return false;
                }

                dynamic_handler->handle_request(req, res);
                return true;
            }
            else
            {
                Response::error_response(res, http::status::internal_server_error, "Internal Server Error: Handler type mismatch");
                return false;
            }
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
            result.pop_back();
            result.pop_back();
        }
        result += " }";
        return result;
    }

    std::string Router::sanitize_input(const std::string &input)
    {
        std::string sanitized = input;
        sanitized = std::regex_replace(sanitized, std::regex("<[^>]*>"), "");
        return sanitized;
    }

    bool Router::validate_parameters(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res)
    {
        for (const auto &[key, value] : params)
        {
            std::string sanitized_value = sanitize_input(value);

            if (key == "id" && !std::regex_match(sanitized_value, std::regex("^[0-9]+$")))
            {
                Response::error_response(res, http::status::bad_request, "Invalid 'id' parameter. Must be a positive integer.");
                return false;
            }

            if (key == "slug" && !std::regex_match(sanitized_value, std::regex("^[a-zA-Z0-9_-]+$")))
            {
                Response::error_response(res, http::status::bad_request, "Invalid 'slug' parameter. Must be alphanumeric, with dashes or underscores.");
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
