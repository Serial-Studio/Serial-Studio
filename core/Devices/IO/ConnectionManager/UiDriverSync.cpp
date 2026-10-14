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

#include "IO/ConnectionManager/UiDriverSync.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QSignalBlocker>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/Runtime.h"
#include "Core/SSAssert.h"
#include "IO/ConnectionManager/DriverUiRegistry.h"
#include "IO/Drivers/BluetoothLE.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serialises a UI driver's properties the way the project stores them, never the
 *        password-typed ones: the project file is shared, so secrets stay in the vault.
 */
[[nodiscard]] static QJsonObject driverSettings(const IO::HAL_Driver& driver)
{
  QJsonObject settings;
  for (const auto& prop : driver.driverProperties()) {
    if (prop.type == IO::DriverProperty::Password)
      continue;

    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));
  }

  const auto deviceId = driver.deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  return settings;
}

//--------------------------------------------------------------------------------------------------
// Constructor & wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the registry, the facade's cached mode, the retained project snapshot the facade
 *        refreshes and the bus the writes go out on.
 */
IO::UiDriverSync::UiDriverSync(DriverUiRegistry& uiDrivers,
                               const SerialStudio::OperationMode& operationMode,
                               const ProjectSnapshot& project,
                               Core::Bus::MessageBus& bus)
  : m_syncingFromProject(false)
  , m_uiDrivers(uiDrivers)
  , m_operationMode(operationMode)
  , m_project(project)
  , m_bus(bus)
{}

/**
 * @brief Serves the project's two asks of the UI drivers (spec 0077): capture a source's settings
 *        from its UI driver, and restore a source's saved settings onto it. Both are answered
 *        directly on the caller's thread, so the API handler that asked reads the result at once.
 *        @p liveDriver resolves a source's live driver, the one a captured edit is mirrored onto.
 */
void IO::UiDriverSync::wire(QObject& receiver, LiveDriverLookup liveDriver)
{
  SS_ASSERT_LOG(liveDriver != nullptr);
  m_liveDriver      = std::move(liveDriver);
  m_captureRequests = m_bus.subscribe<Core::Bus::SourceSettingsCaptureRequested>(
    &receiver,
    [this](const std::shared_ptr<const Core::Bus::SourceSettingsCaptureRequested>& request) {
      serveCapture(request->sourceId, request->busType);
    },
    Qt::DirectConnection);
  m_restoreRequests = m_bus.subscribe<Core::Bus::SourceSettingsRestoreRequested>(
    &receiver,
    [this](const std::shared_ptr<const Core::Bus::SourceSettingsRestoreRequested>& request) {
      serveRestore(request->sourceId);
    },
    Qt::DirectConnection);
}

//--------------------------------------------------------------------------------------------------
// Project to UI driver
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns (lazily configuring) the UI-config driver instance that edits source
 *        @p deviceId: the project's saved settings are applied to it under the re-entrancy fence,
 *        and a BLE editor starts discovering when it has nothing to show yet. The snapshot is
 *        pinned locally: applying settings emits, and an emit may republish the snapshot.
 */
IO::HAL_Driver* IO::UiDriverSync::driverForEditing(int deviceId)
{
  SS_ASSERT_LOG(deviceId >= 0);
  const auto project = m_project;
  SS_ASSERT(project != nullptr, return nullptr);

  const DataModel::Source* srcPtr = nullptr;
  for (const auto& src : project->sources) {
    if (src.sourceId == deviceId) {
      srcPtr = &src;
      break;
    }
  }

  if (!srcPtr)
    return nullptr;

  const auto busType = static_cast<SerialStudio::BusType>(srcPtr->busType);
  HAL_Driver* uiDrv  = m_uiDrivers.forBusType(busType);
  if (!uiDrv)
    return nullptr;

  if (!srcPtr->connectionSettings.isEmpty()) {
    m_syncingFromProject = true;
    uiDrv->applyConnectionSettings(srcPtr->connectionSettings);
    m_syncingFromProject = false;
  }

  if (busType == SerialStudio::BusType::BluetoothLE) {
    auto* ble = qobject_cast<IO::Drivers::BluetoothLE*>(uiDrv);
    if (ble && ble->deviceCount() == 0)
      ble->startDiscovery();
  }

  return uiDrv;
}

/**
 * @brief Applies source[0]'s bus type (through @p applier) and connection settings to the matching
 *        UI-config driver. Only a saved single-source project is mirrored, or a deployment
 *        package's, which has no file but is the session's one project (spec 0096).
 */
bool IO::UiDriverSync::syncFromSource0(SerialStudio::BusType current, const BusTypeApplier& applier)
{
  SS_ASSERT(applier != nullptr, return false);
  const auto project = m_project;
  SS_ASSERT(project != nullptr, return false);

  const auto& srcs = project->sources;
  if (m_operationMode != SerialStudio::ProjectFile || srcs.size() != 1)
    return false;

  if (project->filePath.isEmpty() && !Core::Runtime::sealed())
    return false;

  const auto& src    = srcs[0];
  const auto newType = static_cast<SerialStudio::BusType>(src.busType);

  m_syncingFromProject = true;

  if (current != newType)
    applier(newType);

  HAL_Driver* uiDriver = m_uiDrivers.forBusType(newType);
  if (uiDriver && !src.connectionSettings.isEmpty())
    uiDriver->applyConnectionSettings(src.connectionSettings);

  m_syncingFromProject = false;
  return true;
}

/**
 * @brief Restores a source's saved settings onto its UI driver on the project's request; the
 *        editor call does exactly that, so the two asks share one body.
 */
void IO::UiDriverSync::serveRestore(int sourceId)
{
  SS_ASSERT(sourceId >= 0, return);

  (void)driverForEditing(sourceId);
}

//--------------------------------------------------------------------------------------------------
// UI driver to live driver and back to the project
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors every property of the UI-config driver onto the live driver, silently, unless a
 *        project mirror is mid-flight or the project has several sources.
 */
void IO::UiDriverSync::syncToLive(HAL_Driver* uiDriver, HAL_Driver* liveDriver) const
{
  if (m_syncingFromProject)
    return;

  SS_ASSERT(m_project != nullptr, return);

  const auto& srcs = m_project->sources;
  if (m_operationMode == SerialStudio::ProjectFile && srcs.size() > 1)
    return;

  if (!uiDriver || !liveDriver || liveDriver == uiDriver)
    return;

  QSignalBlocker blocker(liveDriver);
  for (const auto& prop : uiDriver->driverProperties())
    liveDriver->setDriverProperty(prop.key, prop.value);
}

/**
 * @brief Asks the project to mirror source[0]'s bus type only (the settings stay), the Setup
 *        pane's bus switch on an already-built primary device.
 */
void IO::UiDriverSync::publishSource0BusType(SerialStudio::BusType busType) const
{
  m_bus.publish<Core::Bus::Source0ConnectionSettingsChanged>(
    static_cast<int>(busType), QJsonObject(), false, false);
}

/**
 * @brief Captures the UI-config driver's settings, passwords excluded, back into source[0] of a
 *        single-source project (asking the project to autosave when @p autosave is set and it lives
 *        on disk); true when the request went out. A change reported by another driver than the
 *        active one is ignored.
 */
bool IO::UiDriverSync::captureToSource0(SerialStudio::BusType busType,
                                        HAL_Driver* uiDriver,
                                        const QObject* sender,
                                        bool autosave) const
{
  if (m_syncingFromProject || !uiDriver)
    return false;

  SS_ASSERT(m_project != nullptr, return false);

  if (m_operationMode != SerialStudio::ProjectFile || m_project->sources.size() != 1)
    return false;

  if (sender && sender != uiDriver)
    return false;

  m_bus.publish<Core::Bus::Source0ConnectionSettingsChanged>(
    static_cast<int>(busType), driverSettings(*uiDriver), true, autosave);
  return true;
}

/**
 * @brief Retains the active UI driver's bus type and settings, the facts a legacy project without
 *        a sources array is seeded from (spec 0077 replaces the loader's reach into the driver).
 *        A change reported by a driver other than the active one leaves the fact alone.
 */
void IO::UiDriverSync::publishActiveUiDriverSettings(SerialStudio::BusType busType,
                                                     HAL_Driver* uiDriver,
                                                     const QObject* sender) const
{
  if (sender && sender != uiDriver)
    return;

  m_bus.publishState<Core::Bus::ActiveUiDriverSettings>(
    static_cast<int>(busType), uiDriver ? driverSettings(*uiDriver) : QJsonObject());
}

/**
 * @brief Answers a capture request: the UI driver for @p busType is serialised without its
 *        password-typed properties, handed back to the project under @p sourceId and mirrored onto
 *        the source's live driver (syncToLive covers only a single-source project; any other
 *        source's edit used to wait for the next device rebuild, in practice a restart).
 */
void IO::UiDriverSync::serveCapture(int sourceId, int busType) const
{
  SS_ASSERT(sourceId >= 0, return);

  HAL_Driver* driver = m_uiDrivers.forBusType(static_cast<SerialStudio::BusType>(busType));
  if (!driver)
    return;

  m_bus.publish<Core::Bus::SourceConnectionSettingsCaptured>(sourceId, driverSettings(*driver));

  if (m_operationMode != SerialStudio::ProjectFile || !m_liveDriver)
    return;

  HAL_Driver* live = m_liveDriver(sourceId);
  if (!live || live == driver)
    return;

  QSignalBlocker blocker(live);
  for (const auto& prop : driver->driverProperties())
    live->setDriverProperty(prop.key, prop.value);
}
