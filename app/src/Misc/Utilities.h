/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QMessageBox>
#include <QApplication>

typedef QMap<QMessageBox::StandardButton, QString> ButtonTextMap;

namespace Misc
{
/**
 * @brief The Utilities class
 *
 * The @c Utilitities module provides commonly used functionality to the rest of
 * the application. For example, showing a messagebox with a nice format or
 * revealing files in the operating system's preffered file manager.
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
                            QMessageBox::Icon icon = QMessageBox::NoIcon,
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
