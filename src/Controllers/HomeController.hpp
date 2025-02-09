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

namespace Softadastra
{
    class HomeController : public Controller
    {
    public:
        using Controller::Controller;

        void configure(Router &routes) override
        {
            routes.add_route(
                http::verb::post, "/predict",
                std::static_pointer_cast<IRequestHandler>(
                    std::make_shared<DynamicRequestHandler>(
                        [this](const std::unordered_map<std::string, std::string> &params,
                               http::response<http::string_body> &res)
                        {
                            handler_predict(params, res);
                        })));
        }
        void handler_predict(const std::unordered_map<std::string, std::string> &params, http::response<http::string_body> &res)
        {
            try
            {

                json json_data;
                try
                {
                    json_data = json::parse(params.at("body"));
                }
                catch (const std::exception &e)
                {
                    Response::error_response(res, http::status::bad_request, "Invalid JSON");
                    return;
                }

                if (json_data.find("revenu") == json_data.end())
                {
                    Response::error_response(res, http::status::bad_request, "Le champ 'revenu' est manquant");
                    return;
                }

                double revenu = json_data["revenu"];
                double predicted_cost = predict(revenu);

                json response_json;
                response_json["revenu"] = revenu;
                response_json["predicted_cost"] = predicted_cost;

                Response::json_response(res, response_json);
            }
            catch (const json::exception &e)
            {
                std::cout << "JSON exception: " << e.what() << std::endl;
                Response::error_response(res, http::status::bad_request, "Invalid JSON format");
            }
            catch (const std::exception &e)
            {
                std::cout << "General exception: " << e.what() << std::endl;
                Response::error_response(res, http::status::internal_server_error, e.what());
            }
        }

        inline double predict(double revenu)
        {
            return revenu * 0.3;
        }
    };
} // namespace Softadastra

#endif // HOMEROUTES_HPP
