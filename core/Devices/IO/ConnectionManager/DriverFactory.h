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

#include "IO/HAL_Driver.h"
#include "SerialStudio.h"

namespace Misc::Diagnostics {
enum class Bus : int;
}  // namespace Misc::Diagnostics

namespace IO {

class DriverUiRegistry;

/**
 * @brief Builds the fresh driver instance a live source runs on, one case per bus type behind
 *        that bus's licence gate. It hands the driver back unopened and unwired: every
 *        connection whose receiver is the ConnectionManager (configurationChanged, openFinished)
 *        is made by the facade, so the spec-0050 verdict stays wired where the device map is.
 */
class DriverFactory {
public:
  explicit DriverFactory(DriverUiRegistry& uiDrivers);
  DriverFactory(DriverFactory&&)                 = delete;
  DriverFactory(const DriverFactory&)            = delete;
  DriverFactory& operator=(DriverFactory&&)      = delete;
  DriverFactory& operator=(const DriverFactory&) = delete;

  [[nodiscard]] std::unique_ptr<HAL_Driver> create(SerialStudio::BusType type) const;

  [[nodiscard]] static bool diagnosticsBus(HAL_Driver* driver, Misc::Diagnostics::Bus& bus);

private:
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] std::unique_ptr<HAL_Driver> createMqtt() const;
  [[nodiscard]] std::unique_ptr<HAL_Driver> createOpcUa() const;
#endif

private:
#ifdef BUILD_COMMERCIAL
  DriverUiRegistry& m_uiDrivers;
#endif
};

}  // namespace IO
