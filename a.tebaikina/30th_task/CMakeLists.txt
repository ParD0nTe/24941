cmake_minimum_required(VERSION 4.0)
project(30th_task C)

set(CMAKE_C_STANDARD 11)

add_executable(30th_task server.c
        client.c)
