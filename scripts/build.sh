# Build FakeVim
bash ./scripts/fakevim_build.sh

mkdir -p build
cd build
cmake ..
make

