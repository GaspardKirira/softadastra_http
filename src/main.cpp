#include "config/Config.hpp"
#include "kernel/HTTPServer.hpp"

/**
 * @brief Point d'entrée du serveur HTTP.
 *
 * Ce programme charge la configuration, crée une instance de serveur HTTP
 * et lance son exécution. Il gère également les erreurs critiques qui
 * pourraient survenir durant le démarrage du serveur.
 */
int main()
{
    try
    {
        // Chargement de la configuration du serveur
        Config &config = Config::getInstance();
        config.loadConfig();

        // Création et lancement du serveur HTTP
        Softadastra::HTTPServer server(config);
        server.run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Critical error: {}", e.what());
    }

    return 0;
}
