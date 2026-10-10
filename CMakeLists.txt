cmake_minimum_required(VERSION 3.12)
project(Engine3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find the pybind11 installation you just downloaded via pip
set(PYBIND11_FINDPYTHON ON)
execute_process(
    COMMAND python3 -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(pybind11 REQUIRED)

# 2. Compile your bridge.cpp file into an importable Python binary extension
pybind11_add_module(cpp_backend bridge.cpp)