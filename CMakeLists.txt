cmake_minimum_required(VERSION 3.0.0)
project(ntask)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include include/ntask)

add_executable(ntask src/main.cpp src/dangerous_bend.cpp)
set_property(TARGET ntask PROPERTY CXX_STANDARD 20)
target_compile_options(ntask PRIVATE -Wall -Wextra -Werror)

target_link_libraries(ntask expat z)

configure_file(${CMAKE_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR} COPYONLY)