#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>

#include <boost/regex.hpp>
#include <string>

#include <unordered_map>
#include <iostream>
#include <thread>
#include <memory>
#include <functional>
#include <spdlog/spdlog.h>

#include "SimpleRequestHandler.hpp"
#include "IRequestHandler.hpp"
#include "DynamicRequestHandler.hpp"
#include "../config/Config.hpp"
#include "Router.hpp"
#include "Session.hpp"
#include "Response.hpp"
#include "../config/RouteConfigurator.hpp"
#include "ThreadPool.hpp"

namespace Softadastra
{

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    namespace net = boost::asio;

    using tcp = net::ip::tcp;
    using json = nlohmann::json;

    constexpr size_t NUMBER_OF_THREADS = 8;

    /**
     * @class HTTPServer
     * @brief Represents an HTTP server that listens for incoming client connections.
     *
     * The HTTPServer class is responsible for initializing the server, accepting client
     * connections, and delegating requests to appropriate handlers. It uses a thread pool
     * for handling incoming requests asynchronously.
     */
    class HTTPServer
    {
    public:
        /**
         * @brief Constructor to initialize the HTTP server with a configuration object.
         *
         * @param config The configuration object that contains server settings (e.g., port, routes).
         */
        explicit HTTPServer(Config &config);

        void handle_signals();

        void stop();

        /**
         * @brief Starts the server and begins accepting incoming connections.
         *
         * This method initializes the I/O context, starts the acceptor, and begins listening
         * for incoming client connections. Once a connection is accepted, it is handled by
         * the `handle_client` method.
         */
        void run();

        /**
         * @brief Starts the acceptor to listen for incoming client connections.
         *
         * This method begins the asynchronous accept operation, where new client connections
         * will be accepted and passed to the `handle_client` method.
         */
        void start_accept();

    private:
        /**
         * @brief Handles a client connection and processes the request.
         *
         * This method is responsible for managing the client connection, which includes creating
         * a session for the client, routing the request, and sending back the response.
         *
         * @param socket_ptr A shared pointer to the socket used for communication with the client.
         * @param router A reference to the Router used to handle the HTTP request.
         */
        void handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router);

        void close_socket(std::shared_ptr<tcp::socket> socket);

        Config &config_;                                        ///< Configuration object for the server settings.
        std::shared_ptr<net::io_context> io_context_;           ///< I/O context for asynchronous operations.
        std::unique_ptr<tcp::acceptor> acceptor_;               ///< Acceptor for accepting incoming TCP connections.
        Router router_;                                         ///< Router used to route HTTP requests.
        std::unique_ptr<RouteConfigurator> route_configurator_; ///< Configurator for setting up routes.

        Softadastra::ThreadPool request_thread_pool_; ///< Thread pool for handling incoming requests.
        std::vector<std::thread> io_threads_;         ///< Threads for running the I/O context.

        std::atomic<bool> stop_requested_;
    };

};

#endif // HTTPSERVER_HPP
