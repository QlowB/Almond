cmake_minimum_required(VERSION 3.9)

set(ARCH "X86_64" CACHE STRING "Target Architecture")

#message(CMAKE_SYSTEM_PROCESSOR)

project(mandel VERSION 1.0.0 DESCRIPTION "library for mandelbrot calculations")

find_package(OpenCL)
find_package(OpenMP)
#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.53)

find_path(MPFR_INCLUDES
  NAMES
  mpfr.h
  PATHS
  $ENV{MPFR_DIR}
  ${INCLUDE_INSTALL_DIR}
)
find_library(MPFR_LIBRARIES mpfr PATHS $ENV{MPFR_DIR} ${LIB_INSTALL_DIR})


set(CMAKE_CXX_STANDARD 17)


#FILE(GLOB MandelSources src/*.cpp)
SET(MandelSources
    src/ClGenerators.cpp
    src/CpuGenerators.cpp
    src/Generators.cpp
    src/Mandel.cpp
    src/Hardware.cpp
    src/MandelUtil.cpp
    src/Types.cpp
)
FILE(GLOB MandelHeaders include/*.h)

if (ARCH STREQUAL "X86_64" OR ARCH STREQUAL "X86")
    list(APPEND MandelSources src/CpuGeneratorsAVX.cpp src/CpuGeneratorsAVXFMA.cpp src/CpuGeneratorsSSE2.cpp)
elseif(ARCH STREQUAL "ARM")
    #list(APPEND MandelSources src/CpuGeneratorsNeon.cpp)
endif()

#    message(${MandelSources})

add_library(mandel STATIC ${MandelSources})

FILE(GLOB QdSources qd-2.3.22/src/*.cpp)

target_compile_definitions(mandel PUBLIC WITH_QD)
add_library(qd STATIC ${QdSources})
target_include_directories(qd PUBLIC qd-2.3.22/include qd-2.3.22)

#target_include_directories(mandel PUBLI#C qd-2.3.22/include)
target_link_libraries(mandel PUBLIC qd)

include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED)

if (LTO_SUPPORTED AND WITH_LTO)
    message("Enabling link-time optimization.")
    set_property(TARGET mandel PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET qd PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


if(MPFR_INCLUDES AND MPFR_LIBRARIES)
    set(MPFR_FOUND TRUE)
    #target_compile_definitions(mandel PUBLIC WITH_MPFR)
endif()

if(OPENCL_FOUND)
    target_compile_definitions(mandel PUBLIC WITH_OPENCL)
    target_include_directories(mandel PUBLIC
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

    target_compile_definitions(mandel PUBLIC WITH_BOOST)
if(Boost_FOUND)
    target_compile_definitions(mandel PUBLIC WITH_BOOST)
    target_include_directories(mandel PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(mandel PRIVATE ${Boost_LIBRARIES})
endif(Boost_FOUND)

if (ARCH STREQUAL "X86_64" OR ARCH STREQUAL "X86")
    if (MSVC)
        set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS /arch:AVX)
        set_source_files_properties(src/CpuGeneratorsAVXFMA.cpp PROPERTIES COMPILE_FLAGS /arch:AVX)
    else()
        set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS -mavx)
        set_source_files_properties(src/CpuGeneratorsAVXFMA.cpp PROPERTIES COMPILE_FLAGS -mavx)
    endif(MSVC)

    if (MSVC)
        set_source_files_properties(src/CpuGeneratorsAVXFMA.cpp PROPERTIES COMPILE_FLAGS /arch:FMA)
    else()
        set_source_files_properties(src/CpuGeneratorsAVXFMA.cpp PROPERTIES COMPILE_FLAGS -mfma)
    endif(MSVC)

    if (MSVC)
        set_source_files_properties(src/CpuGeneratorsSSE2.cpp PROPERTIES COMPILE_FLAGS /arch:SSE2)
    else()
        set_source_files_properties(src/CpuGeneratorsSSE2.cpp PROPERTIES COMPILE_FLAGS -msse2)
    endif(MSVC)
elseif(ARCH STREQUAL "ARM")
    #set_source_files_properties(src/CpuGeneratorsNeon.cpp PROPERTIES COMPILE_FLAGS -mfpu=neon)
endif()


if(OpenMP_CXX_FOUND)
    target_link_libraries(mandel PUBLIC OpenMP::OpenMP_CXX)
endif()
if(OpenCL_FOUND)
    target_link_libraries(mandel PUBLIC OpenCL::OpenCL)
endif()
if(MPFR_FOUND)
    target_link_libraries(mandel PUBLIC MPFR_LIBRARIES)
endif()

