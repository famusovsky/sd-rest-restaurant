#include <sqlite3.h>
#include <crow.h>
#include <string>
#include "../db.h"

class AuthDB : DB {
public:
    AuthDB();

    void init(const std::string& db_name) override;

    bool isUserLoginAndEmailUnique(const std::string& username, const std::string& email);

    bool isUserLoginAndPasswordCorrect(const std::string& username, const std::string& password);

    bool isUserSessionTokenValid(const std::string& session_token);

    void createUser(const std::string& username, const std::string& email, const std::string& password_hash, const std::string& role);

    void createSessionToken(const std::string& session_token, const std::string& username);

    void deleteSessionToken(const std::string& session_token);

    crow::json::wvalue getUserInfo(const std::string& username);
};