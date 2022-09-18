# EditorConfig Qt Core

EditorConfig core bindings for Qt5 and Qt6. Does not require any external dependencies.

## EditorConfig Project

EditorConfig makes it easy to maintain the correct coding style when switching
between different text editors and between different projects.  The
EditorConfig project maintains a file format and plugins for various text
editors which allow this file format to be read and used by those editors.  For
information on the file format and supported text editors, see the
[EditorConfig website](http://editorconfig.org).

## Build Tool Integration

This project can be built and used with both qmake and cmake.

### QMake

To include the library files it is recommend that you add it as a git submodule to your project.

```bash
git submodule add https://github.com/editorconfig/editorconfig-core-qt.git editorconfig-core-qt
```

Then include the `EditorConfig.pri` file in your .pro project file.

```qmake
include(editorconfig-core-qt/EditorConfig.pri)
```

### CMake

The easiest way should be to use CPM (https://github.com/cpm-cmake/CPM.cmake). Then,
inside your CMake project do:

```CMake
# Or any other sha1 you want.
CPMAddPackage("gh:editorconfig/editorconfig-core-qt#master")
```

You can alternatively add it as a git submodule and manually use `add_subdirectory()` to add the `CMakeLists.txt` file in your project.

## Usage
Once built, the library can then be used within your Qt application.

```c++
#include <EditorConfig>

...

EditorConfigSettings settings = EditorConfig::getFileSettings("path/to/myfile.txt");
for(auto setting : settings.toStdMap()) {
    std::cout << qUtf8Printable(setting.first) << "=" << qUtf8Printable(setting.second) << std::endl;
}
```

## Development

This has been developed with Qt 5.15+ however previous versions *may* work. To develop/test this module you can use the following instructions:

1. Open `CMakeLists.txt` with Qt Creator
1. Configure the project and set `BUILD_EDITORCONFIG_CMD` to `ON`
1. Build the project
1. If ctests is configured for Qt Creator, it will run the tests
1. The majority of tests should pass
