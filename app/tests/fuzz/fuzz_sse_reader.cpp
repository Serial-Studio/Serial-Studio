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

#include <cstddef>
#include <cstdint>
#include <QByteArray>
#include <QJsonObject>
#include <QString>

#include "AI/SseEventReader.h"

/**
 * @brief Feeds provider stream bytes to the SSE parser, once whole and once split at a
 *        byte offset the input itself chooses. The stream is attacker-adjacent -- it is
 *        whatever the model endpoint returns -- and the split pass is what exercises the
 *        carry-over state (pending CR, partial frame) that a single feed never reaches.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if (size == 0)
    return 0;

  const QByteArray input(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  {
    AI::SseEventReader reader;
    QObject::connect(&reader,
                     &AI::SseEventReader::frameReceived,
                     &reader,
                     [](const QString& name, const QJsonObject& o) {
                       (void)name;
                       (void)o;
                     });
    reader.feed(input);
    reader.feed({});
  }

  {
    const qsizetype split = static_cast<qsizetype>(data[0]) % input.size();
    AI::SseEventReader reader;
    reader.feed(input.left(split));
    reader.feed(input.mid(split));
    reader.feed({});
  }

  return 0;
}
