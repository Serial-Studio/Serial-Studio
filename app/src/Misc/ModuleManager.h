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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QSettings>

#include "Platform/NativeWindow.h"

class QQmlContext;
class SessionContext;

namespace UI::Alarms {
class AlarmAnnunciator;
}  // namespace UI::Alarms

#ifdef BUILD_COMMERCIAL
namespace Licensing {
class TrialGate;
}  // namespace Licensing
#endif

namespace Misc {
class SimdSettings;
class ProblemCenter;

/**
 * @brief Manages application module lifecycle, QML engine setup, and headless operation.
 */
class ModuleManager : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool autoUpdaterEnabled
             READ autoUpdaterEnabled
             CONSTANT)
  Q_PROPERTY(bool automaticUpdates
             READ  automaticUpdates
             WRITE setAutomaticUpdates
             NOTIFY automaticUpdatesChanged)
  Q_PROPERTY(bool performanceMode
             READ  performanceMode
             WRITE setPerformanceMode
             NOTIFY performanceModeChanged)
  Q_PROPERTY(bool inhibitIdleSleep
             READ  inhibitIdleSleep
             WRITE setInhibitIdleSleep
             NOTIFY inhibitIdleSleepChanged)
  // clang-format on

signals:
  void performanceModeChanged();
  void inhibitIdleSleepChanged();
  void automaticUpdatesChanged();

public:
  ModuleManager();
  ~ModuleManager() override;
  static void bootstrapCoreServices();
  static void instantiateCoreModules();
  static void bindInterfaces();
  static void registerApiHandlers();
  static void releaseAnnunciator();
  static void wireAnnunciator(bool headless, Misc::ProblemCenter& problemCenter);
  static void constructAnnunciator(SessionContext& ctx, Misc::ProblemCenter* problemCenter);
  static void setupHeadlessSessionConnections();
  static void teardownHeadlessSessionModules();
  static void stopFrameConsumerWorkers();
  [[nodiscard]] bool performanceMode() const noexcept;
  [[nodiscard]] bool inhibitIdleSleep() const noexcept;
  [[nodiscard]] bool autoUpdaterEnabled() const noexcept;
  [[nodiscard]] bool automaticUpdates() const noexcept;
  [[nodiscard]] const QQmlApplicationEngine& engine() const noexcept;

public slots:
  void onQuit();
  void configureUpdater();
  void registerQmlTypes();
  void initializeQmlInterface();
  void setHeadless(const bool headless);
  void setPerformanceMode(const bool enabled);
  void setInhibitIdleSleep(const bool enabled);
  void setAutomaticUpdates(const bool enabled);
  void setEphemeralSession(const bool ephemeral);

private:
  void setupCrossModuleConnections();
  void registerCoreContextProperties(QQmlContext* ctx);
  void registerAppMetadataProperties(QQmlContext* ctx, bool grpcAvailable);
  void registerImageProvidersAndLoadQml();
#ifdef BUILD_COMMERCIAL
  void registerCommercialContextProperties(QQmlContext* ctx);
  void wireTrialGate();
#endif

private:
  bool m_headless;
  bool m_quitHandled;
  QSettings m_settings;
  std::unique_ptr<SimdSettings> m_simdSettings;
  bool m_ephemeralSession;
  bool m_automaticUpdates;
  bool m_performanceMode;
  bool m_inhibitIdleSleep;
  NativeWindow m_nativeWindow;
  QQmlApplicationEngine m_engine;
#ifdef BUILD_COMMERCIAL
  Licensing::TrialGate* m_trialGate;
#endif
};
}  // namespace Misc
