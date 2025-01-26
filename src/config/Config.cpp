#include "Config.hpp"
#include <filesystem>

namespace fs = std::filesystem;

// Chemin de configuration
const std::string CONFIG_FILE_PATH = "../src/config/config.json";

// Constructeur avec valeurs par défaut
Config::Config()
    : db_host("localhost"),
      db_user("root"),
      db_pass(),
      db_name("web243_adastra"),
      db_port(3306),
      server_port(8080)
{
}

using json = nlohmann::json;

void Config::loadConfig()
{
    // Vérifie si le fichier de configuration existe
    if (!fs::exists(CONFIG_FILE_PATH))
    {
        throw std::runtime_error("Le fichier de configuration n'existe pas : " + CONFIG_FILE_PATH);
    }

    std::ifstream config_file(CONFIG_FILE_PATH, std::ios::in);
    if (!config_file.is_open())
    {
        throw std::runtime_error("Impossible d'ouvrir le fichier de configuration : " + CONFIG_FILE_PATH);
    }

    json config;
    try
    {
        config_file >> config; // Charger le fichier JSON
    }
    catch (const json::parse_error &e)
    {
        throw std::runtime_error("Erreur de parsing du fichier JSON : " + std::string(e.what()));
    }

    // Vérification des sections principales
    if (!config.contains("database") || !config.contains("server"))
    {
        throw std::runtime_error("Fichier JSON invalide : sections 'database' ou 'server' manquantes.");
    }

    // Validation des clés et des types
    try
    {
        db_host = config.at("database").at("host").get<std::string>();
        db_user = config.at("database").at("user").get<std::string>();
        db_pass = config.at("database").at("password").get<std::string>();
        db_name = config.at("database").at("name").get<std::string>();
        server_port = config.at("server").at("port").get<int>();
    }
    catch (const json::type_error &e)
    {
        throw std::runtime_error("Types incorrects dans le fichier JSON : " + std::string(e.what()));
    }

    std::cout << "Configuration chargée avec succès. Serveur démarré sur le port " << server_port << std::endl;
}

std::unique_ptr<sql::Connection> Config::getDbConnection()
{
    try
    {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://" + db_host + ":" + std::to_string(db_port), db_user, db_pass));

        // Définir le schéma de la base de données
        con->setSchema(db_name);
        return con;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "Erreur de connexion à la base de données : " << e.what() << std::endl;
        throw std::runtime_error("Erreur de connexion à la base de données.");
    }
}

std::string Config::getDbPasswordFromEnv()
{
    const char *password = std::getenv("DB_PASSWORD");
    if (password == nullptr)
    {
        std::cerr << "Aucun mot de passe DB_PASSWORD trouvé dans les variables d'environnement." << std::endl;
        return "";
    }
    return std::string(password);
}

// Getters pour la configuration
const std::string &Config::getDbHost() const { return db_host; }
const std::string &Config::getDbUser() const { return db_user; }
const std::string &Config::getDbName() const { return db_name; }
int Config::getDbPort() const { return db_port; }
int Config::getServerPort() const { return server_port; }

// Définition de getInstance()
Config &Config::getInstance()
{
    static Config instance; // Instance unique de la configuration
    return instance;        // Retourne cette instance
}
