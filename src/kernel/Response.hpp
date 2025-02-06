#include <string>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>

using json = nlohmann::json;
namespace http = boost::beast::http;
namespace fs = boost::filesystem;

namespace Softadastra
{
    class Response
    {
    public:
        // Méthode statique pour créer une réponse générale
        static void create_response(http::response<http::string_body> &res,
                                    http::status status,
                                    const std::string &message,
                                    const std::string &content_type = "application/json")
        {
            // Définir le statut de la réponse
            res.result(status);

            // Définir l'en-tête Content-Type
            res.set(http::field::content_type, content_type);

            // Ajouter l'en-tête Server
            res.set(http::field::server, "Softadastra/master");

            // Ajouter les en-têtes CORS
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
            res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");

            // Générer la date actuelle au format HTTP (RFC 1123)
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::gmtime(&now_time_t);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
            std::string date = oss.str();

            // Ajouter l'en-tête Date avec la date dynamique
            res.set(http::field::date, date);

            // Ajouter le corps de la réponse avec le message JSON
            res.body() = json{{"message", message}}.dump();
        }

        // Méthode statique pour générer une réponse d'erreur
        static void error_response(http::response<http::string_body> &res,
                                   http::status status,
                                   const std::string &message)
        {
            create_response(res, status, message);
        }

        // Méthode statique pour générer une réponse de succès
        static void success_response(http::response<http::string_body> &res,
                                     const std::string &message)
        {
            create_response(res, http::status::ok, message);
        }

        // Méthode statique pour une réponse No Content (204)
        static void no_content_response(http::response<http::string_body> &res)
        {
            res.result(http::status::no_content);
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "No Content"}}.dump();
            res.set(http::field::server, "Softadastra/master");

            // Ajouter les en-têtes CORS
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
            res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");

            // Générer la date actuelle au format HTTP (RFC 1123)
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::gmtime(&now_time_t);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
            std::string date = oss.str();

            // Ajouter l'en-tête Date avec la date dynamique
            res.set(http::field::date, date);
        }

        // Méthode statique pour gérer une redirection (302)
        static void redirect_response(http::response<http::string_body> &res,
                                      const std::string &location)
        {
            res.result(http::status::found);          // Code HTTP 302 Found
            res.set(http::field::location, location); // Définir l'URL de redirection
            res.set(http::field::content_type, "application/json");
            res.body() = json{{"message", "Redirecting to " + location}}.dump();
            res.set(http::field::server, "Softadastra/master");

            // Ajouter les en-têtes CORS
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
            res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");

            // Générer la date actuelle au format HTTP (RFC 1123)
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::gmtime(&now_time_t);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
            std::string date = oss.str();

            // Ajouter l'en-tête Date avec la date dynamique
            res.set(http::field::date, date);
        }

        // Méthode pour générer une réponse JSON avec des données spécifiques
        static void json_response(http::response<http::string_body> &res,
                                  const json &data)
        {
            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = data.dump();
            res.set(http::field::server, "Softadastra/master");

            // Ajouter les en-têtes CORS
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
            res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");

            // Générer la date actuelle au format HTTP (RFC 1123)
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::gmtime(&now_time_t);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
            std::string date = oss.str();

            // Ajouter l'en-tête Date avec la date dynamique
            res.set(http::field::date, date);
        }
    };
}
