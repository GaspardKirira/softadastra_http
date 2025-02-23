#include "config/Config.hpp"
#include "http/HTTPServer.hpp"

int main()
{
    try
    {
        Config &config = Config::getInstance();
        config.loadConfig();
        Softadastra::HTTPServer server(config);

        server.run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Critical error: {}", e.what());
    }

    return 0;
}
