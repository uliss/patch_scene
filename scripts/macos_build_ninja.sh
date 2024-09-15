#!/bin/sh

cmake -GNinja -B "@CMAKE_BINARY_DIR@" -S "@CMAKE_SOURCE_DIR@" -DCMAKE_BUILD_TYPE=Release

cmake --build "@CMAKE_BINARY_DIR@"
