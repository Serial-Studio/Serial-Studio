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

#pragma once

#include <QCoreApplication>
#include <QList>

#include "Misc/ProblemCenter.h"

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace IO::Audio {
class SoundPlayer;
}  // namespace IO::Audio

namespace UI {
namespace Alarms {

class SoundTheme;

/**
 * @brief Builds the aural-alert findings for the pull-based `alarms.sounds` check (specs 0087,
 *        0088 R7): unusable sound files, a lost or absent output device, and a project that
 *        defines alarms while the master enable is off. Binds the facade's members by
 *        reference; the facade owns registration and lifetime.
 */
class AnnunciatorChecker {
  Q_DECLARE_TR_FUNCTIONS(AnnunciatorChecker)

public:
  AnnunciatorChecker(SoundTheme& theme, IO::Audio::SoundPlayer& player, const bool& deviceLost);
  AnnunciatorChecker(AnnunciatorChecker&&)                 = delete;
  AnnunciatorChecker(const AnnunciatorChecker&)            = delete;
  AnnunciatorChecker& operator=(AnnunciatorChecker&&)      = delete;
  AnnunciatorChecker& operator=(const AnnunciatorChecker&) = delete;

  [[nodiscard]] static bool definesAlarms(const DataModel::ProjectModel& project);

  void collect(QList<Misc::ProblemCenter::Finding>& out, bool projectDefinesAlarms) const;

private:
  void collectDeviceFindings(QList<Misc::ProblemCenter::Finding>& out) const;

  SoundTheme& m_theme;
  IO::Audio::SoundPlayer& m_player;
  const bool& m_deviceLost;
};

}  // namespace Alarms
}  // namespace UI
