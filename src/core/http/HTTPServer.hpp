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
#include "config/Config.hpp"
#include "routing/Router.hpp"
#include "session/Session.hpp"
#include "Response.hpp"
#include "config/RouteConfigurator.hpp"
#include "ThreadPool.hpp"

namespace Softadastra
{

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    namespace net = boost::asio;

    using tcp = net::ip::tcp;
    using json = nlohmann::json;

    constexpr size_t NUMBER_OF_THREADS = 8;
    class HTTPServer
    {
    public:
        explicit HTTPServer(Config &config);
        ~HTTPServer();
        void run();
        void start_accept();
        int calculate_io_thread_count();

    private:
        void handle_client(std::shared_ptr<tcp::socket> socket_ptr, Router &router);
        void close_socket(std::shared_ptr<tcp::socket> socket);
        Config &config_;
        std::shared_ptr<net::io_context> io_context_;
        std::unique_ptr<tcp::acceptor> acceptor_;
        Router router_;
        std::unique_ptr<RouteConfigurator> route_configurator_;
        Softadastra::ThreadPool request_thread_pool_;
        std::vector<std::thread> io_threads_;
        std::atomic<bool> stop_requested_;
    };

};

#endif // HTTPSERVER_HPP
