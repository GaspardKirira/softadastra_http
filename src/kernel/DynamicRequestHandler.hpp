#ifndef DYNAMICREQUESTHANDLER_HPP
#define DYNAMICREQUESTHANDLER_HPP

#include "IRequestHandler.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <functional>
#include <boost/beast/http.hpp>

namespace Softadastra
{
    /**
     * @brief Classe pour gérer les requêtes avec des paramètres dynamiques dans l'URL.
     *
     * Cette classe permet de traiter des routes contenant des paramètres dynamiques,
     * comme /users/{id} ou /products/{slug}, et de passer ces paramètres à un gestionnaire personnalisé.
     */
    class DynamicRequestHandler : public IRequestHandler
    {
    public:
        /**
         * @brief Constructeur pour initialiser le gestionnaire de requêtes dynamiques.
         *
         * @param handler La fonction qui traite la requête en utilisant les paramètres dynamiques.
         */
        explicit DynamicRequestHandler(std::function<void(const std::unordered_map<std::string, std::string> &,
                                                          http::response<http::string_body> &)>
                                           handler);

        /**
         * @brief Implémente la méthode handle_request de IRequestHandler.
         *
         * @param req La requête HTTP reçue (non utilisée ici, car les paramètres sont déjà extraits).
         * @param res La réponse HTTP qui sera envoyée.
         */
        void handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res) override;

        /**
         * @brief Met à jour les paramètres dynamiques pour cette route.
         *
         * @param params Les paramètres extraits de l'URL.
         */
        void set_params(const std::unordered_map<std::string, std::string> &params);

    private:
        std::unordered_map<std::string, std::string> params_; ///< Paramètres dynamiques de la route.
        std::function<void(const std::unordered_map<std::string, std::string> &,
                           http::response<http::string_body> &)>
            handler_; ///< Fonction de gestion de la requête.
    };
};

#endif // DYNAMICREQUESTHANDLER_HPP
