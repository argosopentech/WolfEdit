#!/bin/sh

mkdir -p build/debian_build/WolfEdit/usr/bin
cp -r DEBIAN build/debian_build/WolfEdit
cp build/WolfEdit build/debian_build/WolfEdit/usr/bin/
cd build/
dpkg-deb --build debian_build/WolfEdit

