/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Licensing/MonotonicClock.h"

#include <QSettings>

#include "Licensing/MachineID.h"
#include "Licensing/SimpleCrypt.h"

/**
 * @brief Core flooring over an explicit settings store and cipher (for testing).
 */
QDateTime Licensing::MonotonicClock::nowFloored(QSettings& settings, SimpleCrypt& crypt)
{
  auto effective = QDateTime::currentDateTime();

  settings.beginGroup("licensing");
  const auto stored = settings.value("lastSeen", "").toString();
  settings.endGroup();

  if (!stored.isEmpty()) {
    const auto seen = QDateTime::fromString(crypt.decryptToString(stored), Qt::RFC2822Date);
    if (seen.isValid() && seen > effective)
      effective = seen;
  }

  const auto encoded = crypt.encryptToString(effective.toString(Qt::RFC2822Date));
  settings.beginGroup("licensing");
  settings.setValue("lastSeen", encoded);
  settings.endGroup();

  return effective;
}

/**
 * @brief Returns now floored at the highest wall-clock ever observed (anti clock-rewind).
 */
QDateTime Licensing::MonotonicClock::now()
{
  QSettings settings;
  SimpleCrypt crypt(MachineID::instance().machineSpecificKey());
  crypt.setIntegrityProtectionMode(SimpleCrypt::ProtectionHash);
  return nowFloored(settings, crypt);
}
