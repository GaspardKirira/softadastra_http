#include <iostream>
#include <bcrypt.h>

int main()
{
    std::string password = "123456789qpeiddjnf";

    // Générer un sel bcrypt
    char salt[BCRYPT_HASHSIZE];
    bcrypt_gensalt(12, salt);

    // Hachage du mot de passe
    char hash[BCRYPT_HASHSIZE];
    bcrypt_hashpw(password.c_str(), salt, hash);

    std::cout << "Hash bcrypt: " << hash << std::endl;

    // Comparaison avec un autre mot de passe
    if (bcrypt_checkpw(password.c_str(), hash) == 0)
    {
        std::cout << "Le mot de passe correspond au hash bcrypt!" << std::endl;
    }
    else
    {
        std::cout << "Le mot de passe ne correspond pas au hash bcrypt." << std::endl;
    }

    return 0;
}
