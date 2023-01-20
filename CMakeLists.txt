cmake_minimum_required(VERSION 3.24)
project(CLiTBot)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(CLiTBot
        CLiTBot.h
        interface.cpp
        interface.h
        main.cpp)
