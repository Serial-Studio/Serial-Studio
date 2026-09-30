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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QSet>
#include <QSettings>
#include <QVariantList>

namespace Misc {
/**
 * @brief Manages the Qt Quick scene graph RHI backend selection and the HDR output mode
 *        (spec 0089): persisted request, crash revert, live intensity, active-window roster.
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
  Q_PROPERTY(bool reduceMotion
             READ reduceMotion
             WRITE setReduceMotion
             NOTIFY reduceMotionChanged)
  Q_PROPERTY(bool hdrSupported
             READ hdrSupported
             NOTIFY currentBackendChanged)
  Q_PROPERTY(bool hdrEnabled
             READ hdrEnabled
             WRITE setHdrEnabled
             NOTIFY hdrEnabledChanged)
  Q_PROPERTY(bool hdrRequested
             READ hdrRequested
             CONSTANT)
  Q_PROPERTY(double hdrAutoIntensity
             READ hdrAutoIntensity
             CONSTANT)
  Q_PROPERTY(double hdrSteadyIntensity
             READ hdrSteadyIntensity
             CONSTANT)
  Q_PROPERTY(bool hdrAnyActive
             READ hdrAnyActive
             NOTIFY hdrAnyActiveChanged)
  // clang-format on

signals:
  void reduceMotionChanged();
  void currentBackendChanged();
  void hdrEnabledChanged();
  void hdrAnyActiveChanged();

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
  [[nodiscard]] bool reduceMotion() const noexcept;
  [[nodiscard]] bool effectsEnabled() const noexcept;
  [[nodiscard]] bool hdrSupported() const noexcept;
  [[nodiscard]] bool hdrEnabled() const noexcept;
  [[nodiscard]] bool hdrRequested() const noexcept;
  [[nodiscard]] double hdrAutoIntensity() const noexcept;
  [[nodiscard]] double hdrSteadyIntensity() const noexcept;
  [[nodiscard]] bool hdrAnyActive() const noexcept;
  [[nodiscard]] const QVariantList& availableBackends() const noexcept;

  static void applyConfiguredBackend();

  void setWindowHdrActive(QObject* window, bool active);

public slots:
  void setReduceMotion(bool reduce);
  void setCurrentBackend(int backend);
  void setHdrEnabled(bool enabled);
  void confirmStartupSuccess();
  void promptRestartAndQuit();

private:
  static int readPersistedBackend();
  static bool readPersistedHdrRequest(int backend);
  static int defaultBackendForPlatform() noexcept;
  static bool isBackendAvailable(int backend) noexcept;
  static bool isHdrCapableBackend(int backend) noexcept;
  static const char* settingsKey() noexcept;
  static const char* pendingKey() noexcept;
  static const char* reduceMotionKey() noexcept;
  static const char* hdrKey() noexcept;
  static const char* hdrPendingKey() noexcept;
  void rebuildAvailableBackends();

private:
  int m_currentBackend;
  bool m_configurable;
  bool m_reduceMotion;
  bool m_hdrEnabled;
  QSettings m_settings;
  QVariantList m_availableBackends;
  QSet<QObject*> m_hdrActiveWindows;

  static int s_activeBackend;
  static bool s_hdrRequested;
};
}  // namespace Misc
