#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>

#include <boost/regex.hpp>
#include <string>

#include <unordered_map>
#include <iostream>
#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include "IRequestHandler.hpp"
#include "config/Config.hpp"

namespace Softadastra
{

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    namespace net = boost::asio;

    using tcp = net::ip::tcp;
    using ssl_socket = boost::asio::ssl::stream<tcp::socket>;
    using json = nlohmann::json;

    struct PairHash
    {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const
        {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };

    class Router
    {
    public:
        using RouteKey = std::pair<http::verb, std::string>;

        Router() : routes_(), route_patterns_() {}
        ~Router();
        void add_route(http::verb method, const std::string &route, std::shared_ptr<IRequestHandler> handler);
        bool handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res);

    private:
        bool matches_dynamic_route(const std::string &route_pattern, const std::string &path, std::shared_ptr<IRequestHandler> handler, http::response<http::string_body> &res, const http::request<http::string_body> &req);
        static std::string convert_route_to_regex(const std::string &route_pattern);
        std::string sanitize_input(const std::string &input);
        bool validate_parameters(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res);
        std::unordered_map<RouteKey, std::shared_ptr<IRequestHandler>, PairHash> routes_;
        std::string map_to_string(const std::unordered_map<std::string, std::string> &map);
        std::vector<std::string> route_patterns_;
    };
};

#endif // ROUTER_HPP
