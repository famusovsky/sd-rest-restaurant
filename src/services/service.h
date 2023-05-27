#include <cpr/cpr.h>
#include <crow.h>
#include <crow/utility.h>
#include <sqlite3.h>
#include <string>
#include <vector>

class Service {
public:
    Service() = default;

    inline void init(crow::SimpleApp& app, const std::string& path) {
        app.route_dynamic(path.c_str())
        .methods("POST"_method)
        ([this](const crow::request& req) {
            crow::json::rvalue body = crow::json::load(req.body);
            return post(body);
        });

        app.route_dynamic(path.c_str())
        .methods("GET"_method)
        ([this]() {
            return get(); }
        );
    }

protected:
    virtual crow::response post(crow::json::rvalue body) = 0;
    virtual crow::response get() = 0;
};