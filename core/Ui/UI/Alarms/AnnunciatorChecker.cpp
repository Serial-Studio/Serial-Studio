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

#include "UI/Alarms/AnnunciatorChecker.h"

#include "DataModel/ProjectModel.h"
#include "IO/Audio/SoundPlayer.h"
#include "UI/Alarms/SoundTheme.h"

using Finding = Misc::ProblemCenter::Finding;

/**
 * @brief Walks the project for alarm intent: a band on any enabled dataset, or a sounds map.
 *        Command/rebuild rate only, never per frame.
 */
bool UI::Alarms::AnnunciatorChecker::definesAlarms(const DataModel::ProjectModel& project)
{
  if (!project.sounds().isEmpty())
    return true;

  const auto& groups = project.groups();
  for (const auto& group : groups) {
    if (!group.enabled)
      continue;

    for (const auto& dataset : group.datasets)
      if (dataset.enabled && !dataset.alarmBands.empty())
        return true;
  }

  return false;
}

/**
 * @brief Binds the facade members the findings read; nothing is copied or owned.
 */
UI::Alarms::AnnunciatorChecker::AnnunciatorChecker(SoundTheme& theme,
                                                   IO::Audio::SoundPlayer& player,
                                                   const bool& deviceLost)
  : m_theme(theme), m_player(player), m_deviceLost(deviceLost)
{}

/**
 * @brief Appends every current aural-alert finding: unusable override files, the
 *        disabled-with-alarms warning (spec 0088 R7), and the lost-device pair.
 */
void UI::Alarms::AnnunciatorChecker::collect(QList<Finding>& out, bool projectDefinesAlarms) const
{
  for (const auto& issue : m_theme.issues()) {
    Finding f;
    f.severity    = Misc::ProblemCenter::Warning;
    f.code        = QStringLiteral("alarms.sound-file");
    f.title       = tr("Alarm sound file unavailable: %1").arg(issue.path);
    f.explanation = issue.reason;
    f.remedy = tr("Pick a valid PCM WAV file, or clear the override to use the bundled sound.");
    out.append(f);
  }

  if (projectDefinesAlarms && !m_theme.enabled()) {
    Finding f;
    f.severity    = Misc::ProblemCenter::Warning;
    f.code        = QStringLiteral("alarms.disabled");
    f.title       = tr("Aural alerts are off, but this project defines alarms");
    f.explanation = tr("The project configures alarm bands or alarm sounds, and none of them "
                       "will be heard while the master enable is off.");
    f.remedy      = tr("Enable alarm sounds in Preferences > Sounds, or remove the project's "
                       "alarm configuration.");
    out.append(f);
  }

  if (m_deviceLost)
    collectDeviceFindings(out);
}

/**
 * @brief The lost-device pair: no output at all, or playback fallen back to the system default.
 */
void UI::Alarms::AnnunciatorChecker::collectDeviceFindings(QList<Finding>& out) const
{
  Finding f;
  f.severity = Misc::ProblemCenter::Warning;
  if (m_theme.outputDeviceId().isEmpty() || !m_player.running()) {
    f.code        = QStringLiteral("alarms.no-output-device");
    f.title       = tr("No audio output device is available");
    f.explanation = tr("Alarm sounds cannot play until an output device is present.");
    f.remedy      = tr("Connect an audio output, or disable sounds in Preferences > Sounds.");
    out.append(f);
    return;
  }

  f.code        = QStringLiteral("alarms.output-device");
  f.title       = tr("Alarm sound device '%1' not found").arg(m_theme.outputDeviceName());
  f.explanation = tr("Alarm sounds are playing on the system default output instead.");
  f.remedy      = tr("Reconnect the device, or pick another one in Preferences > Sounds.");
  out.append(f);
}
