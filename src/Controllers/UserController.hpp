#ifndef USERROUTES_HPP
#define USERROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <optional>
#include <cppconn/prepared_statement.h>
#include "Controller.hpp"
#include <nlohmann/json.hpp>
#include <sstream>
#include "http/Response.hpp"
#include "User.hpp"

namespace Softadastra
{
    class UserController : public Controller
    {
    public:
        using Controller::Controller;

        void configure(Router &router) override
        {
            auto self = std::shared_ptr<UserController>(this, [](UserController *) {});

            router.add_route(http::verb::get, "/users",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [self](const std::unordered_map<std::string, std::string> &,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::vector<User> users = self->findAll();

                                             if (!users.empty())
                                             {
                                                 nlohmann::json users_json = nlohmann::json::array();
                                                 for (const auto &user : users)
                                                 {
                                                     users_json.push_back(user.to_json());
                                                 }
                                                 Response::json_response(res, users_json);
                                             }
                                             else
                                             {
                                                 Softadastra::Response::no_content_response(res, "No users found");
                                             }
                                         }
                                         catch (const std::exception &e)
                                         {
                                             Softadastra::Response::error_response(res, http::status::internal_server_error, e.what());
                                         }
                                     })));

            router.add_route(http::verb::get, "/users/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [self](const std::unordered_map<std::string, std::string> &params,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::stringstream ss(params.at("id"));
                                             int id{};
                                             ss >> id;
                                             auto user = self->get_user_by_id(id);

                                             if (user)
                                             {
                                                 Softadastra::Response::json_response(res, user->to_json());
                                             }
                                             else
                                             {
                                                 Softadastra::Response::error_response(res, http::status::not_found, "User not found");
                                             }
                                         }
                                         catch (const std::exception &e)
                                         {
                                             Softadastra::Response::error_response(res, http::status::internal_server_error, e.what());
                                         }
                                     })));

            router.add_route(http::verb::post, "/create",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [self](const std::unordered_map<std::string, std::string> &params,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             json request_json;
                                             try
                                             {
                                                 request_json = json::parse(params.at("body"));
                                             }
                                             catch (const std::exception &e)
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Inavlid JSON body");
                                                 return;
                                             }

                                             if (request_json.find("firstname") == request_json.end())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Le champ 'firstname' est manquant.");
                                                 return;
                                             }
                                             if (request_json.find("email") == request_json.end())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Le champ 'email' est manquant.");
                                                 return;
                                             }

                                             User new_user = self->createUser(request_json["firstname"], request_json["email"]);
                                             Response::create_response(res, http::status::created, "User created successfully");
                                         }
                                         catch (const nlohmann::json::exception &e)
                                         {
                                             Response::error_response(res, http::status::bad_request, "Invalid JSON format");
                                         }
                                         catch (const std::exception &e)
                                         {
                                             Softadastra::Response::error_response(res, http::status::internal_server_error, e.what());
                                         }
                                     })));

            router.add_route(http::verb::put, "/update/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [self](const std::unordered_map<std::string, std::string> &params,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             // Récupérer l'ID de l'utilisateur à partir de l'URL
                                             std::stringstream ss(params.at("id"));
                                             int id{};
                                             ss >> id;

                                             // Vérifier que le corps de la requête contient les champs nécessaires
                                             json request_json;
                                             try
                                             {
                                                 request_json = json::parse(params.at("body"));
                                             }
                                             catch (const std::exception &e)
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Invalid JSON body");
                                                 return;
                                             }

                                             if (request_json.find("firstname") == request_json.end())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Le champ 'firstname' est manquant.");
                                                 return;
                                             }
                                             if (request_json.find("email") == request_json.end())
                                             {
                                                 Response::error_response(res, http::status::bad_request, "Le champ 'email' est manquant.");
                                                 return;
                                             }

                                             // Appeler la fonction updateUser pour mettre à jour l'utilisateur dans la base de données
                                             User updated_user = self->updateUser(id, request_json["firstname"], request_json["email"]);

                                             // Répondre avec un message de succès et les informations de l'utilisateur mis à jour
                                             Softadastra::Response::json_response(res, updated_user.to_json());
                                         }
                                         catch (const std::exception &e)
                                         {
                                             Softadastra::Response::error_response(res, http::status::internal_server_error, e.what());
                                         }
                                     })));
        }

    public:
        std::shared_ptr<sql::Connection> getDbConnection()
        {
            try
            {
                Config &config = Config::getInstance();
                config.loadConfig();
                return config.getDbConnection();
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Erreur de connexion à la base de données.");
            }
        }

        User createUser(const std::string &full_name, const std::string &email)
        {
            try
            {
                std::shared_ptr<sql::Connection> con = getDbConnection();
                if (!con)
                {
                    throw std::runtime_error("La connexion à la base de données a échoué.");
                }

                std::unique_ptr<sql::PreparedStatement> pstmt(
                    con->prepareStatement("INSERT INTO test_user (full_name, email) VALUES (?, ?)"));
                pstmt->setString(1, full_name);
                pstmt->setString(2, email);
                pstmt->executeUpdate();

                std::shared_ptr<sql::Statement> stmt(con->createStatement());
                std::shared_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT LAST_INSERT_ID()"));
                rs->next();
                int new_id = rs->getInt(1);

                User new_user;
                new_user.setId(new_id);
                new_user.setFullName(full_name);
                new_user.setEmail(email);

                return new_user;
            }
            catch (const sql::SQLException &e)
            {
                throw std::runtime_error("Erreur lors de la création de l'utilisateur : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Erreur lors de la création de l'utilisateur : " + std::string(e.what()));
            }
        }

        User updateUser(int user_id, const std::string &full_name, const std::string &email)
        {
            try
            {
                // Se connecter à la base de données
                std::shared_ptr<sql::Connection> con = getDbConnection();
                if (!con)
                {
                    throw std::runtime_error("La connexion à la base de données a échoué.");
                }

                // Préparer la requête SQL UPDATE
                std::unique_ptr<sql::PreparedStatement> pstmt(
                    con->prepareStatement("UPDATE test_user SET full_name = ?, email = ? WHERE id = ?"));

                // Lier les paramètres (full_name, email et user_id)
                pstmt->setString(1, full_name);
                pstmt->setString(2, email);
                pstmt->setInt(3, user_id);

                // Exécuter la requête UPDATE
                int rows_affected = pstmt->executeUpdate();

                // Si aucune ligne n'a été affectée, cela signifie que l'utilisateur n'existe pas
                if (rows_affected == 0)
                {
                    throw std::runtime_error("Aucun utilisateur trouvé avec cet ID.");
                }

                // Récupérer les informations mises à jour
                User updated_user;
                updated_user.setId(user_id);
                updated_user.setFullName(full_name);
                updated_user.setEmail(email);

                return updated_user;
            }
            catch (const sql::SQLException &e)
            {
                throw std::runtime_error("Erreur lors de la mise à jour de l'utilisateur : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Erreur lors de la mise à jour de l'utilisateur : " + std::string(e.what()));
            }
        }

        std::optional<User> get_user_by_id(int userId)
        {
            try
            {
                auto con = getDbConnection();
                if (!con)
                {
                    throw std::runtime_error("La connexion à la base de données a échoué.");
                }

                std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM test_user WHERE id = ?"));
                pstmt->setInt(1, userId);
                std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

                if (res->next())
                {
                    User user;
                    user.setId(res->getInt("id"));
                    user.setFullName(res->getString("full_name"));
                    user.setEmail(res->getString("email"));
                    return user;
                }
                return std::nullopt;
            }
            catch (const sql::SQLException &e)
            {
                throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
            }
        }

        std::vector<User> findAll()
        {
            std::vector<User> users;

            try
            {
                std::shared_ptr<sql::Connection> con = getDbConnection();
                if (!con)
                {
                    throw std::runtime_error("La connexion à la base de données a échoué.");
                }

                std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM test_user"));
                std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

                while (res->next())
                {
                    User user;
                    user.setId(res->getInt("id"));
                    user.setFullName(res->getString("full_name"));
                    user.setEmail(res->getString("email"));
                    users.push_back(user);
                }
            }
            catch (const sql::SQLException &e)
            {
                throw std::runtime_error("Erreur lors de la récupération des utilisateurs : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Erreur lors de la récupération des utilisateurs : " + std::string(e.what()));
            }

            return users;
        }
    };
}

#endif
