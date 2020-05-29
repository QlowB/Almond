cmake_minimum_required(VERSION 3.13)

project(Almond VERSION 1.1.0 DESCRIPTION "Fractal Viewer")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets OpenGL Xml Concurrent REQUIRED)
find_package(OpenMP)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

#message(${FFMPEG_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB AlmondSources src/*.cpp)
FILE(GLOB AlmondHeaders include/*.h)


IF (WIN32)
    add_executable(Almond WIN32 ${AlmondSources}
                   resources/Almond.qrc
                   resources/splash.qrc
                   resources/icon.rc
                   ${AlmondHeaders})
ELSE()
    add_executable(Almond ${AlmondSources}
                   resources/Almond.qrc
                   resources/splash.qrc
                   ${AlmondHeaders})
    set_target_properties(Almond PROPERTIES OUTPUT_NAME "almond")
ENDIF()

target_include_directories(Almond PUBLIC include)

add_subdirectory(libalmond)

target_include_directories(Almond SYSTEM PUBLIC ${FFMPEG_INCLUDE_DIRS})

target_link_libraries(Almond PUBLIC libalmond)
target_link_libraries(Almond PUBLIC Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Xml Qt5::Concurrent)
target_link_libraries(Almond PUBLIC ${FFMPEG_LIBRARIES})

set_property(TARGET Almond PROPERTY AUTOUIC_SEARCH_PATHS "ui")
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
    target_compile_options(Almond PUBLIC ${OpenMP_CXX_FLAGS})
endif()


set(CPACK_PACKAGE_NAME "Almond")
set(CPACK_PACKAGE_VERSION "1.1.0")
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
    install(FILES debian/Almond.desktop DESTINATION "share/applications")
    install(FILES resources/Almond.png DESTINATION "share/pixmaps")
    install(FILES debian/copyright DESTINATION "share/doc/almond/" RENAME "copyright")
    install(FILES debian/Almond.appdata.xml DESTINATION "share/metainfo/" RENAME "org.winfor.Almond.appdata.xml")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_SOURCE_GENERATOR "DEB")
    set(CPACK_COMPONENTS_ALL Almond)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "A fractal viewer")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nicolas Winkler <nicolas.winkler@gmx.ch>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5-dev,libavformat58,libavdevice58,libavfilter7,libavutil56,libswscale5,ocl-icd-libopencl1,libpng16-16,libc6")
    set(CPACK_SET_DESTDIR True)
    set(CPACK_INSTALL_PREFIX "/usr")
    include(CPack)
ENDIF()


