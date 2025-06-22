/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include*
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

/**
 * @brief Constructor for NativeWindow class.
 * @param parent The parent QObject.
 *
 * Connects theme change signals to the appropriate slot for handling UI theme
 * updates.
 */
NativeWindow::NativeWindow(QObject *parent)
  : QObject(parent)
{
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &NativeWindow::onThemeChanged);
}

/**
 * @brief Retrieves the height of the title bar.
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  (void)window;
  return 0;
}

/**
 * @brief Configures the native window customization for *NIX environments.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  (void)window;
  (void)color;
}

/**
 * @brief Handles theme change events.
 */
void NativeWindow::onThemeChanged()
{
  // Nothing to do...
}

/**
 * @brief Handles the active state change of a window.
 */
void NativeWindow::onActiveChanged()
{
  // Nothing to do...
}
