/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <cstdint>
#include <vector>

#include "Sessions/BlockReader.h"
#include "Sessions/StreamBlockCodec.h"

/**
 * @brief Drives the historian's block decoders with untrusted bytes (spec 0075 R14.3). Every blob
 *        in a `blocks` row comes off disk and may be truncated, foreign or hostile, so each
 *        decoder must reject a length it cannot satisfy instead of reading past the blob. The
 *        frame count is taken from the input too, because that is what the row supplies.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if (size < 2)
    return 0;

  const qint64 frames = static_cast<qint64>(data[0]) | (static_cast<qint64>(data[1]) << 8);
  const QByteArray blob(reinterpret_cast<const char*>(data + 2), static_cast<qsizetype>(size - 2));

  std::vector<double> values;
  (void)Sessions::unpackStreamSamples(blob, frames, values);

  std::vector<qint64> times;
  (void)Sessions::unpackStreamTimes(blob, frames, times);

  std::vector<QString> strings;
  (void)Sessions::unpackStreamText(blob, frames, strings);

  std::vector<qint64> expanded;
  (void)Sessions::expandBlockTimes(0, 0, frames, blob, expanded);
  (void)Sessions::expandBlockTimes(1'000, 1'000, frames, QByteArray(), expanded);

  return 0;
}
