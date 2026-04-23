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

#include <QMap>
#include <QObject>
#include <QWindow>

/**
 * @brief Provides native window customization features across operating systems.
 */
class NativeWindow : public QObject {
  Q_OBJECT

signals:
  void quitRequested();

public:
  explicit NativeWindow(QObject* parent = nullptr);

  void installMacOSQuitInterceptor();
  [[nodiscard]] Q_INVOKABLE int titlebarHeight(QObject* window);

public slots:
  void removeWindow(QObject* window);
  void addWindow(QObject* window, const QString& color = "");

private slots:
  void onThemeChanged();
  void onActiveChanged();
  void onWindowStateChanged(Qt::WindowState state);

private:
  QList<QWindow*> m_windows;
  QMap<QWindow*, QString> m_colors;
};
