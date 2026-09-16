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
#include <unordered_map>

namespace Core::Bus {
struct ProjectStructureSnapshot;
}  // namespace Core::Bus

namespace IO {

class DeviceManager;

/**
 * @brief Refuses a connect where two project sources claim one exclusive device, which in
 *        practice is one serial port: the driver that loses the race fails with a port-busy
 *        error naming neither source, and half the project comes up silently. Read-only over the
 *        device table; the only thing it does besides look is tell the user why nothing opened.
 */
class ExclusiveResourceGuard {
public:
  using DeviceTable = std::unordered_map<int, std::unique_ptr<DeviceManager>>;

  ExclusiveResourceGuard(const DeviceTable& devices,
                         const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& project);

  ExclusiveResourceGuard(ExclusiveResourceGuard&&)                 = delete;
  ExclusiveResourceGuard(const ExclusiveResourceGuard&)            = delete;
  ExclusiveResourceGuard& operator=(ExclusiveResourceGuard&&)      = delete;
  ExclusiveResourceGuard& operator=(const ExclusiveResourceGuard&) = delete;

  [[nodiscard]] bool verifyProjectSources() const;

private:
  [[nodiscard]] QString resourceForSource(int sourceId) const;

private:
  const DeviceTable& m_devices;
  const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& m_project;
};

}  // namespace IO
