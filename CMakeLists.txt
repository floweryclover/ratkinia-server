cmake_minimum_required(VERSION 3.26)
project(RatkiniaServer)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ws2_32 mswsock)