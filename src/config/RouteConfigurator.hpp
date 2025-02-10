#ifndef ROUTECONFIGURATOR_HPP
#define ROUTECONFIGURATOR_HPP

#include "../kernel/Router.hpp"
#include "../kernel/SimpleRequestHandler.hpp"
#include "../kernel/DynamicRequestHandler.hpp"
#include "../kernel/IRequestHandler.hpp"
#include "../config/Config.hpp"
#include <unordered_map>
#include <string>
#include <memory>

namespace Softadastra
{
    /**
     * @brief La classe RouteConfigurator est responsable de la configuration des routes pour le routeur.
     *
     * Elle prend en charge l'ajout des routes, telles que les routes statiques et dynamiques,
     * pour le serveur HTTP en utilisant un routeur fourni.
     */
    class RouteConfigurator
    {
    public:
        /**
         * @brief Constructeur de la classe RouteConfigurator.
         *
         * @param router Le routeur qui sera configuré avec les routes.
         */
        explicit RouteConfigurator(Router &router);

        /**
         * @brief Configure les routes pour le serveur HTTP.
         *
         * Cette méthode ajoute des routes statiques et dynamiques au routeur, en définissant des gestionnaires
         * de requêtes qui répondent aux différentes URL du serveur, comme "/users/{id}" ou "/products/{slug}".
         */
        void configure_routes();

    private:
        Router &router_; /**< Le routeur qui sera utilisé pour ajouter les routes. */
    };
}

#endif // ROUTECONFIGURATOR_HPP
