#ifndef RESPONSESENDER_HPP
#define RESPONSESENDER_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace Softadastra
{
    namespace http = boost::beast::http;
    namespace net = boost::asio;

    using json = nlohmann::json;
    using tcp = net::ip::tcp;

    /**
     * @brief Classe utilitaire pour envoyer des réponses HTTP.
     *
     * Cette classe fournit des méthodes statiques pour envoyer des réponses HTTP standard
     * ainsi que des erreurs sous forme de réponses HTTP, facilitant ainsi l'envoi de
     * réponses au client sur un socket TCP.
     */
    class Response
    {
    public:
        /**
         * @brief Envoie une réponse HTTP au client.
         *
         * Cette méthode envoie la réponse HTTP spécifiée à travers un socket TCP.
         *
         * @param socket Le socket TCP sur lequel envoyer la réponse.
         * @param res La réponse HTTP à envoyer.
         */
        static void send(tcp::socket &socket, http::response<http::string_body> &res);

        /**
         * @brief Envoie une réponse d'erreur HTTP au client.
         *
         * Cette méthode envoie une réponse HTTP indiquant une erreur au client.
         * La réponse contient un message d'erreur formaté en JSON et a un statut HTTP 400 (Bad Request).
         *
         * @param socket Le socket TCP sur lequel envoyer la réponse.
         * @param error_message Le message d'erreur à envoyer dans la réponse.
         */
        static void send_error(tcp::socket &socket, const std::string &error_message);
    };
}

#endif // RESPONSESENDER_HPP
