/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
#include <QSettings>
#include <QVariantList>

namespace Misc {
/**
 * @brief Manages the Qt Quick scene graph RHI backend selection.
 */
class GraphicsBackend : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int currentBackend
             READ currentBackend
             WRITE setCurrentBackend
             NOTIFY currentBackendChanged)
  Q_PROPERTY(QVariantList availableBackends
             READ availableBackends
             CONSTANT)
  Q_PROPERTY(bool configurable
             READ configurable
             CONSTANT)
  Q_PROPERTY(bool effectsEnabled
             READ effectsEnabled
             CONSTANT)
  // clang-format on

signals:
  void currentBackendChanged();

public:
  /**
   * @brief Identifiers persisted to QSettings; mapped to QSGRendererInterface::GraphicsApi.
   */
  enum Backend {
    Default    = 0,
    OpenGL     = 1,
    Vulkan     = 2,
    Direct3D11 = 3,
    Metal      = 4,
    Software   = 5
  };
  Q_ENUM(Backend)

private:
  explicit GraphicsBackend();
  GraphicsBackend(GraphicsBackend&&)                 = delete;
  GraphicsBackend(const GraphicsBackend&)            = delete;
  GraphicsBackend& operator=(GraphicsBackend&&)      = delete;
  GraphicsBackend& operator=(const GraphicsBackend&) = delete;

public:
  [[nodiscard]] static GraphicsBackend& instance();

  [[nodiscard]] int currentBackend() const noexcept;
  [[nodiscard]] bool configurable() const noexcept;
  [[nodiscard]] bool effectsEnabled() const noexcept;
  [[nodiscard]] const QVariantList& availableBackends() const noexcept;

  static void applyConfiguredBackend();

public slots:
  void setCurrentBackend(int backend);
  void confirmStartupSuccess();
  void promptRestartAndQuit();

private:
  static int readPersistedBackend();
  static int defaultBackendForPlatform() noexcept;
  static bool isBackendAvailable(int backend) noexcept;
  static const char* settingsKey() noexcept;
  static const char* pendingKey() noexcept;
  void rebuildAvailableBackends();

private:
  int m_currentBackend;
  bool m_configurable;
  QSettings m_settings;
  QVariantList m_availableBackends;

  static int s_activeBackend;
};
}  // namespace Misc
