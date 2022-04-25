project(editorconfig-core-qt)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Network Widgets REQUIRED)

#enable_testing()
set(EDITORCONFIG_CMD ${CMAKE_BINARY_DIR}/editorconfig-app.exe)
#add_subdirectory(tests)

if (BUILD_STATIC)
    set(CMAKE_STATIC_LIBRARY_SUFFIX "_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
    add_library(editorconfig-core-qt STATIC
        EditorConfig.cpp
        EditorConfig.h
    )
else()
    add_library(editorconfig-core-qt SHARED
        EditorConfig.cpp
        EditorConfig.h
    )
endif()

target_link_libraries(editorconfig-core-qt PRIVATE Qt5::Widgets )
target_include_directories(editorconfig-core-qt PUBLIC . )
target_include_directories(editorconfig-core-qt PRIVATE . )

