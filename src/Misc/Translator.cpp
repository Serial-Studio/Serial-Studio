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

#include <Misc/Translator.h>

/**
 * Constructor function
 */
Misc::Translator::Translator()
{
    setLanguage(m_settings.value("language", systemLanguage()).toInt());
}

/**
 * Returns the only instance of the class
 */
Misc::Translator &Misc::Translator::instance()
{
    static Translator singleton;
    return singleton;
}

/**
 * Returns the current language ID, which corresponds to the indexes of the
 * languages returned by the \c availableLanguages() function.
 */
int Misc::Translator::language() const
{
    return m_language;
}

/**
 * Returns the appropiate language ID based on the current locale settings of
 * the host's operating system.
 */
int Misc::Translator::systemLanguage() const
{
    int lang;
    switch (QLocale::system().language())
    {
        case QLocale::English:
            lang = 0;
            break;
        case QLocale::Spanish:
            lang = 1;
            break;
        case QLocale::Chinese:
            lang = 2;
            break;
        case QLocale::German:
            lang = 3;
            break;
        case QLocale::Russian:
            lang = 4;
            break;
        default:
            lang = 0;
            break;
    }

    return lang;
}

/**
 * Returns the welcome text displayed on the console
 */
QString Misc::Translator::welcomeConsoleText() const
{
    QString lang;
    switch (language())
    {
        case 0:
            lang = "EN";
            break;
        case 1:
            lang = "ES";
            break;
        case 2:
            lang = "ZH";
            break;
        case 3:
            lang = "DE";
            break;
        case 4:
            lang = "RU";
            break;
        default:
            lang = "EN";
            break;
    }

    QString text = QObject::tr("Failed to load welcome text :(");
    QFile file(":/messages/Welcome_" + lang + ".txt");
    if (file.open(QFile::ReadOnly))
    {
        text = QString::fromUtf8(file.readAll());
        file.close();
    }

    return text;
}

/**
 * Returns the acknowledgements text.
 */
QString Misc::Translator::acknowledgementsText() const
{
    QString text = "";
    QFile file(":/messages/Acknowledgements.txt");
    if (file.open(QFile::ReadOnly))
    {
        text = QString::fromUtf8(file.readAll());
        file.close();
    }

    return text;
}

/**
 * Returns a list with the available translation languages.
 */
StringList Misc::Translator::availableLanguages() const
{
    return StringList { "English", "Español", "简体中文", "Deutsch", "Русский" };
}

/**
 * Changes the language of the application and emits the signals appropiate to
 * reload every string that uses the Qt translator system.
 *
 * @param language language ID based on the indexes of the \a
 * availableLanguages() function
 */
void Misc::Translator::setLanguage(const int language)
{
    QString langName;
    QLocale locale;
    switch (language)
    {
        case 0:
            langName = "en";
            locale = QLocale(QLocale::English);
            break;
        case 1:
            langName = "es";
            locale = QLocale(QLocale::Spanish);
            break;
        case 2:
            langName = "zh";
            locale = QLocale(QLocale::Chinese);
            break;
        case 3:
            langName = "de";
            locale = QLocale(QLocale::German);
            break;
        case 4:
            langName = "ru";
            locale = QLocale(QLocale::Russian);
            break;
        default:
            langName = "en";
            locale = QLocale(QLocale::English);
            break;
    }

    m_language = language;
    m_settings.setValue("language", m_language);

    setLanguage(locale, langName);
}

/**
 * Changes the language of the application and emits the signals neccesary to
 * reload every string that uses the Qt translator system.
 *
 * @param locale    user-set locale
 * @param language  name of the *.qm file to load from the "translations"
 *                  directory inside the application's resources
 */
void Misc::Translator::setLanguage(const QLocale &locale, const QString &language)
{
    qApp->removeTranslator(&m_translator);
    if (m_translator.load(locale, ":/translations/" + language + ".qm"))
    {
        qApp->installTranslator(&m_translator);
        Q_EMIT languageChanged();
    }
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Translator.cpp"
#endif
