cmake_minimum_required(VERSION 3.0.0)
project(ntask)

add_executable(ntask main.cpp)
set_property(TARGET ntask PROPERTY CXX_STANDARD 20)
target_compile_options(ntask PRIVATE -Wall -Wextra -Werror)