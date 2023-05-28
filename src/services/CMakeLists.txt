add_library(services STATIC
    db.h

    authentification-service/auth.h
    authentification-service/auth.cpp
    authentification-service/auth-db.cpp
    authentification-service/auth-db.h

    orders-processing-service/orders.h
    orders-processing-service/orders.cpp
    orders-processing-service/orders-db.h
    orders-processing-service/orders-db.cpp
)

target_link_libraries(services PUBLIC
    Crow::Crow
    cpr::cpr
    SQLite3
)