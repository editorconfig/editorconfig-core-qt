cmake_minimum_required(VERSION 3.20)

project(editorconfig-core-qt
        VERSION 0.0.1
        DESCRIPTION "EditorConfig Qt5/6 core bindings"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(editorconfig-core-qt
    EditorConfig.cpp
    EditorConfig.h
)

target_link_libraries(editorconfig-core-qt PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(editorconfig-core-qt PUBLIC . )
target_include_directories(editorconfig-core-qt PRIVATE . )

option(BUILD_EDITORCONFIG_CMD "Build executable to run EditorConfig tests" OFF)

if (BUILD_EDITORCONFIG_CMD)
    add_executable(editorconfig-app src/editorconfig-app/main.cpp)
    target_link_libraries(editorconfig-app PRIVATE editorconfig-core-qt Qt${QT_VERSION_MAJOR}::Core)

    enable_testing()
    set(EDITORCONFIG_CMD ${CMAKE_BINARY_DIR}/editorconfig-app.exe)
    add_subdirectory(tests)
endif()
