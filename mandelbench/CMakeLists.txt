cmake_minimum_required(VERSION 3.9)

project(Mandelbench VERSION 1.0.0 DESCRIPTION "benchmarks for libmandel")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libmandel ${CMAKE_CURRENT_BINARY_DIR}/libmandel)
set(CMAKE_CXX_STANDARD 17)

add_executable(mandelbench mandelbench.cpp)

target_link_libraries(mandelbench mandel)

