cmake_minimum_required(VERSION 3.0.0)
project(ntask)

add_executable(ntask main.cpp)
target_compile_options(ntask PRIVATE -Wall -Wextra -Werror)