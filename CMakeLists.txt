cmake_minimum_required(VERSION 3.26)
project(RatkiniaServer)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

file(GLOB PROTOBUF_LIBS "${CMAKE_SOURCE_DIR}/Thirdparty/Protobuf/lib/*lib")
file(GLOB OPENSSL_LIBS "${CMAKE_SOURCE_DIR}/Thirdparty/OpenSSL/lib/*lib")

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 mswsock ${PROTOBUF_LIBS} ${OPENSSL_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/Thirdparty/Protobuf/include"
        "${CMAKE_SOURCE_DIR}/Thirdparty/OpenSSL/include")