cmake_minimum_required(VERSION 3.18)
project(Engine3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find pybind11 from the Python environment used for this build
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
set(PYBIND11_FINDPYTHON ON)
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PYBIND11_CMAKEDIR_RESULT
)
if(NOT PYBIND11_CMAKEDIR_RESULT EQUAL 0)
    message(FATAL_ERROR "pybind11 is not installed for ${Python3_EXECUTABLE}. Install it with: ${Python3_EXECUTABLE} -m pip install --user pybind11")
endif()
find_package(pybind11 CONFIG REQUIRED)

find_package(OpenGL REQUIRED)
include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# 2. Compile your bridge.cpp file into an importable Python binary module
pybind11_add_module(cpp_backend bridge.cpp)

target_link_libraries(cpp_backend PRIVATE pybind11::module)

# 3. Compile the standalone window application
add_executable(graphics_window main.cpp)

target_link_libraries(graphics_window PRIVATE glfw OpenGL::GL)
