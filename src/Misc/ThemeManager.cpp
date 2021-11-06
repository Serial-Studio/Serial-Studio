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

#include "AppInfo.h"
#include "Utilities.h"
#include "ThemeManager.h"

#include <QDir>
#include <QFile>
#include <QPalette>
#include <QProcess>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QJsonDocument>

namespace Misc
{
static ThemeManager *THEME_MANAGER = Q_NULLPTR;

/**
 * Constructor function, searches for available themes & loads
 * the theme variant selected by the user.
 */
ThemeManager::ThemeManager()
{
    populateThemes();
    loadTheme(m_settings.value("themeId", 0).toInt());
}

/**
 * Returns a pointer to the only instance of this class
 */
ThemeManager *ThemeManager::getInstance()
{
    if (!THEME_MANAGER)
        THEME_MANAGER = new ThemeManager;

    return THEME_MANAGER;
}

/**
 * Returns the ID of the theme that the user has selected.
 */
int ThemeManager::themeId() const
{
    return m_themeId;
}

/**
 * Updates the theme ID to be used & saves the changes to the
 * application settings.
 *
 * Finally, this function prompts the user to restart the application
 * in order to apply changes.
 *
 * Unfortunately, an app restart is required because the application
 * palette must be set before the GUI is initialized.
 */
void ThemeManager::setTheme(const int id)
{
    // Validate theme ID
    if (id >= m_availableThemesPaths.count())
        return;

    // Save settings for next run
    m_themeId = id;
    m_settings.setValue("themeId", m_themeId);

    // Ask user to quit application
    // clang-format off
    auto ans = Utilities::showMessageBox(
                tr("The theme change will take effect after restart"),
                tr("Do you want to restart %1 now?").arg(APP_NAME), APP_NAME,
                QMessageBox::Yes | QMessageBox::No);
    // clang-format on

    // Restart application
    if (ans == QMessageBox::Yes)
    {
#ifdef Q_OS_MAC
        auto bundle = qApp->applicationDirPath() + "/../../";
        QProcess::startDetached("open", { "-n", "-a", bundle });
#else
        QProcess::startDetached(qApp->applicationFilePath());
#endif
        qApp->exit();
    }
}

/**
 * Parses the JSON theme definition file for the given theme ID.
 * The colors are then "extracted" from the JSON file & loaded into the
 * class, which is later used to set the colors of the QML user interface.
 */
void ThemeManager::loadTheme(const int id)
{
    // Validate theme ID
    if (id >= m_availableThemesPaths.count())
        return;

    // Open theme
    QFile file(m_availableThemesPaths.at(id));
    if (!file.open(QFile::ReadOnly))
        return;

    // Read theme data into JSON
    auto document = QJsonDocument::fromJson(file.readAll());
    if (document.isEmpty())
        return;

    // Get colors object
    auto colors = document.object().value("colors").toObject();
    if (colors.isEmpty())
        return;

    // Read colors from JSON file
    // clang-format off
    m_base = QColor(colors.value("base").toString());
    m_link = QColor(colors.value("link").toString());
    m_button = QColor(colors.value("button").toString());
    m_window = QColor(colors.value("window").toString());
    m_text = QColor(colors.value("text").toString());
    m_midlight = QColor(colors.value("midlight").toString());
    m_highlight = QColor(colors.value("highlight").toString());
    m_brightText = QColor(colors.value("brightText").toString());
    m_buttonText = QColor(colors.value("buttonText").toString());
    m_windowText = QColor(colors.value("windowText").toString());
    m_tooltipText = QColor(colors.value("tooltipText").toString());
    m_tooltipBase = QColor(colors.value("tooltipBase").toString());
    m_highlightedText = QColor(colors.value("highlightedText").toString());
    m_highlightedTextAlternative = QColor(colors.value("highlightedTextAlternative").toString());
    m_placeholderText = QColor(colors.value("placeholderText").toString());
    m_toolbarGradient1 = QColor(colors.value("toolbarGradient1").toString());
    m_toolbarGradient2 = QColor(colors.value("toolbarGradient2").toString());
    m_consoleText = QColor(colors.value("consoleText").toString());
    m_consoleBase = QColor(colors.value("consoleBase").toString());
    m_consoleButton = QColor(colors.value("consoleButton").toString());
    m_consoleWindow = QColor(colors.value("consoleWindow").toString());
    m_consoleHighlight = QColor(colors.value("consoleHighlight").toString());
    m_consoleHighlightedText = QColor(colors.value("consoleHighlightedText").toString());
    m_consolePlaceholderText = QColor(colors.value("consolePlaceholderText").toString());
    m_windowBackground = QColor(colors.value("windowBackground").toString());
    m_windowGradient1 = QColor(colors.value("windowGradient1").toString());
    m_windowGradient2 = QColor(colors.value("windowGradient2").toString());
    m_menubarGradient1 = QColor(colors.value("menubarGradient1").toString());
    m_menubarGradient2 = QColor(colors.value("menubarGradient2").toString());
    m_menubarText = QColor(colors.value("menubarText").toString());
    m_dialogBackground = QColor(colors.value("dialogBackground").toString());
    m_alternativeHighlight = QColor(colors.value("alternativeHighlight").toString());
    m_setupPanelBackground = QColor(colors.value("setupPanelBackground").toString());
    m_widgetTextPrimary = QColor(colors.value("widgetTextPrimary").toString());
    m_widgetTextSecondary = QColor(colors.value("widgetTextSecondary").toString());
    m_widgetWindowBackground = QColor(colors.value("widgetWindowBackground").toString());
    m_widgetWindowBorder = QColor(colors.value("widgetWindowBorder").toString());
    m_paneWindowBackground = QColor(colors.value("paneWindowBackground").toString());
    m_ledEnabled = QColor(colors.value("ledEnabled").toString());
    m_ledDisabled = QColor(colors.value("ledDisabled").toString());
    m_csvCheckbox = QColor(colors.value("csvCheckbox").toString());
    m_widgetForegroundPrimary = QColor(colors.value("widgetForegroundPrimary").toString());
    m_widgetForegroundSecondary = QColor(colors.value("widgetForegroundSecondary").toString());
    m_widgetIndicator = QColor(colors.value("widgetIndicator").toString());
    m_widgetControlBackground = QColor(colors.value("widgetControlBackground").toString());
    m_connectButtonChecked = QColor(colors.value("connectButtonChecked").toString());
    m_connectButtonUnchecked = QColor(colors.value("connectButtonUnchecked").toString());
    // clang-format on

    // Read bar widget colors
    m_widgetColors.clear();
    auto list = colors.value("widgetColors").toArray();
    for (int i = 0; i < list.count(); ++i)
        m_widgetColors.append(list.at(i).toString());

    // Update application palette
    QPalette palette;
    palette.setColor(QPalette::Base, base());
    palette.setColor(QPalette::Link, link());
    palette.setColor(QPalette::Text, text());
    palette.setColor(QPalette::Button, button());
    palette.setColor(QPalette::Window, window());
    palette.setColor(QPalette::Midlight, midlight());
    palette.setColor(QPalette::Highlight, highlight());
    palette.setColor(QPalette::BrightText, brightText());
    palette.setColor(QPalette::ButtonText, buttonText());
    palette.setColor(QPalette::WindowText, windowText());
    palette.setColor(QPalette::ToolTipBase, tooltipBase());
    palette.setColor(QPalette::ToolTipText, tooltipText());
    palette.setColor(QPalette::HighlightedText, highlightedText());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    palette.setColor(QPalette::PlaceholderText, placeholderText());
#endif
    qApp->setPalette(palette);

    // Update user interface
    m_themeId = id;
    emit themeChanged();
}

/**
 * Reads all the available theme files from the application resources
 * folder.
 * @note theme definitions are bundled during the compilatopn process.
 */
void ThemeManager::populateThemes()
{
    // Clear available thems
    m_availableThemes.clear();
    m_availableThemesPaths.clear();

    // Scan themes directory & get list of files
    auto themeList = QDir(":/themes").entryList();

    // Open each JSON file & get theme names
    for (int i = 0; i < themeList.count(); ++i)
    {
        QFile file(QString(":/themes/%1").arg(themeList.at(i)));
        if (file.open(QFile::ReadOnly))
        {
            auto data = file.readAll();
            file.close();

            auto document = QJsonDocument::fromJson(data);
            auto name = document.object().value("name").toString();
            if (!name.isEmpty())
            {
                m_availableThemes.append(name);
                m_availableThemesPaths.append(file.fileName());
            }
        }
    }

    // Update UI
    emit availableThemesChanged();
}

//--------------------------------------------------------------------------------------------------
// Dumb access functions
//--------------------------------------------------------------------------------------------------

QColor ThemeManager::base() const
{
    return m_base;
}

QColor ThemeManager::link() const
{
    return m_link;
}

QColor ThemeManager::button() const
{
    return m_button;
}

QColor ThemeManager::window() const
{
    return m_window;
}

QColor ThemeManager::text() const
{
    return m_text;
}

QColor ThemeManager::midlight() const
{
    return m_midlight;
}

QColor ThemeManager::highlight() const
{
    return m_highlight;
}

QColor ThemeManager::brightText() const
{
    return m_brightText;
}

QColor ThemeManager::buttonText() const
{
    return m_buttonText;
}

QColor ThemeManager::windowText() const
{
    return m_windowText;
}

QColor ThemeManager::tooltipText() const
{
    return m_tooltipText;
}

QColor ThemeManager::tooltipBase() const
{
    return m_tooltipBase;
}

QColor ThemeManager::highlightedText() const
{
    return m_highlightedText;
}

QColor ThemeManager::highlightedTextAlternative() const
{
    return m_highlightedTextAlternative;
}

QColor ThemeManager::placeholderText() const
{
    return m_placeholderText;
}

QColor ThemeManager::toolbarGradient1() const
{
    return m_toolbarGradient1;
}

QColor ThemeManager::toolbarGradient2() const
{
    return m_toolbarGradient2;
}

QColor ThemeManager::menubarGradient1() const
{
    return m_menubarGradient1;
}

QColor ThemeManager::menubarGradient2() const
{
    return m_menubarGradient2;
}

QColor ThemeManager::menubarText() const
{
    return m_menubarText;
}

QColor ThemeManager::dialogBackground() const
{
    return m_dialogBackground;
}

QColor ThemeManager::consoleText() const
{
    return m_consoleText;
}

QColor ThemeManager::consoleBase() const
{
    return m_consoleBase;
}

QColor ThemeManager::consoleButton() const
{
    return m_consoleButton;
}

QColor ThemeManager::consoleWindow() const
{
    return m_consoleWindow;
}

QColor ThemeManager::consoleHighlight() const
{
    return m_consoleHighlight;
}

QColor ThemeManager::consoleHighlightedText() const
{
    return m_consoleHighlightedText;
}

QColor ThemeManager::consolePlaceholderText() const
{
    return m_consolePlaceholderText;
}

QColor ThemeManager::windowBackground() const
{
    return m_windowBackground;
}

QColor ThemeManager::windowGradient1() const
{
    return m_windowGradient1;
}

QColor ThemeManager::windowGradient2() const
{
    return m_windowGradient2;
}

QColor ThemeManager::alternativeHighlight() const
{
    return m_alternativeHighlight;
}

QColor ThemeManager::setupPanelBackground() const
{
    return m_setupPanelBackground;
}

QColor ThemeManager::widgetTextPrimary() const
{
    return m_widgetTextPrimary;
}

QColor ThemeManager::widgetTextSecondary() const
{
    return m_widgetTextSecondary;
}

QColor ThemeManager::widgetWindowBackground() const
{
    return m_widgetWindowBackground;
}

QColor ThemeManager::widgetWindowBorder() const
{
    return m_widgetWindowBorder;
}

QColor ThemeManager::paneWindowBackground() const
{
    return m_paneWindowBackground;
}

QColor ThemeManager::ledEnabled() const
{
    return m_ledEnabled;
}

QColor ThemeManager::ledDisabled() const
{
    return m_ledDisabled;
}

QColor ThemeManager::csvCheckbox() const
{
    return m_csvCheckbox;
}

QColor ThemeManager::widgetForegroundPrimary() const
{
    return m_widgetForegroundPrimary;
}

QColor ThemeManager::widgetForegroundSecondary() const
{
    return m_widgetForegroundSecondary;
}

QColor ThemeManager::widgetIndicator() const
{
    return m_widgetIndicator;
}

QColor ThemeManager::widgetControlBackground() const
{
    return m_widgetControlBackground;
}

QColor ThemeManager::connectButtonChecked() const
{
    return m_connectButtonChecked;
}

QColor ThemeManager::connectButtonUnchecked() const
{
    return m_connectButtonUnchecked;
}

StringList ThemeManager::widgetColors() const
{
    return m_widgetColors;
}

StringList ThemeManager::availableThemes() const
{
    return m_availableThemes;
}
}
