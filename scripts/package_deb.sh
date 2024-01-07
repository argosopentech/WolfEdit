#!/bin/sh

mkdir -p build/deb/usr/bin
cp -r DEBIAN build/deb/
cp build/WolfEdit build/deb/usr/bin/
cd build/
dpkg-deb --build deb

