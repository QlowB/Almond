cmake_minimum_required(VERSION 3.13)

project(Almond VERSION 1.0.0 DESCRIPTION "Fractal Viewer")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets OpenGL Xml REQUIRED)
find_package(OpenMP)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

message( ${FFMPEG_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB AlmondSources *.cpp)
FILE(GLOB AlmondHeaders *.h)


IF (WIN32)
    add_executable(Almond WIN32 ${AlmondSources} Almond.qrc icon.rc)
ELSE()
    add_executable(Almond ${AlmondSources} Almond.qrc)
ENDIF()

add_subdirectory(libmandel)

target_include_directories(Almond PUBLIC ${FFMPEG_INCLUDE_DIRS})

target_link_libraries(Almond PUBLIC mandel asmjit qd)
target_link_libraries(Almond PUBLIC Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Xml)
target_link_libraries(Almond PUBLIC ${FFMPEG_LIBRARIES})
target_link_libraries(Almond PUBLIC OpenGL::OpenGL)

if(Boost_FOUND)
    target_compile_definitions(Almond PUBLIC WITH_BOOST)
    target_include_directories(Almond PUBLIC ${Boost_INCLUDE_DIRS})
    #target_link_libraries(Almond PRIVATE ${Boost_LIBRARIES})
endif(Boost_FOUND)


if(OpenMP_CXX_FOUND)
    target_link_libraries(mandel PUBLIC OpenMP::OpenMP_CXX)
endif()
