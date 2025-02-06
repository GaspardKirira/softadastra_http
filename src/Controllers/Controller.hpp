#ifndef IROUTES_HPP
#define IROUTES_HPP

#include "../kernel/Router.hpp"
#include "../config/Config.hpp"
#include "../kernel/Response.hpp"

namespace Softadastra
{
    /**
     * @class Controller
     * @brief Base class for defining routes in the application.
     *
     * The Controller class is intended to be inherited by concrete controllers
     * that define routes and configure routing behavior. It uses the Router to
     * map specific routes to handler functions.
     */
    class Controller
    {
    public:
        /**
         * @brief Constructor for the Controller class.
         *
         * Initializes the controller with a reference to the configuration object.
         * This configuration is used for setting up routes and other server parameters.
         *
         * @param config The configuration object that contains server settings.
         */
        explicit Controller(Config &config) : config_(config) {}

        /**
         * @brief Virtual destructor for the Controller class.
         *
         * The virtual destructor ensures proper cleanup when derived classes are destroyed.
         */
        virtual ~Controller() = default;

        /**
         * @brief Pure virtual method to configure routes for the controller.
         *
         * This method must be implemented in derived classes to configure specific
         * routes (e.g., HTTP methods and paths) that the controller will handle.
         *
         * @param routes The Router object used to map routes to handler functions.
         */
        virtual void configure(Router &routes) = 0;

    protected:
        Config &config_; ///< A reference to the configuration object used by the controller.
    };
} // namespace Softadastra

#endif // IROUTES_HPP
