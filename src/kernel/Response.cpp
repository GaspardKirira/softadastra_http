#include "Response.hpp"

namespace Softadastra
{
    void Response::send(tcp::socket &socket, http::response<http::string_body> &res)
    {
        http::write(socket, res);
    }

    void Response::send_error(tcp::socket &socket, const std::string &error_message)
    {
        http::response<http::string_body> res;
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "application/json");

        res.body() = json{{"message", error_message}}.dump();

        send(socket, res);
    }

}