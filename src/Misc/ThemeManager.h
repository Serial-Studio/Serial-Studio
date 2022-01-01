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

#include <QColor>
#include <QObject>
#include <QSettings>
#include <DataTypes.h>

namespace Misc
{
/**
 * @brief The ThemeManager class
 *
 * The @c ThemeManager class reads all the colors that are required to build a Serial
 * Studio theme and makes them available to the rest of the application.
 *
 * Themes are stored as JSON files in the "assets/themes" folder. The class automatically
 * builds a model with the available themes. The only requirement to create your own
 * themes is to create a JSON theme file and add it to the application resources file.
 */
class ThemeManager : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(int themeId
               READ themeId
               NOTIFY themeChanged)
    Q_PROPERTY(bool titlebarSeparator
               READ titlebarSeparator
               CONSTANT)
    Q_PROPERTY(QColor base
               READ base
               CONSTANT)
    Q_PROPERTY(QColor link
               READ link
               CONSTANT)
    Q_PROPERTY(QColor button
               READ button
               CONSTANT)
    Q_PROPERTY(QColor window
               READ window
               CONSTANT)
    Q_PROPERTY(QColor text
               READ text
               CONSTANT)
    Q_PROPERTY(QColor border
               READ border
               CONSTANT)
    Q_PROPERTY(QColor midlight
               READ midlight
               CONSTANT)
    Q_PROPERTY(QColor highlight
               READ highlight
               CONSTANT)
    Q_PROPERTY(QColor brightText
               READ brightText
               CONSTANT)
    Q_PROPERTY(QColor buttonText
               READ buttonText
               CONSTANT)
    Q_PROPERTY(QColor windowText
               READ windowText
               CONSTANT)
    Q_PROPERTY(QColor tooltipText
               READ tooltipText
               CONSTANT)
    Q_PROPERTY(QColor tooltipBase
               READ tooltipBase
               CONSTANT)
    Q_PROPERTY(QColor highlightedText
               READ highlightedText
               CONSTANT)
    Q_PROPERTY(QColor highlightedTextAlternative
               READ highlightedTextAlternative
               CONSTANT)
    Q_PROPERTY(QColor placeholderText
               READ placeholderText
               CONSTANT)
    Q_PROPERTY(QColor toolbarGradient1
               READ toolbarGradient1
               CONSTANT)
    Q_PROPERTY(QColor toolbarGradient2
               READ toolbarGradient2
               CONSTANT)
    Q_PROPERTY(QColor consoleText
               READ consoleText
               CONSTANT)
    Q_PROPERTY(QColor consoleBase
               READ consoleBase
               CONSTANT)
    Q_PROPERTY(QColor consoleButton
               READ consoleButton
               CONSTANT)
    Q_PROPERTY(QColor consoleWindow
               READ consoleWindow
               CONSTANT)
    Q_PROPERTY(QColor consoleHighlight
               READ consoleHighlight
               CONSTANT)
    Q_PROPERTY(QColor consoleHighlightedText
               READ consoleHighlightedText
               CONSTANT)
    Q_PROPERTY(QColor consolePlaceholderText
               READ consolePlaceholderText
               CONSTANT)
    Q_PROPERTY(QColor windowBackground
               READ windowBackground
               CONSTANT)
    Q_PROPERTY(QColor windowGradient1
               READ windowGradient1
               CONSTANT)
    Q_PROPERTY(QColor windowGradient2
               READ windowGradient2
               CONSTANT)
    Q_PROPERTY(QColor menubarText
               READ menubarText
               CONSTANT)
    Q_PROPERTY(QColor dialogBackground
               READ dialogBackground
               CONSTANT)
    Q_PROPERTY(QColor alternativeHighlight
               READ alternativeHighlight
               CONSTANT)
    Q_PROPERTY(QColor setupPanelBackground
               READ setupPanelBackground
               CONSTANT)
    Q_PROPERTY(QColor widgetTextPrimary
               READ widgetTextPrimary
               CONSTANT)
    Q_PROPERTY(QColor widgetTextSecondary
               READ widgetTextSecondary
               CONSTANT)
    Q_PROPERTY(QColor widgetWindowBackground
               READ widgetWindowBackground
               CONSTANT)
    Q_PROPERTY(QColor widgetWindowBorder
               READ widgetWindowBorder
               CONSTANT)
    Q_PROPERTY(QColor paneWindowBackground
               READ paneWindowBackground
               CONSTANT)
    Q_PROPERTY(QColor ledEnabled
               READ ledEnabled
               CONSTANT)
    Q_PROPERTY(QColor ledDisabled
               READ ledDisabled
               CONSTANT)
    Q_PROPERTY(QColor csvCheckbox
               READ csvCheckbox
               CONSTANT)
    Q_PROPERTY(QColor widgetForegroundPrimary
               READ widgetForegroundPrimary
               CONSTANT)
    Q_PROPERTY(QColor widgetForegroundSecondary
               READ widgetForegroundSecondary
               CONSTANT)
    Q_PROPERTY(QColor widgetIndicator
               READ widgetIndicator
               CONSTANT)
    Q_PROPERTY(QColor widgetControlBackground
               READ widgetControlBackground
               CONSTANT)
    Q_PROPERTY(QColor connectButtonChecked
               READ connectButtonChecked
               CONSTANT)
    Q_PROPERTY(QColor connectButtonUnchecked
               READ connectButtonUnchecked
               CONSTANT)
    Q_PROPERTY(QColor mqttButton
               READ mqttButton
               CONSTANT)
    Q_PROPERTY(StringList widgetColors
               READ widgetColors
               CONSTANT)
    Q_PROPERTY(StringList availableThemes
               READ availableThemes
               NOTIFY availableThemesChanged)
    Q_PROPERTY(bool customWindowDecorations
               READ customWindowDecorations
               WRITE setCustomWindowDecorations
               NOTIFY customWindowDecorationsChanged)
    // clang-format on

Q_SIGNALS:
    void themeChanged();
    void availableThemesChanged();
    void customWindowDecorationsChanged();

private:
    explicit ThemeManager();
    ThemeManager(ThemeManager &&) = delete;
    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(ThemeManager &&) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;

public:
    static ThemeManager &instance();

    int themeId() const;
    bool customWindowDecorations() const;

    bool titlebarSeparator() const;
    QColor base() const;
    QColor link() const;
    QColor button() const;
    QColor window() const;
    QColor text() const;
    QColor border() const;
    QColor midlight() const;
    QColor highlight() const;
    QColor brightText() const;
    QColor buttonText() const;
    QColor windowText() const;
    QColor tooltipText() const;
    QColor tooltipBase() const;
    QColor highlightedText() const;
    QColor highlightedTextAlternative() const;
    QColor placeholderText() const;
    QColor toolbarGradient1() const;
    QColor toolbarGradient2() const;
    QColor menubarText() const;
    QColor dialogBackground() const;
    QColor consoleText() const;
    QColor consoleBase() const;
    QColor consoleButton() const;
    QColor consoleWindow() const;
    QColor consoleHighlight() const;
    QColor consoleHighlightedText() const;
    QColor consolePlaceholderText() const;
    QColor windowBackground() const;
    QColor windowGradient1() const;
    QColor windowGradient2() const;
    QColor alternativeHighlight() const;
    QColor setupPanelBackground() const;
    QColor widgetTextPrimary() const;
    QColor widgetTextSecondary() const;
    QColor widgetWindowBackground() const;
    QColor widgetWindowBorder() const;
    QColor paneWindowBackground() const;
    QColor ledEnabled() const;
    QColor ledDisabled() const;
    QColor csvCheckbox() const;
    QColor widgetForegroundPrimary() const;
    QColor widgetForegroundSecondary() const;
    QColor widgetIndicator() const;
    QColor widgetControlBackground() const;
    QColor connectButtonChecked() const;
    QColor connectButtonUnchecked() const;
    QColor mqttButton() const;

    StringList widgetColors() const;
    StringList availableThemes() const;

public Q_SLOTS:
    void setTheme(const int id);
    void setCustomWindowDecorations(const bool enabled);

private Q_SLOTS:
    void populateThemes();
    void loadTheme(const int id);

private:
    int m_themeId;
    bool m_customWindowDecorations;

    QSettings m_settings;
    bool m_titlebarSeparator;
    StringList m_availableThemes;
    StringList m_availableThemesPaths;

    QColor m_base;
    QColor m_link;
    QColor m_button;
    QColor m_window;
    QColor m_text;
    QColor m_border;
    QColor m_midlight;
    QColor m_highlight;
    QColor m_brightText;
    QColor m_buttonText;
    QColor m_windowText;
    QColor m_tooltipText;
    QColor m_tooltipBase;
    QColor m_highlightedText;
    QColor m_highlightedTextAlternative;
    QColor m_placeholderText;
    QColor m_toolbarGradient1;
    QColor m_toolbarGradient2;
    QColor m_menubarText;
    QColor m_dialogBackground;
    QColor m_consoleText;
    QColor m_consoleBase;
    QColor m_consoleButton;
    QColor m_consoleWindow;
    QColor m_consoleHighlight;
    QColor m_consoleHighlightedText;
    QColor m_consolePlaceholderText;
    QColor m_windowBackground;
    QColor m_windowGradient1;
    QColor m_windowGradient2;
    QColor m_alternativeHighlight;
    QColor m_setupPanelBackground;
    QColor m_widgetTextPrimary;
    QColor m_widgetTextSecondary;
    QColor m_widgetWindowBackground;
    QColor m_widgetWindowBorder;
    QColor m_paneWindowBackground;
    QColor m_ledEnabled;
    QColor m_ledDisabled;
    QColor m_csvCheckbox;
    QColor m_widgetForegroundPrimary;
    QColor m_widgetForegroundSecondary;
    QColor m_widgetIndicator;
    QColor m_widgetControlBackground;
    QColor m_connectButtonChecked;
    QColor m_connectButtonUnchecked;
    QColor m_mqttButton;
    StringList m_widgetColors;
};
}

inline QString QSS(const QString &style, const QColor &color)
{
    return QString(style).arg(color.name());
}

inline QString QSS(const QString &style, const QColor &c1, const QColor &c2)
{
    return QString(style).arg(c1.name(), c2.name());
}

inline QString QSS(const QString &style, const QColor &c1, const QColor &c2,
                   const QColor &c3)
{
    return QString(style).arg(c1.name(), c2.name(), c3.name());
}
