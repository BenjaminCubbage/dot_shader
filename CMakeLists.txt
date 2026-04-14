cmake_minimum_required(VERSION 3.16)
project(dot_shader)

set(
    CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/artifacts)

add_executable(
    dot_shader
    main.cpp)