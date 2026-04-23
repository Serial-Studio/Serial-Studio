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

#include <QApplication>
#include <QMessageBox>
#include <QObject>

typedef QMap<QMessageBox::StandardButton, QString> ButtonTextMap;

namespace Misc {
/**
 * @brief Application-wide utility functions for common UI and system operations.
 */
class Utilities : public QObject {
  Q_OBJECT

public:
  [[nodiscard]] static Utilities& instance();
  static void rebootApplication();
  static QPixmap getHiDpiPixmap(const QString& path);

  [[nodiscard]] Q_INVOKABLE static bool askAutomaticUpdates();
  [[nodiscard]] Q_INVOKABLE static QString hdpiImagePath(const QString& path);

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
  static void copyText(const QString& text);
  static void revealFile(const QString& pathToReveal);
};
}  // namespace Misc
