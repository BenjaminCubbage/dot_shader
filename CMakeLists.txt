cmake_minimum_required(VERSION 3.16)
project(dot_shader)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(directx
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG        v1.619.1)
FetchContent_MakeAvailable(directx)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY                ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_BINARY_DIR}/artifacts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_BINARY_DIR}/artifacts)

add_executable(dot_shader
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/env/env.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/env/env.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/fs/fs.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/fs/fs.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/helpers/ascii_to_w.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/helpers/ascii_to_w.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/helpers/throw_if_failed.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/helpers/throw_if_failed.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/render/render_surface.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/render/render_surface.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/render/renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/render/renderer.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/thread/mpmc_queue.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/thread/mpmc_queue.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_manager.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_inst.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window_inst.h
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window.cpp
    ${CMAKE_SOURCE_DIR}/src/dot_shader/window/window.h)

target_precompile_headers(dot_shader PUBLIC
    ${CMAKE_SOURCE_DIR}/src/pch.h)

target_include_directories(dot_shader PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${directx_SOURCE_DIR}/include)

target_link_libraries(dot_shader PUBLIC 
    DirectX-Headers
    d3d12
    dxgi
    D3DCompiler)