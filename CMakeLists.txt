cmake_minimum_required(VERSION 3.13)

project(Almond VERSION 1.0.0 DESCRIPTION "Fractal Viewer")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets OpenGL Xml REQUIRED)
find_package(OpenMP)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

#message(${FFMPEG_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB AlmondSources *.cpp)
FILE(GLOB AlmondHeaders *.h)


IF (WIN32)
    add_executable(Almond WIN32 ${AlmondSources} Almond.qrc icon.rc)
ELSE()
    add_executable(Almond ${AlmondSources} Almond.qrc)
ENDIF()

add_subdirectory(libmandel)

target_include_directories(Almond SYSTEM PUBLIC ${FFMPEG_INCLUDE_DIRS})

target_link_libraries(Almond PUBLIC mandel)
target_link_libraries(Almond PUBLIC Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Xml)
target_link_libraries(Almond PUBLIC ${FFMPEG_LIBRARIES})
target_link_libraries(Almond PUBLIC OpenGL::GL)

set_property(TARGET Almond PROPERTY AUTOMOC ON)
set_property(TARGET Almond PROPERTY AUTORCC ON)
set_property(TARGET Almond PROPERTY AUTOUIC ON)



if(Boost_FOUND)
    target_compile_definitions(Almond PUBLIC WITH_BOOST)
    target_include_directories(Almond SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
    #target_link_libraries(Almond PRIVATE ${Boost_LIBRARIES})
endif(Boost_FOUND)


if(OpenMP_CXX_FOUND)
    target_link_libraries(Almond PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(Almond PUBLIC ${OpenMP_CXX_FLAGS})
endif()


set(CPACK_PACKAGE_NAME "Almond")
set(CPACK_PACKAGE_VERSION "1.0.0")
IF (WIN32)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MyPackage Installation Example")
    include(CPack)
    include(CPackIFW)
    cpack_add_component(Almond
        DISPLAY_NAME "Almond"
        DESCRIPTION "Almond Application")
    cpack_ifw_configure_component(Almond)

ELSEIF (UNIX AND NOT APPLE)
    install(TARGETS Almond RUNTIME DESTINATION "bin")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_SOURCE_GENERATOR "DEB")
    set(CPACK_COMPONENTS_ALL Almond)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nicolas Winkler")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default,libavformat58,libavdevice58,libavfilter7,libavutil56,libswscale5,libgl1,ocl-icd-libopencl1")
    set(CPACK_SET_DESTDIR True)
    set(CPACK_INSTALL_PREFIX "/usr")
    include(CPack)
ENDIF()


