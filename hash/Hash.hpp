// Hash.hpp
#ifndef HASH_HPP
#define HASH_HPP

#include <string>

namespace Softadastra
{
    class Hash
    {
    public:
        static std::string hashPassword(const std::string &password);
        static bool comparePassword(const std::string &password, const std::string &stored_hash);
    };
} // namespace Softadastra

#endif // HASH_HPP
