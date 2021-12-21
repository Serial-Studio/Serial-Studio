/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QLocale>
#include <QObject>
#include <QTranslator>
#include <DataTypes.h>

#ifdef QT_QML_LIB
#    include <QtQml>
#endif

namespace Misc
{
/**
 * @brief The Translator class
 *
 * The @c Translator module provides the user interface with a list of available
 * translations, and loads the specified translation file during application startup or
 * when the user changes the language of the application.
 */
class Translator : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(int language
               READ language
               WRITE setLanguage
               NOTIFY languageChanged)
    Q_PROPERTY(StringList availableLanguages
               READ availableLanguages
               CONSTANT)
    // clang-format on

Q_SIGNALS:
    void languageChanged();

private:
    explicit Translator();
    Translator(Translator &&) = delete;
    Translator(const Translator &) = delete;
    Translator &operator=(Translator &&) = delete;
    Translator &operator=(const Translator &) = delete;

public:
    static Translator &instance();

    int language() const;
    int systemLanguage() const;
    StringList availableLanguages() const;
    Q_INVOKABLE QString welcomeConsoleText() const;
    Q_INVOKABLE QString acknowledgementsText() const;

public Q_SLOTS:
    void setLanguage(const int language);
    void setLanguage(const QLocale &locale, const QString &language);

private:
    int m_language;
    QSettings m_settings;
    QTranslator m_translator;
};
}
