cmake_minimum_required(VERSION 3.16)
project(CRasterizer C)

# Set the output directory for built binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Use vendored SDL_ttf and its dependencies
set(SDLTTF_VENDORED ON)

# Add vendored SDL and SDL_ttf source trees
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)

# Create executable target
add_executable(${PROJECT_NAME} main.c renderer.c calcs.c ImportObj.c eventMgr.c)

# Link executable with vendored SDL3 and SDL3_ttf targets
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_ttf::SDL3_ttf SDL3::SDL3 m)
