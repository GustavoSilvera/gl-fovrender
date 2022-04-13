cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(PROJECT_NAME "ogl-playground")
project(${PROJECT_NAME} VERSION 1.0)

add_definitions(-std=c++17)
set(CXX_FLAGS "-Wall" "-Werror" "-Wextra" "-O3" "-fsanitize=undefined,address")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")
set(CMAKE_BUILD_TYPE Debug)


add_executable(${PROJECT_NAME} src/shader_utils.cpp src/renderer.cpp src/main.cpp)

find_package(glfw3 3.4 REQUIRED)
find_package(OpenGL REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
endif (APPLE)
target_link_libraries(${PROJECT_NAME} glfw ${OPENGL_gl_LIBRARY})
