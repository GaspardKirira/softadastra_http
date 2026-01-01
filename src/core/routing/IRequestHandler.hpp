#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

class IRequestHandler
{
public:
    virtual void handle_request(const http::request<http::string_body> &req,
                                http::response<http::string_body> &res) = 0;
    IRequestHandler() = default;
    virtual ~IRequestHandler() = default;
    IRequestHandler(const IRequestHandler &) = delete;
    void operator=(const IRequestHandler &) = delete;
};

#endif // IREQUESTHANDLER_HPP
