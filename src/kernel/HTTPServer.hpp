#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include <boost/regex.hpp>
#include <string>

#include <unordered_map>
#include <iostream>
#include <thread>
#include <memory>
#include <functional>
#include <spdlog/spdlog.h>

#include "SimpleRequestHandler.hpp"
#include "IRequestHandler.hpp"
#include "DynamicRequestHandler.hpp"
#include "../config/Config.hpp"
#include "Router.hpp"
#include "Session.hpp"
#include "Response.hpp"
#include "../config/RouteConfigurator.hpp"

namespace Softadastra
{

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    namespace net = boost::asio;

    using tcp = net::ip::tcp;
    using ssl_socket = boost::asio::ssl::stream<tcp::socket>;
    using json = nlohmann::json;

   constexpr size_t NUMBER_OF_THREADS = 6;

    /**
     * @brief Classe représentant un serveur HTTP.
     *
     * Cette classe gère le serveur HTTP, y compris l'acceptation des connexions, la gestion des routes,
     * et l'envoi de réponses HTTP.
     */
    class HTTPServer
    {
    public:
        /**
         * @brief Constructeur pour initialiser le serveur HTTP avec la configuration fournie.
         *
         * @param config La configuration du serveur, incluant le port d'écoute.
         */
        explicit HTTPServer(Config &config);

        /**
         * @brief Démarre le serveur HTTP et commence à accepter les connexions.
         */
        void run();

    private:
        /**
         * @brief Gère la session pour un client connecté.
         *
         * @param socket Le socket de la connexion client.
         * @param router Le routeur utilisé pour acheminer les requêtes HTTP.
         */
        void handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router);

        Config &config_;                                        ///< Référence à la configuration du serveur.
        std::unique_ptr<net::io_context> io_context_;           ///< Contexte d'entrée/sortie pour le serveur.
        std::unique_ptr<tcp::acceptor> acceptor_;               ///< Accepteur pour accepter les connexions entrantes.
        Router router_;                                         ///< Router pour acheminer les requêtes HTTP.
        std::unique_ptr<RouteConfigurator> route_configurator_; ///< Responsable de la configuration des routes.
    };
};

#endif // HTTPSERVER_HPP
