cmake_minimum_required(VERSION 3.26)
project(RatkiniaServer)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cc")

add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty")

find_package(protobuf CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

add_link_options("/IGNORE:4099")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ws2_32
        mswsock
        protobuf::libprotobuf-lite
        OpenSSL::SSL
        OpenSSL::Crypto
        libpqxx::pqxx)
