cmake_minimum_required(VERSION 3.16)
project(IOGLVirtualScene LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# -----------------------------------------------------------------------------
# Dependencies: use the system packages when they exist, otherwise download
# them automatically from GitHub (FetchContent). This makes the project build
# out of the box on Windows (Visual Studio), Linux and macOS.
# -----------------------------------------------------------------------------
include(FetchContent)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

option(IOGL_FORCE_FETCH "Always download GLFW/GLEW/GLM instead of using system packages" OFF)

if(NOT IOGL_FORCE_FETCH)
    find_package(glfw3 3.3 QUIET)
    find_package(GLEW QUIET)
    find_package(glm QUIET)
endif()

if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found -> fetching it")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)   # X11 only on Linux: fewer system dependencies
    FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(glfw)
endif()

if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found -> fetching it")
    set(ONLY_LIBS ON CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
    FetchContent_Declare(glew GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git GIT_TAG glew-cmake-2.2.0 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(glew)
    set(IOGL_GLEW_TARGET libglew_static)
    set(IOGL_GLEW_STATIC ON)
else()
    set(IOGL_GLEW_TARGET GLEW::GLEW)
    set(IOGL_GLEW_STATIC OFF)
endif()

if(NOT glm_FOUND)
    message(STATUS "GLM not found -> fetching it")
    FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(glm)
endif()

# -----------------------------------------------------------------------------
# Application
# -----------------------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Camera.cpp
    src/CameraPath.cpp
    src/DemoScript.cpp
    src/Fireflies.cpp
    src/Mesh.cpp
    src/PostProcessor.cpp
    src/Scene.cpp
    src/ShaderProgram.cpp
    src/ShadowMap.cpp
    src/Sky.cpp
    src/Texture2D.cpp
    src/VideoRecorder.cpp
)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS shaders/*.vert shaders/*.frag)

add_executable(VirtualScene ${SOURCES} ${HEADERS} ${SHADERS})
target_include_directories(VirtualScene PRIVATE src third_party)
target_link_libraries(VirtualScene PRIVATE glfw ${IOGL_GLEW_TARGET} glm::glm OpenGL::GL)
if(IOGL_GLEW_STATIC)
    target_compile_definitions(VirtualScene PRIVATE GLEW_STATIC)
endif()

# The program looks for "shaders/" and "assets/" in the working directory first,
# then in the source directory (so it runs from the IDE or from the build folder).
target_compile_definitions(VirtualScene PRIVATE IOGL_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions(VirtualScene PRIVATE GLM_ENABLE_EXPERIMENTAL)

if(MSVC)
    target_compile_options(VirtualScene PRIVATE /W3 /MP /utf-8)
    target_compile_definitions(VirtualScene PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VirtualScene)
    set_target_properties(VirtualScene PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
else()
    target_compile_options(VirtualScene PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

source_group("Shaders" FILES ${SHADERS})
