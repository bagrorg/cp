cmake_minimum_required(VERSION 3.16)
project("my_cp")
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp file/FileUtils.cpp cp_utils/cp_utils.cpp cp_utils/content_copy.cpp cp_utils/path_processing.cpp wrappers/posix_helpers.cpp)