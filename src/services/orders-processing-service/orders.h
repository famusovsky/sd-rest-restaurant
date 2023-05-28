#include <cpr/cpr.h>
#include <crow.h>
#include <string>
#include <vector>
#include "orders-db.h"

class OrdersService {
public:
    OrdersService();

    void init(crow::SimpleApp& app, const std::string& path);

private:
    OrdersDB db_;
    // TODO: change this to the real url
    std::string basic_url_ = "http://localhost:8080";

    crow::response createOrder(const crow::json::rvalue& body);
    crow::response getOrder(const crow::json::rvalue& body);
    crow::response manageOrder(const crow::json::rvalue& body);
    crow::response manageDish(const crow::json::rvalue& body);

    void processOrders();
};