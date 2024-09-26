/*
 * Copyright (C) Matusica S.A. de C.V. - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Written by Alex Spataru <https://github.com/alex-spataru>, March 2024
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
