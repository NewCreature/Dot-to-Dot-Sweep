#!/bin/bash

sudo /Volumes/MacOSExternal/Development/scripts/change_sdk.sh MacOSX10.13
make clean
make MACOS_ARCHITECTURES="-arch i386 -arch x86_64"
cp ../bin/dot-to-dot-sweep ../dot-to-dot-sweep-x86
make clean
sudo /Volumes/MacOSExternal/Development/scripts/change_sdk.sh MacOSX11.1
make MACOS_ARCHITECTURES="-arch arm64"
cp ../bin/dot-to-dot-sweep ../dot-to-dot-sweep-arm64
lipo -create ../bin/dot-to-dot-sweep-x86 ../bin/dot-to-dot-sweep-arm64 -output ../bin/dot-to-dot-sweep
cp ../bin/dot-to-dot-sweep "../packages/Dot to Dot Sweep.app/Contents/MacOS/Dot to Dot Sweep"
