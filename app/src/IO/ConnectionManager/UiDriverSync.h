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

#include <functional>

#include "SerialStudio.h"

class QObject;
class AppState;

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace IO {

class HAL_Driver;
class DriverUiRegistry;

/**
 * @brief The single-source project settings mirror: source[0] onto the UI-config driver, the
 *        UI-config driver onto the live one, and the UI-config driver back into source[0]. All
 *        three directions share ONE re-entrancy latch, which is why they live together: split
 *        them and a project load echoes straight back, churning undo history and autosave.
 */
class UiDriverSync {
public:
  using BusTypeApplier = std::function<void(SerialStudio::BusType)>;

  UiDriverSync(DriverUiRegistry& uiDrivers,
               AppState& appState,
               DataModel::ProjectModel& projectModel);
  UiDriverSync(UiDriverSync&&)                 = delete;
  UiDriverSync(const UiDriverSync&)            = delete;
  UiDriverSync& operator=(UiDriverSync&&)      = delete;
  UiDriverSync& operator=(const UiDriverSync&) = delete;

  void autosaveSource0() const;
  void syncToLive(HAL_Driver* uiDriver, HAL_Driver* liveDriver) const;

  [[nodiscard]] HAL_Driver* driverForEditing(int deviceId);
  [[nodiscard]] bool syncFromSource0(SerialStudio::BusType current, const BusTypeApplier& applier);
  [[nodiscard]] bool captureToSource0(SerialStudio::BusType busType,
                                      HAL_Driver* uiDriver,
                                      const QObject* sender) const;

private:
  bool m_syncingFromProject;
  DriverUiRegistry& m_uiDrivers;
  AppState& m_appState;
  DataModel::ProjectModel& m_projectModel;
};

}  // namespace IO
