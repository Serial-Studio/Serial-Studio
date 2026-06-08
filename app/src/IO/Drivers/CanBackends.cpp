/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/CanBackends.h"

#include "IO/Drivers/GsUsbCanBackend.h"
#include "IO/Drivers/SeeedCanBackend.h"
#include "IO/Drivers/SlcanBackend.h"

/**
 * @brief Returns every synthetic CAN backend; add a row here to register a new one.
 */
const QList<IO::Drivers::CanBackends::Entry>& IO::Drivers::CanBackends::all()
{
  static const QList<Entry> entries = {
    GsUsbCanBackend::registration(),
    SlcanBackend::registration(),
    SeeedCanBackend::registration(),
  };
  return entries;
}

/**
 * @brief Returns the backend registered under @p key, or nullptr when none matches.
 */
const IO::Drivers::CanBackends::Entry* IO::Drivers::CanBackends::find(const QString& key)
{
  for (const Entry& entry : all())
    if (entry.key == key)
      return &entry;

  return nullptr;
}
