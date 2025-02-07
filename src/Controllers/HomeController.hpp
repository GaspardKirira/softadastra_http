#ifndef HOMEROUTES_HPP
#define HOMEROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <ctime>
#include <unordered_set>
#include "Controller.hpp"

namespace Softadastra
{
    class HomeController : public Controller
    {
    public:
        using Controller::Controller;

        void configure(Router &routes) override
        {
            routes.add_route(
                http::verb::get, "/",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<DynamicRequestHandler>(
                        [](const std::unordered_map<std::string, std::string> &,
                           http::response<http::string_body> &res)
                        {
                            Response::success_response(res, "Welcome to the Softadastra HTTP Server");
                        })));
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
