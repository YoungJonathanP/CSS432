cmake_minimum_required(VERSION 3.15)
project(Socket)

set(CMAKE_CXX_STANDARD 14)

add_executable(Socket server.cpp client.cpp)