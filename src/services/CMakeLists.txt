add_library(services STATIC
    authentification-service/auth.h
    authentification-service/auth.cpp
    authentification-service/db.cpp
    authentification-service/db.h

    orders-processing-service/orders.h
    orders-processing-service/orders.cpp
)

target_link_libraries(services PUBLIC
    Crow::Crow
    cpr::cpr
    SQLite3
)