#include <crow.h>
#include "services/authentification-service/auth.h"
#include "services/orders-processing-service/orders.h"

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

    AuthentificationService auth_service;
    OrdersService orders_service;

    try {
        auth_service.init(app, "/auth");
        orders_service.init(app, "/orders");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    

    app.port(8080)
    .multithreaded()
    .run();

    return 0;
}
