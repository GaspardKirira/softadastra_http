#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <utility>  // Pour std::pair
#include <iterator> // Pour les itérateurs

namespace Softadastra
{
    // Exception personnalisée pour clé inexistante
    class KeyNotFoundException : public std::out_of_range
    {
    public:
        KeyNotFoundException() : std::out_of_range("La clé spécifiée n'existe pas dans la map.") {}
    };

    // Classe UnorderedMap générique
    template <typename KeyType, typename ValueType>
    class UnorderedMap
    {
    private:
        std::unordered_map<KeyType, ValueType> elems;

    public:
        // Constructeur par défaut
        UnorderedMap() = default;

        // Constructeur de copie
        UnorderedMap(const UnorderedMap &other) : elems(other.elems) {}

        // Constructeur par déplacement
        UnorderedMap(UnorderedMap &&other) noexcept : elems(std::move(other.elems)) {}

        // Opérateur d'affectation par copie
        UnorderedMap &operator=(const UnorderedMap &other)
        {
            if (this != &other)
            {
                elems = other.elems;
            }
            return *this;
        }

        // Opérateur d'affectation par déplacement
        UnorderedMap &operator=(UnorderedMap &&other) noexcept
        {
            if (this != &other)
            {
                elems = std::move(other.elems);
            }
            return *this;
        }

        // Ajouter une clé-valeur à la map
        void insert(const KeyType &key, const ValueType &value)
        {
            elems[key] = value;
        }

        // Ajouter une clé-valeur à la map (par déplacement)
        void insert(KeyType &&key, ValueType &&value)
        {
            elems[std::move(key)] = std::move(value);
        }

        // Accéder à la valeur associée à une clé
        ValueType &operator[](const KeyType &key)
        {
            return elems[key];
        }

        // Accéder à la valeur associée à une clé (const)
        const ValueType &operator[](const KeyType &key) const
        {
            return elems.at(key);
        }

        // Recherche d'une clé dans la map
        bool contains(const KeyType &key) const
        {
            return elems.find(key) != elems.end();
        }

        // Retirer un élément de la map par sa clé
        void erase(const KeyType &key)
        {
            auto it = elems.find(key);
            if (it != elems.end())
            {
                elems.erase(it);
            }
        }

        // Taille de la map
        size_t size() const
        {
            return elems.size();
        }

        // Vérifier si la map est vide
        bool empty() const
        {
            return elems.empty();
        }

        // Vider la map
        void clear()
        {
            elems.clear();
        }

        // Retourner la capacité de la map (estimation de la mémoire allouée)
        size_t capacity() const
        {
            return elems.bucket_count();
        }

        // Itérateur sur le début de la map
        typename std::unordered_map<KeyType, ValueType>::iterator begin()
        {
            return elems.begin();
        }

        // Itérateur sur la fin de la map
        typename std::unordered_map<KeyType, ValueType>::iterator end()
        {
            return elems.end();
        }

        // Itérateur constant sur le début de la map
        typename std::unordered_map<KeyType, ValueType>::const_iterator begin() const
        {
            return elems.begin();
        }

        // Itérateur constant sur la fin de la map
        typename std::unordered_map<KeyType, ValueType>::const_iterator end() const
        {
            return elems.end();
        }

        // Méthode pour récupérer une paire (clé, valeur) d'une clé donnée
        std::pair<KeyType, ValueType> get_pair(const KeyType &key) const
        {
            auto it = elems.find(key);
            if (it == elems.end())
            {
                throw KeyNotFoundException();
            }
            return *it;
        }

        // Méthode pour afficher le contenu de la map
        void print() const
        {
            for (const auto &pair : elems)
            {
                std::cout << pair.first << " : " << pair.second << std::endl;
            }
        }
    };

    // Fonction swap pour échanger deux maps
    template <typename KeyType, typename ValueType>
    void swap(UnorderedMap<KeyType, ValueType> &map1, UnorderedMap<KeyType, ValueType> &map2)
    {
        using std::swap;
        swap(map1.elems, map2.elems);
    }
} // namespace Softadastra
