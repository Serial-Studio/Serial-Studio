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

#include "DataModel/ProjectModel.h"
#include "IO/DeviceManager.h"
#include "IO/FrameReader.h"
#include "IO/HAL_Driver.h"
#include "SSAssert.h"

/**
 * @brief Binds the live device table and the project it is built from; both outlive this object.
 */
IO::DeviceTableQuery::DeviceTableQuery(const DeviceTable& devices,
                                       DataModel::ProjectModel& projectModel)
  : m_devices(devices), m_projectModel(projectModel)
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
 * @brief Sums the per-device frame-reader counters for the 1 Hz diagnostics sample. No caching and
 *        no signal: this is pulled once per second and must never be called on the frame path.
 */
IO::LinkStats IO::DeviceTableQuery::linkStats() const
{
  LinkStats stats{};
  for (const auto& [id, dm] : m_devices) {
    const auto* reader = dm ? dm->frameReader() : nullptr;
    if (!reader)
      continue;

    stats.bytesIn         += reader->bytesReceived();
    stats.droppedFrames   += reader->droppedFrameCount();
    stats.overflowBytes   += reader->overflowBytes();
    stats.checksumErrors  += reader->checksumErrorCount();
    stats.framesExtracted += reader->framesExtracted();
  }

  return stats;
}

/**
 * @brief True when every project source has a device whose driver is configured.
 */
bool IO::DeviceTableQuery::projectConfigurationOk() const
{
  const auto& sources = m_projectModel.sources();
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
