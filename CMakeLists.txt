# OpenRogue

cmake_minimum_required(VERSION 3.15)
project(OpenRogue LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_compile_definitions(_WIN32_WINNT=0x0501 WINVER=0x0501)
endif()

include_directories(${CMAKE_SOURCE_DIR}/shared)

file(GLOB_RECURSE SHARED_CPP CONFIGURE_DEPENDS "shared/*.cpp")
file(GLOB_RECURSE SHARED_H CONFIGURE_DEPENDS "shared/*.h" "shared/*.hpp")
if(SHARED_CPP)
    add_library(SharedLib STATIC ${SHARED_CPP} ${SHARED_H})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/gui/main_gui.cpp")
    add_executable(MainApp WIN32 gui/main_gui.cpp)
    if(TARGET SharedLib)
        target_link_libraries(MainApp PRIVATE SharedLib)
    endif()

    if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_options(MainApp PRIVATE "/SUBSYSTEM:WINDOWS,5.01")
    endif()
endif()

file(GLOB EXP_SOURCES CONFIGURE_DEPENDS "experiments/*.cpp")
foreach(EXP_FILE ${EXP_SOURCES})
    get_filename_component(EXP_NAME ${EXP_FILE} NAME_WE)
    add_executable(${EXP_NAME} ${EXP_FILE} "driver/vuln_sim/driver.c" "experiments/persistence.cpp")
    if(TARGET SharedLib)
        target_link_libraries(${EXP_NAME} PRIVATE SharedLib)
    endif()

    if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_options(${EXP_NAME} PRIVATE "/SUBSYSTEM:CONSOLE,5.01")
    endif()
endforeach()

option(BUILD_DRIVERS "Build kernel drivers" ON)
if(BUILD_DRIVERS)
    file(GLOB DRIVER_CMAKE_FILES CONFIGURE_DEPENDS "driver/*/CMakeLists.txt")
    if(DRIVER_CMAKE_FILES)
        foreach(DRIVER_CMAKE ${DRIVER_CMAKE_FILES})
            get_filename_component(DRIVER_DIR ${DRIVER_CMAKE} DIRECTORY)
            add_subdirectory(${DRIVER_DIR})
        endforeach()
    else()
        message(WARNING "BUILD_DRIVERS is ON, but no 'driver/*/CMakeLists.txt' was found.")
    endif()
endif()