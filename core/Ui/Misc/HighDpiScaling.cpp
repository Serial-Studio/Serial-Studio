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

#include "Misc/HighDpiScaling.h"

#include <QApplication>
#include <QByteArray>
#include <QObject>

#include "Misc/CrashTracker.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Settings keys
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the QSettings key holding the user's chosen scaling mode.
 */
const char* Misc::HighDpiScaling::modeKey() noexcept
{
  return "App/HighDpiScaling";
}

/**
 * @brief Returns the QSettings key holding the user's custom scale percentage.
 */
const char* Misc::HighDpiScaling::percentKey() noexcept
{
  return "App/HighDpiScalingPercent";
}

//--------------------------------------------------------------------------------------------------
// Value validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the given mode identifier is one we know how to apply.
 */
bool Misc::HighDpiScaling::isModeAvailable(int mode) noexcept
{
  return mode >= Mode::SystemDefault && mode <= Mode::Custom;
}

/**
 * @brief Clamps a percentage into the supported [kMinPercent, kMaxPercent] range.
 */
int Misc::HighDpiScaling::clampPercent(int percent) noexcept
{
  return qBound(kMinPercent, percent, kMaxPercent);
}

//--------------------------------------------------------------------------------------------------
// Apply policy before QApplication exists
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the persisted mode, defaulting to Fractional (the app's historical behavior).
 */
int Misc::HighDpiScaling::readPersistedMode()
{
  QSettings settings;
  const int configured = settings.value(modeKey(), Mode::Fractional).toInt();
  if (!isModeAvailable(configured))
    return Mode::Fractional;

  return configured;
}

/**
 * @brief Reads the persisted custom scale percentage, clamped to the supported range.
 */
int Misc::HighDpiScaling::readPersistedPercent()
{
  QSettings settings;
  return clampPercent(settings.value(percentKey(), kDefaultPercent).toInt());
}

/**
 * @brief Called from main() before QApplication; sets Qt's rounding policy or a scale override.
 *        Clears both scaling env vars first so a value leaked from a previous selection into
 *        this process (a self-restart inherits the parent's environment) can't stick.
 */
void Misc::HighDpiScaling::applyConfiguredPolicy()
{
  const int mode = readPersistedMode();

  qunsetenv("QT_SCALE_FACTOR");
  qunsetenv("QT_ENABLE_HIGHDPI_SCALING");

  if (mode == Mode::Disabled) {
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    return;
  }

  if (mode == Mode::Custom) {
    constexpr double kPercentToFactor = 1.0 / 100.0;
    const double factor               = readPersistedPercent() * kPercentToFactor;
    qputenv("QT_SCALE_FACTOR", QByteArray::number(factor, 'g', 4));
    return;
  }

  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::Unset;
  switch (mode) {
    case Mode::Fractional:
      policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
      break;
    case Mode::RoundNearest:
      policy = Qt::HighDpiScaleFactorRoundingPolicy::Round;
      break;
    case Mode::RoundUp:
      policy = Qt::HighDpiScaleFactorRoundingPolicy::Ceil;
      break;
    case Mode::RoundDown:
      policy = Qt::HighDpiScaleFactorRoundingPolicy::Floor;
      break;
    default:
      break;
  }

  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);
}

//--------------------------------------------------------------------------------------------------
// Singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide HighDpiScaling instance.
 */
Misc::HighDpiScaling& Misc::HighDpiScaling::instance()
{
  static HighDpiScaling singleton;
  return singleton;
}

/**
 * @brief Loads the persisted mode/percentage and builds the QML-visible list of options.
 */
Misc::HighDpiScaling::HighDpiScaling()
  : m_currentMode(Mode::Fractional), m_customPercent(kDefaultPercent)
{
  m_currentMode = m_settings.value(modeKey(), Mode::Fractional).toInt();
  if (!isModeAvailable(m_currentMode))
    m_currentMode = Mode::Fractional;

  m_customPercent = clampPercent(m_settings.value(percentKey(), kDefaultPercent).toInt());
  rebuildAvailableModes();
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the persisted scaling-mode identifier the user has selected.
 */
int Misc::HighDpiScaling::currentMode() const noexcept
{
  return m_currentMode;
}

/**
 * @brief Returns the persisted custom scale percentage.
 */
int Misc::HighDpiScaling::customPercent() const noexcept
{
  return m_customPercent;
}

/**
 * @brief Returns whether the Custom mode is currently selected (drives the percent field).
 */
bool Misc::HighDpiScaling::customSelected() const noexcept
{
  return m_currentMode == Mode::Custom;
}

/**
 * @brief Returns whether the current platform allows the user to switch policies. macOS uses
 *        integer backing-scale factors, so the rounding policy is a no-op there.
 */
bool Misc::HighDpiScaling::configurable() const noexcept
{
#if defined(Q_OS_MACOS)
  return false;
#else
  return true;
#endif
}

/**
 * @brief Returns the smallest custom scale percentage the spin box should allow.
 */
int Misc::HighDpiScaling::minimumPercent() const noexcept
{
  return kMinPercent;
}

/**
 * @brief Returns the largest custom scale percentage the spin box should allow.
 */
int Misc::HighDpiScaling::maximumPercent() const noexcept
{
  return kMaxPercent;
}

/**
 * @brief Returns the list of selectable scaling-mode entries for QML.
 */
const QVariantList& Misc::HighDpiScaling::availableModes() const noexcept
{
  return m_availableModes;
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Persists the chosen mode; the change takes effect after the next restart.
 */
void Misc::HighDpiScaling::setCurrentMode(int mode)
{
  if (m_currentMode == mode)
    return;

  if (!isModeAvailable(mode))
    return;

  m_currentMode = mode;
  m_settings.setValue(modeKey(), mode);
  m_settings.sync();
  Q_EMIT currentModeChanged();
}

/**
 * @brief Persists the custom scale percentage (clamped); takes effect after the next restart.
 */
void Misc::HighDpiScaling::setCustomPercent(int percent)
{
  const int clamped = clampPercent(percent);
  if (m_customPercent == clamped)
    return;

  m_customPercent = clamped;
  m_settings.setValue(percentKey(), clamped);
  m_settings.sync();
  Q_EMIT customPercentChanged();
}

/**
 * @brief Asks the user via a native message box whether to relaunch to apply the change.
 */
void Misc::HighDpiScaling::promptRestartAndQuit()
{
  const int choice = Misc::Utilities::showMessageBox(
    tr("Restart Required"),
    tr("The new display scaling setting will take effect after restarting Serial Studio. "
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

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the QML-visible list of mode entries (id + display label).
 */
void Misc::HighDpiScaling::rebuildAvailableModes()
{
  m_availableModes.clear();

  auto add = [this](int id, const QString& label) {
    QVariantMap entry;
    entry.insert(QStringLiteral("id"), id);
    entry.insert(QStringLiteral("label"), label);
    m_availableModes.append(entry);
  };

  add(Mode::SystemDefault, QObject::tr("Automatic (System Default)"));
  add(Mode::Fractional, QObject::tr("Fractional"));
  add(Mode::RoundNearest, QObject::tr("Round to Nearest"));
  add(Mode::RoundUp, QObject::tr("Round Up"));
  add(Mode::RoundDown, QObject::tr("Round Down"));
  add(Mode::Disabled, QObject::tr("Disable Scaling"));
  add(Mode::Custom, QObject::tr("Custom…"));
}
