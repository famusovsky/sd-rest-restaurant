#include <crow.h>

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([]() { return "Hello world"; });

    CROW_ROUTE(app, "/")
    .methods("POST"_method)
    ([&app](const crow::request& req) {
        std::string body = req.body;
        if (body == "The End") {
            app.stop();
        }
        return crow::response(200);
    });

    app.port(8080)
    .multithreaded()
    .run();

    return 0;
}
