#!/bin/bash
#set -x

if [ -x "$(command -v cindex)" ]; then
	cindex .
fi

pushd $(dirname $0)

CMAKE_CXX_COMPILER="clang++-18"
CMAKE_CXX_CLANG_TIDY="/usr/bin/clang-tidy-18"

PROJECT_DIR="$(echo "$(pwd)" | sed -e 's/^\/home\/samiur\/src\///')"
SOURCE_DIR=$(pwd)
BUILD_DIR=$HOME/build/$PROJECT_DIR/

if [ ! -d "$BUILD_DIR" ]; then
	mkdir -p $BUILD_DIR
	cmake -B $BUILD_DIR -G Ninja -S $SOURCE_DIR -DCMAKE_CXX_COMPILER=$CMAKE_CXX_COMPILER -DCLANG_TIDY_PATH=$CMAKE_CXX_CLANG_TIDY
fi

cmake --build $BUILD_DIR

popd
