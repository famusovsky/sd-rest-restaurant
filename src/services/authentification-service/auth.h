#include <cpr/cpr.h>
#include <crow.h>
#include <string>
#include <vector>
#include "auth-db.h"

class AuthentificationService {
public:
    AuthentificationService();

    void init(crow::SimpleApp& app, const std::string& path);

private:
    AuthDB db_;

    crow::response signIn(const crow::json::rvalue& body);
    crow::response signUp(const crow::json::rvalue& body);
    crow::response signOut(const crow::json::rvalue& body);
    crow::response getInfo(const crow::json::rvalue& body);
};