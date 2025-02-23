#include "Session.hpp"
#include "http/Response.hpp"
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <spdlog/spdlog.h>

namespace Softadastra
{

    Session::Session(tcp::socket socket, Router &router)
        : socket_(std::move(socket)), router_(router), buffer_(8060), req_()
    {
        // spdlog::info("Session initialized for client: {}", socket_.remote_endpoint().address().to_string());
    }

    Session::~Session() {}

    void Session::run()
    {
        auto self = shared_from_this();
        read_request();
    }

    void Session::read_request()
    {
        if (!socket_.is_open())
        {
            spdlog::error("Socket is not open, cannot read request!");
            return;
        }

        auto self = shared_from_this();
        buffer_.consume(buffer_.size());

        auto timer = std::make_shared<boost::asio::steady_timer>(socket_.get_executor());
        timer->expires_after(std::chrono::seconds(5));

        std::weak_ptr<boost::asio::steady_timer> weak_timer = timer;
        timer->async_wait([this, self, weak_timer](boost::system::error_code ec)
                          {
            auto timer = weak_timer.lock();
            if (!timer)
            {
                spdlog::info("Timer is no longer available.");
                return;
            }
    
            if (!ec)
            {
                spdlog::warn("Timeout: No request received after 5 seconds!");
                close_socket();
            } });

        http::async_read(socket_, buffer_, req_,
                         [this, self, timer](boost::system::error_code ec, std::size_t bytes_transferred)
                         {
                             timer->cancel();
                             if (ec)
                             {
                                 if (ec == boost::asio::error::eof)
                                 {
                                     spdlog::info("Connection closed cleanly by peer.");
                                 }
                                 else if (ec != boost::asio::error::operation_aborted)
                                 {
                                     spdlog::error("Error during async_read: {}", ec.message());
                                 }
                                 close_socket();
                                 return;
                             }

                             spdlog::info("Request read successfully ({} bytes)", bytes_transferred);

                             handle_request(ec);
                         });
    }

    void Session::handle_request(const boost::system::error_code &ec)
    {
        if (ec)
        {
            spdlog::error("Error handling request: {}", ec.message());
            return;
        }

        if (req_.body().size() > MAX_REQUEST_BODY_SIZE)
        {
            spdlog::warn("Request too large: {} bytes", req_.body().size());
            send_error("Request too large");
            return;
        }

        http::response<http::string_body> res;
        bool success = router_.handle_request(req_, res);

        if (!success)
        {
            if (res.result() == http::status::method_not_allowed)
            {
                send_error("Method Not Allowed");
            }
            else if (res.result() == http::status::not_found)
            {
                send_error("Route Not Found");
            }
            else
            {
                send_error("Invalid request");
            }
            return;
        }

        send_response(res);
    }

    void Session::send_response(http::response<http::string_body> &res)
    {
        if (!socket_.is_open())
        {
            spdlog::error("Socket is not open, cannot send response!");
            return;
        }

        auto self = shared_from_this();
        auto res_ptr = std::make_shared<http::response<http::string_body>>(std::move(res));

        http::async_write(socket_, *res_ptr,
                          [this, self, res_ptr](boost::system::error_code ec, std::size_t)
                          {
                              if (ec)
                              {
                                  spdlog::error("Error sending response: {}", ec.message());
                                  close_socket();
                                  return;
                              }

                              static std::chrono::steady_clock::time_point last_log_time = std::chrono::steady_clock::now();
                              auto now = std::chrono::steady_clock::now();
                              if (now - last_log_time > std::chrono::seconds(10))
                              {
                                  spdlog::info("Response sent successfully.");
                                  last_log_time = now;
                              }

                              socket_.shutdown(tcp::socket::shutdown_both);
                              socket_.close();
                          });
    }

    void Session::send_error(const std::string &error_message)
    {
        http::response<http::string_body> res;
        Response::error_response(res, http::status::bad_request, error_message);

        send_response(res);
    }

    void Session::close_socket()
    {
        boost::system::error_code ignored_ec;
        if (socket_.is_open())
        {
            socket_.close(ignored_ec);
            if (ignored_ec)
            {
                spdlog::warn("Error closing socket: {}", ignored_ec.message());
            }
            else
            {
                spdlog::info("Socket closed.");
            }
        }
        else
        {
            spdlog::warn("Socket already closed or not open.");
        }
    }

    bool Session::waf_check_request(const http::request<http::string_body> &req)
    {
        // Détection d'une attaque XSS
        if (req.target().find("<script>") != std::string::npos)
        {
            spdlog::warn("Possible XSS attack detected in URL: {}", req.target());
            return false;
        }

        // Détection d'une injection SQL simple
        if (req.body().find("SELECT * FROM") != std::string::npos)
        {
            spdlog::warn("Possible SQL injection detected in body: {}", req.body());
            return false;
        }

        // Vérification de la taille du corps de la requête
        if (req.body().size() > MAX_REQUEST_BODY_SIZE)
        {
            spdlog::warn("Request body too large: {} bytes", req.body().size());
            return false;
        }

        // Détection d'un User-Agent suspect
        if (req.find("User-Agent") != req.end() &&
            req["User-Agent"].find("curl") != std::string::npos &&
            req["User-Agent"].find("8.5.0") == std::string::npos) // Exclure certaines versions de curl
        {
            spdlog::warn("Suspicious User-Agent detected: {}", req["User-Agent"]);
            return false;
        }

        // Vérification de la présence de certains en-têtes suspects (par exemple, Origin ou Referer)
        if (req.find("Origin") != req.end() || req.find("Referer") != req.end())
        {
            spdlog::warn("Suspicious Origin or Referer headers detected.");
            return false;
        }

        return true;
    }

} // namespace Softadastra
