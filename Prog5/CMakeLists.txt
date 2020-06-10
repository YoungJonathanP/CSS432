cmake_minimum_required(VERSION 3.15)
project(Prog5)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Prog5
        FTPClient.cpp
        FTPClient.h
        main.cpp
        Socket.cpp
        Socket.h)
