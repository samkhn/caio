#!/bin/bash
#set -x

if [ -x "$(command -v cindex)" ]; then
	cindex .
fi

pushd $(dirname $0)

PROJECT_DIR="$(echo "$(pwd)" | sed -e 's/^\/home\/samiur\/src\///')"
SOURCE_DIR=$(pwd)
BUILD_DIR=$HOME/build/$PROJECT_DIR/

if [ ! -d "$BUILD_DIR" ]; then
	mkdir -p $BUILD_DIR
	cmake -B $BUILD_DIR -G Ninja -S $SOURCE_DIR
fi

cmake --build $BUILD_DIR

popd
