add_subdirectory(services)

add_executable( main
    main.cpp
)

target_link_libraries(main PUBLIC
    Crow::Crow
    # cpr::cpr
    services
)