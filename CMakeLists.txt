cmake_minimum_required(VERSION 2.8)

project(BerkeleyDBSamples)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/submodules/cmake/FindBerkeleyDB")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} "main.cpp")
