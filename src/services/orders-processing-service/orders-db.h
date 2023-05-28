#include <sqlite3.h>
#include <crow.h>
#include <string>
#include "../db.h"

class OrdersDB : DB {
public:
    OrdersDB();

    void init(const std::string& db_name) override;

    void createOrder(const std::string& user_id, const std::string& dish_name, const std::string& quantity);
    void addNewDish(const std::string& dish_name, const std::string& price);
    void changeDishQuantity(const std::string& dish_name, const std::string& quantity);
    void changeOrderStatus(const std::string& order_id, const std::string& status);
    crow::json::wvalue getOrder(const std::string& order_id);
    crow::json::wvalue getAllOrders();
private:
    size_t orders_count_ = 0;
};