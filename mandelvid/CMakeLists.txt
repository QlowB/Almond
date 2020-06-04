cmake_minimum_required(VERSION 3.13)

project(mandelvid VERSION 1.0.0 DESCRIPTION "mandel video generator")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMakeModules)

add_subdirectory(../libalmond ./libalmond)

set(CMAKE_CXX_STANDARD 17)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65 REQUIRED COMPONENTS program_options)

FILE(GLOB mvidsources src/*.cpp)
FILE(GLOB mvidheaders include/*.h)

add_executable(mvg ${mvidsources})
target_include_directories(mvg PUBLIC "include")
target_link_libraries(mvg PUBLIC libalmond)
target_link_libraries(mvg PUBLIC Boost::program_options)

