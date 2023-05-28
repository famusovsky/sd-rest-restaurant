#include <sqlite3.h>
#include <crow.h>
#include <string>
#include "../db.h"

class OrdersDB : DB {
public:
    OrdersDB();

    void init(const std::string& db_name) override;
};