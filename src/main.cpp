#include "config/Config.hpp"
#include "kernel/HTTPServer.hpp"

/**
 * @brief Point d'entrée du programme.
 *
 * Cette fonction initialise et démarre le serveur HTTP. Elle commence par charger
 * la configuration depuis un fichier, crée un serveur HTTP avec cette configuration,
 * puis lance le serveur pour accepter et traiter les requêtes des clients.
 * En cas d'erreur critique, une exception est capturée et un message d'erreur est loggé.
 *
 * @return 0 si l'exécution est réussie, ou un autre code d'erreur en cas d'échec.
 */
int main()
{
    try
    {
        // Récupère l'instance unique de la configuration et charge les paramètres de configuration.
        Config &config = Config::getInstance();
        config.loadConfig();

        // Crée une instance du serveur HTTP en lui passant la configuration chargée.
        Softadastra::HTTPServer server(config);

        // Démarre le serveur HTTP en l'exécutant.
        server.run();
    }
    catch (const std::exception &e)
    {
        // Capture toute exception qui pourrait survenir et log l'erreur avec un message approprié.
        spdlog::error("Critical error: {}", e.what());
    }

    return 0;
}
