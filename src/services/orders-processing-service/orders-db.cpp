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
        // TODO: create table
        std::string sql =
            "CREATE TABLE IF NOT EXISTS dish ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name VARCHAR(100) NOT NULL,"
            "description TEXT,"
            "price DECIMAL(10, 2) NOT NULL,"
            "quantity INT NOT NULL,"
            "is_available INTEGER NOT NULL,"
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

        // TODO: create table
        sql =
            "CREATE TABLE IF NOT EXISTS orders ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INT NOT NULL,"
            "status VARCHAR(50) NOT NULL,"
            "special_requests TEXT,"
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
            "price DECIMAL(10, 2) NOT NULL,"
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
