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
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/ThinkTagSplitter.h"

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts in Detect: nothing is published until the stream shows whether it opens a block.
 */
AI::ThinkTagSplitter::ThinkTagSplitter() : m_scan(Scan::Detect), m_carry() {}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scans @p chunk and returns the runs that can be published now. A partial tag at the end
 *        of the buffer is held back, so a tag split across two chunks is never mistaken for text.
 */
std::vector<AI::ThinkChunk> AI::ThinkTagSplitter::append(const QString& chunk)
{
  std::vector<ThinkChunk> out;
  m_carry.append(chunk);
  drain(false, out);
  return out;
}

/**
 * @brief Releases everything still buffered at end of stream, tag or not.
 */
std::vector<AI::ThinkChunk> AI::ThinkTagSplitter::flush()
{
  std::vector<ThinkChunk> out;
  drain(true, out);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Scanner
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drains the buffered carry across scanner states, holding back only bytes that could
 *        still complete a tag split across chunks. @p atEnd releases everything.
 */
void AI::ThinkTagSplitter::drain(bool atEnd, std::vector<ThinkChunk>& out)
{
  static const auto kOpen  = QStringLiteral("<think>");
  static const auto kClose = QStringLiteral("</think>");
  const int max_passes     = m_carry.size() + 2;
  SS_ASSERT_LOG(max_passes >= 2);

  for (int pass = 0; pass < max_passes && !m_carry.isEmpty(); ++pass) {
    if (m_scan == Scan::Passthrough) {
      out.push_back(ThinkChunk{ThinkChannel::Text, m_carry});
      m_carry.clear();
      return;
    }

    if (m_scan == Scan::Detect) {
      int ws = 0;
      while (ws < m_carry.size() && m_carry.at(ws).isSpace())
        ++ws;

      if (ws > 0)
        m_carry.remove(0, ws);

      if (m_carry.isEmpty())
        return;

      if (m_carry.startsWith(kOpen)) {
        m_carry.remove(0, kOpen.size());
        m_scan = Scan::Thinking;
        continue;
      }

      if (!atEnd && m_carry.size() < kOpen.size() && kOpen.startsWith(m_carry))
        return;

      m_scan = Scan::Passthrough;
      continue;
    }

    const int close_at = m_carry.indexOf(kClose);
    if (close_at >= 0) {
      if (close_at > 0)
        out.push_back(ThinkChunk{ThinkChannel::Thinking, m_carry.left(close_at)});

      m_carry.remove(0, close_at + kClose.size());
      m_scan = Scan::Detect;
      continue;
    }

    const int keep = atEnd ? 0 : kClose.size() - 1;
    const int cut  = m_carry.size() - keep;
    SS_ASSERT_LOG(cut <= m_carry.size());
    if (cut > 0) {
      out.push_back(ThinkChunk{ThinkChannel::Thinking, m_carry.left(cut)});
      m_carry.remove(0, cut);
    }

    return;
  }
}
