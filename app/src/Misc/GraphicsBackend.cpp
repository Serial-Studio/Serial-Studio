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

#include "Misc/GraphicsBackend.h"

#include <QCoreApplication>
#include <QObject>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include "Misc/CrashTracker.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Active backend (set by applyConfiguredBackend before the singleton exists)
//--------------------------------------------------------------------------------------------------

int Misc::GraphicsBackend::s_activeBackend = Misc::GraphicsBackend::Backend::Default;

//--------------------------------------------------------------------------------------------------
// Settings keys
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the QSettings key holding the user's chosen backend.
 */
const char* Misc::GraphicsBackend::settingsKey() noexcept
{
  return "App/GraphicsBackend";
}

/**
 * @brief Returns the QSettings key set just before applying a non-default backend.
 */
const char* Misc::GraphicsBackend::pendingKey() noexcept
{
  return "App/GraphicsBackendPending";
}

//--------------------------------------------------------------------------------------------------
// Platform support
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the default platform backend identifier (Default = let Qt decide).
 */
int Misc::GraphicsBackend::defaultBackendForPlatform() noexcept
{
  return Backend::Default;
}

/**
 * @brief Returns whether the given backend is selectable on this platform.
 */
bool Misc::GraphicsBackend::isBackendAvailable(int backend) noexcept
{
  if (backend == Backend::Default || backend == Backend::Software)
    return true;

#if defined(Q_OS_WIN)
  return backend == Backend::OpenGL || backend == Backend::Vulkan || backend == Backend::Direct3D11;
#elif defined(Q_OS_MACOS)
  return backend == Backend::Metal || backend == Backend::Vulkan;
#elif defined(Q_OS_LINUX)
  return backend == Backend::OpenGL || backend == Backend::Vulkan;
#else
  return backend == Backend::OpenGL;
#endif
}

//--------------------------------------------------------------------------------------------------
// Apply backend before any QQuickWindow exists
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads QSettings and reverts to Default if the previous attempt crashed.
 */
int Misc::GraphicsBackend::readPersistedBackend()
{
  QSettings settings;
  const int configured = settings.value(settingsKey(), Backend::Default).toInt();
  const int pending    = settings.value(pendingKey(), Backend::Default).toInt();

  if (pending != Backend::Default && pending == configured) {
    settings.setValue(settingsKey(), Backend::Default);
    settings.remove(pendingKey());
    settings.sync();
    return Backend::Default;
  }

  if (!isBackendAvailable(configured))
    return Backend::Default;

  return configured;
}

/**
 * @brief Called from main() before QApplication; sets QQuickWindow's graphics API.
 */
void Misc::GraphicsBackend::applyConfiguredBackend()
{
  const int backend = readPersistedBackend();
  s_activeBackend   = backend;

  if (backend == Backend::Default)
    return;

  QSettings settings;
  settings.setValue(pendingKey(), backend);
  settings.sync();

  switch (backend) {
    case Backend::OpenGL:
      QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
      break;
    case Backend::Vulkan:
      QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);
      break;
    case Backend::Direct3D11:
      QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
      break;
    case Backend::Metal:
      QQuickWindow::setGraphicsApi(QSGRendererInterface::Metal);
      break;
    case Backend::Software:
      QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
      break;
    default:
      break;
  }
}

//--------------------------------------------------------------------------------------------------
// Singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide GraphicsBackend instance.
 */
Misc::GraphicsBackend& Misc::GraphicsBackend::instance()
{
  static GraphicsBackend singleton;
  return singleton;
}

/**
 * @brief Builds the list of selectable backends for the current platform.
 */
Misc::GraphicsBackend::GraphicsBackend() : m_currentBackend(Backend::Default), m_configurable(false)
{
  m_currentBackend = m_settings.value(settingsKey(), Backend::Default).toInt();

#if defined(Q_OS_MACOS)
  m_configurable = false;
#else
  m_configurable = true;
#endif

  if (!isBackendAvailable(m_currentBackend))
    m_currentBackend = Backend::Default;

  rebuildAvailableBackends();
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the persisted backend identifier the user has selected.
 */
int Misc::GraphicsBackend::currentBackend() const noexcept
{
  return m_currentBackend;
}

/**
 * @brief Returns whether the current platform allows the user to switch backends.
 */
bool Misc::GraphicsBackend::configurable() const noexcept
{
  return m_configurable;
}

/**
 * @brief Returns false when the active backend is Software; layer effects can't render then.
 */
bool Misc::GraphicsBackend::effectsEnabled() const noexcept
{
  return s_activeBackend != Backend::Software;
}

/**
 * @brief Returns the platform-filtered list of selectable backend entries for QML.
 */
const QVariantList& Misc::GraphicsBackend::availableBackends() const noexcept
{
  return m_availableBackends;
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Persists the chosen backend; the change takes effect after the next restart.
 */
void Misc::GraphicsBackend::setCurrentBackend(int backend)
{
  if (m_currentBackend == backend)
    return;

  if (!isBackendAvailable(backend))
    return;

  m_currentBackend = backend;
  m_settings.setValue(settingsKey(), backend);
  m_settings.sync();
  Q_EMIT currentBackendChanged();
}

/**
 * @brief Asks the user via a native message box whether to relaunch to apply the change.
 */
void Misc::GraphicsBackend::promptRestartAndQuit()
{
  const int choice = Misc::Utilities::showMessageBox(
    tr("Restart Required"),
    tr("The new rendering backend will take effect after restarting Serial Studio. "
       "Restart now to apply the change?"),
    QMessageBox::Question,
    qAppName(),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::Yes);

  if (choice != QMessageBox::Yes)
    return;

  Misc::CrashTracker::instance().markCleanExit();
  Misc::Utilities::rebootApplication();
}

/**
 * @brief Clears the "startup pending" flag once QML has loaded without crashing.
 */
void Misc::GraphicsBackend::confirmStartupSuccess()
{
  if (!m_settings.contains(pendingKey()))
    return;

  m_settings.remove(pendingKey());
  m_settings.sync();
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the QML-visible list of backend entries (id + display label).
 */
void Misc::GraphicsBackend::rebuildAvailableBackends()
{
  m_availableBackends.clear();

  auto add = [this](int id, const QString& label) {
    if (!isBackendAvailable(id))
      return;

    QVariantMap entry;
    entry.insert(QStringLiteral("id"), id);
    entry.insert(QStringLiteral("label"), label);
    m_availableBackends.append(entry);
  };

  add(Backend::Default, QObject::tr("Automatic (Platform Default)"));
  add(Backend::OpenGL, QStringLiteral("OpenGL"));
  add(Backend::Vulkan, QStringLiteral("Vulkan"));
  add(Backend::Direct3D11, QStringLiteral("Direct3D 11"));
  add(Backend::Metal, QStringLiteral("Metal"));
  add(Backend::Software, QObject::tr("Software (Fallback)"));
}
