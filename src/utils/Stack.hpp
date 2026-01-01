#include <vector>
#include <stdexcept>
#include <iterator>
#include <algorithm>

namespace Softadastra
{
    class StackEmptyException : public std::underflow_error
    {
    public:
        StackEmptyException() : std::underflow_error("La pile est vide.") {}
    };

    template <typename T>
    class Stack
    {
    private:
        std::vector<T> elems;

    public:
        Stack() = default;
        Stack(const Stack &other) : elems(other.elems) {}
        Stack(Stack &&other) noexcept : elems(std::move(other.elems)) {}

        Stack &operator=(const Stack &other)
        {
            if (this != &other)
            {
                elems = other.elems;
            }
            return *this;
        }

        Stack &operator=(Stack &&other) noexcept
        {
            if (this != &other)
            {
                elems = std::move(other.elems);
            }
            return *this;
        }

        void push(const T &elem)
        {
            elems.push_back(elem);
        }

        void push(T &&elem)
        {
            elems.push_back(std::move(elem));
        }

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

        T top() const
        {
            if (empty())
            {
                throw StackEmptyException();
            }

            return elems.back();
        }

        bool empty() const
        {
            return elems.empty();
        }

        size_t size() const
        {
            return elems.size();
        }

        size_t capacity() const
        {
            return elems.capacity();
        }

        void shrink_to_fit()
        {
            elems.shrink_to_fit();
        }

        void clear()
        {
            elems.clear();
        }

        void swap(Stack &other)
        {
            std::swap(elems, other.elems);
        }

        typename std::vector<T>::iterator begin()
        {
            return elems.begin();
        }

        typename std::vector<T>::iterator end()
        {
            return elems.end();
        }

        typename std::vector<T>::const_iterator begin() const
        {
            return elems.begin();
        }

        typename std::vector<T>::const_iterator end() const
        {
            return elems.end();
        }
    };

    template <typename T>
    void swap(Stack<T> &stack1, Stack<T> &stack2)
    {
        stack1.swap(stack2);
    }
} // namespace Softadastra
