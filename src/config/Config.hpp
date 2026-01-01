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
    Config();
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
    static Config &getInstance();
    void loadConfig();
    void loadConfigOnce();
    std::shared_ptr<sql::Connection> getDbConnection();
    std::string getDbPasswordFromEnv();
    const std::string &getDbHost() const;
    const std::string &getDbUser() const;
    const std::string &getDbName() const;
    int getDbPort() const;
    int getServerPort() const;

private:
    std::string db_host;
    std::string db_user;
    std::string db_pass;
    std::string db_name;
    int db_port;
    int server_port;
};

#endif // CONFIG_HPP
