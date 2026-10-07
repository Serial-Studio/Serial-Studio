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

#include <memory>
#include <QString>
#include <QtGlobal>
#include <unordered_map>
#include <vector>

#include "Core/SerialStudio.h"

namespace Core::Bus {
struct ProjectStructureSnapshot;
}  // namespace Core::Bus

namespace SerialStudio {
struct ProContentSummary;
}  // namespace SerialStudio

namespace IO {

class DeviceManager;
class HAL_Driver;

/**
 * @brief Every read over the live device table (spec 0075, C14): open counts, the link state, the
 *        configuration verdict, and the id lookups the connect fan-outs iterate. Read-only by
 * construction, so no path through here can mutate a device or emit a signal.
 */
class DeviceTableQuery {
public:
  using DeviceTable = std::unordered_map<int, std::unique_ptr<DeviceManager>>;

  /**
   * @brief Why a connect needs an entitlement, in the order the checks run.
   */
  enum class ConnectRefusal {
    None,
    ProBus,
    MultiSource,
    ProContent,
  };

  DeviceTableQuery(const DeviceTable& devices,
                   const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& project);

  DeviceTableQuery(DeviceTableQuery&&)                 = delete;
  DeviceTableQuery(const DeviceTableQuery&)            = delete;
  DeviceTableQuery& operator=(DeviceTableQuery&&)      = delete;
  DeviceTableQuery& operator=(const DeviceTableQuery&) = delete;

  [[nodiscard]] bool anyOpen() const;
  [[nodiscard]] bool primaryOpen() const;
  [[nodiscard]] bool isDeviceConnected(int deviceId) const;
  [[nodiscard]] bool anyDeviceConnecting() const;
  [[nodiscard]] int connectedDeviceCount() const;
  [[nodiscard]] bool projectConfigurationOk() const;
  [[nodiscard]] ConnectRefusal busRefusal(SerialStudio::BusType busType,
                                          SerialStudio::OperationMode mode) const;
  [[nodiscard]] ConnectRefusal connectRefusal(SerialStudio::BusType busType,
                                              SerialStudio::OperationMode mode) const;
  [[nodiscard]] SerialStudio::ProContentSummary projectContent(
    SerialStudio::OperationMode mode) const;
  [[nodiscard]] int deviceIdForDriver(const HAL_Driver* driver) const;
  [[nodiscard]] std::vector<int> deviceIdSnapshot(bool projectSourcesOnly) const;

  [[nodiscard]] static QString linkState(bool connected, bool connecting);

private:
  const DeviceTable& m_devices;
  const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& m_project;
};

}  // namespace IO
