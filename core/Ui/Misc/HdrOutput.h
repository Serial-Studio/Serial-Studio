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

#include <QElapsedTimer>
#include <QObject>
#include <QPointer>
#include <QQuickWindow>

namespace Misc {
class GraphicsBackend;

/**
 * @brief Per-window HDR output helper (spec 0089): requests the FP16 swapchain before first
 *        expose, then reports what the swapchain actually granted. QML-instantiable, never a
 *        singleton; sampled only inside beforeSynchronizing (render thread, GUI blocked) at
 *        ~1 Hz, published queued -- no blocking connection in either direction.
 */
class HdrOutput : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QQuickWindow* window
             READ window
             WRITE setWindow
             NOTIFY windowChanged)
  Q_PROPERTY(bool active
             READ active
             NOTIFY activeChanged)
  Q_PROPERTY(double headroom
             READ headroom
             NOTIFY activeChanged)
  Q_PROPERTY(double sdrWhiteScale
             READ sdrWhiteScale
             NOTIFY activeChanged)
  // clang-format on

signals:
  void windowChanged();
  void activeChanged();

public:
  explicit HdrOutput(QObject* parent = nullptr);
  ~HdrOutput() override;

  static void bindBackend(GraphicsBackend* backend) noexcept;
  [[nodiscard]] static float effectiveBoost(const QQuickWindow* window);

  [[nodiscard]] QQuickWindow* window() const noexcept;
  [[nodiscard]] bool active() const noexcept;
  [[nodiscard]] double headroom() const noexcept;
  [[nodiscard]] double sdrWhiteScale() const noexcept;

public slots:
  void setWindow(QQuickWindow* window);

private slots:
  void sampleSwapChain();

private:
  void publishSample(bool active, double headroom, double sdrWhiteScale);

private:
  bool m_active;
  double m_headroom;
  double m_sdrWhiteScale;
  GraphicsBackend* m_backend;
  QPointer<QQuickWindow> m_window;
  QElapsedTimer m_sampleTimer;
  QMetaObject::Connection m_syncConnection;

  static GraphicsBackend* s_backend;
};
}  // namespace Misc
