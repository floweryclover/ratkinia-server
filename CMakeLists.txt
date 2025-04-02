cmake_minimum_required(VERSION 3.28)
project(RatkiniaServer)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp
        NetworkServer.cpp
        NetworkServer.h
        MainServer.cpp
        MainServer.h
)

target_link_libraries(${PROJECT_NAME} ws2_32)