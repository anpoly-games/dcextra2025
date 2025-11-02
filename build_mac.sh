#!/bin/sh

rm -rf build

cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build --config=Release

rm -rf macos

mkdir macos
pushd macos

mkdir TheHiddenFacility.app
pushd TheHiddenFacility.app

mkdir Contents
pushd Contents

cp ../../../Info.plist .

mkdir Resources
cp -r ../../../res Resources

mkdir MacOS
pushd MacOS

cp ../../../../build/src/game TheHiddenFacility
chmod +x TheHiddenFacility

popd
popd
popd
popd
