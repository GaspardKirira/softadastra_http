#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>

#include <boost/regex.hpp>
#include <string>

#include <unordered_map>
#include <iostream>
#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include "IRequestHandler.hpp"
#include "Config.hpp"

namespace Softadastra
{

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    namespace net = boost::asio;

    using tcp = net::ip::tcp;
    using ssl_socket = boost::asio::ssl::stream<tcp::socket>;
    using json = nlohmann::json;

    /**
     * @brief Structure utilisée pour générer un hash unique pour une paire clé-valeur.
     *
     * Cette structure est utilisée dans la table de hachage pour permettre l'indexation des routes.
     */
    struct PairHash
    {
        /**
         * @brief Génère un hash pour une paire de clés.
         *
         * Cette fonction combine les hashes des deux éléments de la paire pour générer un hash unique.
         * @tparam T1 Le type de la première clé de la paire.
         * @tparam T2 Le type de la seconde clé de la paire.
         * @param p La paire de clés à hacher.
         * @return Le hash combiné des deux éléments de la paire.
         */
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const
        {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1); // Combine les hashs de manière simple
        }
    };

    /**
     * @brief Classe qui représente un routeur pour l'acheminement des requêtes HTTP.
     *
     * Le routeur est responsable de l'acheminement des requêtes vers le bon gestionnaire de requêtes en fonction de la méthode HTTP et de l'URL.
     */
    class Router
    {
    public:
        using RouteKey = std::pair<http::verb, std::string>;

        /**
         * @brief Constructeur du routeur, initialisant la table des routes.
         */
        Router() : routes_() {}

        /**
         * @brief Ajoute une nouvelle route au routeur.
         *
         * Permet d'ajouter une route avec une méthode HTTP spécifique et une URL, ainsi qu'un gestionnaire pour traiter les requêtes qui correspondent à cette route.
         * Les routes dynamiques, comme celles contenant des paramètres (par exemple "/users/{id}"), sont également prises en charge.
         * @param method La méthode HTTP (GET, POST, etc.) associée à la route.
         * @param route L'URL de la route à ajouter.
         * @param handler Le gestionnaire qui traite les requêtes pour cette route.
         */
        void add_route(http::verb method, const std::string &route, std::shared_ptr<IRequestHandler> handler);

        /**
         * @brief Traite une requête HTTP.
         *
         * Cette fonction recherche une route correspondant à la requête (par méthode et URL), et envoie la réponse via le gestionnaire associé à la route.
         * Si la route est dynamique, elle vérifie les correspondances avec des paramètres dans l'URL.
         * @param req La requête HTTP à traiter.
         * @param res La réponse HTTP à générer et à envoyer.
         * @return true si une route a été trouvée et la requête a été traitée, sinon false.
         */
        bool handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res);

    private:
        /**
         * @brief Vérifie si une route dynamique correspond à l'URL de la requête.
         *
         * Les routes dynamiques contiennent des paramètres (par exemple "/users/{id}"), et cette fonction utilise une expression régulière pour déterminer si l'URL de la requête correspond à un modèle dynamique.
         * @param route_pattern Le modèle de route, qui peut contenir des paramètres dynamiques.
         * @param path L'URL de la requête.
         * @param handler Le gestionnaire à associer à la route si elle correspond.
         * @param res La réponse HTTP à générer en cas de correspondance.
         * @return true si une correspondance dynamique est trouvée, sinon false.
         */
        bool matches_dynamic_route(const std::string &route_pattern, const std::string &path, std::shared_ptr<IRequestHandler> handler, http::response<http::string_body> &res, const http::request<http::string_body> &req);

        /**
         * @brief Convertit une route en une expression régulière pour la comparer à l'URL de la requête.
         *
         * Cette fonction prend une route avec des paramètres dynamiques (par exemple "/users/{id}") et la convertit en une expression régulière qui peut être utilisée pour effectuer une correspondance avec l'URL de la requête.
         * @param route La route contenant potentiellement des paramètres dynamiques.
         * @return L'expression régulière qui correspond à la route.
         */
        std::string convert_route_to_regex(const std::string &route);

        bool validate_parameters(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res);

        /**
         * @brief La table des routes, indexée par une paire (méthode HTTP, URL).
         *
         * Les routes sont stockées sous forme d'un map, avec une clé composée de la méthode HTTP (GET, POST, etc.) et de l'URL, et une valeur associée au gestionnaire de requêtes.
         */
        std::unordered_map<RouteKey, std::shared_ptr<IRequestHandler>, PairHash> routes_;
        std::string map_to_string(const std::unordered_map<std::string, std::string> &map);
    };
};

#endif // ROUTER_HPP
