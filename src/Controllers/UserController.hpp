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

namespace Softadastra
{

    class User
    {
    public:
        User() : id_(), full_name_(), email_() {}

        const int &getId() const { return id_; }
        const std::string &getName() const { return full_name_; }
        const std::string &getEmail() const { return email_; }

        void setId(int id)
        {
            id_ = id;
        }

        void setFullName(const std::string &name)
        {
            full_name_ = name;
        }

        void setEmail(const std::string &userEmail)
        {
            email_ = userEmail;
        }
        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"id", id_},
                {"full_name", full_name_},
                {"email", email_}};
        }

    private:
        int id_;
        std::string full_name_;
        std::string email_;
    };

    class UserController : public Controller
    {
    public:
        using Controller::Controller;

        void configure(Router &router)
        {
            auto self = std::shared_ptr<UserController>(this, [](UserController *) {});

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
                                                 res.result(http::status::ok);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = user->to_json().dump();
                                             }
                                             else
                                             {
                                                 res.result(http::status::not_found);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = nlohmann::json{{"error", "User not found"}}.dump();
                                             }
                                         }
                                         catch (const std::exception &e)
                                         {
                                             res.result(http::status::internal_server_error);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = nlohmann::json{{"error", e.what()}}.dump();
                                         }
                                     })));

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
                                                 res.result(http::status::ok);
                                                 res.set(http::field::content_type, "application/json");
                                                 nlohmann::json users_json = nlohmann::json::array();
                                                 for (const auto &user : users)
                                                 {
                                                     users_json.push_back(user.to_json());
                                                 }
                                                 res.body() = users_json.dump();
                                             }
                                             else
                                             {
                                                 res.result(http::status::no_content); 
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = nlohmann::json{{"message", "No users found"}}.dump();
                                             }
                                         }
                                         catch (const std::exception &e)
                                         {
                                             res.result(http::status::internal_server_error);
                                             res.set(http::field::content_type, "application/json");
                                             res.body() = nlohmann::json{{"error", e.what()}}.dump();
                                         }
                                     })));
        }

    private:
        std::optional<User> get_user_by_id(int userId)
        {
            try
            {
                Config &config = Config::getInstance();
                config.loadConfig();

                std::unique_ptr<sql::Connection> con = config.getDbConnection();
                if (!con)
                {
                    throw std::runtime_error("La connexion à la base de données a échoué.");
                }
                std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM tbl_user WHERE id = ?"));
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
                std::cerr << "Erreur SQL : " << e.what() << std::endl;
                throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Erreur générique : " << e.what() << std::endl;
                throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
            }
        }

        std::vector<User> findAll()
        {
            std::vector<User> users;

            try
            {
                Config &config = Config::getInstance();
                config.loadConfig();

                std::unique_ptr<sql::Connection> con = config.getDbConnection();
                if (!con)
                {
                    throw std::runtime_error("La connexion à la base de données a échoué.");
                }

                std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM tbl_user"));
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
                std::cerr << "Erreur SQL : " << e.what() << std::endl;
                throw std::runtime_error("Erreur lors de la récupération des utilisateurs : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Erreur générique : " << e.what() << std::endl;
                throw std::runtime_error("Erreur lors de la récupération des utilisateurs : " + std::string(e.what()));
            }

            return users;
        }
    };
}

#endif
