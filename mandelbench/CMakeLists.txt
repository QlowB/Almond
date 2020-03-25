cmake_minimum_required(VERSION 3.9)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.53)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libmandel ${CMAKE_CURRENT_BINARY_DIR}/libmandel)
set(CMAKE_CXX_STANDARD 17)

add_executable(mandelbench mandelbench.cpp)

target_include_directories(mandelbench PUBLIC ../libmandel/include)

target_link_libraries(mandelbench mandel)

if(Boost_FOUND)
    target_compile_definitions(mandelbench PUBLIC WITH_BOOST)
    target_include_directories(mandelbench PRIVATE ${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)

