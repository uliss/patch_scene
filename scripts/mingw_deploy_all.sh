#!/bin/sh

cmake --build "@CMAKE_BINARY_DIR@" --target git_version
cmake --build "@CMAKE_BINARY_DIR@" --target all
cmake --install "@CMAKE_BINARY_DIR@"
cmake --build "@CMAKE_BINARY_DIR@" --target inno
cmake --build "@CMAKE_BINARY_DIR@" --target app-zip
cmake --build "@CMAKE_BINARY_DIR@" --target dev_upload
