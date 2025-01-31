#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio/ip/tcp.hpp>
#include "Response.hpp"
#include "Router.hpp"
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <memory> // Pour std::enable_shared_from_this

namespace Softadastra
{
    namespace http = boost::beast::http;
    namespace net = boost::asio;
    namespace beast = boost::beast;
    using tcp = net::ip::tcp;
    using json = nlohmann::json;

    constexpr size_t MAX_REQUEST_SIZE = 8192; // Limite de taille de la requête

    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(tcp::socket socket, Softadastra::Router &router);

        void run();

    private:
        void read_request();
        void close_socket();
        void handle_request(const boost::system::error_code &ec);

        void send_response(http::response<http::string_body> &res);
        void send_error(const std::string &error_message);

        tcp::socket socket_;
        Softadastra::Router &router_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
    };
};

#endif // SESSION_HPP