#ifndef SIMPLEREQUESTHANDLER_HPP
#define SIMPLEREQUESTHANDLER_HPP

#include <algorithm>
#include "IRequestHandler.hpp"

namespace Softadastra
{
    /**
     * @brief Classe implémentant un gestionnaire de requêtes simple.
     *
     * Cette classe permet d'utiliser une fonction personnalisée comme gestionnaire
     * de requêtes HTTP en encapsulant la logique de traitement dans un objet
     * `std::function`.
     */
    class SimpleRequestHandler : public IRequestHandler
    {
    public:
        /**
         * @brief Constructeur qui prend une fonction personnalisée comme gestionnaire.
         *
         * @param handler Fonction à exécuter pour gérer la requête.
         */
        explicit SimpleRequestHandler(std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler);

        /**
         * @brief Gère une requête HTTP en exécutant la fonction personnalisée fournie.
         *
         * @param req Requête HTTP reçue.
         * @param res Réponse HTTP à envoyer.
         */
        void handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res) override;

    private:
        std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler_;
    };
};

#endif // SIMPLEREQUESTHANDLER_HPP
