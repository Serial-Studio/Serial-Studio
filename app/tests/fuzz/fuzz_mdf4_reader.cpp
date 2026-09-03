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

#include <cstdint>
#include <QByteArray>
#include <QTemporaryFile>

#include "MDF4/PlayerLoaderWorker.h"

/**
 * @brief Drives the MDF4 replay decode with untrusted file bytes (spec 0075 R14.3). An .mf4 is
 *        opened straight from the user's disk, so a truncated header, a channel group claiming
 *        more samples than the data section holds, or a master channel with a nonsense sync type
 *        must come back as a failed payload rather than a read past the file.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if (size == 0 || size > (4u << 20))
    return 0;

  QTemporaryFile file;
  if (!file.open())
    return 0;

  file.write(reinterpret_cast<const char*>(data), static_cast<qint64>(size));
  file.flush();

  MDF4::PlayerLoaderWorker worker;
  worker.decodeFile(file.fileName(), 1, QVector<quint8>{});
  return 0;
}
