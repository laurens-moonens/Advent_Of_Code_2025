cmake_minimum_required(VERSION 3.24)

project(cafetaria)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(p1 src/part_1.c)
add_executable(p2 src/part_2.c)

target_compile_options(p1 PRIVATE -Werror -Wall -Wextra)
target_compile_options(p2 PRIVATE -Werror -Wall -Wextra)
