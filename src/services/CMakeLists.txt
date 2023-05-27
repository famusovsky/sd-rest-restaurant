add_library(services STATIC
    service.h

    authentification-service/auth.h
    authentification-service/auth.cpp

    orders-processing-service/orders.h
    orders-processing-service/orders.cpp
)

target_link_libraries(services PUBLIC
    Crow::Crow
    cpr::cpr
)