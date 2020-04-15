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


#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QVector>
#include <QMap>
#include <QFileInfo>
#include <QException>
#include <QDebug>

#include "EditorConfig.h"


const int MAX_SECTION_NAME_LENGTH = 4096;
const int MAX_KEY_LENGTH = 50;
const int MAX_VALUE_LENGTH = 255;


class EditorConfigFileNotFound : public QException
{
public:
    void raise() const override { throw *this; }
    EditorConfigFileNotFound *clone() const override { return new EditorConfigFileNotFound(*this); }
};


static QStringList getAllPossibleConfigFileLocations(const QString &path, const QString &filename)
{
    // Given a path e.g. C:/a/b/c and a config file name e.g. .editorconfig
    // Generate all paths that may contain a config file:
    //  C:/a/b/c/.editorconfig
    //  C:/a/b/.editorconfig
    //  C:/a/.editorconfig
    //  C:/.editorconfig

    QString curPath = path;
    QStringList filenames;

    Q_ASSERT(!curPath.endsWith("/"));

    do {
        curPath.chop(curPath.length() - curPath.lastIndexOf("/"));
        filenames.append(curPath + "/" + filename);
    } while(curPath.length() > 0 && curPath.contains("/"));

    return filenames;
}

static int findNextUnescapedCharacter(QStringView pattern, const QChar c)
{
    int index = pattern.indexOf(c, 1);

    while (index != -1) {
        // Check if previous char is not the escape sequence
        if (pattern[index - 1] != '\\') {
            return index;
        }
        else {
            index = pattern.indexOf(c, index + 1);
        }
    }

    return index;
}

static bool isNextElementLiteral(QStringView pattern)
{
    return pattern.startsWith('\\') && pattern.size() > 1;
}

static bool isNextElementOptionalDirectory(QStringView pattern)
{
    return pattern.startsWith(QLatin1String("/**/"));
}

static bool isNextElementAll(QStringView pattern)
{
    return pattern.startsWith(QLatin1String("**"));
}

static bool isNextElementAllExceptSlash(QStringView pattern)
{
    return pattern.startsWith('*');
}

static bool isNextElementSequence(QStringView pattern)
{
    if (pattern.startsWith(QLatin1String("[!")) || pattern.startsWith(QLatin1String("["))) {
        int nextClosingBracket = findNextUnescapedCharacter(pattern, ']');

        if (nextClosingBracket != -1) {
             // NOTE: spec doesn't specify that if / appears in the sequence, then treat the entire thing as literal
            QString sequence(pattern.data(), nextClosingBracket);

            return !sequence.contains('/');
        }
    }

    return false;
}

static bool isNextElementStringList(QStringView pattern)
{
    if (pattern.startsWith(QLatin1String("{"))) {
        int nextClosingBrace = findNextUnescapedCharacter(pattern, '}');

        if (nextClosingBrace != -1) {
             // NOTE: spec doesn't specify that if ',' doesnt appears in the sequence, then treat the entire thing as literal
            QString stringList(pattern.data(), nextClosingBrace);

            return stringList.contains(',');
        }
    }

    return false;
}

static QString patternToRegex(QStringView pattern)
{
    QString regex;

    const QChar *data = pattern.data();

    while (!data->isNull()) {
        if (isNextElementLiteral(data)) {
            regex.append('\\' + data[1]);
            data += 2;
        }
        else if (isNextElementOptionalDirectory(data)) {
            regex.append("(\\/|\\/.*\\/)");
            data += 4;
        }
        else if (isNextElementAll(data)) {
            regex.append(".*?");
            data += 2;
        }
        else if (isNextElementAllExceptSlash(data)) {
            regex.append("[^/]*");
            data++;
        }
        else if (data[0] == '?') {
            regex.append("[^/]");
            data++;
        }
        else if (isNextElementSequence(data)) {
            regex.append('[');
            data++;

            if (data[0] == '!') {
                regex.append('^');
                data++;
            }

            int len = findNextUnescapedCharacter(data, ']');
            regex.append(data, len);
            data += len;

            regex.append(']');
            data++;
        }
        else if (isNextElementStringList(data)) {
            regex.append('(');
            data++;

            int len = findNextUnescapedCharacter(data, '}');

            QString rawList(data, len);

            data += len;

            QStringList stringList = rawList.split(',', QString::KeepEmptyParts);

            bool hasEmptyElement = stringList.removeAll(QString("")) > 0;

            regex.append(stringList.join('|'));

            regex.append(')');

            if (hasEmptyElement) {
                regex.append('?');
            }

            data++;
        }
        else {
            regex.append(QRegularExpression::escape(data[0]));
            data++;
        }
    }

    regex.append("$");

    return regex;
}

static bool isComment(QStringView line)
{
    return line[0] == '#' || line[0] == ';';
}

static bool isSectionHeader(QStringView line)
{
    return line.startsWith('[') && line.endsWith(']');
}

static bool isKeyValuePair(QStringView line)
{
    return line.contains('=');
}

static bool isPatternRelativeToConfigFile(QStringView pattern)
{
    // If the pattern contains an unescaped / then it is relative to the config file
    return findNextUnescapedCharacter(pattern, '/') != -1;
}

EditorConfigSettings settingsFromConfigFile(const QString &ecFilePath, const QString &absoluteFilePath)
{
    // The file may not exist
    if (!QFileInfo::exists(ecFilePath)) {
        throw EditorConfigFileNotFound();
    }

    QFile inputFile(ecFilePath);
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);

    bool isInApplicableSection = false;
    bool isInPreamble = true;
    EditorConfigSettings settings;
    QTextStream in(&inputFile);

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }
        else if (isComment(line)) {
            continue;
        }
        else if (isSectionHeader(line)) {
            // We've hit the first section so not in the preamble any more
            isInPreamble = false;

            // Drop leading [ and trailing ]
            QString sectionName = line.mid(1, line.length() - 2);

            if (sectionName.length() <= MAX_SECTION_NAME_LENGTH) {
                if (isPatternRelativeToConfigFile(sectionName)) {
                    // Make sre it starts with /
                    if (!sectionName.startsWith('/')) {
                        sectionName.prepend('/');
                    }
                }
                else {
                    // It can be anywhere within the directory
                    sectionName.prepend("/**/");
                }

                QString regexString = QRegularExpression::escape(QFileInfo(inputFile).dir().canonicalPath());
                regexString += patternToRegex(sectionName);

                QRegularExpression re(regexString);
                if (re.isValid()) {
                    isInApplicableSection = re.match(absoluteFilePath).hasMatch();
                }
                else {
                    isInApplicableSection = false;
                }
            }
            else {
                isInApplicableSection = false;
            }
        }
        else if (isKeyValuePair(line)) {
            QString key = line
                    .left(line.indexOf('='))
                    .trimmed()
                    .toLower();
            QString value = line
                    .right(line.size() - line.indexOf('=') - 1) // Extra 1 for the =
                    .trimmed();

            if (key.length() <= MAX_KEY_LENGTH && value.length() <= MAX_VALUE_LENGTH) {
                if (isInPreamble) {
                    if (key == "root") {
                        settings.insert(key, value.toLower());
                    }
                    // ignore everything else in the preamble
                }
                else if (isInApplicableSection) {
                    settings.insert(key, value);
                }
            }
        }
        else {
            // Unknown line. No sane way to recover so clear all settings and stop
            settings.clear();
            break;
        }
    }

    inputFile.close();

    return settings;
}

static EditorConfigSettings mergeMaps(QVector<EditorConfigSettings> &maps)
{
    EditorConfigSettings mergedMap;

    for (const auto &map : maps) {
        for (const auto &kv : map.keys()) {
            mergedMap.insert(kv, map[kv]);
        }
    }

    return mergedMap;
}

static void postProcessSettings(EditorConfigSettings &settings)
{
    // if indent_style == "tab" and !indent_size: indent_size = "tab"
    if (settings.contains("indent_style") && settings["indent_style"] == "tab" && !settings.contains("indent_size")) {
        settings["indent_size"] = "tab";
    }

    // if indent_size != "tab" and !tab_width: tab_width = indent_size
    if (settings.contains("indent_size") && settings["indent_size"] != "tab" && !settings.contains("tab_width")) {
        settings["tab_width"] = settings["indent_size"];
    }

    // if indent_size == "tab": indent_size = tab_width
    if (settings.contains("indent_size") && settings["indent_size"] == "tab" && settings.contains("tab_width")) {
        settings["indent_size"] = settings["tab_width"];
    }

    // Don't need the "root" key saved any more
    settings.remove("root");
}

EditorConfigSettings EditorConfig::getFileSettings(const QString &filePath, const QString &configName)
{
    const QString absoluteFilePath = QDir(filePath).absolutePath();
    QStringList locationsToCheck = getAllPossibleConfigFileLocations(absoluteFilePath, configName);
    QVector<EditorConfigSettings> individualConfigFileSettings;

    foreach (const QString &ecFilePath, locationsToCheck) {
        try {
            EditorConfigSettings configFileSettings = settingsFromConfigFile(ecFilePath, absoluteFilePath);

            // Config files higher up in the directory tree are applied first and can get overriden
            // by settings in lower (closer) config files, so prepend settings as we go up the directory tree.
            individualConfigFileSettings.prepend(configFileSettings);

            // Check to see if we are done yet
            if (configFileSettings.contains("root") && configFileSettings["root"] == "true") {
                break;
            }
        }
        catch (const EditorConfigFileNotFound &e) {
            Q_UNUSED(e);
        }
    }

    EditorConfigSettings settings = mergeMaps(individualConfigFileSettings);

    postProcessSettings(settings);

    return settings;
}
