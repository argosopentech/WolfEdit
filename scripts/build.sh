CMAKE_FLAGS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

# Build FakeVim
bash ./scripts/fakevim_build.sh

mkdir -p build
cd build
cmake $CMAKE_FLAGS ..
make

