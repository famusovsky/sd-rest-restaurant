#include <crow/http_parser_merged.h>
#include <crow/http_response.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "orders.h"

OrdersService::OrdersService() = default;

void OrdersService::init(crow::SimpleApp& app, const std::string& path) {
    try {
        db_.init("orders.db");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw e;
    }

    app.route_dynamic((path + "/create").c_str())
    .methods("POST"_method)
    ([this](const crow::request& req) {
        crow::json::rvalue body = crow::json::load(req.body);
        return createOrder(body);
    });

    app.route_dynamic(path.c_str())
    .methods("GET"_method)
    ([this](const crow::request& req) {
        crow::json::rvalue body = crow::json::load(req.body);
        return getOrder(body);
    });

    app.route_dynamic((path + "/manage-order").c_str())
    .methods("POST"_method)
    ([this](const crow::request& req) {
        crow::json::rvalue body = crow::json::load(req.body);
        return manageOrder(body);
    });

    app.route_dynamic((path + "/manage-dishes").c_str())
    .methods("POST"_method)
    ([this](const crow::request& req) {
        crow::json::rvalue body = crow::json::load(req.body);
        return manageDish(body);
    });

    // FIXME
    // std::thread t([this]() {
    //     while (true) {
    //         std::this_thread::sleep_for(std::chrono::seconds(2));
    //         processOrders();
    //     }
    // });

    // t.detach();
}


// FIXME
void OrdersService::processOrders() {
    std::string orders_raw_string;
    try {
        orders_raw_string = db_.getAllOrders().dump();
    } catch (const std::runtime_error& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << orders_raw_string << std::endl;

    std::vector<std::string> orders;
    std::string delimiter = "},";
    int pos = 0;
    if ((pos = orders_raw_string.find(":\"")) != std::string::npos) {
        orders_raw_string.erase(0, pos + 2);
    }
    while ((pos = orders_raw_string.find(delimiter)) != std::string::npos) {
        orders.push_back(orders_raw_string.substr(0, pos + 1));
        orders_raw_string.erase(0, pos + delimiter.length());
        std::cout << orders.back() << std::endl;
    }

    for (auto& order_string : orders) {
        crow::json::rvalue order;
        
        try {
            std::cout << order << std::endl;
            std::cout << order_string << std::endl;
            order = crow::json::load(order_string);
            std::cout << order << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        if (order["status"].s() == "processing") {
            std::string dish_name = order["dish_name"].s();
            int quantity = order["quantity"].i();

            cpr::Response r = cpr::Get(cpr::Url{basic_url_ + "/dishes"},
                                       cpr::Header{{"Content-Type", "application/json"}},
                                       cpr::Body{"{\"dish_name\": \"" + dish_name + "\"}"});

            if (r.status_code != 200) {
                std::cerr << "Error: " << r.text << std::endl;
                continue;
            }

            crow::json::rvalue dish = crow::json::load(r.text);

            if (dish["quantity"].i() < quantity) {
                std::cerr << "Error: not enough dishes" << std::endl;
                continue;
            }

            try {
                db_.changeDishQuantity(dish_name, std::to_string(dish["quantity"].i() - quantity));
                db_.changeOrderStatus(order["order_id"].s(), "processed");
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                continue;
            }
        } else if (order["status"].s() == "new") {
            try {
                db_.changeOrderStatus(order["order_id"].s(), "processing");
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                continue;
            }
        }
    }
}

crow::response OrdersService::createOrder(const crow::json::rvalue& body) {
    std::string session_token = body["session_token"].s();
    std::string dish_name = body["dish_name"].s();
    std::string quantity = body["quantity"].s();

    if (session_token.empty() || dish_name.empty() || quantity.empty()) {
        return crow::response(400, "Missing required fields");
    }

    cpr::Response r = cpr::Get(cpr::Url{basic_url_ + "/auth"},
                                 cpr::Header{{"Content-Type", "application/json"}},
                                 cpr::Body{"{\"session_token\": \"" + session_token + "\"}"});

    if (r.status_code != 200) {
        return crow::response(401, "Invalid session token");
    }

    crow::json::rvalue auth_body = crow::json::load(r.text);

    if (auth_body["role"].s() != "customer") {
        return crow::response(403, "You are not allowed to create orders");
    }

    std::string user_id = auth_body["id"].s();

    try {
        std::cout << "Creating order" << std::endl;
        db_.createOrder(user_id, dish_name, quantity);
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }

    return crow::response(200, "Order created successfully");
}

crow::response OrdersService::getOrder(const crow::json::rvalue& body) {
    std::string session_token = body["session_token"].s();
    std::string order_id = body["order_id"].s();

    if (session_token.empty() || order_id.empty()) {
        return crow::response(400, "Missing required fields");
    }

    cpr::Response r = cpr::Get(cpr::Url{basic_url_ + "/auth"},
                                 cpr::Header{{"Content-Type", "application/json"}},
                                 cpr::Body{"{\"session_token\": \"" + session_token + "\"}"});

    if (r.status_code != 200) {
        return crow::response(401, "Invalid session token");
    }

    crow::json::rvalue auth_body = crow::json::load(r.text);

    if (auth_body["role"].s() != "manager" && auth_body["role"].s() != "chef") {
        return crow::response(403, "You are not allowed to get orders info");
    }

    crow::json::wvalue response_body;

    try {
        response_body = db_.getOrder(order_id);
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }

    return crow::response(200, response_body);
}

crow::response OrdersService::manageOrder(const crow::json::rvalue& body) {
    std::string session_token = body["session_token"].s();
    std::string order_id = body["order_id"].s();
    std::string status = body["status"].s();

    if (session_token.empty() || order_id.empty() || status.empty()) {
        return crow::response(400, "Missing required fields");
    }

    cpr::Response r = cpr::Get(cpr::Url{basic_url_ + "/auth"},
                                 cpr::Header{{"Content-Type", "application/json"}},
                                 cpr::Body{"{\"session_token\": \"" + session_token + "\"}"});

    if (r.status_code != 200) {
        return crow::response(401, "Invalid session token");
    }

    crow::json::rvalue auth_body = crow::json::load(r.text);

    if (auth_body["role"].s() != "manager") {
        return crow::response(403, "You are not allowed to manage orders");
    }

    try {
        db_.changeOrderStatus(order_id, status);
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }

    return crow::response(200, "Order status changed successfully");
}

crow::response OrdersService::manageDish(const crow::json::rvalue& body) {
    std::string session_token = body["session_token"].s();
    std::string dish_id = body["dish_name"].s();
    std::string dish_name = body["dish_name"].s();
    std::string quantity = body["quantity"].s();

    if (session_token.empty() || (dish_name.empty() && dish_id.empty()) || quantity.empty()) {
        return crow::response(400, "Missing required fields");
    }

    cpr::Response r = cpr::Get(cpr::Url{basic_url_ + "/auth"},
                                 cpr::Header{{"Content-Type", "application/json"}},
                                 cpr::Body{"{\"session_token\": \"" + session_token + "\"}"});

    if (r.status_code != 200) {
        return crow::response(401, "Invalid session token");
    }

    crow::json::rvalue auth_body = crow::json::load(r.text);

    if (auth_body["role"].s() != "manager") {
        return crow::response(403, "You are not allowed to manage dishes");
    }

    try {
        if (!dish_name.empty()) {
            db_.addNewDish(dish_name, quantity);
        }
        if (!dish_id.empty()) {
            db_.changeDishQuantity(dish_id, quantity);
        }
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }

    return crow::response(200, "Dish updated successfully");
}