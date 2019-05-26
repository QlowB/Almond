cmake_minimum_required(VERSION 3.9)



set(ARCH "X86_64" CACHE STRING "Target Architecture")

message(CMAKE_SYSTEM_PROCESSOR)

project(mandel VERSION 1.0.0 DESCRIPTION "library for mandelbrot calculations")

find_package(OpenCL)
find_package(OpenMP)

set(CMAKE_CXX_STANDARD 17)

#FILE(GLOB MandelSources src/*.cpp)
SET(MandelSources
    src/ClGenerators.cpp
    src/CpuGenerators.cpp
    src/Generators.cpp
    src/mandel.cpp
    src/Hardware.cpp
    src/MandelUtil.cpp
)
FILE(GLOB MandelHeaders include/*.h)

if (ARCH STREQUAL "X86_64" OR ARCH STREQUAL "X86")
    list(APPEND MandelSources src/CpuGeneratorsAVX.cpp src/CpuGeneratorsSSE2.cpp)
elseif(ARCH STREQUAL "ARM")
    list(APPEND MandelSources src/CpuGeneratorsNeon.cpp)
endif()

#    message(${MandelSources})

if(OPENCL_FOUND)
    target_compile_definitions(mandel WITH_OPENCL)
    include_directories(
        "include"
        ${OpenCL_INCLUDE_DIRS}
    )
    link_directories(${OpenCL_LIBRARY})
else(OPENCL_FOUND)
    include_directories("include")
endif(OPENCL_FOUND)

if (APPLE AND OpenCL_FOUND)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -framework OpenCL")
endif()

if (ARCH STREQUAL "X86_64" OR ARCH STREQUAL "X86")
    if (MSVC)
        set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS /arch:AVX)
    else()
        set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS -mavx)
    endif(MSVC)


    if (MSVC)
        set_source_files_properties(src/CpuGeneratorsSSE2.cpp PROPERTIES COMPILE_FLAGS /arch:SSE2)
    else()
        set_source_files_properties(src/CpuGeneratorsSSE2.cpp PROPERTIES COMPILE_FLAGS -msse2)
    endif(MSVC)
elseif(ARCH STREQUAL "ARM")
    #set_source_files_properties(src/CpuGeneratorsNeon.cpp PROPERTIES COMPILE_FLAGS -mfpu=neon)
endif()

add_library(mandel STATIC ${MandelSources})


if(OpenMP_CXX_FOUND)
    target_link_libraries(mandel PUBLIC OpenMP::OpenMP_CXX)
endif()
if(OpenCL_FOUND)
    target_link_libraries(mandel PUBLIC OpenCL::OpenCL)
endif()
