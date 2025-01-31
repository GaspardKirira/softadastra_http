#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
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
    using ssl_socket = boost::asio::ssl::stream<tcp::socket>;
    using json = nlohmann::json;

    constexpr size_t NUMBER_OF_THREADS = 6;

    /**
     * @brief Classe représentant un serveur HTTP.
     *
     * Cette classe gère le serveur HTTP, y compris l'acceptation des connexions, la gestion des routes,
     * et l'envoi de réponses HTTP.
     */
    class HTTPServer
    {
    public:
        explicit HTTPServer(Config &config);
        void run();
        void start_accept();

    private:
        void handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router);

        Config &config_;
        std::shared_ptr<net::io_context> io_context_;
        std::unique_ptr<tcp::acceptor> acceptor_;
        Router router_;
        std::unique_ptr<RouteConfigurator> route_configurator_;

        Softadastra::ThreadPool request_thread_pool_; // Nouveau pool pour requêtes
        std::vector<std::thread> io_threads_;         // Threads pour io_context_
    };

};

#endif // HTTPSERVER_HPP
