cmake_minimum_required(VERSION 3.12)
project(MyRaylibGame LANGUAGES CXX)

# Force C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch raylib directly from GitHub automatically
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG        master # Or use a specific version like "5.0"
)

# Make raylib targets available 
FetchContent_MakeAvailable(raylib)

# Define your executable and its source files
add_executable(${PROJECT_NAME} src/main.cpp)

# Link your application against the raylib library
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
