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

#include "IO/ConnectionManager/ExclusiveResourceGuard.h"

#include <QCoreApplication>
#include <QHash>

#include "Core/Bus/Messages.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SSAssert.h"
#include "IO/DeviceManager.h"

/**
 * @brief Translates a conflict message in the connection manager's context, where the connect
 *        failures the user already sees are filed.
 */
[[nodiscard]] static QString trConflict(const char* text)
{
  return QCoreApplication::translate("IO::ConnectionManager", text);
}

/**
 * @brief Names a source the way the project editor shows it, so the message points at something
 *        the user can actually find and edit.
 */
[[nodiscard]] static QString sourceLabel(const DataModel::Source& source)
{
  if (!source.title.isEmpty())
    return source.title;

  return trConflict("Source %1").arg(source.sourceId);
}

/**
 * @brief Tells the user which two sources collide, and on what.
 */
static void reportConflict(const QString& first, const QString& second, const QString& resource)
{
  Core::Prompt::showMessageBox(
    trConflict("Two sources use the same port"),
    trConflict("\"%1\" and \"%2\" are both set to %3, and a serial port can only be opened by "
               "one source. Assign a different port to one of them before connecting.")
      .arg(first, second, resource),
    Core::Prompt::Critical,
    trConflict("Connection Aborted"));
}

/**
 * @brief Binds the live device table and the project snapshot it is built from; both outlive this
 *        object.
 */
IO::ExclusiveResourceGuard::ExclusiveResourceGuard(
  const DeviceTable& devices,
  const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& project)
  : m_devices(devices), m_project(project)
{}

/**
 * @brief Returns true when every project source claims a different exclusive device, reporting
 *        the first collision it finds. Sources are walked in project order, so the message names
 *        the same pair on every attempt instead of whichever one the device map happened to hash
 *        first.
 */
bool IO::ExclusiveResourceGuard::verifyProjectSources() const
{
  SS_ASSERT(m_project != nullptr, return true);

  QHash<QString, QString> claims;
  for (const auto& source : m_project->sources) {
    const QString resource = resourceForSource(source.sourceId);
    if (resource.isEmpty())
      continue;

    const QString key = resource.toLower();
    const auto claim  = claims.constFind(key);
    if (claim == claims.constEnd()) {
      claims.insert(key, sourceLabel(source));
      continue;
    }

    reportConflict(claim.value(), sourceLabel(source), resource);
    return false;
  }

  return true;
}

/**
 * @brief Asks the driver behind @p sourceId what exclusive device it claims; a source with no
 *        device, no driver or nothing exclusive to claim answers empty.
 */
QString IO::ExclusiveResourceGuard::resourceForSource(int sourceId) const
{
  auto it = m_devices.find(sourceId);
  if (it == m_devices.end() || !it->second || !it->second->driver())
    return {};

  return it->second->driver()->exclusiveResource();
}
