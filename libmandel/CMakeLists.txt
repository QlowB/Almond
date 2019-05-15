cmake_minimum_required(VERSION 3.9)


project(mandel VERSION 1.0.0 DESCRIPTION "library for mandelbrot calculations")

find_package(OpenCL REQUIRED)

FILE(GLOB MandelSources src/*.cpp
FILE(GLOB MandelHeaders include/*.h))


include_directories(
    "include"
)

if (MSVC)
    set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS /arch:AVX)
else()
    set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS -mavx)
endif(MSVC)

add_library(mandel STATIC ${MandelSources})


target_link_libraries(mandel OpenCL::OpenCL)
