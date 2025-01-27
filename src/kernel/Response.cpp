#include "Response.hpp"

namespace Softadastra
{
    void Response::send(tcp::socket &socket, http::response<http::string_body> &res)
    {
        // Envoi de la réponse HTTP au client via le socket
        http::write(socket, res);
    }

    void Response::send_error(tcp::socket &socket, const std::string &error_message)
    {
        // Création de la réponse d'erreur avec un code HTTP 400 (Bad Request)
        http::response<http::string_body> res;
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "application/json");

        // Formatage du message d'erreur en JSON
        res.body() = json{{"message", error_message}}.dump();

        // Envoi de la réponse d'erreur
        send(socket, res);
    }

}