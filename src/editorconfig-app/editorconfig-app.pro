# MIT License
#
# Copyright (c) 2017 Justin Dailey
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

QT -= gui

CONFIG += console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
SOURCES = main.cpp
TARGET = editorconfig-app

DESTDIR = $$PWD/../../build
win32:QMAKE_POST_LINK = $$quote($$[QT_INSTALL_BINS]/windeployqt --force --no-translations \"$$DESTDIR\"$$escape_expand(\n\t))

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../editorconfig-core-qt/release/ -leditorconfig-core-qt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../editorconfig-core-qt/debug/ -leditorconfig-core-qt
else:unix: LIBS += -L$$OUT_PWD/../editorconfig-core-qt/ -leditorconfig-core-qt

INCLUDEPATH += $$PWD/../editorconfig-core-qt
DEPENDPATH += $$PWD/../editorconfig-core-qt

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../editorconfig-core-qt/release/libeditorconfig-core-qt.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../editorconfig-core-qt/debug/libeditorconfig-core-qt.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../editorconfig-core-qt/release/editorconfig-core-qt.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../editorconfig-core-qt/debug/editorconfig-core-qt.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../editorconfig-core-qt/libeditorconfig-core-qt.a
