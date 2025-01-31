#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio/ip/tcp.hpp>
#include "Response.hpp"
#include "Router.hpp"
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <memory>

namespace Softadastra
{
    namespace http = boost::beast::http;
    namespace net = boost::asio;
    namespace beast = boost::beast;
    using tcp = net::ip::tcp;
    using json = nlohmann::json;

    constexpr size_t MAX_REQUEST_SIZE = 8192;

    /**
     * @class Session
     * @brief Represents a single HTTP session (request-response cycle).
     *
     * The Session class handles communication with a client over a TCP socket.
     * It reads HTTP requests, processes them, and sends appropriate responses.
     */
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        /**
         * @brief Constructor for creating a new HTTP session with a given socket.
         *
         * @param socket The TCP socket representing the client connection.
         * @param router A reference to the Router that will handle request routing.
         */
        explicit Session(tcp::socket socket, Softadastra::Router &router);

        /**
         * @brief Starts the session by initiating the request reading process.
         *
         * This method begins the asynchronous reading of an HTTP request from the client.
         * Once the request is read, it is processed.
         */
        void run();

    private:
        /**
         * @brief Reads an HTTP request from the client socket.
         *
         * This method is responsible for reading data from the socket into the request buffer.
         * It performs the actual reading asynchronously and handles potential errors.
         */
        void read_request();

        /**
         * @brief Closes the client socket.
         *
         * This method shuts down and closes the TCP socket used for the session.
         * It is called when the session is finished or an error occurs.
         */
        void close_socket();

        /**
         * @brief Handles the HTTP request after it has been read.
         *
         * This method processes the HTTP request and generates the corresponding response.
         * It is called after the request has been successfully read from the socket.
         *
         * @param ec The error code from the read operation (if any).
         */
        void handle_request(const boost::system::error_code &ec);

        /**
         * @brief Sends a response to the client.
         *
         * This method sends an HTTP response back to the client over the socket.
         *
         * @param res The HTTP response to send.
         */
        void send_response(http::response<http::string_body> &res);

        /**
         * @brief Sends an error message to the client.
         *
         * This method sends a simple error message as an HTTP response.
         * It is typically used when an error occurs in processing the request.
         *
         * @param error_message The error message to send.
         */
        void send_error(const std::string &error_message);

        tcp::socket socket_;                   ///< The TCP socket for client communication.
        Softadastra::Router &router_;          ///< Reference to the Router used to route requests.
        beast::flat_buffer buffer_;            ///< Buffer for reading HTTP requests.
        http::request<http::string_body> req_; ///< HTTP request object to store the incoming request.
    };
};

#endif // SESSION_HPP
