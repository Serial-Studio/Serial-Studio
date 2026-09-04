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

#include "Misc/Diagnostics/AudioChecks.h"

#include <QCoreApplication>
#include <QSettings>
#include <QStringList>

#if QT_CONFIG(permissions)
#  include <QPermissions>
#endif

#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Local aliases
//--------------------------------------------------------------------------------------------------

using Misc::Diagnostics::Bus;
using Misc::Diagnostics::makeResult;
using Misc::Diagnostics::Result;
using Misc::Diagnostics::trDiag;
using Misc::Diagnostics::Verdict;

//--------------------------------------------------------------------------------------------------
// Individual checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports a microphone permission the system has refused. The status is read, never
 *        requested, so a run cannot raise a permission dialog the user did not ask for.
 */
static void reportPermission(QList<Result>& out)
{
#if QT_CONFIG(permissions)
  auto* app = QCoreApplication::instance();
  if (app == nullptr)
    return;

  if (app->checkPermission(QMicrophonePermission{}) != Qt::PermissionStatus::Denied)
    return;

  out.append(makeResult(Bus::Audio,
                        Verdict::Failure,
                        "audio-permission-denied",
                        trDiag("Microphone access is not permitted"),
                        trDiag("The system has refused Serial Studio permission to record "
                               "audio, so no input device can be opened."),
                        trDiag("Allow Serial Studio to use the microphone in the system's "
                               "privacy settings, then restart the application.")));
#else
  Q_UNUSED(out)
#endif
}

/**
 * @brief Reports a previously selected input device the backend no longer enumerates.
 */
static void reportMissingSelection(const QStringList& devices, QList<Result>& out)
{
  QSettings settings;
  const auto key      = QStringLiteral("AudioDriver/inputDeviceName");
  const auto selected = settings.value(key).toString();
  if (selected.isEmpty() || devices.contains(selected))
    return;

  out.append(makeResult(Bus::Audio,
                        Verdict::Warning,
                        "audio-input-missing",
                        trDiag("The selected audio input is gone"),
                        trDiag("Input device %1 was selected previously and the audio backend "
                               "does not report it any more.")
                          .arg(selected),
                        trDiag("Reconnect the device, or select another input in the device "
                               "setup pane.")));
}

/**
 * @brief Reports the backend and device-list state; a dead backend suppresses the device checks
 *        below it, which would otherwise report an empty list as a missing microphone.
 */
static void reportBackend(QList<Result>& out)
{
  static auto& manager = IO::ConnectionManager::instance();

  auto* audio = manager.audio();
  if (audio == nullptr)
    return;

  if (!audio->backendReady()) {
    out.append(makeResult(Bus::Audio,
                          Verdict::Failure,
                          "audio-backend-failed",
                          trDiag("The audio backend did not start"),
                          trDiag("Serial Studio could not initialize the system's audio "
                                 "backend, so no input or output device is available."),
                          trDiag("Confirm the system's audio service is running, then restart "
                                 "the application.")));
    return;
  }

  const auto devices = audio->inputDeviceList();
  if (devices.isEmpty()) {
    out.append(makeResult(Bus::Audio,
                          Verdict::Warning,
                          "no-audio-inputs",
                          trDiag("No audio input devices were found"),
                          trDiag("The audio backend reports no capture device, so there is "
                                 "nothing for this bus to record from."),
                          trDiag("Connect a microphone or line-in device, then run the check "
                                 "again.")));
    return;
  }

  reportMissingSelection(devices, out);
}

//--------------------------------------------------------------------------------------------------
// Collection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs every instant audio check in declaration order.
 */
void Misc::Diagnostics::AudioChecks::collect(QList<Result>& out)
{
  reportPermission(out);
  reportBackend(out);
}
