cmake_minimum_required(VERSION 3.10)
project(MyProject)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -g -fsanitize=address")
include_directories(${CMAKE_SOURCE_DIR}/bcrypt)
link_directories(${CMAKE_SOURCE_DIR}/bcrypt)
set(SOURCES
    main.cpp
    Hash.cpp
)
add_executable(main ${SOURCES})
target_link_libraries(main bcrypt ssl crypto)
set_target_properties(main PROPERTIES LINK_FLAGS "-fsanitize=address")
