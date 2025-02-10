#include "Session.hpp"
#include "Response.hpp"
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <spdlog/spdlog.h>

namespace Softadastra
{

    // Constructeur modifié pour accepter un tcp::socket
    Session::Session(tcp::socket socket, Router &router)
        : socket_(std::move(socket)), router_(router), buffer_(8060), req_()
    {
        spdlog::info("Session initialized for client: {}", socket_.remote_endpoint().address().to_string());
    }

    void Session::run()
    {
        auto self = shared_from_this();

        // Suppression du SSL handshake, car nous utilisons une connexion non sécurisée
        read_request();
    }

    void Session::read_request()
    {
        // Vérification si le socket est ouvert
        if (!socket_.is_open())
        {
            spdlog::error("Socket is not open, cannot read request!");
            return;
        }

        auto self = shared_from_this();
        buffer_.consume(buffer_.size());

        // Timer pour les délais de requêtes
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

        // Lecture de la requête
        boost::beast::http::async_read(socket_, buffer_, req_,
                                       [this, self, timer](boost::system::error_code ec, std::size_t bytes_transferred)
                                       {
                                           timer->cancel();

                                           if (ec)
                                           {
                                               spdlog::error("Error during async_read: {}", ec.message());
                                               close_socket();
                                               return;
                                           }

                                           spdlog::info("Request read successfully ({} bytes)", bytes_transferred);

                                           // Vérification WAF avant de traiter la requête
                                           if (!waf_check_request(req_))
                                           {
                                               spdlog::warn("Suspicious request blocked.");
                                               send_error("Forbidden: Suspicious request");
                                               return;
                                           }

                                           spdlog::info("Method: {}", req_.method_string());
                                           spdlog::info("Target: {}", req_.target());
                                           spdlog::info("Body: {}", req_.body());

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

        // Vérification de la taille de la requête
        if (req_.body().size() > MAX_REQUEST_SIZE)
        {
            spdlog::warn("Request too large: {} bytes", req_.body().size());
            send_error("Request too large");
            return;
        }

        boost::beast::http::response<boost::beast::http::string_body> res;
        bool success = router_.handle_request(req_, res);

        if (!success)
        {
            if (res.result() == boost::beast::http::status::method_not_allowed)
            {
                send_error("Method Not Allowed");
            }
            else if (res.result() == boost::beast::http::status::not_found)
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

    void Session::send_response(boost::beast::http::response<boost::beast::http::string_body> &res)
    {
        // Vérification si le socket est ouvert
        if (!socket_.is_open())
        {
            spdlog::error("Socket is not open, cannot send response!");
            return;
        }

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
        boost::beast::http::response<boost::beast::http::string_body> res;
        Response::error_response(res, boost::beast::http::status::bad_request, error_message);

        send_response(res);
    }

    void Session::close_socket()
    {
        boost::system::error_code ignored_ec;
        // Vérification si le socket est ouvert
        if (socket_.is_open())
        {
            socket_.close(ignored_ec);
            spdlog::info("Socket closed.");
        }
        else
        {
            spdlog::error("Socket already closed or not open.");
        }
    }

    bool Session::waf_check_request(const boost::beast::http::request<boost::beast::http::string_body> &req)
    {
        // Exemple simple de détection XSS
        if (req.target().find("<script>") != std::string::npos)
        {
            spdlog::warn("Possible XSS attack detected in URL: {}", req.target());
            return false;
        }

        // Vérification d'une injection SQL basique
        if (req.body().find("SELECT * FROM") != std::string::npos)
        {
            spdlog::warn("Possible SQL injection detected in body: {}", req.body());
            return false;
        }

        // Limitation de la taille des en-têtes et du corps
        if (req.body().size() > MAX_REQUEST_BODY_SIZE)
        {
            spdlog::warn("Request body too large: {} bytes", req.body().size());
            return false;
        }

        // Exemple de détection d'un User-Agent suspect
        if (req.find("User-Agent") != req.end() &&
            req["User-Agent"].find("curl") != std::string::npos &&
            req["User-Agent"].find("8.5.0") == std::string::npos) // Ne bloque pas curl/8.5.0
        {
            spdlog::warn("Suspicious User-Agent detected: {}", req["User-Agent"]);
            return false;
        }

        return true;
    }

} // namespace Softadastra
