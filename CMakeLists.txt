cmake_minimum_required(VERSION 3.16)
project("my_cp")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp file/FileUtils.cpp)