cmake_minimum_required(VERSION 3.16)
project(dot_shader)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY                ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_BINARY_DIR}/artifacts)

add_executable(dot_shader
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_event.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_event.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_manager.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window.h)

target_precompile_headers(dot_shader PUBLIC
    ${CMAKE_SOURCE_DIR}/src/pch.h)

target_include_directories(dot_shader PRIVATE 
    ${CMAKE_SOURCE_DIR}/src)