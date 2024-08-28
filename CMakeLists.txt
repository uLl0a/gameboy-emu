cmake_minimum_required(VERSION 3.10)

# avoid create project on project root directory
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
        message(FATAL_ERROR "Building in-source is not supported! Create a
        build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

set(GEMU_EXECUTABLE gbemu)
set(PROJECT ${GEMU_EXECUTABLE})
set(MAJOR_VERSION 0.1.0)
project(${PROJECT} VERSION ${MAJOR_VERSION})

file(GLOB_RECURSE ${GEMU_EXECUTABLE}_SOURCE CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/core/*.h
        ${CMAKE_SOURCE_DIR}/src/core/*.c
    )
add_compile_definitions(GBEMU_VERBOSE=1)
add_executable(${PROJECT} ${${GEMU_EXECUTABLE}_SOURCE})
