#include <stdexcept>
#include "auth.h"

AuthentificationService::AuthentificationService() = default;

void AuthentificationService::init(crow::SimpleApp& app, const std::string& path) {
    try {
        db_.init("auth.db");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw e;
    }

    app.route_dynamic((path + "/sign-in").c_str())
        .methods("POST"_method)
        ([this](const crow::request& req) {
            crow::json::rvalue body = crow::json::load(req.body);
            return signIn(body);
        });

    app.route_dynamic((path + "/sign-up").c_str())
        .methods("POST"_method)
        ([this](const crow::request& req) {
            crow::json::rvalue body = crow::json::load(req.body);
            return signUp(body);
        });

    app.route_dynamic((path + "/sign-out").c_str())
        .methods("POST"_method)
        ([this](const crow::request& req) {
            crow::json::rvalue body = crow::json::load(req.body);
            return signOut(body);
        });

    app.route_dynamic(path.c_str())
        .methods("GET"_method)
        ([this](const crow::request& req) { 
            crow::json::rvalue body = crow::json::load(req.body);
            return getInfo(body);
        });
}

crow::response AuthentificationService::signIn(const crow::json::rvalue& body) {
    // Parse the request body
    std::string username = body["username"].s();
    std::string password = body["password"].s();

    // Check if all required fields are present
    if (username.empty() || password.empty()) {
        return crow::response(400, "Missing required fields");
    }

    std::cout << "Username: " << username << std::endl;

    try {
        if (!db_.isUserLoginAndPasswordCorrect(username, password)) {
            return crow::response(400, "Invalid username or password");
        } else {
            db_.createSessionToken(body["session_token"].s(), username);
            return crow::response(200, "User logged in successfully");
        }
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }
}

crow::response AuthentificationService::signUp(const crow::json::rvalue& body) {
    // Parse the request body
    std::string username = body["username"].s();
    std::string email = body["email"].s();
    std::string password = body["password"].s();
    std::string role = body["role"].s();

    // Check if all required fields are present
    if (username.empty() || email.empty() || password.empty()) {
        return crow::response(400, "Missing required fields");
    }

    // Check if email is valid
    if (email.find('@') == std::string::npos) {
        return crow::response(400, "Invalid email address");
    }

    try {
        if (!db_.isUserLoginAndEmailUnique(username, email)) {
            return crow::response(400, "Username or email already taken");
        }

        std::cout << "Login and email are unique" << std::endl;
        std::cout << "Creating user " << username << ' ' << email << ' ' << password << ' ' << role << std::endl;

        db_.createUser(username, email, password, role);
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }

    // Return a success response
    crow::json::wvalue response_body;
    response_body["message"] = "User registered successfully";
    return crow::response(201, response_body);
}

crow::response AuthentificationService::signOut(const crow::json::rvalue& body) {
    std::string session_token = body["session_token"].s();

    try {
        db_.deleteSessionToken(session_token);
        crow::json::wvalue response_body;
        response_body["message"] = "User logged out successfully";
        return crow::response(200, response_body);
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }
}

crow::response AuthentificationService::getInfo(const crow::json::rvalue& body) {
    std::string session_token = body["session_token"].s();

    try {
        crow::json::wvalue info = db_.getUserInfo(session_token);
        return crow::response(200, info);
    } catch (const std::runtime_error& e) {
        return crow::response(500, e.what());
    }
}