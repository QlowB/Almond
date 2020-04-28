cmake_minimum_required(VERSION 3.13)

project(libalmond VERSION 1.0.0 DESCRIPTION "almond functionality")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMakeModules)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

add_subdirectory(../libmandel ./libmandel)

#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)


set(CMAKE_CXX_STANDARD 17)


FILE(GLOB libalmondsources src/*.cpp)
FILE(GLOB libalmondheaders include/*.h)

add_library(libalmond STATIC ${libalmondsources})
target_include_directories(libalmond PUBLIC "include")
target_include_directories(libalmond SYSTEM PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(libalmond PUBLIC mandel)
target_link_libraries(libalmond PUBLIC ${FFMPEG_LIBRARIES})
set_target_properties(libalmond PROPERTIES OUTPUT_NAME almond)



