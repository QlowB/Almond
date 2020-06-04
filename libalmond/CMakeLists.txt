cmake_minimum_required(VERSION 3.13)

project(libalmond VERSION 1.0.0 DESCRIPTION "almond functionality")

option(LIBALMOND_LIBJPEG "use libjpeg to export jpeg images" ON)
option(LIBALMOND_LIBPNG "use libpng to export png images" ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMakeModules)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

add_subdirectory(../libmandel ./libmandel)

#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)
find_package(OpenMP)
find_package(PNG)
find_package (JPEG)

set(CMAKE_CXX_STANDARD 17)


FILE(GLOB libalmondsources src/*.cpp)
FILE(GLOB libalmondheaders include/*.h)

list(APPEND libalmondsources tinyxml2/tinyxml2.cpp)
list(APPEND libalmondheaders tinyxml2/tinyxml2.h)

add_library(libalmond STATIC ${libalmondsources})
target_include_directories(libalmond PUBLIC "include" "tinyxml2")
target_link_libraries(libalmond PUBLIC mandel)
set_target_properties(libalmond PROPERTIES OUTPUT_NAME almond)

target_include_directories(libalmond SYSTEM PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(libalmond PUBLIC ${FFMPEG_LIBRARIES})

if (PNG_FOUND AND LIBALMOND_LIBPNG)
    target_link_libraries(libalmond PUBLIC PNG::PNG)
    target_compile_definitions(libalmond PUBLIC WITH_LIBPNG)
endif()

if (JPEG_FOUND AND LIBALMOND_LIBJPEG)
    target_link_libraries(libalmond PUBLIC JPEG::JPEG)
    target_compile_definitions(libalmond PUBLIC WITH_LIBJPEG)
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(libalmond PUBLIC OpenMP::OpenMP_CXX)
endif()

