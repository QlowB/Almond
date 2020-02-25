cmake_minimum_required(VERSION 3.9)

project(mandelvid VERSION 1.0.0 DESCRIPTION "mandel video generator")

set(ARCH "X86_64" CACHE STRING "Target Architecture")


find_package(PkgConfig)
pkg_check_modules(LIBAVCODEC REQUIRED libavcodec)
pkg_check_modules(LIBAVUTIL REQUIRED libavutil)
pkg_check_modules(LIBAVFORMAT REQUIRED libavformat)
pkg_check_modules(LIBSWSCALE REQUIRED libswscale)


add_subdirectory(../libmandel ./libmandel)

find_package(OpenCL)
find_package(OpenMP)
#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.53)


set(CMAKE_CXX_STANDARD 17)


FILE(GLOB mvidsources src/*.cpp)
FILE(GLOB mvidheaders include/*.h)

add_executable(mvg ${mvidsources})
target_include_directories(mvg PUBLIC "include")
target_include_directories(mvg PUBLIC ${LIBAVCODEC_INCLUDE_DIRS})
target_include_directories(mvg PUBLIC ${LIBAVUTIL_INCLUDE_DIRS})
target_include_directories(mvg PUBLIC ${LIBAVFORMAT_INCLUDE_DIRS})
target_include_directories(mvg PUBLIC ${LIBSWSCALE_INCLUDE_DIRS})
target_link_libraries(mvg PUBLIC mandel)
target_link_libraries(mvg PUBLIC qd)
target_link_libraries(mvg PUBLIC ${LIBAVCODEC_LIBRARIES})
target_link_libraries(mvg PUBLIC ${LIBAVUTIL_LIBRARIES})
target_link_libraries(mvg PUBLIC ${LIBAVFORMAT_LIBRARIES})
target_link_libraries(mvg PUBLIC ${LIBSWSCALE_LIBRARIES})

include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED)

if (LTO_SUPPORTED AND WITH_LTO)
    message("Enabling link-time optimization.")
    set_property(TARGET mvg PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


#if(OPENCL_FOUND)
#    target_compile_definitions(mvg PUBLIC WITH_OPENCL)
#    target_include_directories(mvg PUBLIC
#        "include"
#        ${OpenCL_INCLUDE_DIRS}
#    )
#    link_directories(${OpenCL_LIBRARY})
#else(OPENCL_FOUND)
#    include_directories("include")
#endif(OPENCL_FOUND)
#
#if (APPLE AND OpenCL_FOUND)
#    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -framework OpenCL")
#endif()
#
#    target_compile_definitions(mandel PUBLIC WITH_BOOST)
#if(Boost_FOUND)
#    target_compile_definitions(mandel PUBLIC WITH_BOOST)
#    target_include_directories(mandel PRIVATE ${Boost_INCLUDE_DIRS})
#    target_link_libraries(mandel PRIVATE ${Boost_LIBRARIES})
#endif(Boost_FOUND)
#
#if(OpenMP_CXX_FOUND)
#    target_link_libraries(mandel PUBLIC OpenMP::OpenMP_CXX)
#endif()
#if(OpenCL_FOUND)
#    target_link_libraries(mandel PUBLIC OpenCL::OpenCL)
#endif()

