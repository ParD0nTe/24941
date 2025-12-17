cmake_minimum_required(VERSION 3.28)
project(32nd_task C)

set(CMAKE_C_STANDARD 11)

add_executable(32nd_task server.c
        client_1.c
        client_2.c
        client_3.cpp)
