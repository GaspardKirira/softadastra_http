#ifndef HOMEROUTES_HPP
#define HOMEROUTES_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <ctime>
#include <unordered_set>
#include <sstream>
#include "Controller.hpp"
#include "routing/UnifiedRequestHandler.hpp"

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
                    std::make_shared<UnifiedRequestHandler>(
                        [this](const http::request<http::string_body> &req [[maybe_unused]],
                               http::response<http::string_body> &res [[maybe_unused]])
                        {
                            Response::success_response(res, "Hello world");
                        })));
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
