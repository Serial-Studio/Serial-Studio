/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QWindow>

#include "Platform/KioskControl.h"

class QScreen;

namespace Platform {

/**
 * @brief Where a kiosk (--kiosk) goes and who may move it (spec 0125): the screen it is confined
 *        to, the strip reserved at the left edge of the leftmost screen, and the launcher's
 *        control channel. QML keeps placing the windows; this object supplies the rectangle.
 */
class KioskSession : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool confined
             READ confined
             NOTIFY placementChanged)
  Q_PROPERTY(bool controlled
             READ controlled
             CONSTANT)
  Q_PROPERTY(QRectF screenGeometry
             READ screenGeometry
             NOTIFY placementChanged)
  // clang-format on

signals:
  void raiseRequested();
  void closeRequested();
  void placementChanged();

public:
  explicit KioskSession(QObject* parent = nullptr);

  void configure(const QString& screen, int reserve_left, const QString& control_server);

  [[nodiscard]] bool confined() const noexcept;
  [[nodiscard]] bool controlled() const noexcept;
  [[nodiscard]] QRectF screenGeometry() const;
  [[nodiscard]] Q_INVOKABLE QRectF fillRect(const QRectF& screen) const;

  [[nodiscard]] static int leftmostIndex(const QList<QRect>& screens, int primary_index);
  [[nodiscard]] static QRect insetRect(const QList<QRect>& screens,
                                       int primary_index,
                                       const QRect& target,
                                       int reserve_left);

public slots:
  void setMainWindow(QObject* window);

private slots:
  void greet();
  void publishState();
  void onScreensChanged();
  void moveToScreen(const QString& name, int reserve_left);

private:
  void reportState(bool force);
  [[nodiscard]] QScreen* confinedScreen() const;
  [[nodiscard]] QString currentScreenName() const;

  KioskControl m_control;
  QPointer<QWindow> m_window;
  QString m_controlServer;
  QString m_sentScreen;
  QString m_screenName;
  int m_reserveLeft;
  bool m_sentActive;
  bool m_confined;
};

}  // namespace Platform
