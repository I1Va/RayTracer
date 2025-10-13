cmake_minimum_required(VERSION 3.10)

project(RayTracer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(RayTracer STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RayTracer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RTGeometry.cpp
)

target_include_directories(RayTracer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(RayTracer PUBLIC
    GeomLib
)