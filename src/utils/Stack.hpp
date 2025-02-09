#include <vector>
#include <stdexcept> // Pour std::underflow_error et std::out_of_range
#include <iterator>  // Pour itérateurs
#include <algorithm> // Pour std::swap

namespace Softadastra
{
    // Exception personnalisée pour la pile vide
    class StackEmptyException : public std::underflow_error
    {
    public:
        StackEmptyException() : std::underflow_error("La pile est vide.") {}
    };

    // Classe Stack générique
    template <typename T>
    class Stack
    {
    private:
        std::vector<T> elems;

    public:
        // Constructeur par défaut
        Stack() = default;

        // Constructeur de copie
        Stack(const Stack &other) : elems(other.elems) {}

        // Constructeur par déplacement
        Stack(Stack &&other) noexcept : elems(std::move(other.elems)) {}

        // Opérateur d'affectation par copie
        Stack &operator=(const Stack &other)
        {
            if (this != &other)
            {
                elems = other.elems;
            }
            return *this;
        }

        // Opérateur d'affectation par déplacement
        Stack &operator=(Stack &&other) noexcept
        {
            if (this != &other)
            {
                elems = std::move(other.elems);
            }
            return *this;
        }

        // Ajouter un élément à la pile
        void push(const T &elem)
        {
            elems.push_back(elem);
        }

        // Ajouter un élément à la pile (par déplacement)
        void push(T &&elem)
        {
            elems.push_back(std::move(elem));
        }

        // Retirer et retourner l'élément du sommet de la pile
        T pop()
        {
            if (empty())
            {
                throw StackEmptyException();
            }

            T elem = elems.back();
            elems.pop_back();
            return elem;
        }

        // Accéder à l'élément du sommet sans le retirer
        T top() const
        {
            if (empty())
            {
                throw StackEmptyException();
            }

            return elems.back();
        }

        // Vérifier si la pile est vide
        bool empty() const
        {
            return elems.empty();
        }

        // Retourne la taille de la pile
        size_t size() const
        {
            return elems.size();
        }

        // Retourne la capacité actuelle de la pile
        size_t capacity() const
        {
            return elems.capacity();
        }

        // Réduire la capacité de la pile (optimisation mémoire)
        void shrink_to_fit()
        {
            elems.shrink_to_fit();
        }

        // Vider la pile
        void clear()
        {
            elems.clear();
        }

        // Méthode swap pour échanger deux piles
        void swap(Stack &other)
        {
            std::swap(elems, other.elems);
        }

        // Itérateur sur le début de la pile
        typename std::vector<T>::iterator begin()
        {
            return elems.begin();
        }

        // Itérateur sur la fin de la pile
        typename std::vector<T>::iterator end()
        {
            return elems.end();
        }

        // Itérateur constant sur le début de la pile
        typename std::vector<T>::const_iterator begin() const
        {
            return elems.begin();
        }

        // Itérateur constant sur la fin de la pile
        typename std::vector<T>::const_iterator end() const
        {
            return elems.end();
        }
    };

    // Fonction swap pour échanger deux piles
    template <typename T>
    void swap(Stack<T> &stack1, Stack<T> &stack2)
    {
        stack1.swap(stack2);
    }
} // namespace Softadastra
