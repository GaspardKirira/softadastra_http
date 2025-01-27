#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio/ip/tcp.hpp>
#include "Response.hpp"
#include "Router.hpp"
#include <algorithm>

namespace Softadastra
{
    namespace http = boost::beast::http;
    namespace net = boost::asio;
    namespace beast = boost::beast;
    using tcp = net::ip::tcp;

    constexpr size_t MAX_REQUEST_SIZE = 8192; // Limite de taille de la requête

    /**
     * @brief Classe représentant une session de communication avec un client via un socket TCP.
     *
     * Une session est responsable de la gestion d'une connexion client, de la lecture de la requête
     * HTTP envoyée par ce client et de l'envoi d'une réponse via le socket.
     */
    class Session
    {
    public:
        /**
         * @brief Constructeur de la session.
         *
         * Le constructeur initialise une session avec un socket TCP spécifique et un routeur
         * qui permettra de gérer les requêtes HTTP du client.
         *
         * @param socket Le socket TCP utilisé pour la communication avec le client.
         * @param router Une référence à un objet `Router` qui gère les requêtes HTTP.
         */
        // Déclaration du constructeur
        explicit Session(tcp::socket socket, Softadastra::Router &router);

        /**
         * @brief Démarre la session et traite la requête du client.
         *
         * Cette méthode démarre la session en lisant la requête HTTP du client et en la transmettant au
         * routeur pour traitement. Ensuite, elle envoie la réponse au client.
         */
        void run();

    private:
        tcp::socket socket_;          /**< Le socket TCP pour la communication avec le client. */
        Softadastra::Router &router_; /**< Le routeur utilisé pour gérer les requêtes HTTP. */
    };
};

#endif // SESSION_HPP
