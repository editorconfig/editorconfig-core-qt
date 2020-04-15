# This file is used for testing only

# To perform the test, run `cmake .` at the root of the project tree followed
# by ctest .

cmake_minimum_required(VERSION 2.6)

# Do not check any compiler
project(editorconfig-core-qt NONE)

enable_testing()
set(EDITORCONFIG_CMD ${CMAKE_BINARY_DIR}/editorconfig-app.exe)
add_subdirectory(tests)
