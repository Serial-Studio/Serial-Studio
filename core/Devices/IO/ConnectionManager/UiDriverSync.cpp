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

#include "AppState.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager/DriverUiRegistry.h"
#include "IO/Drivers/BluetoothLE.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the mirror to the registry it edits and the two modules that decide whether a
 *        mirror pass is legal at all (single-source ProjectFile with a saved file).
 */
IO::UiDriverSync::UiDriverSync(DriverUiRegistry& uiDrivers,
                               AppState& appState,
                               DataModel::ProjectModel& projectModel)
  : m_syncingFromProject(false)
  , m_uiDrivers(uiDrivers)
  , m_appState(appState)
  , m_projectModel(projectModel)
{}

//--------------------------------------------------------------------------------------------------
// Project to UI driver
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns (lazily configuring) the UI-config driver that edits source @p deviceId. The
 *        applied settings are fenced by the latch so the driver's own configurationChanged
 *        cannot be mistaken for a user edit and written straight back to the project.
 */
IO::HAL_Driver* IO::UiDriverSync::driverForEditing(int deviceId)
{
  SS_ASSERT_LOG(deviceId >= 0);

  const DataModel::Source* srcPtr = nullptr;
  for (const auto& src : m_projectModel.sources()) {
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
 * @brief Applies source[0]'s busType and connectionSettings to the matching UI-config driver,
 *        reporting whether anything was applied. Unsaved projects (empty json path) are skipped
 *        so API-configured hardware settings are not clobbered. The applier runs inside the latch
 *        when the bus type moved, keeping busTypeChanged ahead of the settings, as it always was.
 */
bool IO::UiDriverSync::syncFromSource0(SerialStudio::BusType current, const BusTypeApplier& applier)
{
  SS_ASSERT(applier != nullptr, return false);

  const auto& srcs = m_projectModel.sources();
  if (m_appState.operationMode() != SerialStudio::ProjectFile || srcs.size() != 1)
    return false;

  if (m_projectModel.jsonFilePath().isEmpty())
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

//--------------------------------------------------------------------------------------------------
// UI driver to live driver and back to the project
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors every property of @p uiDriver onto @p liveDriver; the live driver is
 *        signal-blocked to suppress its fan-out (the UI driver's configurationChanged still
 *        notifies downstream, so nothing is lost).
 */
void IO::UiDriverSync::syncToLive(HAL_Driver* uiDriver, HAL_Driver* liveDriver) const
{
  if (m_syncingFromProject)
    return;

  const auto& srcs = m_projectModel.sources();
  if (m_appState.operationMode() == SerialStudio::ProjectFile && srcs.size() > 1)
    return;

  if (!uiDriver || !liveDriver || liveDriver == uiDriver)
    return;

  QSignalBlocker blocker(liveDriver);
  for (const auto& prop : uiDriver->driverProperties())
    liveDriver->setDriverProperty(prop.key, prop.value);
}

/**
 * @brief Captures @p uiDriver's current settings back into source[0], reporting whether the
 *        project is saved on disk and therefore owes a debounced autosave. @p sender is the
 *        object that reported the edit: a report from any driver other than the active one is
 *        another bus's echo and never touches the project.
 */
bool IO::UiDriverSync::captureToSource0(SerialStudio::BusType busType,
                                        HAL_Driver* uiDriver,
                                        const QObject* sender) const
{
  if (m_syncingFromProject || !uiDriver)
    return false;

  if (m_appState.operationMode() != SerialStudio::ProjectFile
      || m_projectModel.sources().size() != 1)
    return false;

  if (sender && sender != uiDriver)
    return false;

  QJsonObject settings;
  for (const auto& prop : uiDriver->driverProperties())
    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

  const auto deviceId = uiDriver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  m_projectModel.setSource0ConnectionSettings(settings);
  m_projectModel.setSource0BusType(static_cast<int>(busType));

  return !m_projectModel.jsonFilePath().isEmpty();
}

/**
 * @brief Writes the project file after the UI-driver edit debounce elapses, but only while it
 *        is still the single-source project file the debounce was armed for.
 */
void IO::UiDriverSync::autosaveSource0() const
{
  if (m_projectModel.jsonFilePath().isEmpty())
    return;

  if (m_appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_projectModel.sources().size() != 1)
    return;

  (void)m_projectModel.saveJsonFile(false);
}
