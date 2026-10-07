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

#include "IO/ConnectionManager/DeviceTableQuery.h"

#include "Core/Bus/Messages.h"
#include "Core/DataModel/FrameSupport.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/SSAssert.h"
#include "IO/DeviceManager.h"

/**
 * @brief Binds the live device table and the project snapshot it is built from; both outlive this
 *        object.
 */
IO::DeviceTableQuery::DeviceTableQuery(
  const DeviceTable& devices,
  const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& project)
  : m_devices(devices), m_project(project)
{}

/**
 * @brief True when ANY device is open; the project-mode connected verdict.
 */
bool IO::DeviceTableQuery::anyOpen() const
{
  for (const auto& [id, dm] : m_devices)
    if (dm && dm->isOpen())
      return true;

  return false;
}

/**
 * @brief True when device 0 is open; the single-source connected verdict.
 */
bool IO::DeviceTableQuery::primaryOpen() const
{
  auto it = m_devices.find(0);
  return it != m_devices.end() && it->second && it->second->isOpen();
}

/**
 * @brief Returns whether the device with the given source ID is currently open.
 */
bool IO::DeviceTableQuery::isDeviceConnected(int deviceId) const
{
  auto it = m_devices.find(deviceId);
  return it != m_devices.end() && it->second && it->second->isOpen();
}

/**
 * @brief True while any device's driver still has a dial in flight.
 */
bool IO::DeviceTableQuery::anyDeviceConnecting() const
{
  for (const auto& [id, dm] : m_devices)
    if (dm && dm->driver() && dm->driver()->isConnecting())
      return true;

  return false;
}

/**
 * @brief Returns the number of currently open devices.
 */
int IO::DeviceTableQuery::connectedDeviceCount() const
{
  int count = 0;
  for (const auto& [id, dm] : m_devices)
    if (dm && dm->isOpen())
      ++count;

  return count;
}

/**
 * @brief Reports the link as connected, connecting or idle. A live session outranks a device
 *        still dialing beside it (multi-source), so connected wins over connecting.
 */
QString IO::DeviceTableQuery::linkState(bool connected, bool connecting)
{
  if (connected)
    return QStringLiteral("connected");

  if (connecting)
    return QStringLiteral("connecting");

  return QStringLiteral("idle");
}

/**
 * @brief True when every project source has a device whose driver is configured.
 */
bool IO::DeviceTableQuery::projectConfigurationOk() const
{
  const auto& sources = m_project->sources;
  if (sources.empty())
    return false;

  for (const auto& src : sources) {
    auto it = m_devices.find(src.sourceId);
    if (it == m_devices.end() || !it->second || !it->second->driver())
      return false;

    if (!it->second->driver()->configurationOk())
      return false;
  }

  return true;
}

/**
 * @brief Names what the buses of the pending connect need: free buses connect in every licensing
 *        state (spec 0092), a new bus is Pro until someone decides otherwise (as in DriverFactory),
 *        and a project with several sources is Pro. An empty snapshot falls back to the bus.
 */
IO::DeviceTableQuery::ConnectRefusal IO::DeviceTableQuery::busRefusal(
  const SerialStudio::BusType busType, const SerialStudio::OperationMode mode) const
{
  const auto freeBus = [](SerialStudio::BusType type) {
    return type == SerialStudio::BusType::UART || type == SerialStudio::BusType::Network
        || type == SerialStudio::BusType::BluetoothLE;
  };

  if (mode != SerialStudio::ProjectFile || !m_project || m_project->sources.empty())
    return freeBus(busType) ? ConnectRefusal::None : ConnectRefusal::ProBus;

  if (m_project->sources.size() > 1)
    return ConnectRefusal::MultiSource;

  for (const auto& src : m_project->sources)
    if (!freeBus(static_cast<SerialStudio::BusType>(src.busType)))
      return ConnectRefusal::ProBus;

  return ConnectRefusal::None;
}

/**
 * @brief Names why the pending connect needs an entitlement: its buses first, then the project's
 *        transforms and user tables (spec 0094), read from the snapshot so no binding can be
 *        forgotten.
 */
IO::DeviceTableQuery::ConnectRefusal IO::DeviceTableQuery::connectRefusal(
  const SerialStudio::BusType busType, const SerialStudio::OperationMode mode) const
{
  const auto bus = busRefusal(busType, mode);
  if (bus != ConnectRefusal::None)
    return bus;

  const auto content = projectContent(mode);
  if (content.transforms > 0 || content.tables > 0)
    return ConnectRefusal::ProContent;

  return ConnectRefusal::None;
}

/**
 * @brief Counts the Pro content the project would run: none outside ProjectFile mode, where no
 *        project transform or table is in play.
 */
SerialStudio::ProContentSummary IO::DeviceTableQuery::projectContent(
  const SerialStudio::OperationMode mode) const
{
  if (mode != SerialStudio::ProjectFile || !m_project)
    return {0, 0};

  return SerialStudio::proContentSummary(m_project->groups, m_project->userTableCount);
}

/**
 * @brief Returns the device id @p driver backs, or -1 when no device owns it. A null driver is an
 *        ordinary miss: the recovery paths call this with whatever the sender handed them.
 */
int IO::DeviceTableQuery::deviceIdForDriver(const HAL_Driver* driver) const
{
  if (driver == nullptr)
    return -1;

  for (const auto& [id, dm] : m_devices)
    if (dm && dm->driver() == driver)
      return id;

  return -1;
}

/**
 * @brief Snapshots the device ids, optionally skipping the primary. Every fan-out iterates this
 *        copy instead of the table: an open or a close can spin the event loop (error boxes,
 *        control scripts), and a rebuild landing there would invalidate a live iterator.
 */
std::vector<int> IO::DeviceTableQuery::deviceIdSnapshot(bool projectSourcesOnly) const
{
  std::vector<int> ids;
  ids.reserve(m_devices.size());
  for (const auto& [id, dm] : m_devices)
    if (id > 0 || !projectSourcesOnly)
      ids.push_back(id);

  return ids;
}
