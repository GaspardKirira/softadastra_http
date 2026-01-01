cmake_minimum_required(VERSION 3.10)

project(MonProjet CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -g -fsanitize=address")

find_package(Boost 1.78 REQUIRED COMPONENTS filesystem system)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/http
    ${CMAKE_SOURCE_DIR}/src/core/routing
    ${CMAKE_SOURCE_DIR}/src/core/session
    ${CMAKE_SOURCE_DIR}/src/core/threading
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/controllers
    ${CMAKE_SOURCE_DIR}/src/utils
)

include_directories(/usr/include/mysql)
link_directories(/usr/lib)
include_directories(/usr/local/include/spdlog)
link_directories(/usr/local/lib)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/config/*.cpp src/core/**/*.cpp)
add_executable(prog ${SOURCE_FILES})
target_link_libraries(prog PRIVATE mysqlcppconn)
target_link_libraries(prog PRIVATE ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(prog PRIVATE spdlog)
target_link_libraries(prog PRIVATE OpenSSL::SSL OpenSSL::Crypto)
