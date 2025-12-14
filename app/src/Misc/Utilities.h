/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
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

#pragma once

#include <QObject>
#include <QMessageBox>
#include <QApplication>

typedef QMap<QMessageBox::StandardButton, QString> ButtonTextMap;

namespace Misc
{
/**
 * @class Misc::Utilities
 * @brief Application-wide utility functions for common UI and system
 * operations.
 *
 * The Utilities class provides a collection of static helper functions for
 * operations that are used throughout the application. It centralizes common
 * functionality to ensure consistent behavior and reduce code duplication.
 *
 * Key Features:
 * - **Message Dialogs**: Standardized, well-formatted message boxes with
 *   custom button text support
 * - **File Revelation**: Cross-platform file manager integration to reveal
 *   files in Explorer/Finder/File Manager
 * - **HiDPI Support**: Automatic pixel-density-aware image loading
 * - **Application Reboot**: Clean application restart functionality
 * - **Qt About Dialog**: Shows Qt framework information
 * - **Auto-Update Prompts**: User consent dialogs for automatic updates
 *
 * Platform Support:
 * - **Windows**: Uses Explorer for file revelation
 * - **macOS**: Uses Finder for file revelation
 * - **Linux**: Uses xdg-open or fallback file managers
 *
 * All functions are static and can be called without instantiating the class.
 * The singleton instance() method exists primarily for QML property exposure.
 *
 * @note Most methods are static utilities; instance() is provided for QML
 *       integration purposes.
 */
class Utilities : public QObject
{
  Q_OBJECT

public:
  static Utilities &instance();
  static void rebootApplication();
  static QPixmap getHiDpiPixmap(const QString &path);

  Q_INVOKABLE static bool askAutomaticUpdates();
  Q_INVOKABLE static QString hdpiImagePath(const QString &path);

  // clang-format off
  static int showMessageBox(const QString& text,
                            const QString& informativeText = "",
                            QMessageBox::Icon icon = QMessageBox::Information,
                            const QString& windowTitle = "",
                            QMessageBox::StandardButtons bt = QMessageBox::Ok,
                            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
                            const ButtonTextMap& buttonTexts = ButtonTextMap());
  // clang-format on

public slots:
  static void aboutQt();
  static void revealFile(const QString &pathToReveal);
};
} // namespace Misc
