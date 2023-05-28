#include <crow/json.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include "auth-db.h"

AuthDB::AuthDB() = default;

void AuthDB::init(const std::string& db_name) {
    db_name_ = db_name;

    int exit = 0;
    exit = sqlite3_open(db_name_.c_str(), &db_);

    if (exit) {
        std::string message = "Error open DB " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    try {
        std::string sql =
            "CREATE TABLE IF NOT EXISTS user ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username VARCHAR(50) UNIQUE NOT NULL,"
            "email VARCHAR(100) UNIQUE NOT NULL,"
            "password_hash VARCHAR(255) NOT NULL,"
            "role VARCHAR(10) NOT NULL CHECK (role IN ('customer', 'chef', 'manager')),"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ");"
            "CREATE TRIGGER IF NOT EXISTS update_user_timestamp "
            "AFTER UPDATE ON user "
            "FOR EACH ROW "
            "BEGIN "
            "UPDATE user SET updated_at = CURRENT_TIMESTAMP WHERE id = old.id; "
            "END;";

        int exit = runSQL(sql, db_, {});

        if (exit != SQLITE_DONE) {
            std::string message =
                "Error creating table: " + std::string(sqlite3_errmsg(db_)) + "\n";
            throw std::runtime_error(message);
        }

        sql =
            "CREATE TABLE IF NOT EXISTS session ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INT NOT NULL,"
            "session_token VARCHAR(255) NOT NULL,"
            "expires_at TIMESTAMP NOT NULL DEFAULT (datetime('now', '+1 day')),"
            "FOREIGN KEY (user_id) REFERENCES user(id)"
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

bool AuthDB::isUserLoginAndEmailUnique(const std::string& username, const std::string& email) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "SELECT * FROM user WHERE username = ? OR email = ?;";

    int exit = runSQL(sql, db_, {username, email});

    if (exit == SQLITE_ROW) {
        return false;
    } else if (exit == SQLITE_DONE) {
        return true;
    } else {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

bool AuthDB::isUserLoginAndPasswordCorrect(const std::string& username,
                                           const std::string& password) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "SELECT * FROM user WHERE username = ? AND password_hash = ?;";

    int exit = runSQL(sql, db_, {username, password});

    if (exit == SQLITE_ROW) {
        return true;
    } else if (exit == SQLITE_DONE) {
        return false;
    } else {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

bool AuthDB::isUserSessionTokenValid(const std::string& session_token) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "SELECT * FROM session WHERE session_token = ? AND session.expires_at > datetime('now');";

    int exit = runSQL(sql, db_, {session_token});

    if (exit == SQLITE_ROW) {
        return true;
    } else if (exit == SQLITE_DONE) {
        return false;
    } else {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

void AuthDB::createUser(const std::string& username, const std::string& email,
                        const std::string& password_hash, const std::string& role) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql =
        "INSERT INTO user (username, email, password_hash, role) VALUES (?, ?, ?, ?);";

    int exit = runSQL(sql, db_, {username, email, password_hash, role});

    if (exit != SQLITE_DONE) {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

void AuthDB::createSessionToken(const std::string& session_token, const std::string& username) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql =
        "INSERT INTO session (session_token, user_id) VALUES (?, "
        "(SELECT id FROM user WHERE username = ?));";

    int exit = runSQL(sql, db_, {session_token, username});

    if (exit != SQLITE_DONE) {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

void AuthDB::deleteSessionToken(const std::string& session_token) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    std::string sql = "DELETE FROM session WHERE session_token = ?;";

    int exit = runSQL(sql, db_, {session_token});

    if (exit != SQLITE_DONE) {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }
}

crow::json::wvalue AuthDB::getUserInfo(const std::string& session_token) {
    if (db_ == nullptr) {
        throw std::runtime_error("DB is not initialized");
    }

    sqlite3_stmt* stmt;
    std::string sql =
        "SELECT user.username, user.email, user.role "
        "FROM user "
        "INNER JOIN session ON user.id = session.user_id "
        "WHERE session.session_token = ? AND session.expires_at > datetime('now');";

    int exit = sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt, nullptr);

    if (exit != SQLITE_OK) {
        std::string message = "Error Prepare Statement " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_bind_text(stmt, 1, session_token.c_str(), session_token.length(), SQLITE_STATIC);

    if (exit != SQLITE_OK) {
        std::string message = "Error Bind Text " + std::string(sqlite3_errmsg(db_)) + "\n";
        throw std::runtime_error(message);
    }

    exit = sqlite3_step(stmt);

    if (exit == SQLITE_ROW) {
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        sqlite3_finalize(stmt);

        crow::json::wvalue result;
        result["username"] = username;
        result["email"] = email;
        result["role"] = role;

        return result;
    } else if (exit == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return "no user";
    } else {
        std::string message = "Error Step " + std::string(sqlite3_errmsg(db_)) + "\n";
        sqlite3_finalize(stmt);
        throw std::runtime_error(message);
    }
}
