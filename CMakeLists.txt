cmake_minimum_required(VERSION 3.20)

include(GenerateExportHeader)

project(editorconfig-core-qt
    VERSION 0.0.1
    DESCRIPTION "EditorConfig Qt5/6 core bindings"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core)
endif()

add_library(editorconfig-core-qt
    EditorConfig.cpp
    EditorConfig.h
)

target_link_libraries(editorconfig-core-qt
    PRIVATE
    Qt::Core
)

target_include_directories(editorconfig-core-qt
    PUBLIC
    .
    ${CMAKE_CURRENT_BINARY_DIR}
)

generate_export_header(editorconfig-core-qt
    EXPORT_FILE_NAME
    editorconfig-core-qt-export.h
)

option(EDITORCONFIG_QT_BUILD_CMD "Build executable to run EditorConfig tests" OFF)

if (EDITORCONFIG_QT_BUILD_CMD)
    add_executable(editorconfig-app src/editorconfig-app/main.cpp)
    target_link_libraries(editorconfig-app PRIVATE editorconfig-core-qt Qt::Core)

    enable_testing()
    set(EDITORCONFIG_CMD ${CMAKE_BINARY_DIR}/editorconfig-app.exe)
    add_subdirectory(tests)
endif()
