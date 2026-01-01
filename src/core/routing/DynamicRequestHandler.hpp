#ifndef DYNAMICREQUESTHANDLER_HPP
#define DYNAMICREQUESTHANDLER_HPP

#include "IRequestHandler.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <functional>
#include <boost/beast/http.hpp>

namespace Softadastra
{
    class DynamicRequestHandler : public IRequestHandler
    {
    public:
        explicit DynamicRequestHandler(std::function<void(const std::unordered_map<std::string, std::string> &,
                                                          http::response<http::string_body> &)>
                                           handler);
        ~DynamicRequestHandler();
        void handle_request(const http::request<http::string_body> &req,
                            http::response<http::string_body> &res) override;
        void set_params(
            const std::unordered_map<std::string, std::string> &params,
            http::response<http::string_body> &res);

    private:
        std::unordered_map<std::string, std::string> params_;
        std::function<void(const std::unordered_map<std::string, std::string> &,
                           http::response<http::string_body> &)>
            handler_;
    };
};

#endif // DYNAMICREQUESTHANDLER_HPP
