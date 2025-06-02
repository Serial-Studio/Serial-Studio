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
