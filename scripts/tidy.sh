# Run clang-tidy
SOURCE_FILES="main.cpp src/*.cpp src/*.h"
clang-tidy -p build/ --checks=* --header-filter='.*' $SOURCE_FILES
# Add --fix to apply fixes