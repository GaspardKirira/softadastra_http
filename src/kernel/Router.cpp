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
        RouteKey key = {req.method(), std::string(req.target())};

        // Recherche de la route exacte
        auto it = routes_.find(key);
        if (it != routes_.end())
        {
            spdlog::info("Exact match found!");
            it->second->handle_request(req, res);
            return true;
        }

        // Recherche de route dynamique
        spdlog::info("Exact match not found, trying dynamic routes...");

        for (auto &[route_key, handler] : routes_)
        {
            if (matches_dynamic_route(route_key.second, std::string(req.target()), handler, res))
            {
                return true;
            }
        }

        // Si aucune route n'a été trouvée, on envoie une réponse appropriée
        spdlog::warn("Route not found for method '{}' and path '{}'", req.method_string(), req.target());

        // Gestion des méthodes non prises en charge
        if (req.method() != http::verb::get && req.method() != http::verb::post)
        {
            res.result(http::status::method_not_allowed); // 405 Method Not Allowed
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "Method Not Allowed"}}.dump();
        }
        else
        {
            // Si la route n'est pas trouvée, on renvoie 404
            res.result(http::status::not_found);
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "Route not found"}}.dump();
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

    bool Router::matches_dynamic_route(const std::string &route_pattern, const std::string &path, std::shared_ptr<IRequestHandler> handler, http::response<http::string_body> &res)
    {
        std::string regex_pattern = convert_route_to_regex(route_pattern);
        spdlog::info("Converted route pattern: {}", regex_pattern);

        boost::regex dynamic_route(regex_pattern);
        boost::smatch match;

        if (boost::regex_match(path, match, dynamic_route))
        {
            spdlog::info("Path '{}' matches route pattern '{}'", path, route_pattern);

            std::unordered_map<std::string, std::string> params;

            // Extraire les paramètres
            size_t param_count = 0;
            for (size_t start = 0; (start = route_pattern.find('{', start)) != std::string::npos;)
            {
                size_t end = route_pattern.find('}', start);
                if (end != std::string::npos)
                {
                    std::string param_name = route_pattern.substr(start + 1, end - start - 1);
                    if (param_count < match.size() - 1)
                    {
                        params[param_name] = match[param_count + 1].str(); // Associer à la valeur capturée
                        param_count++;
                    }
                    start = end + 1;
                }
            }

            spdlog::info("Extracted parameters: {}", map_to_string(params));

            // Valider les paramètres id et slug
            for (const auto &[key, value] : params)
            {
                if (key == "id")
                {
                    if (!std::regex_match(value, std::regex("^[0-9]+$")))
                    {
                        spdlog::warn("Invalid 'id' parameter: {}", value);
                        res.result(http::status::bad_request);
                        res.set(http::field::content_type, "application/json");
                        res.body() = json{{"message", "Invalid 'id' parameter. Must be a positive integer."}}.dump();
                        return false;
                    }
                }
                if (key == "slug")
                {
                    if (!std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                    {
                        spdlog::warn("Invalid 'slug' parameter: {}", value);
                        res.result(http::status::bad_request);
                        res.set(http::field::content_type, "application/json");
                        res.body() = json{{"message", "Invalid 'slug' parameter. Must be alphanumeric."}}.dump();
                        return false;
                    }
                }
            }

            dynamic_cast<DynamicRequestHandler *>(handler.get())->set_params(params);
            handler->handle_request({}, res); // Passer une requête vide si besoin
            return true;
        }

        spdlog::info("Path '{}' does not match route pattern '{}'", path, route_pattern);
        return false;
    }

    std::string Router::convert_route_to_regex(const std::string &route)
    {
        std::string regex = "^";
        bool inside_placeholder = false;
        std::string param_name;

        for (size_t i = 0; i < route.size(); ++i)
        {
            char c = route[i];
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
