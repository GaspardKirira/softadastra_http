#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <utility>
#include <iterator>

namespace Softadastra
{
    class KeyNotFoundException : public std::out_of_range
    {
    public:
        KeyNotFoundException() : std::out_of_range("La clé spécifiée n'existe pas dans la map.") {}
    };

    template <typename KeyType, typename ValueType>
    class UnorderedMap
    {
    private:
        std::unordered_map<KeyType, ValueType> elems;

    public:
        UnorderedMap() = default;
        UnorderedMap(const UnorderedMap &other) : elems(other.elems) {}
        UnorderedMap(UnorderedMap &&other) noexcept : elems(std::move(other.elems)) {}

        UnorderedMap &operator=(const UnorderedMap &other)
        {
            if (this != &other)
            {
                elems = other.elems;
            }
            return *this;
        }

        UnorderedMap &operator=(UnorderedMap &&other) noexcept
        {
            if (this != &other)
            {
                elems = std::move(other.elems);
            }
            return *this;
        }

        void insert(const KeyType &key, const ValueType &value)
        {
            elems[key] = value;
        }

        void insert(KeyType &&key, ValueType &&value)
        {
            elems[std::move(key)] = std::move(value);
        }

        ValueType &operator[](const KeyType &key)
        {
            return elems[key];
        }

        const ValueType &operator[](const KeyType &key) const
        {
            return elems.at(key);
        }

        bool contains(const KeyType &key) const
        {
            return elems.find(key) != elems.end();
        }

        void erase(const KeyType &key)
        {
            auto it = elems.find(key);
            if (it != elems.end())
            {
                elems.erase(it);
            }
        }

        size_t size() const
        {
            return elems.size();
        }

        bool empty() const
        {
            return elems.empty();
        }

        void clear()
        {
            elems.clear();
        }

        size_t capacity() const
        {
            return elems.bucket_count();
        }

        typename std::unordered_map<KeyType, ValueType>::iterator begin()
        {
            return elems.begin();
        }

        typename std::unordered_map<KeyType, ValueType>::iterator end()
        {
            return elems.end();
        }

        typename std::unordered_map<KeyType, ValueType>::const_iterator begin() const
        {
            return elems.begin();
        }

        typename std::unordered_map<KeyType, ValueType>::const_iterator end() const
        {
            return elems.end();
        }

        std::pair<KeyType, ValueType> get_pair(const KeyType &key) const
        {
            auto it = elems.find(key);
            if (it == elems.end())
            {
                throw KeyNotFoundException();
            }
            return *it;
        }

        void print() const
        {
            for (const auto &pair : elems)
            {
                std::cout << pair.first << " : " << pair.second << std::endl;
            }
        }
    };

    template <typename KeyType, typename ValueType>
    void swap(UnorderedMap<KeyType, ValueType> &map1, UnorderedMap<KeyType, ValueType> &map2)
    {
        using std::swap;
        swap(map1.elems, map2.elems);
    }
} // namespace Softadastra
