cmake_minimum_required(VERSION 3.14)

project(lobby)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(lobby src/main.c)
