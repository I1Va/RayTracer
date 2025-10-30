find_package(OpenMP REQUIRED)

add_library(RayTracer STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RayTracer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RTGeometry.cpp
)

target_include_directories(RayTracer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP flags: ${OpenMP_CXX_FLAGS}")
    target_compile_options(RayTracer PUBLIC ${OpenMP_CXX_FLAGS})
    target_link_options(RayTracer PUBLIC ${OpenMP_CXX_FLAGS})
endif()

target_link_libraries(RayTracer 
    PRIVATE GeomLib
    PRIVATE OpenMP::OpenMP_CXX
)
