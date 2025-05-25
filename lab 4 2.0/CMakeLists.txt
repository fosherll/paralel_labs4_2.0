cmake_minimum_required(VERSION 3.30)
project(lab_4_2_0)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab_4_2_0 Task.h
        ThreadPool.h
        Matrix.h
        main.cpp
        Matrix.cpp
        Task.cpp
        ThreadPool.cpp
)
target_link_libraries(lab_4_2_0 ws2_32)