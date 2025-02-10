#ifndef USER_HPP
#define USER_HPP
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

    json to_json() const
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

#endif
