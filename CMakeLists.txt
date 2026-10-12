cmake_minimum_required(VERSION 3.16)
project(SimpleQtApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic compilation handling for Qt features
set(CMAKE_AUTOMOC ON) # Handles Meta-Object Compiler for signals/slots
set(CMAKE_AUTOUIC ON) # Handles UI XML files if you use them
set(CMAKE_AUTORCC ON) # Handles Resource files (.qrc)

# Find the mandatory Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Define your application executable and its source files
add_executable(Graphing_Calculator
    main.cpp
    mywidget.cpp
)

# Link the Qt modules to your executable target
target_link_libraries(Graphing_Calculator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)