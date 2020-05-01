cmake_minimum_required(VERSION 3.12)

include(CheckCXXCompilerFlag)

set(ARCH "X86_64" CACHE STRING "Target Architecture")
option(AVX512 "generate code that can make use of avx-512-instructions" ON)
option(WITH_ASMJIT "use just-in-time-compilation capabilities of asmjit" ON)
option(MANDEL_BUILD_NATIVE
	"use the -march=native flags if supported WARNING: when compiling with this flag, the binary might not run on machines other than the one it was compiled on"
	OFF)

#message(CMAKE_SYSTEM_PROCESSOR)

project(mandel VERSION 1.0.0 DESCRIPTION "library for mandelbrot calculations")

find_package(OpenCL REQUIRED)
find_package(OpenMP)
#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)
set(ASMJIT_STATIC ON)
find_package(Boost 1.65 REQUIRED)


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
    src/OpenClCode.cpp
    src/IterationGenerator.cpp
    src/IterationFormula.cpp
    src/IterationCompiler.cpp
    src/IterationIR.cpp
    src/NaiveIRGenerator.cpp
)
FILE(GLOB MandelHeaders include/*.h)

if (ARCH STREQUAL "X86_64" OR ARCH STREQUAL "X86")
    list(APPEND MandelSources src/CpuGeneratorsAVX.cpp src/CpuGeneratorsAVXFMA.cpp src/CpuGeneratorsSSE2.cpp)
    if (AVX512)
        list(APPEND MandelSources src/CpuGeneratorsAVX512.cpp)
    endif()
elseif(ARCH STREQUAL "ARM")
    list(APPEND MandelSources src/CpuGeneratorsNeon.cpp)
endif()


# use both flags just to be sure
CHECK_CXX_COMPILER_FLAG("-march=native" MARCH_NATIVE_SUPPORTED)
CHECK_CXX_COMPILER_FLAG("-mtune=native" MTUNE_NATIVE_SUPPORTED)
if(MARCH_NATIVE_SUPPORTED AND MANDEL_BUILD_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()
if(MTUNE_NATIVE_SUPPORTED AND MANDEL_BUILD_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=native")
endif()


#    message(${MandelSources})

add_library(mandel STATIC ${MandelSources})

FILE(GLOB QdSources qd-2.3.22/src/*.cpp)

target_compile_definitions(mandel PUBLIC WITH_QD)
add_library(qd STATIC ${QdSources})
target_include_directories(qd PUBLIC qd-2.3.22/include qd-2.3.22)

target_link_libraries(mandel PUBLIC qd)

if(WITH_ASMJIT)
	add_subdirectory(asmjit)
	target_compile_definitions(mandel PUBLIC WITH_ASMJIT)
	target_link_libraries(mandel PUBLIC asmjit)
endif(WITH_ASMJIT)


if(OPENCL_FOUND)
    target_compile_definitions(mandel PUBLIC WITH_OPENCL)
    target_include_directories(mandel PUBLIC "include")
    target_include_directories(mandel SYSTEM PUBLIC
        ${OpenCL_INCLUDE_DIRS}
    )
    link_directories(${OpenCL_LIBRARY})
else(OPENCL_FOUND)
    include_directories("include")
endif(OPENCL_FOUND)

if (APPLE AND OpenCL_FOUND)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -framework OpenCL")
endif()

if(Boost_FOUND)
    target_compile_definitions(mandel PUBLIC WITH_BOOST)
    target_include_directories(mandel PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(mandel PRIVATE ${Boost_LIBRARIES})
endif(Boost_FOUND)

if (ARCH STREQUAL "X86_64" OR ARCH STREQUAL "X86")
    if (AVX512)
        target_compile_definitions(mandel PUBLIC WITH_AVX512)
        if (MSVC)
            set_source_files_properties(src/CpuGeneratorsAVX512.cpp PROPERTIES COMPILE_FLAGS /arch:AVX512F)
        else()
            set_source_files_properties(src/CpuGeneratorsAVX512.cpp PROPERTIES COMPILE_FLAGS -mavx512f)
        endif(MSVC)
    endif()

    if (MSVC)
        set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS /arch:AVX)
        set_source_files_properties(src/CpuGeneratorsAVXFMA.cpp PROPERTIES COMPILE_FLAGS /arch:AVX2)
        set_source_files_properties(src/CpuGeneratorsSSE2.cpp PROPERTIES COMPILE_FLAGS /arch:SSE2)
    else()
        set_source_files_properties(src/CpuGeneratorsAVX.cpp PROPERTIES COMPILE_FLAGS -mavx)
        set_source_files_properties(src/CpuGeneratorsAVXFMA.cpp PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
        set_source_files_properties(src/CpuGeneratorsSSE2.cpp PROPERTIES COMPILE_FLAGS -msse2)
    endif(MSVC)

elseif(ARCH STREQUAL "ARM")
    set_source_files_properties(src/CpuGeneratorsNeon.cpp PROPERTIES COMPILE_FLAGS -march=armv8-a+simd)
endif()


if(OpenMP_CXX_FOUND)
    target_link_libraries(mandel PUBLIC OpenMP::OpenMP_CXX)
endif()
if(OpenCL_FOUND)
    target_link_libraries(mandel PUBLIC OpenCL::OpenCL)
endif()
