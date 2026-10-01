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
bool Misc::GraphicsBackend::s_hdrRequested = false;

static constexpr double kAutoHdrIntensityDark    = 2.0;
static constexpr double kAutoHdrIntensityLight   = 1.0;
static constexpr double kFlashHdrIntensity       = 1.7;
static constexpr double kSteadyHdrIntensityDark  = 1.4;
static constexpr double kSteadyHdrIntensityLight = 1.2;

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

/**
 * @brief Returns the QSettings key holding the reduce-motion preference.
 */
const char* Misc::GraphicsBackend::reduceMotionKey() noexcept
{
  return "App/ReduceMotion";
}

/**
 * @brief Returns the QSettings key holding the HDR output preference (spec 0089).
 */
const char* Misc::GraphicsBackend::hdrKey() noexcept
{
  return "App/HdrEnabled";
}

/**
 * @brief Returns the QSettings key set just before applying HDR output at startup.
 */
const char* Misc::GraphicsBackend::hdrPendingKey() noexcept
{
  return "App/HdrPending";
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

/**
 * @brief Returns whether @p backend can drive an HDR swapchain on this platform: Metal on
 *        macOS, Direct3D 11 on Windows (each also as the platform Default), nothing else
 *        (spec 0089 -- OpenGL has no HDR path in Qt, Software none at all).
 */
bool Misc::GraphicsBackend::isHdrCapableBackend(int backend) noexcept
{
#if defined(Q_OS_MACOS)
  return backend == Backend::Default || backend == Backend::Metal;
#elif defined(Q_OS_WIN)
  return backend == Backend::Default || backend == Backend::Direct3D11;
#else
  Q_UNUSED(backend);
  return false;
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
 * @brief Reads the persisted HDR request for @p backend; a pending flag left from the previous
 *        launch means that attempt crashed, so the preference reverts to off (same contract as
 *        readPersistedBackend).
 */
bool Misc::GraphicsBackend::readPersistedHdrRequest(int backend)
{
  if (!isHdrCapableBackend(backend))
    return false;

  QSettings settings;
  const bool enabled = settings.value(hdrKey(), false).toBool();
  const bool pending = settings.value(hdrPendingKey(), false).toBool();

  if (pending && enabled) {
    settings.setValue(hdrKey(), false);
    settings.remove(hdrPendingKey());
    settings.sync();
    return false;
  }

  return enabled;
}

/**
 * @brief Returns whether this launch resolved to HDR output (latched before QApplication,
 *        constant for the session); read by HdrSurface's Loader gate and by Misc::HdrOutput
 *        when a window applies its swapchain-format request.
 */
bool Misc::GraphicsBackend::hdrRequested() const noexcept
{
  return s_hdrRequested;
}

/**
 * @brief Called from main() before QApplication; sets QQuickWindow's graphics API.
 */
void Misc::GraphicsBackend::applyConfiguredBackend()
{
  const int backend = readPersistedBackend();
  s_activeBackend   = backend;
  s_hdrRequested    = readPersistedHdrRequest(backend);

  if (s_hdrRequested) {
    QSettings settings;
    settings.setValue(hdrPendingKey(), true);
    settings.sync();
  }

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
 * @brief Builds the list of selectable backends; reduce-motion defaults on under the Software
 *        backend, where every animated frame is a CPU repaint.
 */
Misc::GraphicsBackend::GraphicsBackend()
  : m_currentBackend(Backend::Default)
  , m_configurable(false)
  , m_reduceMotion(false)
  , m_hdrEnabled(false)
  , m_darkTheme(false)
{
  m_currentBackend = m_settings.value(settingsKey(), Backend::Default).toInt();
  m_reduceMotion =
    m_settings.value(reduceMotionKey(), s_activeBackend == Backend::Software).toBool();
  m_hdrEnabled = m_settings.value(hdrKey(), false).toBool();

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
 * @brief Returns true when interface motion (scale, slide, bounce) is off; fades stay.
 */
bool Misc::GraphicsBackend::reduceMotion() const noexcept
{
  return m_reduceMotion;
}

/**
 * @brief Returns false when the active backend is Software; layer effects can't render then.
 */
bool Misc::GraphicsBackend::effectsEnabled() const noexcept
{
  return s_activeBackend != Backend::Software;
}

/**
 * @brief Returns whether the user's selected backend could drive HDR output after a restart;
 *        controls the visibility of the HDR rows in Settings.
 */
bool Misc::GraphicsBackend::hdrSupported() const noexcept
{
  return isHdrCapableBackend(m_currentBackend);
}

/**
 * @brief Returns the persisted HDR output preference (restart-applied).
 */
bool Misc::GraphicsBackend::hdrEnabled() const noexcept
{
  return m_hdrEnabled;
}

/**
 * @brief Returns the emissive ceiling for thin data strokes (plot curves, FFT markers,
 *        waterfall top-of-scale), clamped per window to the display's headroom. Dark themes
 *        only: emission helps only when ink ends up brighter than its surround, so light
 *        themes return 1.0 and strokes keep the SDR material.
 */
double Misc::GraphicsBackend::hdrAutoIntensity() const noexcept
{
  return m_darkTheme ? kAutoHdrIntensityDark : kAutoHdrIntensityLight;
}

/**
 * @brief Returns the alarm-flash peak intensity: above the steady tier so a blinking element
 *        reads hotter, below the stroke ceiling because temporal modulation already carries
 *        the salience (ISA-101: reserve maximum emphasis for abnormal, thin-element cases).
 */
double Misc::GraphicsBackend::hdrFlashIntensity() const noexcept
{
  return kFlashHdrIntensity;
}

/**
 * @brief Returns the softer emissive intensity for always-on surfaces (band arcs, bar fills,
 *        steady-lit LEDs); flashing elements take hdrAutoIntensity instead. Theme-weighted: a
 *        dark theme leaves more perceptual headroom over the chrome, so steady emissives sit
 *        higher there and lower on a light theme (the composition root syncs the flag).
 */
double Misc::GraphicsBackend::hdrSteadyIntensity() const noexcept
{
  return m_darkTheme ? kSteadyHdrIntensityDark : kSteadyHdrIntensityLight;
}

/**
 * @brief Returns whether any window currently renders through an HDR swapchain.
 */
bool Misc::GraphicsBackend::hdrAnyActive() const noexcept
{
  return !m_hdrActiveWindows.isEmpty();
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
 * @brief Persists the reduce-motion preference; QML reads it live, so no restart is needed.
 */
void Misc::GraphicsBackend::setReduceMotion(bool reduce)
{
  if (m_reduceMotion == reduce)
    return;

  m_reduceMotion = reduce;
  m_settings.setValue(reduceMotionKey(), reduce);
  m_settings.sync();
  Q_EMIT reduceMotionChanged();
}

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
 * @brief Persists the HDR output preference; takes effect after the next restart.
 */
void Misc::GraphicsBackend::setHdrEnabled(bool enabled)
{
  if (m_hdrEnabled == enabled)
    return;

  m_hdrEnabled = enabled;
  m_settings.setValue(hdrKey(), enabled);
  m_settings.sync();
  Q_EMIT hdrEnabledChanged();
}

/**
 * @brief Records whether the active theme is dark; the composition root forwards it from
 *        Misc::ThemeManager so hdrSteadyIntensity can follow the theme without this class
 *        reaching into the theme singleton.
 */
void Misc::GraphicsBackend::setDarkTheme(bool dark)
{
  if (m_darkTheme == dark)
    return;

  m_darkTheme = dark;
  Q_EMIT hdrAutoIntensityChanged();
  Q_EMIT hdrSteadyIntensityChanged();
}

/**
 * @brief Tracks which windows currently render through an HDR swapchain; called by
 *        Misc::HdrOutput on activation changes and from its destructor. A tracked window
 *        also self-removes on destroyed(), so a teardown order that nulls the helper's
 *        QPointer first cannot leave a dangling roster entry latching hdrAnyActive.
 */
void Misc::GraphicsBackend::setWindowHdrActive(QObject* window, bool active)
{
  if (window == nullptr)
    return;

  const bool was = !m_hdrActiveWindows.isEmpty();
  if (active && !m_hdrActiveWindows.contains(window)) {
    m_hdrActiveWindows.insert(window);
    connect(window, &QObject::destroyed, this, [this](QObject* gone) {
      setWindowHdrActive(gone, false);
    });
  } else if (!active) {
    m_hdrActiveWindows.remove(window);
  }

  if (was != !m_hdrActiveWindows.isEmpty())
    Q_EMIT hdrAnyActiveChanged();
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

  static auto& crashTracker = Misc::CrashTracker::instance();
  crashTracker.markCleanExit();
  Misc::Utilities::rebootApplication();
}

/**
 * @brief Clears the "startup pending" flags (backend and HDR) once QML has loaded without
 *        crashing; both move in lockstep with the writes in applyConfiguredBackend().
 */
void Misc::GraphicsBackend::confirmStartupSuccess()
{
  if (!m_settings.contains(pendingKey()) && !m_settings.contains(hdrPendingKey()))
    return;

  m_settings.remove(pendingKey());
  m_settings.remove(hdrPendingKey());
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
