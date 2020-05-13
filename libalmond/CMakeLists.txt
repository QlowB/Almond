cmake_minimum_required(VERSION 3.13)

project(libalmond VERSION 1.0.0 DESCRIPTION "almond functionality")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMakeModules)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

add_subdirectory(../libmandel ./libmandel)

#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)
find_package(PNG)
find_package (JPEG)

set(CMAKE_CXX_STANDARD 17)


FILE(GLOB libalmondsources src/*.cpp)
FILE(GLOB libalmondheaders include/*.h)

add_library(libalmond STATIC ${libalmondsources})
target_include_directories(libalmond PUBLIC "include")
target_link_libraries(libalmond PUBLIC mandel)
set_target_properties(libalmond PROPERTIES OUTPUT_NAME almond)

target_include_directories(libalmond SYSTEM PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(libalmond PUBLIC ${FFMPEG_LIBRARIES})

if (PNG_FOUND)
    #target_include_directories(libalmond SYSTEM PUBLIC ${PNG_INCLUDE_DIRS})
    #target_link_libraries(libalmond PUBLIC ${PNG_LIBRARIES})
    target_link_libraries(libalmond PUBLIC PNG::PNG)
else()
    set(PNG_BUILD_ZLIB ON CACHE BOOL "build zlib ourselves")
    add_subdirectory(zlib-1.2.11)
    set(ZLIB_LIBRARY zlibstatic)
    #foreach(header ${ZLIB_PUBLIC_HDRS})
    #    get_filename_component(the_incluude ${header} DIRECTORY)
    #    list(APPEND ZLIB_PUB_INCLUDE ${the_incluude})
    #endforeach()
    #set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.11 ${ZLIB_PUB_INCLUDE} )
    
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(lpng1637)
    #foreach(header ${libpng_public_hdrs})
    #    get_filename_component(the_incluude ${header} DIRECTORY)
    #    list(APPEND PNG_PUB_INCLUDE ${the_incluude})
    #endforeach()
    target_link_libraries(png_static PRIVATE zlibstatic)
    #target_include_directories(libalmond PRIVATE ${PNG_PUB_INCLUDE})
    #target_include_directories(libalmond PRIVATE ${ZLIB_INCLUDE_DIR})
    target_link_libraries(libalmond PRIVATE png_static)
endif()

if (JPEG_FOUND AND WITH_LIBJPEG)
    target_link_libraries(libalmond PRIVATE JPEG::JPEG)
    target_compile_definitions(libalmond PUBLIC WITH_LIBJPEG)
endif()


