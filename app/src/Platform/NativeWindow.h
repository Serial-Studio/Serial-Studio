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
#include <QSettings>
#include <QWindow>

/**
 * @brief Provides native window customization features across operating systems.
 */
class NativeWindow : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool csdAvailable
             READ csdAvailable
             CONSTANT)
  Q_PROPERTY(bool csdEnabled
             READ csdEnabled
             WRITE setCsdEnabled
             NOTIFY csdEnabledChanged)
  Q_PROPERTY(bool csdShadowEnabled
             READ csdShadowEnabled
             WRITE setCsdShadowEnabled
             NOTIFY csdShadowEnabledChanged)
  // clang-format on

signals:
  void quitRequested();
  void csdEnabledChanged();
  void csdShadowEnabledChanged();

public:
  explicit NativeWindow(QObject* parent = nullptr);

  void installMacOSQuitInterceptor();
  [[nodiscard]] Q_INVOKABLE int titlebarHeight(QObject* window);
  [[nodiscard]] Q_INVOKABLE int frameMargin(QObject* window);
  [[nodiscard]] Q_INVOKABLE int frameTopInset(QObject* window);

  [[nodiscard]] bool csdAvailable() const;
  [[nodiscard]] bool csdEnabled() const;
  [[nodiscard]] bool csdShadowEnabled() const;

public slots:
  void removeWindow(QObject* window);
  void addWindow(QObject* window, const QString& color = "");
  void setCsdEnabled(bool enabled);
  void setCsdShadowEnabled(bool enabled);

private slots:
  void onThemeChanged();
  void onActiveChanged();
  void onWindowStateChanged(Qt::WindowState state);

private:
  QMap<QWindow*, QString> m_colors;
  QList<QWindow*> m_windows;
  QSettings m_settings;
  bool m_csdShadowEnabled;
  bool m_csdEnabled;
};
