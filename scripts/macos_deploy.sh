#!/bin/sh

cmake --build "@CMAKE_BINARY_DIR@" --target git_version
cmake --build "@CMAKE_BINARY_DIR@" --target all
cmake --build "@CMAKE_BINARY_DIR@" --target dmg
cmake --build "@CMAKE_BINARY_DIR@" --target app-zip
cmake --build "@CMAKE_BINARY_DIR@" --target dev_upload
