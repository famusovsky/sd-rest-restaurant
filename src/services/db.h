#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <crow.h>
#include <string>

class DB {
public:
    DB() {
        db_ = nullptr;
    }

    ~DB() {
        if (db_ != nullptr) {
            sqlite3_close(db_);
            std::cout << "Database connection closed\n";
        }
    }

    virtual void init(const std::string& db_name) = 0;

protected:
    sqlite3* db_;
    std::string db_name_;

    int runSQL(const std::string& sql, sqlite3* db, const std::vector<std::string>& args) {
        sqlite3_stmt* stmt;
        int exit = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);

        if (exit != SQLITE_OK) {
            std::string message = "Error creating table: " + std::string(sqlite3_errmsg(db)) + "\n";
            sqlite3_finalize(stmt);
            throw std::runtime_error(message);
        }

        for (int i = 0; i < args.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, args[i].c_str(), args[i].length(), SQLITE_STATIC);

            if (exit != SQLITE_OK) {
                std::string message = "Error Bind Text " + std::string(sqlite3_errmsg(db)) + "\n";
                throw std::runtime_error(message);
            }
        }

        exit = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return exit;
    }
};

#endif // DB_H
