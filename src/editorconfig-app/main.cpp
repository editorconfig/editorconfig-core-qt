/*
 * MIT License
 *
 * Copyright (c) 2017 Justin Dailey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>

#include <EditorConfig>


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("EditorConfig");
    QCoreApplication::setApplicationVersion("v0.1.0");

    QCommandLineParser parser;
    QCommandLineOption configFileOption("f", " Specify conf filename other than \".editorconfig\"", "filename");

    parser.setApplicationDescription("EditorConfig core for Qt");

    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(configFileOption);
    parser.addPositionalArgument("file", "");

    parser.process(app);

    const QString configFile = parser.isSet(configFileOption) ? parser.value(configFileOption) : ".editorconfig";

    foreach (const QString fileName, parser.positionalArguments()) {
        EditorConfigSettings settings = EditorConfig::getFileSettings(fileName, configFile);

        for(auto setting : settings.toStdMap()) {
            std::cout << qUtf8Printable(setting.first) << "=" << qUtf8Printable(setting.second) << std::endl;
        }
    }

    return 0;
}
