cmake_minimum_required(VERSION 3.26)
project(RatkiniaServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty")

find_package(protobuf CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cc")

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME} ${SOURCE})

add_link_options("/IGNORE:4099")

target_include_directories(${PROJECT_NAME}
        PRIVATE
        "Source"
        "Source/System"
        "Source/GlobalObject"
        "Source/RatkiniaProtocol"
        "Source/Event")

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