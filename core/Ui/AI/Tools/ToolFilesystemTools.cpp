/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolFilesystemTools.h"

#include "AI/FileSandbox.h"
#include "AI/Tools/ToolSchemas.h"

namespace AI::ToolDetail {

/**
 * @brief Routes an fs.* tool call to the bounded FileSandbox primitive on the calling thread.
 *        The conversation runs fs.read and fs.search on a worker (spec 0075, J3), so nothing
 *        added here may touch a GUI-owned object: the sandbox guards its own state and every
 *        primitive returns a value rather than mutating session state.
 */
QJsonObject executeFsTool(const QString& name, const QJsonObject& args)
{
  static auto& sandbox = AI::FileSandbox::instance();
  if (name == QStringLiteral("fs.list"))
    return sandbox.list(args);

  if (name == QStringLiteral("fs.read"))
    return sandbox.read(args);

  if (name == QStringLiteral("fs.search"))
    return sandbox.search(args);

  if (name == QStringLiteral("fs.write"))
    return sandbox.write(args);

  if (name == QStringLiteral("fs.append"))
    return sandbox.append(args);

  if (name == QStringLiteral("fs.delete"))
    return sandbox.remove(args);

  QJsonObject out;
  out[QStringLiteral("ok")]    = false;
  out[QStringLiteral("error")] = QStringLiteral("unknown_fs_tool");
  return out;
}

}  // namespace AI::ToolDetail
