cmake_minimum_required(VERSION 3.9)


project(mandel VERSION 1.0.0 DESCRIPTION "library for mandelbrot calculations")

find_package(OpenCL REQUIRED)
find_package(OpenMP)


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


if (MSVC)
    set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS /arch:SSE2)
else()
    set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS -msse2)
endif(MSVC)

add_library(mandel STATIC ${MandelSources})


if(OpenMP_CXX_FOUND)
    target_link_libraries(mandel PUBLIC OpenCL::OpenCL OpenMP::OpenMP_CXX)
else()
    target_link_libraries(mandel OpenCL::OpenCL)
endif()