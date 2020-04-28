cmake_minimum_required(VERSION 3.13)

project(mandelvid VERSION 1.0.0 DESCRIPTION "mandel video generator")

add_subdirectory(../libalmond ./libalmond)

set(CMAKE_CXX_STANDARD 17)


FILE(GLOB mvidsources src/*.cpp)
FILE(GLOB mvidheaders include/*.h)

add_executable(mvg ${mvidsources})
#target_include_directories(mvg PUBLIC "include")
target_link_libraries(mvg PUBLIC libalmond)

