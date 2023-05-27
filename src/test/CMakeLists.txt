add_library(test STATIC
    test.cpp
    test.h
)

target_link_libraries(test PRIVATE
    Crow::Crow
)