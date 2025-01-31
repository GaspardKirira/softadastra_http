#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Softadastra
{
    class User
    {
    public:
        // Constructeur de l'utilisateur
        User(const std::string &id, const std::string &name, const std::string &email)
            : id_(id), full_name_(name), email_(email) {}

        // Getters pour accéder aux attributs
        const std::string &getId() const { return id_; }
        const std::string &getName() const { return full_name_; }
        const std::string &getEmail() const { return email_; }

        // Définir la méthode to_json pour la classe User
        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"id", id_},
                {"full_name", full_name_},
                {"email", email_}};
        }

    private:
        std::string id_;
        std::string full_name_;
        std::string email_;
    };
}

#endif // USER_HPP
