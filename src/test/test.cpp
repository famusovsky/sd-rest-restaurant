#include "test.h"

Test::Test() {
    strings_.push_back("Hello");
    strings_.push_back("World");
}

void Test::init(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/test")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        crow::json::rvalue body = crow::json::load(req.body);
        return post(body);
    });

    CROW_ROUTE(app, "/test")
    .methods("GET"_method)
    ([this]() {
        return get();
    });
}

crow::response Test::post(crow::json::rvalue body) {
    strings_.push_back(body["str"].s());
    return crow::response(200);
}

crow::response Test::get() {
    std::ostringstream os;
    for (auto& str : strings_) {
        os << str << std::endl;
    }
    return crow::response(os.str());
}