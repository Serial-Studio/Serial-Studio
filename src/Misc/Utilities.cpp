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

#include "Utilities.h"

#include <QDir>
#include <QUrl>
#include <QPalette>
#include <QProcess>
#include <QFileInfo>
#include <QQuickStyle>
#include <QMessageBox>
#include <QApplication>
#include <QStyleFactory>
#include <QDesktopServices>

#include <AppInfo.h>

using namespace Misc;

/**
 * Returns the onlt instance of the class, this is to be used by the QML interface
 */
static Utilities *INSTANCE = nullptr;
Utilities *Utilities::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Utilities;

    return INSTANCE;
}

/**
 * Shows a macOS-like message box with the given properties
 */
int Utilities::showMessageBox(QString text, QString informativeText, QString windowTitle,
                              QMessageBox::StandardButtons bt)
{
    // Get app icon
    auto icon = QPixmap(APP_ICON).scaled(64, 64, Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);

    // Create message box & set options
    QMessageBox box;
    box.setIconPixmap(icon);
    box.setStandardButtons(bt);
    box.setWindowTitle(windowTitle);
    box.setText("<h3>" + text + "</h3>");
    box.setInformativeText(informativeText);

    // Show message box & return user decision to caller
    return box.exec();
}

/**
 * Displays the about Qt dialog
 */
void Utilities::aboutQt()
{
    qApp->aboutQt();
}

/**
 * Displays the location of the current log file in the Finder window
 */
void Utilities::openLogFile()
{
    revealFile(LOG_FILE);
}

/**
 * Changes the application palette so that a dark UI can be displayed
 */
void Utilities::configureDarkUi()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QQuickStyle::setStyle("Fusion");

    // clang-format off
    QPalette palette;
    palette.setColor(QPalette::Base,            QColor("#21373f"));
    palette.setColor(QPalette::Link,            QColor("#409da0"));
    palette.setColor(QPalette::Button,          QColor("#21373f"));
    palette.setColor(QPalette::Window,          QColor("#21373f"));
    palette.setColor(QPalette::Text,            QColor("#ffffff"));
    palette.setColor(QPalette::Midlight,        QColor("#0e1419"));
    palette.setColor(QPalette::Highlight,       QColor("#409da0"));
    palette.setColor(QPalette::BrightText,      QColor("#ffffff"));
    palette.setColor(QPalette::ButtonText,      QColor("#ffffff"));
    palette.setColor(QPalette::WindowText,      QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipBase,     QColor("#e6e0b2"));
    palette.setColor(QPalette::ToolTipText,     QColor("#e6e0b2"));
    palette.setColor(QPalette::HighlightedText, QColor("#e6e0b2"));
    qApp->setPalette(palette);
    // clang-format on
}

/**
 * Reveals the file contained in @a pathToReveal in Explorer/Finder.
 * On GNU/Linux, this function shall open the file directly with the desktop
 * services.
 *
 * Hacking details:
 * http://stackoverflow.com/questions/3490336/how-to-reveal-in-finder-or-show-in-explorer-with-qt
 */
void Utilities::revealFile(const QString &pathToReveal)
{
#if defined(Q_OS_WIN)
    QStringList param;
    const QFileInfo fileInfo(pathToReveal);
    if (!fileInfo.isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
    QProcess::startDetached("explorer.exe", param);
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1(
                      "tell application \"Finder\" to reveal POSIX file \"%1\"")
                      .arg(pathToReveal);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(pathToReveal));
#endif
}
