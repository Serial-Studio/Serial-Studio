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

#include "DataModel/FrameBuilder/QuickPlotFormatState.h"

#include <algorithm>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction and reset
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts unestablished; the per-position lookup cache is sized once so indexOf never
 *        allocates.
 */
DataModel::QuickPlotFormatState::QuickPlotFormatState() : m_pinned(false), m_established(false)
{
  m_lastIndex.assign(static_cast<size_t>(kMaxLineFields), -1);
}

/**
 * @brief Forgets the format, the registry, the header state and the replay pin; the next line
 *        detects again. Called on connect and on an operation-mode change.
 */
void DataModel::QuickPlotFormatState::reset()
{
  m_pinned      = false;
  m_established = false;
  m_format      = LineFormat();
  m_headerCandidate.clear();
  m_headerTitles.clear();
  clearRegistry();
}

/**
 * @brief Pins the positional comma format the replay codecs and the audio lane deliver, with
 *        @p count channels; zero channels leaves establishment to the first row.
 */
void DataModel::QuickPlotFormatState::pinPositional(const int count)
{
  SS_ASSERT(count >= 0, return);
  SS_ASSERT(kMaxLineFields >= count, return);

  reset();
  m_pinned = true;
  for (int i = 0; i < count; ++i) {
    QuickPlotChannel channel;
    channel.position = i;
    m_channels.push_back(channel);
  }

  rebuildPositionIndex();
  m_established = count > 0;
}

//--------------------------------------------------------------------------------------------------
// Observation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Live lane entry: detects until established, then tokenizes under the fixed format. A
 *        pinned state never detects and is tokenized under its pin.
 */
DataModel::QuickPlotLineOutcome DataModel::QuickPlotFormatState::observe(QByteArrayView line,
                                                                         LineFields& fields)
{
  SS_ASSERT(kMaxLineFields >= fields.count, fields.count = 0);
  if (m_established || m_pinned) {
    (void)tokenizeLine(line, m_format, fields);
    return classify(line, fields);
  }

  if (!detectLineFormat(line, m_format, fields)) {
    rememberHeaderCandidate(line);
    return m_headerCandidate.isEmpty() ? QuickPlotLineOutcome::Text : QuickPlotLineOutcome::Header;
  }

  establish(fields);
  acceptHeader(line, fields);
  return m_established ? QuickPlotLineOutcome::Rebuild : QuickPlotLineOutcome::Text;
}

/**
 * @brief Replay entry for rows the player codecs already split: pinned positional, never text,
 *        never detected, so playback keeps today's behavior exactly.
 */
DataModel::QuickPlotLineOutcome DataModel::QuickPlotFormatState::observeTokenized(
  const LineFields& fields)
{
  SS_ASSERT(m_pinned, pinPositional(0));
  SS_ASSERT(kMaxLineFields >= fields.count, return QuickPlotLineOutcome::Text);
  if (fields.count <= 0)
    return QuickPlotLineOutcome::Text;

  if (!m_established) {
    establish(fields);
    return QuickPlotLineOutcome::Rebuild;
  }

  return grow(fields) ? QuickPlotLineOutcome::Rebuild : QuickPlotLineOutcome::Apply;
}

/**
 * @brief Established-state classification: no numeric field means a text line unless a fresh
 *        detection finds another format (spec 0106 R9), otherwise the registry grows or applies.
 */
DataModel::QuickPlotLineOutcome DataModel::QuickPlotFormatState::classify(QByteArrayView line,
                                                                          LineFields& fields)
{
  SS_ASSERT(m_established || m_pinned, return QuickPlotLineOutcome::Text);
  if (fields.count <= 0 || numericLineFieldCount(fields) == 0) {
    if (m_pinned)
      return QuickPlotLineOutcome::Text;

    LineFormat fresh;
    if (!detectLineFormat(line, fresh, fields) || fresh == m_format)
      return QuickPlotLineOutcome::Text;

    m_format = fresh;
    establish(fields);
    return QuickPlotLineOutcome::Rebuild;
  }

  return grow(fields) ? QuickPlotLineOutcome::Rebuild : QuickPlotLineOutcome::Apply;
}

//--------------------------------------------------------------------------------------------------
// Registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when every channel is addressed by position (a plain delimited stream).
 */
bool DataModel::QuickPlotFormatState::positionalOnly() const noexcept
{
  SS_ASSERT(static_cast<size_t>(kMaxLineFields) >= m_channels.size(), return true);
  SS_ASSERT(m_lastIndex.size() == static_cast<size_t>(kMaxLineFields), return true);
  for (const auto& channel : m_channels)
    if (!channel.name.isEmpty())
      return false;

  return true;
}

/**
 * @brief The title a channel carries on the wire or from an accepted header row; empty when the
 *        builder should fall back to its numbered default.
 */
QString DataModel::QuickPlotFormatState::channelTitle(const int index) const
{
  SS_ASSERT(index >= 0, return QString());
  SS_ASSERT(static_cast<size_t>(index) < m_channels.size(), return QString());

  const auto& channel = m_channels[static_cast<size_t>(index)];
  if (!channel.name.isEmpty())
    return QString::fromUtf8(channel.name);

  if (positionalOnly() && index < m_headerTitles.size())
    return m_headerTitles.at(index);

  return QString();
}

/**
 * @brief Channel index of @p field found at @p position in its line, or -1. Named fields try the
 *        channel matched at that position last time before a bounded linear scan.
 */
int DataModel::QuickPlotFormatState::indexOf(const LineField& field,
                                             const qsizetype position) noexcept
{
  SS_ASSERT(position >= 0, return -1);
  SS_ASSERT(kMaxLineFields > position, return -1);

  if (field.name.isEmpty()) {
    const auto p = static_cast<size_t>(position);
    return p < m_positionIndex.size() ? m_positionIndex[p] : -1;
  }

  const int guess = m_lastIndex[static_cast<size_t>(position)];
  if (guess >= 0 && static_cast<size_t>(guess) < m_channels.size()
      && QByteArrayView(m_channels[static_cast<size_t>(guess)].name) == field.name)
    return guess;

  const int count = static_cast<int>(m_channels.size());
  for (int i = 0; i < count; ++i) {
    if (QByteArrayView(m_channels[static_cast<size_t>(i)].name) != field.name)
      continue;

    m_lastIndex[static_cast<size_t>(position)] = i;
    return i;
  }

  return -1;
}

/**
 * @brief Grows the registry for fields it does not know. A plain positional stream keeps today's
 *        contract and re-establishes on any field-count change; a named or mixed stream only
 *        appends, so a missing name never shifts the others, and stops at kMaxLineFields channels
 *        so a stream whose names never repeat cannot grow the dashboard without bound.
 */
bool DataModel::QuickPlotFormatState::grow(const LineFields& fields)
{
  SS_ASSERT(fields.count >= 0, return false);
  SS_ASSERT(kMaxLineFields >= fields.count, return false);

  const bool positional = positionalOnly() && !m_format.named;
  if (positional) {
    bool anyNamed = false;
    for (qsizetype i = 0; i < fields.count && !anyNamed; ++i)
      anyNamed = !fields.items[i].name.isEmpty();

    if (!anyNamed) {
      if (static_cast<size_t>(fields.count) == m_channels.size())
        return false;

      establish(fields);
      return true;
    }
  }

  bool grown = false;
  for (qsizetype i = 0; i < fields.count; ++i) {
    if (indexOf(fields.items[i], i) >= 0)
      continue;

    if (m_channels.size() >= static_cast<size_t>(kMaxLineFields))
      break;

    appendChannel(fields.items[i], i);
    grown = true;
  }

  if (grown)
    rebuildPositionIndex();

  return grown;
}

/**
 * @brief Replaces the registry with one channel per field of the establishing line.
 */
void DataModel::QuickPlotFormatState::establish(const LineFields& fields)
{
  SS_ASSERT(fields.count > 0, return);
  SS_ASSERT(kMaxLineFields >= fields.count, return);

  clearRegistry();
  for (qsizetype i = 0; i < fields.count; ++i)
    appendChannel(fields.items[i], i);

  rebuildPositionIndex();
  m_established = true;
}

/**
 * @brief Promotes the header candidate to titles when it is all text and has exactly as many
 *        fields as the establishing line (spec 0106 R8), then restores @p fields to that line.
 */
void DataModel::QuickPlotFormatState::acceptHeader(QByteArrayView line, LineFields& fields)
{
  SS_ASSERT(m_established, return);
  if (m_headerCandidate.isEmpty())
    return;

  if (positionalOnly()) {
    const qsizetype count = tokenizeLine(m_headerCandidate, m_format, fields);
    const bool matches    = count > 0 && static_cast<size_t>(count) == m_channels.size()
                         && numericLineFieldCount(fields) == 0;
    if (matches) {
      m_headerTitles.clear();
      for (qsizetype i = 0; i < count; ++i)
        m_headerTitles.append(QString::fromUtf8(fields.items[i].token));
    }

    const qsizetype restored = tokenizeLine(line, m_format, fields);
    SS_ASSERT(static_cast<size_t>(restored) == m_channels.size(), clearRegistry());
  }

  m_headerCandidate.clear();
}

/**
 * @brief Keeps the latest non-qualifying line before establishment as the header candidate.
 */
void DataModel::QuickPlotFormatState::rememberHeaderCandidate(QByteArrayView line)
{
  SS_ASSERT(!m_established, return);
  const auto text = trimLineAscii(line);
  if (text.isEmpty())
    return;

  m_headerCandidate = QByteArray(text.data(), text.size());
}

/**
 * @brief Appends a channel for @p field: by name when it has one, else by its line position.
 */
void DataModel::QuickPlotFormatState::appendChannel(const LineField& field,
                                                    const qsizetype position)
{
  SS_ASSERT(position >= 0, return);
  SS_ASSERT(kMaxLineFields > position, return);

  QuickPlotChannel channel;
  if (field.name.isEmpty()) {
    channel.position = static_cast<int>(position);
  } else {
    channel.name   = QByteArray(field.name.data(), field.name.size());
    m_format.named = true;
  }

  if (!field.unit.isEmpty())
    channel.unit = QByteArray(field.unit.data(), field.unit.size());

  m_channels.push_back(channel);
}

/**
 * @brief Rebuilds the position-to-channel table after the registry changed.
 */
void DataModel::QuickPlotFormatState::rebuildPositionIndex()
{
  int maxPosition = -1;
  for (const auto& channel : m_channels)
    maxPosition = std::max(maxPosition, channel.position);

  m_positionIndex.assign(static_cast<size_t>(maxPosition + 1), -1);
  const int count = static_cast<int>(m_channels.size());
  for (int i = 0; i < count; ++i) {
    const int position = m_channels[static_cast<size_t>(i)].position;
    if (position >= 0)
      m_positionIndex[static_cast<size_t>(position)] = i;
  }

  SS_ASSERT(m_positionIndex.size() == static_cast<size_t>(maxPosition + 1), return);
  SS_ASSERT(m_lastIndex.size() == static_cast<size_t>(kMaxLineFields), return);
}

/**
 * @brief Drops every channel and the lookup caches; the format and header state stay.
 */
void DataModel::QuickPlotFormatState::clearRegistry()
{
  m_channels.clear();
  m_positionIndex.clear();
  std::fill(m_lastIndex.begin(), m_lastIndex.end(), -1);
  m_established = false;
}
