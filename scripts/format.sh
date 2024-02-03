SOURCE_FILES="*.cpp src/*.cpp src/*.h"
clang-format -i $SOURCE_FILES

# Run clang-tidy
#clang-tidy -p <path_to_build_directory> --checks=* --fix --header-filter='.*' repl.h repl.cpp