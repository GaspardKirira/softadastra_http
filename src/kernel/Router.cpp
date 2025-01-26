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
            std::cout << "Exact match found!" << std::endl;
            it->second->handle_request(req, res);
            return true;
        }

        // Recherche de route dynamique
        std::cout << "Exact match not found, trying dynamic routes..." << std::endl;

        for (auto &[route_key, handler] : routes_)
        {
            if (matches_dynamic_route(route_key.second, std::string(req.target()), handler, res))
            {
                return true;
            }
        }

        std::cout << "Route not found" << std::endl;
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
                if (key == "id" && !std::regex_match(value, std::regex("^[0-9]+$")))
                {
                    spdlog::warn("Invalid 'id' parameter: {}", value);
                    throw std::invalid_argument("Invalid parameter value for 'id'. Must be a positive integer.");
                }
                if (key == "slug" && !std::regex_match(value, std::regex("^[a-zA-Z0-9_-]+$")))
                {
                    spdlog::warn("Invalid 'slug' parameter: {}", value);
                    throw std::invalid_argument("Invalid parameter value for 'slug'. Must be alphanumeric.");
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
