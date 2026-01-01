#ifndef SIMPLEREQUESTHANDLER_HPP
#define SIMPLEREQUESTHANDLER_HPP

#include <algorithm>
#include "IRequestHandler.hpp"

namespace Softadastra
{
    class SimpleRequestHandler : public IRequestHandler
    {
    public:
        explicit SimpleRequestHandler(std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler);
        ~SimpleRequestHandler() {}
        void handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res) override;

    private:
        std::function<void(const http::request<http::string_body> &, http::response<http::string_body> &)> handler_;
    };
};

#endif // SIMPLEREQUESTHANDLER_HPP
