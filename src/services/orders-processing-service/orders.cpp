#include <stdexcept>
#include "orders.h"

OrdersService::OrdersService() = default;

void OrdersService::init(crow::SimpleApp& app, const std::string& path) {
    try {
        db_.init("orders.db");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw e;
    }

    // TODO: add routes
}