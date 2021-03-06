cmake_minimum_required(VERSION 3.13)

project(Almond VERSION 1.2.0 DESCRIPTION "Fractal Viewer")

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Doxygen)
find_package(Qt5 COMPONENTS Core Widgets OpenGL Xml Concurrent LinguistTools REQUIRED)
find_package(OpenMP)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED)

find_package(FFmpeg COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL SWSCALE REQUIRED)

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB AlmondSources src/*.cpp)
FILE(GLOB AlmondHeaders include/*.h)
FILE(GLOB AlmondUIs ui/*.ui)
FILE(GLOB AlmondTranslations translations/*.ts)

set_source_files_properties(${AlmondTranslations} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/translations)
QT5_CREATE_TRANSLATION( QT_TRANSLATIONS_FILES ${MOC_HEADER_FILES} ${QT_SOURCE_FILES} translations/almond.fr.ts)
qt5_add_translation(AlmondQMs ${AlmondTranslations})

IF (WIN32)
    add_executable (
        Almond WIN32 ${AlmondSources}
        resources/Almond.qrc
        resources/splash.qrc
        resources/icon.rc
        ${AlmondQMs}
        ${AlmondUIs}
        ${AlmondHeaders}
    )
ELSE()
    add_executable (
        Almond ${AlmondSources}
        resources/Almond.qrc
        resources/splash.qrc
        ${AlmondQMs}
        ${AlmondUIs}
        ${AlmondHeaders}
    )
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
endif(Boost_FOUND)


if(OpenMP_CXX_FOUND)
    target_link_libraries(Almond PUBLIC OpenMP::OpenMP_CXX)
    target_compile_options(Almond PUBLIC ${OpenMP_CXX_FLAGS})
endif()

if (DOXYGEN_FOUND)
    set(DOXYGEN_IN doxyfile)
    add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building documentation")
endif()


set(CPACK_PACKAGE_NAME "Almond")
set(CPACK_PACKAGE_VERSION "1.2.0")
IF (WIN32)
#    include(${CMAKE_MODULE_PATH}/Windeployqt.cmake)
#    install(TARGETS Almond)
    #install(TARGETS ${FFMPEG_DLLS})
#    windeployqt(Almond ${CMAKE_CURRENT_BINARY_DIR})

#   potentially need to set -DQTIFWDIR="path/to/qt/Tools/QtInstallerFramework/3.x/"
    set(CPACK_GENERATOR "IFW")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Almond Installation Package")
    set(CPACK_IFW_PACKAGE_ICON "resources/almond.ico")
    set(CPACK_IFW_PACKAGE_WINDOW_ICON "resources/Almond.png")
    set(CPACK_IFW_VERBOSE ON)
    include(CPack)
    include(CPackIFW)
    cpack_add_component(Almond
        DISPLAY_NAME "Almond"
        DESCRIPTION "Almond Application")
    cpack_ifw_configure_component(Almond
        VERSION "1.2.0"
        SCRIPT "installer/packages/almond/meta/installscript.qs"
        USER_INTERFACES "installer/packages/almond/meta/page.ui"
        LICENSES "zlib" "installer/packages/almond/meta/zlib.txt")

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


