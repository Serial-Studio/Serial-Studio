/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QString>
#include <vector>

#include "Misc/IconRegistry.h"
#include "SerialStudio.h"

namespace DataModel {

/**
 * @brief Builds a folder's "/"-joined path from any folder vector (root -> leaf).
 */
template<typename Folder>
QString folderDisplayPath(const std::vector<Folder>& folders, int folderId)
{
  QString path;
  int cur        = folderId;
  const int kMax = static_cast<int>(folders.size());
  for (int i = 0; i <= kMax && cur != -1; ++i) {
    const Folder* match = nullptr;
    for (const auto& f : folders)
      if (f.folderId == cur) {
        match = &f;
        break;
      }

    if (!match)
      break;

    path = path.isEmpty() ? match->title : (match->title + QLatin1Char('/') + path);
    cur  = match->parentFolderId;
  }

  return path;
}

/**
 * @brief Returns the QML icon path for a SerialStudio::BusType integer.
 */
inline QString busTypeIcon(int busType)
{
  static auto& registry = Misc::IconRegistry::instance();
  const char* name      = "uart";
  switch (static_cast<SerialStudio::BusType>(busType)) {
    case SerialStudio::BusType::Network:
      name = "network";
      break;
    case SerialStudio::BusType::BluetoothLE:
      name = "bluetooth";
      break;
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      name = "audio";
      break;
    case SerialStudio::BusType::ModBus:
      name = "modbus";
      break;
    case SerialStudio::BusType::CanBus:
      name = "canbus";
      break;
    case SerialStudio::BusType::RawUsb:
      name = "usb";
      break;
    case SerialStudio::BusType::HidDevice:
      name = "hid";
      break;
    case SerialStudio::BusType::Process:
      name = "process";
      break;
    case SerialStudio::BusType::Mqtt:
      name = "mqtt";
      break;
#endif
    default:
      break;
  }

  return registry.icon(QStringLiteral("devices"), QLatin1String(name), 16);
}

}  // namespace DataModel
