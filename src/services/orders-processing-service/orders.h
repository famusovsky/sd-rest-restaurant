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
};