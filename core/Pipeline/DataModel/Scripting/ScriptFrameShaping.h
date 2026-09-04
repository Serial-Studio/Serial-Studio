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

#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <utility>

/**
 * @brief Shape rules a parse() return value goes through before it becomes frames, shared by the
 *        JS and Lua engines so a mixed scalar/vector return unzips identically in both languages.
 *        Header-inline on purpose: this runs per frame for a mixed-shape parser, and the engines
 *        live in different translation units.
 */
namespace DataModel::ScriptFrames {

/**
 * @brief Hard cap on how many frames one mixed return value may unzip into; a malformed script
 *        result cannot turn into an out-of-memory.
 */
inline constexpr qsizetype kMaxVectorLength = 10000;

/**
 * @brief Unzips a mixed scalar/vector parse result into one frame per vector index: every vector
 *        is padded to the longest with its own last value, and each frame carries all the scalars
 *        followed by that index's element of every vector. A result with no vectors is one frame.
 *        @p vectors is padded in place.
 */
[[nodiscard]] inline QList<QStringList> unzipMixedFrames(const QStringList& scalars,
                                                         QList<QStringList>& vectors,
                                                         qsizetype maxVectorLength)
{
  QList<QStringList> results;
  if (vectors.isEmpty()) [[unlikely]] {
    results.append(scalars);
    return results;
  }

  const qsizetype length = qMin(maxVectorLength, kMaxVectorLength);

  for (auto& vec : vectors) {
    if (!vec.isEmpty() && vec.size() < length) {
      const QString lastValue = vec.last();
      while (vec.size() < length)
        vec.append(lastValue);
    }
  }

  results.reserve(length);
  for (qsizetype i = 0; i < length; ++i) {
    QStringList frame;
    frame.reserve(scalars.size() + vectors.size());
    frame.append(scalars);

    for (const auto& vec : std::as_const(vectors))
      if (i < vec.size())
        frame.append(vec[i]);

    results.append(frame);
  }

  return results;
}

}  // namespace DataModel::ScriptFrames
