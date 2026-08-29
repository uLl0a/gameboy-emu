cmake_minimum_required(VERSION 3.10)

# avoid create project on project root directory
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
        message(FATAL_ERROR "Building in-source is not supported! Create a
        build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(GEMU_EXECUTABLE gbemu)
set(PROJECT ${GEMU_EXECUTABLE})
set(MAJOR_VERSION 0.1.0)
project(${PROJECT} VERSION ${MAJOR_VERSION})

file(GLOB_RECURSE ${GEMU_EXECUTABLE}_SOURCE CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/core/*.h
        ${CMAKE_SOURCE_DIR}/src/core/*.c
    )

add_compile_definitions(GBEMU_VERBOSE=1)

if(WIN32)
    set(CMAKE_GENERATOR_PLATFORM Win32 CACHE STRING "Build platform" FORCE)
    set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/vendor/SDL2")
    find_library(SDL2_LIBRARY NAMES SDL2 PATHS "${SDL2_ROOT}/lib/x86" NO_DEFAULT_PATH REQUIRED)
    if(NOT EXISTS "${SDL2_ROOT}/lib/x86/SDL2.dll")
        message(FATAL_ERROR "SDL2.dll not found in ${SDL2_ROOT}/lib/x86")
    endif()
    include_directories("${SDL2_ROOT}/include")
    link_directories("${SDL2_ROOT}/lib/x86")
    set(SDL2_LIBRARIES ${SDL2_LIBRARY})
else()
    find_package(SDL2 REQUIRED)
    set(SDL2_LIBRARIES SDL2::SDL2)
endif()

add_executable(${PROJECT} ${${GEMU_EXECUTABLE}_SOURCE})
target_link_libraries(${PROJECT} PRIVATE ${SDL2_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET ${PROJECT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/vendor/SDL2/lib/x86/SDL2.dll"
            $<TARGET_FILE_DIR:${PROJECT}>
    )
endif()
