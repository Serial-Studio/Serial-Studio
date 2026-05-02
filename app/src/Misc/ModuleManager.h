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
#include <QQmlApplicationEngine>
#include <QSettings>

#include "Platform/NativeWindow.h"

class QQmlContext;

namespace Misc {
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
  // clang-format on

signals:
  void automaticUpdatesChanged();

public:
  ModuleManager();
  [[nodiscard]] bool autoUpdaterEnabled() const noexcept;
  [[nodiscard]] bool automaticUpdates() const noexcept;
  [[nodiscard]] const QQmlApplicationEngine& engine() const noexcept;

public slots:
  void onQuit();
  void configureUpdater();
  void registerQmlTypes();
  void initializeQmlInterface();
  void setHeadless(const bool headless);
  void setAutomaticUpdates(const bool enabled);

private:
  void setupCrossModuleConnections();
  void registerCoreContextProperties(QQmlContext* ctx);
  void registerAppMetadataProperties(QQmlContext* ctx, bool grpcAvailable);
  void registerImageProvidersAndLoadQml();
#ifdef BUILD_COMMERCIAL
  void registerCommercialContextProperties(QQmlContext* ctx);
#endif

private:
  bool m_headless;
  QSettings m_settings;
  bool m_automaticUpdates;
  NativeWindow m_nativeWindow;
  QQmlApplicationEngine m_engine;
};
}  // namespace Misc
