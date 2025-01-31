#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <mutex>
#include <stdexcept>
#include <cstdlib>
#include <memory>
#include <filesystem>

class Config
{
public:
    /**
     * @brief Constructeur de la classe Config.
     *
     * Initialise les membres de la classe avec des valeurs par défaut.
     */
    Config();

    /**
     * @brief Constructeur de copie supprimé.
     *
     * Empêche la copie de l'objet Config.
     */
    Config(const Config &) = delete;

    /**
     * @brief Opérateur d'affectation supprimé.
     *
     * Empêche l'assignation d'un objet Config à un autre.
     */
    Config &operator=(const Config &) = delete;

    /**
     * @brief Accède à l'instance unique de la configuration.
     *
     * @return L'instance unique de la configuration.
     */
    static Config &getInstance();

    /**
     * @brief Charge la configuration depuis un fichier JSON.
     *
     * Lit et charge les informations de configuration à partir du fichier `config.json`.
     * Les valeurs sont affectées aux membres de la classe.
     *
     * @throws std::runtime_error Si le fichier ne peut être ouvert ou si le parsing échoue.
     */
    void loadConfig();

    /**
     * @brief Obtient une connexion à la base de données MySQL.
     *
     * Crée et retourne une connexion à la base de données MySQL en utilisant
     * les informations de connexion spécifiées dans la configuration.
     *
     * @return Une connexion MySQL valide.
     * @throws std::runtime_error Si la connexion échoue.
     */
    std::unique_ptr<sql::Connection> getDbConnection();

    /**
     * @brief Récupère le mot de passe de la base de données depuis les variables d'environnement.
     *
     * Vérifie si une variable d'environnement `DB_PASSWORD` est définie et la retourne.
     * Si elle n'est pas définie, retourne une chaîne vide.
     *
     * @return Le mot de passe de la base de données ou une chaîne vide si non défini.
     */
    std::string getDbPasswordFromEnv();

    /**
     * @brief Obtient l'hôte de la base de données.
     *
     * @return L'hôte de la base de données.
     */
    const std::string &getDbHost() const;

    /**
     * @brief Obtient le nom d'utilisateur pour la base de données.
     *
     * @return Le nom d'utilisateur pour la base de données.
     */
    const std::string &getDbUser() const;

    /**
     * @brief Obtient le nom de la base de données.
     *
     * @return Le nom de la base de données.
     */
    const std::string &getDbName() const;

    /**
     * @brief Obtient le port de la base de données.
     *
     * @return Le port de la base de données.
     */
    int getDbPort() const;

    /**
     * @brief Obtient le port du serveur.
     *
     * @return Le port du serveur web.
     */
    int getServerPort() const;

private:
    // Variables de configuration
    std::string db_host; ///< Hôte de la base de données
    std::string db_user; ///< Utilisateur de la base de données
    std::string db_pass; ///< Mot de passe de la base de données
    std::string db_name; ///< Nom de la base de données
    int db_port;         ///< Port de la base de données
    int server_port;     ///< Port du serveur web
};

#endif // CONFIG_HPP
