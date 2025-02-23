#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <memory>

namespace beast = boost::beast;   // For beast::http and beast::core
namespace http = beast::http;     // For http::request and http::response
namespace net = boost::asio;      // For io_context
using tcp = boost::asio::ip::tcp; // For TCP endpoints

// La classe pour gérer une connexion client
class session : public std::enable_shared_from_this<session>
{
public:
    explicit session(tcp::socket socket)
        : socket_(std::move(socket)) {}

    void start()
    {
        do_read();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;

    void do_read()
    {
        // Asynchrone: Lire une requête HTTP
        http::async_read(socket_, buffer_, req_,
                         [self = shared_from_this()](boost::system::error_code ec, std::size_t)
                         {
                             if (!ec)
                             {
                                 self->handle_request();
                             }
                         });
    }

    void handle_request()
    {
        // Créer une réponse HTTP simple
        http::response<http::string_body> res{http::status::ok, req_.version()};
        res.set(http::field::server, "Boost.Beast Server");
        res.set(http::field::content_type, "text/plain");
        res.body() = "Hello, world!";
        res.prepare_payload();

        // Asynchrone: Envoyer la réponse
        http::async_write(socket_, res,
                          [self = shared_from_this()](boost::system::error_code ec, std::size_t)
                          {
                              if (!ec)
                              {
                                  self->do_close();
                              }
                          });
    }

    void do_close()
    {
        // Fermer la connexion proprement
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }
};

// Classe pour accepter les connexions
class server
{
public:
    server(net::io_context &ioc, tcp::endpoint endpoint)
        : acceptor_(ioc, endpoint)
    {
        do_accept();
    }

private:
    tcp::acceptor acceptor_;

    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<session>(std::move(socket))->start();
                }
                do_accept();
            });
    }
};

int main()
{
    try
    {
        // Le contexte IO pour les opérations asynchrones
        net::io_context ioc;

        // Adresse et port du serveur
        tcp::endpoint endpoint(tcp::v4(), 8080);

        // Lancer le serveur
        server srv(ioc, endpoint);

        // Exécuter l'IO context
        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erreur: " << e.what() << std::endl;
    }

    return 0;
}
