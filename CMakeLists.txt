# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(WolfEdit)

# Find required packages (you may need to adjust these depending on your project)
find_package(Qt5 COMPONENTS REQUIRED Core Widgets)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add your source files
set(SOURCES
    main.cpp
    # Add other source files here
)

# Generate MOC files for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Create executable
add_executable(WolfEdit ${SOURCES})

# Link against Qt libraries
target_link_libraries(WolfEdit
    Qt5::Core
    Qt5::Widgets
)

# Set the output directory for the executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

