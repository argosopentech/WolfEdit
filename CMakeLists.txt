# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(WolfEdit)

# Find required packages (you may need to adjust these depending on your project)
find_package(Qt5 COMPONENTS REQUIRED Core Widgets)

# Add your source files
set(SOURCES
    main.cpp
    src/WolfEdit.h
    src/editor.h
    src/editor.cpp
)

# Generate MOC files for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Create executable
add_executable(WolfEdit ${SOURCES})

target_link_libraries(WolfEdit
    Qt5::Core
    Qt5::Widgets
    fakevim
)

# Set the output directory for the executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

# Add the FakeVim library
add_subdirectory(third_party/FakeVim)

# Include directories for FakeVim
target_include_directories(WolfEdit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FakeVim
)
