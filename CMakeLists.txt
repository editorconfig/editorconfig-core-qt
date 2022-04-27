cmake_minimum_required(VERSION 3.20)

project(editorconfig-core-qt
        VERSION 0.0.1
        DESCRIPTION "EditorConfig Qt5/6 core bindings"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

enable_testing()
set(EDITORCONFIG_CMD ${CMAKE_BINARY_DIR}/src/editorconfig-app/editorconfig-app)
add_subdirectory(tests)

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

target_link_libraries(editorconfig-core-qt PRIVATE Qt5::Core )
target_include_directories(editorconfig-core-qt PUBLIC . )
target_include_directories(editorconfig-core-qt PRIVATE . )

add_executable(editorconfig-app src/editorconfig-app/main.cpp)
target_link_libraries(editorconfig-app PRIVATE editorconfig-core-qt Qt5::Core)
