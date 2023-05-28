cmake_minimum_required(VERSION 3.26)

project(rest-restaurant)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow REQUIRED)
find_package(cpr REQUIRED)
find_package(SQLite3 REQUIRED)

add_subdirectory(src)