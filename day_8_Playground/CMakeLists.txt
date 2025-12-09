cmake_minimum_required(VERSION 3.24)

project(playground)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(p1 src/part_1.c)
add_executable(p2 src/part_2.c)
target_link_libraries(p1 m)
target_link_libraries(p2 m)
