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
     * Cette structure est utilisée pour indexer efficacement les routes dans une table de hachage.
     * Elle combine les valeurs des clés pour créer un hash unique permettant de stocker et retrouver rapidement les gestionnaires associés aux routes.
     */
    struct PairHash
    {
        /**
         * @brief Génère un hash pour une paire de clés.
         *
         * Cette fonction combine les valeurs hashées des deux éléments de la paire pour produire un hash unique qui peut être utilisé
         * comme clé dans des structures de données comme `std::unordered_map`.
         *
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
     * @brief Classe représentant un routeur pour l'acheminement des requêtes HTTP.
     *
     * Le routeur gère l'acheminement des requêtes entrantes vers le gestionnaire approprié en fonction de la méthode HTTP (GET, POST, etc.)
     * et de l'URL. Il permet également de gérer des routes dynamiques et d'assurer la validation des paramètres dans les requêtes.
     */
    class Router
    {
    public:
        using RouteKey = std::pair<http::verb, std::string>;

        /**
         * @brief Constructeur par défaut pour initialiser un routeur.
         *
         * Ce constructeur initialise une table de routage vide, prête à accepter des routes.
         */
        Router() : routes_() {}

        /**
         * @brief Ajoute une nouvelle route au routeur.
         *
         * Cette méthode permet d'ajouter une route spécifique, en associant une méthode HTTP et un chemin d'URL
         * à un gestionnaire de requêtes. Le gestionnaire sera appelé lorsque cette route est invoquée.
         *
         * @param method La méthode HTTP pour laquelle cette route sera active (GET, POST, etc.).
         * @param route Le chemin de l'URL associé à cette route.
         * @param handler Un gestionnaire de requêtes, chargé de traiter les requêtes pour cette route.
         */
        void add_route(http::verb method, const std::string &route, std::shared_ptr<IRequestHandler> handler);

        /**
         * @brief Traite une requête HTTP entrante.
         *
         * Cette méthode est responsable de l'acheminement de la requête vers le gestionnaire approprié,
         * en fonction de la méthode HTTP et de l'URL. Elle génère également une réponse HTTP.
         *
         * @param req La requête HTTP entrante.
         * @param res La réponse HTTP à remplir en fonction de la requête.
         * @return Retourne `true` si la requête a été traitée avec succès, `false` sinon.
         */
        bool handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res);

    private:
        /**
         * @brief Vérifie si le chemin de la requête correspond à une route dynamique.
         *
         * Cette méthode est utilisée pour tester si le chemin de la requête correspond à une route qui contient des paramètres dynamiques.
         * Elle tente de faire correspondre la requête avec des routes définies dynamiquement à l'aide de motifs.
         *
         * @param route_pattern Le motif de la route dynamique à tester.
         * @param path Le chemin de la requête.
         * @param handler Le gestionnaire de requêtes à associer si la route correspond.
         * @param res La réponse HTTP qui sera générée si la route correspond.
         * @param req La requête HTTP à traiter.
         * @return Retourne `true` si la route dynamique correspond, `false` sinon.
         */
        bool matches_dynamic_route(const std::string &route_pattern, const std::string &path, std::shared_ptr<IRequestHandler> handler, http::response<http::string_body> &res, const http::request<http::string_body> &req);

        /**
         * @brief Convertit un modèle de route en une expression régulière.
         *
         * Cette méthode prend un modèle de route sous forme de chaîne de caractères et le convertit en une expression régulière,
         * permettant ainsi de faire correspondre des routes dynamiques.
         *
         * @param route_pattern Le modèle de route sous forme de chaîne.
         * @return L'expression régulière correspondant au modèle de la route.
         */
        static std::string convert_route_to_regex(const std::string &route_pattern);

        /**
         * @brief Assainit une chaîne d'entrée pour éviter les risques de sécurité.
         *
         * Cette méthode nettoie la chaîne d'entrée en supprimant ou échappant certains caractères
         * susceptibles de poser problème (comme des caractères spéciaux dans une URL).
         *
         * @param input La chaîne d'entrée à assainir.
         * @return La chaîne assainie.
         */
        std::string sanitize_input(const std::string &input);

        /**
         * @brief Valide les paramètres extraits de l'URL ou du corps de la requête.
         *
         * Cette méthode valide les paramètres associés à la requête (par exemple, les paramètres de l'URL)
         * avant d'envoyer la réponse. Elle s'assure que les paramètres sont conformes aux attentes.
         *
         * @param params La map des paramètres extraits.
         * @param res La réponse HTTP à renvoyer.
         * @return Retourne `true` si les paramètres sont valides, `false` sinon.
         */
        bool validate_parameters(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res);

        /**
         * @brief La table des routes, indexée par une paire (méthode HTTP, URL).
         *
         * Cette table de routage stocke toutes les routes enregistrées sous forme de paires clé-valeur.
         * La clé est une paire composée de la méthode HTTP et de l'URL, et la valeur est un gestionnaire
         * qui traite les requêtes correspondant à cette route.
         */
        std::unordered_map<RouteKey, std::shared_ptr<IRequestHandler>, PairHash> routes_;

        /**
         * @brief Convertit une map de paramètres en une chaîne de caractères.
         *
         * Cette méthode prend une map de paramètres et la transforme en une chaîne formatée,
         * afin de pouvoir l'afficher ou l'utiliser dans une réponse.
         *
         * @param map La map des paramètres à convertir.
         * @return La chaîne formatée représentant la map des paramètres.
         */
        std::string map_to_string(const std::unordered_map<std::string, std::string> &map);
    };
};

#endif // ROUTER_HPP
