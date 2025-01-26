#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

/**
 * @brief Interface pour les gestionnaires de route HTTP.
 *
 * Cette interface définit la méthode `handle_request`, qui doit être implémentée par
 * toute classe souhaitant traiter une requête HTTP et envoyer une réponse.
 */
class IRequestHandler
{
public:
    /**
     * @brief Traite une requête HTTP et génère une réponse.
     *
     * Cette méthode est appelée lorsque le routeur trouve une route qui correspond à
     * une requête HTTP. Elle est responsable de la création de la réponse appropriée.
     *
     * @param req La requête HTTP reçue.
     * @param res La réponse HTTP à renvoyer au client.
     */
    virtual void handle_request(const http::request<http::string_body> &req,
                                http::response<http::string_body> &res) = 0;

    // Destructeur virtuel pour permettre la gestion correcte des objets dérivés.
    virtual ~IRequestHandler() = default;
};

#endif // IREQUESTHANDLER_HPP
