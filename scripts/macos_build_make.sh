#!/bin/sh

cmake -G"Unix Makefiles" -B "@CMAKE_BINARY_DIR@" -S "@CMAKE_SOURCE_DIR@"

cmake --build "@CMAKE_BINARY_DIR@"
