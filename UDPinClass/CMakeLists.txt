cmake_minimum_required(VERSION 3.15)
project(UDPinClass)

set(CMAKE_CXX_STANDARD 14)

add_executable(UDPinClass main.cpp UDPClient.cpp UDPClient.h)