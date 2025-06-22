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
#include <QSettings>
#include <QQmlApplicationEngine>

#include "Platform/NativeWindow.h"

namespace Misc
{
/**
 * @brief The ModuleManager class
 *
 * The @c ModuleManager class is in charge of initializing all the C++ modules
 * that are part of Serial Studio in the correct order.
 */
class ModuleManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool autoUpdaterEnabled READ autoUpdaterEnabled CONSTANT)
  Q_PROPERTY(bool softwareRendering READ softwareRendering WRITE
                 setSoftwareRendering NOTIFY softwareRenderingChanged)

signals:
  void softwareRenderingChanged();

public:
  ModuleManager();

  [[nodiscard]] bool softwareRendering() const;
  [[nodiscard]] bool autoUpdaterEnabled() const;
  [[nodiscard]] const QQmlApplicationEngine &engine() const;

public slots:
  void onQuit();
  void configureUpdater();
  void registerQmlTypes();
  void initializeQmlInterface();
  void setSoftwareRendering(const bool enabled);

private:
  QSettings m_settings;
  bool m_softwareRendering;
  NativeWindow m_nativeWindow;
  QQmlApplicationEngine m_engine;
};
} // namespace Misc
