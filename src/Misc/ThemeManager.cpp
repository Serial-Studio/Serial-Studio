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

#include <QDir>
#include <QFile>
#include <QPalette>
#include <QProcess>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QJsonDocument>

#include <AppInfo.h>
#include <Misc/Utilities.h>
#include <Misc/ThemeManager.h>

/**
 * Constructor function, searches for available themes & loads
 * the theme variant selected by the user.
 */
Misc::ThemeManager::ThemeManager()
{
    populateThemes();
    loadTheme(m_settings.value("themeId", 0).toInt());

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    setCustomWindowDecorations(m_settings.value("customWindows", false).toBool());
#else
    setCustomWindowDecorations(m_settings.value("customWindows", true).toBool());
#endif
}

/**
 * Returns a pointer to the only instance of this class
 */
Misc::ThemeManager &Misc::ThemeManager::instance()
{
    static ThemeManager singleton;
    return singleton;
}

/**
 * Returns the ID of the theme that the user has selected.
 */
int Misc::ThemeManager::themeId() const
{
    return m_themeId;
}

/**
 * Returns @c true if the application should draw the window decorations & controls by
 * itself. This feature makes it look cooler, but it can lead to some trouble on
 * not-so-common desktop environments, such as CDE.
 */
bool Misc::ThemeManager::customWindowDecorations() const
{
    return m_customWindowDecorations;
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
void Misc::ThemeManager::setTheme(const int id)
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
        Utilities::rebootApplication();
}

/**
 * Enables/disables the custom window feature. See the @c customWindowDecorations()
 * function for more information.
 */
void Misc::ThemeManager::setCustomWindowDecorations(const bool enabled)
{
    m_customWindowDecorations = enabled;
    m_settings.setValue("customWindows", enabled);
    Q_EMIT customWindowDecorationsChanged();
}

/**
 * Parses the JSON theme definition file for the given theme ID.
 * The colors are then "extracted" from the JSON file & loaded into the
 * class, which is later used to set the colors of the QML user interface.
 */
void Misc::ThemeManager::loadTheme(const int id)
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
    m_titlebarSeparator = document.object().value("titlebarSeparator").toBool();
    m_base = QColor(colors.value("base").toString());
    m_link = QColor(colors.value("link").toString());
    m_button = QColor(colors.value("button").toString());
    m_window = QColor(colors.value("window").toString());
    m_text = QColor(colors.value("text").toString());
    m_border = QColor(colors.value("border").toString());
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
    m_mqttButton = QColor(colors.value("mqttButton").toString());
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
    Q_EMIT themeChanged();
}

/**
 * Reads all the available theme files from the application resources
 * folder.
 * @note theme definitions are bundled during the compilatopn process.
 */
void Misc::ThemeManager::populateThemes()
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
    Q_EMIT availableThemesChanged();
}

//----------------------------------------------------------------------------------------
// Dumb access functions
//----------------------------------------------------------------------------------------

bool Misc::ThemeManager::titlebarSeparator() const
{
    return m_titlebarSeparator;
}

QColor Misc::ThemeManager::base() const
{
    return m_base;
}

QColor Misc::ThemeManager::link() const
{
    return m_link;
}

QColor Misc::ThemeManager::button() const
{
    return m_button;
}

QColor Misc::ThemeManager::window() const
{
    return m_window;
}

QColor Misc::ThemeManager::text() const
{
    return m_text;
}

QColor Misc::ThemeManager::border() const
{
    return m_border;
}

QColor Misc::ThemeManager::midlight() const
{
    return m_midlight;
}

QColor Misc::ThemeManager::highlight() const
{
    return m_highlight;
}

QColor Misc::ThemeManager::brightText() const
{
    return m_brightText;
}

QColor Misc::ThemeManager::buttonText() const
{
    return m_buttonText;
}

QColor Misc::ThemeManager::windowText() const
{
    return m_windowText;
}

QColor Misc::ThemeManager::tooltipText() const
{
    return m_tooltipText;
}

QColor Misc::ThemeManager::tooltipBase() const
{
    return m_tooltipBase;
}

QColor Misc::ThemeManager::highlightedText() const
{
    return m_highlightedText;
}

QColor Misc::ThemeManager::highlightedTextAlternative() const
{
    return m_highlightedTextAlternative;
}

QColor Misc::ThemeManager::placeholderText() const
{
    return m_placeholderText;
}

QColor Misc::ThemeManager::toolbarGradient1() const
{
    return m_toolbarGradient1;
}

QColor Misc::ThemeManager::toolbarGradient2() const
{
    return m_toolbarGradient2;
}

QColor Misc::ThemeManager::menubarText() const
{
    return m_menubarText;
}

QColor Misc::ThemeManager::dialogBackground() const
{
    return m_dialogBackground;
}

QColor Misc::ThemeManager::consoleText() const
{
    return m_consoleText;
}

QColor Misc::ThemeManager::consoleBase() const
{
    return m_consoleBase;
}

QColor Misc::ThemeManager::consoleButton() const
{
    return m_consoleButton;
}

QColor Misc::ThemeManager::consoleWindow() const
{
    return m_consoleWindow;
}

QColor Misc::ThemeManager::consoleHighlight() const
{
    return m_consoleHighlight;
}

QColor Misc::ThemeManager::consoleHighlightedText() const
{
    return m_consoleHighlightedText;
}

QColor Misc::ThemeManager::consolePlaceholderText() const
{
    return m_consolePlaceholderText;
}

QColor Misc::ThemeManager::windowBackground() const
{
    return m_windowBackground;
}

QColor Misc::ThemeManager::windowGradient1() const
{
    return m_windowGradient1;
}

QColor Misc::ThemeManager::windowGradient2() const
{
    return m_windowGradient2;
}

QColor Misc::ThemeManager::alternativeHighlight() const
{
    return m_alternativeHighlight;
}

QColor Misc::ThemeManager::setupPanelBackground() const
{
    return m_setupPanelBackground;
}

QColor Misc::ThemeManager::widgetTextPrimary() const
{
    return m_widgetTextPrimary;
}

QColor Misc::ThemeManager::widgetTextSecondary() const
{
    return m_widgetTextSecondary;
}

QColor Misc::ThemeManager::widgetWindowBackground() const
{
    return m_widgetWindowBackground;
}

QColor Misc::ThemeManager::widgetWindowBorder() const
{
    return m_widgetWindowBorder;
}

QColor Misc::ThemeManager::paneWindowBackground() const
{
    return m_paneWindowBackground;
}

QColor Misc::ThemeManager::ledEnabled() const
{
    return m_ledEnabled;
}

QColor Misc::ThemeManager::ledDisabled() const
{
    return m_ledDisabled;
}

QColor Misc::ThemeManager::csvCheckbox() const
{
    return m_csvCheckbox;
}

QColor Misc::ThemeManager::widgetForegroundPrimary() const
{
    return m_widgetForegroundPrimary;
}

QColor Misc::ThemeManager::widgetForegroundSecondary() const
{
    return m_widgetForegroundSecondary;
}

QColor Misc::ThemeManager::widgetIndicator() const
{
    return m_widgetIndicator;
}

QColor Misc::ThemeManager::widgetControlBackground() const
{
    return m_widgetControlBackground;
}

QColor Misc::ThemeManager::connectButtonChecked() const
{
    return m_connectButtonChecked;
}

QColor Misc::ThemeManager::connectButtonUnchecked() const
{
    return m_connectButtonUnchecked;
}

QColor Misc::ThemeManager::mqttButton() const
{
    return m_mqttButton;
}

StringList Misc::ThemeManager::widgetColors() const
{
    return m_widgetColors;
}

StringList Misc::ThemeManager::availableThemes() const
{
    return m_availableThemes;
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_ThemeManager.cpp"
#endif
