cmake_minimum_required(VERSION 3.26)
project(RatkiniaServer)

set(absl_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/Protobuf/lib/cmake/absl")
set(utf8_range_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/Protobuf/lib/cmake/utf8_range")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty")

find_package(utf8_range CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cc")

add_executable(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME}
        PRIVATE
        "Source"
        "Source/Actor"
        "Source/System"
        "Source/GlobalObject"
        "Source/Component"
        "Source/Event"
        "Source/Database"
        "Source/Util")

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ../ratkinia-protocol/Out/Server/Include)

target_include_directories(${PROJECT_NAME} PRIVATE Thirdparty/libbcrypt/include)

target_link_directories(${PROJECT_NAME} PRIVATE Thirdparty/libbcrypt/lib)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ws2_32
        mswsock
        protobuf::libprotobuf-lite
        OpenSSL::SSL
        OpenSSL::Crypto
        libpqxx::pqxx
        bcrypt)