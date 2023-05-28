#include <crow/json.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include "orders-db.h"

OrdersDB::OrdersDB() = default;

void OrdersDB::init(const std::string& db_name) {
    db_name_ = db_name;

    int exit = 0;
    exit = sqlite3_open(db_name_.c_str(), &db_);

    if (exit) {
        std::string message = "Error open DB " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    try {
        std::string sql =
            "CREATE TABLE IF NOT EXISTS dish ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name VARCHAR(100) NOT NULL,"
            // "description TEXT,"
            // "price DECIMAL(10, 2) NOT NULL,"
            "quantity INT NOT NULL,"
            // "is_available INTEGER NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ");"
            "CREATE TRIGGER IF NOT EXISTS update_dish_timestamp "
            "AFTER UPDATE ON dish "
            "FOR EACH ROW "
            "BEGIN "
            "UPDATE dish SET updated_at = CURRENT_TIMESTAMP WHERE id = old.id; "
            "END;";


        int exit = runSQL(sql, db_, {});

        if (exit != SQLITE_DONE) {
            std::string message =
                "Error creating table: " + std::string(sqlite3_errmsg(db_)) + "\n";
            throw std::runtime_error(message);
        }

        sql =
            "CREATE TABLE IF NOT EXISTS orders ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INT NOT NULL,"
            "status VARCHAR(50) NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "FOREIGN KEY (user_id) REFERENCES users(id)"
            ");"
            "CREATE TRIGGER IF NOT EXISTS update_orders_timestamp "
            "AFTER UPDATE ON orders "
            "FOR EACH ROW "
            "BEGIN "
            "UPDATE orders SET updated_at = CURRENT_TIMESTAMP WHERE id = old.id; "
            "END;";


        exit = runSQL(sql, db_, {});

        if (exit != SQLITE_DONE) {
            std::string message =
                "Error creating table: " + std::string(sqlite3_errmsg(db_)) + "\n";
            throw std::runtime_error(message);
        }

        sql =
            "CREATE TABLE IF NOT EXISTS orders_dish ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "orders_id INT NOT NULL,"
            "dish_id INT NOT NULL,"
            "quantity INT NOT NULL,"
            // "price DECIMAL(10, 2) NOT NULL,"
            "FOREIGN KEY (orders_id) REFERENCES orders(id),"
            "FOREIGN KEY (dish_id) REFERENCES dish(id)"
            ");";

        exit = runSQL(sql, db_, {});

        if (exit != SQLITE_DONE) {
            std::string message =
                "Error creating table: " + std::string(sqlite3_errmsg(db_)) + "\n";
            throw std::runtime_error(message);
        }
    } catch (std::runtime_error& e) {
        sqlite3_close(db_);
        throw e;
    }
}

void OrdersDB::createOrder(const std::string& user_id, const std::string& dish_name, const std::string& quantity) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "INSERT INTO orders (user_id, status) VALUES (?, ?);";

    int exit = runSQL(sql, db_, {user_id, "new"});

    if (exit != SQLITE_DONE) {
        std::string message =
            "Error inserting data: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    // rows count in orders table
    int order_id = ++orders_count_;

    sql = "SELECT id FROM dish WHERE name = ?;";
    
    sqlite3_stmt* stmt;

    exit = sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt, nullptr);

    if (exit != SQLITE_OK) {
        std::string message =
            "Error preparing statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_bind_text(stmt, 1, dish_name.c_str(), dish_name.length(), SQLITE_STATIC);

    if (exit != SQLITE_OK) {
        std::string message =
            "Error binding statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_step(stmt);

    if (exit != SQLITE_ROW) {
        std::string message =
            "Error executing statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    std::string dish_id = std::to_string(sqlite3_column_int(stmt, 0));

    sql = "INSERT INTO orders_dish (orders_id, dish_id, quantity) VALUES (?, ?, ?);";

    exit = runSQL(sql, db_, {std::to_string(order_id), dish_id, quantity});

    if (exit != SQLITE_DONE) {
        std::string message =
            "Error inserting data: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

crow::json::wvalue OrdersDB::getOrder(const std::string& order_id) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "SELECT * FROM orders WHERE id = ?;";

    int exit = runSQL(sql, db_, {order_id});

    if (exit != SQLITE_DONE) {
        std::string message =
            "Error inserting data: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    crow::json::wvalue order;

    sql = "SELECT * FROM orders_dish WHERE orders_id = ?;";

    sqlite3_stmt* stmt;

    exit = sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt, nullptr);

    if (exit != SQLITE_OK) {
        std::string message =
            "Error preparing statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_bind_text(stmt, 1, order_id.c_str(), order_id.length(), SQLITE_STATIC);

    if (exit != SQLITE_OK) {
        std::string message =
            "Error binding statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_step(stmt);

    if (exit != SQLITE_ROW) {
        std::string message =
            "Error stepping statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    std::string dishes;

    do {
        crow::json::wvalue dish;
        dish["id"] = sqlite3_column_int(stmt, 0);
        dish["orders_id"] = sqlite3_column_int(stmt, 1);
        dish["dish_id"] = sqlite3_column_int(stmt, 2);
        dish["quantity"] = sqlite3_column_int(stmt, 3);
        dishes += dish.dump() + ";";
    } while (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    order["dishes"] = dishes;

    return order;
}

crow::json::wvalue OrdersDB::getAllOrders() {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "SELECT orders.id, orders.user_id, orders.status FROM orders;";
    sqlite3_stmt* stmt;

    int exit = sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt, nullptr);

    if (exit != SQLITE_OK) {
        std::string message =
            "Error preparing statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_step(stmt);

    if (exit != SQLITE_ROW) {
        std::string message =
            "Error stepping statement: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    std::string orders;

    do {
        crow::json::wvalue order;
        order["id"] = sqlite3_column_int(stmt, 0);
        order["user_id"] = sqlite3_column_int(stmt, 1);
        order["status"] = sqlite3_column_int(stmt, 2);
        orders += order.dump() + ";";
    } while (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    crow::json::wvalue result;
    result["orders"] = orders;

    return result;
}

void OrdersDB::changeOrderStatus(const std::string& order_id, const std::string& status) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "UPDATE orders SET status = ? WHERE id = ?;";

    int exit = runSQL(sql, db_, {status, order_id});

    if (exit != SQLITE_DONE) {
        std::string message =
            "Error inserting data: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

void OrdersDB::changeDishQuantity(const std::string& dish_id, const std::string& quantity) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "UPDATE dish SET quantity = ? WHERE id = ?;";

    int exit = runSQL(sql, db_, {quantity, dish_id});

    if (exit != SQLITE_DONE) {
        std::string message =
            "Error inserting data: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

void OrdersDB::addNewDish(const std::string& dish_name, const std::string& quantity) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "INSERT OR REPLACE INTO dish (name, quantity) VALUES (?, ?);";

    int exit = runSQL(sql, db_, {dish_name, quantity});

    if (exit != SQLITE_DONE) {
        std::string message =
            "Error inserting data: " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}