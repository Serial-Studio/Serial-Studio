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

#include <QElapsedTimer>
#include <QSettings>

#include "Licensing/MachineID.h"
#include "Licensing/SimpleCrypt.h"

// Shortest interval between two persisted floors: the getters reading it are QML-bound (K10)
static constexpr qint64 kPersistIntervalMs = 60000;

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

    if (seen.isValid() && seen <= effective && seen.msecsTo(effective) < kPersistIntervalMs)
      return effective;
  }

  const auto encoded = crypt.encryptToString(effective.toString(Qt::RFC2822Date));
  settings.beginGroup("licensing");
  settings.setValue("lastSeen", encoded);
  settings.endGroup();

  return effective;
}

/**
 * @brief Returns now floored at the highest wall-clock ever observed (anti clock-rewind). The
 *        stored floor is re-read and re-written at most once a minute; in between, the floor
 *        cached from that pass still catches a rewind, so the anti-rewind guarantee is unchanged
 *        while a property read costs no settings access at all.
 */
QDateTime Licensing::MonotonicClock::now()
{
  static bool primed = false;
  static QDateTime floored;
  static QElapsedTimer sincePersist;

  const auto wall = QDateTime::currentDateTime();
  if (!primed || sincePersist.hasExpired(kPersistIntervalMs)) {
    QSettings settings;
    static auto& machineId = MachineID::instance();
    SimpleCrypt crypt(machineId.machineSpecificKey());
    crypt.setIntegrityProtectionMode(SimpleCrypt::ProtectionHash);

    floored = nowFloored(settings, crypt);
    sincePersist.restart();
    primed = true;
    return floored;
  }

  if (floored.isValid() && floored > wall)
    return floored;

  return wall;
}
