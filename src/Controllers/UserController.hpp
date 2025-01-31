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

namespace Softadastra
{

    class User
    {
    public:
        // Constructeur de l'utilisateur
        User() : id_(), full_name_(), email_() {}

        // Getters pour accéder aux attributs
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
        // Définir la méthode to_json pour la classe User
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
        using Controller::Controller; // Utilise le constructeur de Controller

        void configure(Router &router)
        {
            // Créer un pointeur partagé pour capturer 'this' de manière sécurisée
            auto self = std::shared_ptr<UserController>(this, [](UserController *) {});

            router.add_route(http::verb::get, "/users/{id}",
                             std::static_pointer_cast<IRequestHandler>(
                                 std::make_shared<DynamicRequestHandler>(
                                     [self](const std::unordered_map<std::string, std::string> &params,
                                            http::response<http::string_body> &res)
                                     {
                                         try
                                         {
                                             std::string user_id = params.at("id");
                                             try
                                             {
                                                 int id = std::stoi(user_id);          // Conversion de l'ID
                                                 auto user = self->get_user_by_id(id); // Appel à get_user_by_id

                                                 if (user)
                                                 {
                                                     res.result(http::status::ok);
                                                     res.set(http::field::content_type, "application/json");
                                                     res.body() = user->to_json().dump(); // Conversion en JSON
                                                 }
                                                 else
                                                 {
                                                     res.result(http::status::not_found);
                                                     res.set(http::field::content_type, "application/json");
                                                     res.body() = nlohmann::json{{"error", "User not found"}}.dump();
                                                 }
                                             }
                                             catch (const std::invalid_argument &e)
                                             {
                                                 std::cerr << "ID invalide : " << e.what() << std::endl;
                                                 res.result(http::status::bad_request);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = nlohmann::json{{"error", "Invalid ID format"}}.dump();
                                             }
                                             catch (const std::out_of_range &e)
                                             {
                                                 std::cerr << "ID hors de portée : " << e.what() << std::endl;
                                                 res.result(http::status::bad_request);
                                                 res.set(http::field::content_type, "application/json");
                                                 res.body() = nlohmann::json{{"error", "ID out of range"}}.dump();
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
                // Utilisation de shared_ptr pour gérer la connexion à la base de données
                std::unique_ptr<sql::mysql::MySQL_Driver> driver(sql::mysql::get_driver_instance());
                if (!driver)
                {
                    std::cerr << "Failed to get MYSQL driver" << std::endl;
                }

                std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1", "root", ""));
                if (!con)
                {
                    std::cerr << "Failed to connect to the database" << std::endl;
                }
                con->setSchema("softadastra");
                std::unique_ptr<sql::Statement> stmt(con->createStatement());
                if (!stmt)
                {
                    std::cerr << "Failed to create statement" << std::endl;
                }

                // Création d'un PreparedStatement avec shared_ptr
                std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM users WHERE id = ?"));
                // Définir le paramètre de la requête
                pstmt->setInt(1, userId);
                // Exécuter la requête et obtenir le résultat
                std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

                // Si un utilisateur est trouvé
                if (res->next())
                {
                    User user;
                    user.setId(res->getInt("id"));
                    user.setFullName(res->getString("full_name"));
                    user.setEmail(res->getString("email"));

                    return user; // Retourner l'utilisateur trouvé
                }

                // Aucun utilisateur trouvé, retourner std::nullopt
                return std::nullopt;
            }
            catch (const sql::SQLException &e)
            {
                // Gestion des erreurs SQL
                std::cerr << "Erreur SQL : " << e.what() << std::endl;
                throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
            }
            catch (const std::exception &e)
            {
                // Gestion des autres exceptions
                std::cerr << "Erreur générique : " << e.what() << std::endl;
                throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
            }
        }
    };
}

#endif // USERROUTES_HPP
