#include "Session.hpp"

namespace Softadastra
{

    Session::Session(tcp::socket socket, Router &router)
        : socket_(std::move(socket)), router_(router), buffer_(8060), req_() {}

    void Session::run()
    {
        read_request();
    }

    void Session::read_request()
    {
        auto self = shared_from_this();
        spdlog::info("Reading request from client...");

        buffer_.consume(buffer_.size()); // Vider le buffer existant
        spdlog::info("Buffer cleared, size: {}", buffer_.size());

        auto timer = std::make_shared<boost::asio::steady_timer>(socket_.get_executor());
        timer->expires_after(std::chrono::seconds(5));

        timer->async_wait([this, self](boost::system::error_code ec)
                          {
                          if (!ec)
                          {
                              spdlog::warn("Timeout: No request received after 5 seconds!");
                              boost::system::error_code ignored_ec;
                              socket_.close(ignored_ec); // Fermez le socket proprement
                          } });

        spdlog::info("Calling async_read()...");

        http::async_read(socket_, buffer_, req_,
                         [this, self, timer](boost::system::error_code ec, std::size_t bytes_transferred)
                         {
                             timer->cancel(); // Annulez le timer si la lecture aboutit

                             if (ec)
                             {
                                 spdlog::error("Error during async_read: {}", ec.message());
                                 boost::system::error_code ignored_ec;
                                 socket_.close(ignored_ec); // Fermez le socket proprement en cas d'erreur
                                 return;
                             }

                             spdlog::info("Request read successfully ({} bytes)", bytes_transferred);
                             spdlog::info("Method: {}", req_.method_string());
                             spdlog::info("Target: {}", req_.target());
                             spdlog::info("Body: {}", req_.body());

                             handle_request(ec); // Traitez la requête après une lecture réussie
                         });

        spdlog::info("async_read() called!");
    }

    void Session::handle_request(const boost::system::error_code &ec)
    {
        if (ec)
        {
            spdlog::error("Error handling request: {}", ec.message());
            return;
        }

        if (req_.body().size() > MAX_REQUEST_SIZE)
        {
            spdlog::warn("Request too large: {} bytes", req_.body().size());
            send_error("Request too large");
            return;
        }

        http::response<http::string_body> res;
        if (!router_.handle_request(req_, res))
        {
            spdlog::warn("Route not found for request: {}", req_.target());
            send_error("Route not found");
        }
        else
        {
            send_response(res);
        }
    }

    void Session::send_response(http::response<http::string_body> &res)
    {
        auto self = shared_from_this();
        auto res_ptr = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(std::move(res));

        boost::beast::http::async_write(socket_, *res_ptr,
                                        [this, self, res_ptr](boost::system::error_code ec, std::size_t)
                                        {
                                            if (ec)
                                            {
                                                spdlog::error("Error sending response: {}", ec.message());
                                                return;
                                            }

                                            spdlog::info("Response sent successfully.");
                                            socket_.shutdown(tcp::socket::shutdown_both);
                                            socket_.close();
                                        });
    }

    void Session::send_error(const std::string &error_message)
    {
        http::response<http::string_body> res;
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "application/json");
        res.body() = json{{"message", error_message}}.dump();

        send_response(res);
    }

}