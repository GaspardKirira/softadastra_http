#include "Session.hpp"

namespace Softadastra
{

    Session::Session(tcp::socket socket, Router &router)
        : socket_(std::move(socket)), router_(router) {}

    void Session::run()
    {
        try
        {
            beast::flat_buffer buffer{8060}; // Buffer pour lire la requête
            http::request<http::string_body> req;
            http::read(socket_, buffer, req);

            // Vérification de la taille de la requête
            if (req.body().size() > MAX_REQUEST_SIZE)
            {
                // Log d'erreur côté serveur
                spdlog::warn("Request too large: {} bytes", req.body().size());
                Response::send_error(socket_, "Request too large");
                return;
            }

            http::response<http::string_body> res;
            if (!router_.handle_request(req, res))
            {
                // Route non trouvée, envoie d'une erreur
                spdlog::warn("Route not found for request: {}", req.target());
                Response::send_error(socket_, "Route not found");
            }
            else
            {
                // Réponse validée et envoyée
                Response::send(socket_, res);
            }
        }
        catch (const std::exception &e)
        {
            // Log de l'erreur avec message envoyé au client
            spdlog::error("Error during session: {}", e.what());
            std::string error_msg = "Session Error: " + std::string(e.what());
            Response::send_error(socket_, error_msg);
            spdlog::warn("Sent error to client: {}", error_msg);
        }
    }

}
