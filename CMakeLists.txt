cmake_minimum_required(VERSION 3.2.18)
project(CRasterizer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(SDL3 REQUIRED)

add_executable(${PROJECT_NAME} main.c renderer.c calcs.c ImportObj.c eventMgr.c)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 m)
