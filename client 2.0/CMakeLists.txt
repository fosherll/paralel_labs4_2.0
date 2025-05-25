cmake_minimum_required(VERSION 3.30)
project(client_2_0)

set(CMAKE_CXX_STANDARD 20)

add_executable(client_2_0 main.cpp
Message.cpp
Message.h)

target_link_libraries(client_2_0 ws2_32)