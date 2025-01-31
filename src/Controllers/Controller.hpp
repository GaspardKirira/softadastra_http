#ifndef IROUTES_HPP
#define IROUTES_HPP

#include "../kernel/Router.hpp"
#include "../config/Config.hpp" // Inclure Config pour l'injection

namespace Softadastra
{
    class Controller
    {
    public:
        // Constructeur prenant une référence à Config
        explicit Controller(Config &config) : config_(config) {}

        virtual ~Controller() = default;

        // Méthode virtuelle pure pour configurer les routes
        virtual void configure(Router &routes) = 0;

    protected:
        Config &config_; // Référence à la configuration, accessible dans les classes dérivées
    };
} // namespace Softadastra

#endif // IROUTES_HPP
