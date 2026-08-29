/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/OpcUa/OpcUaFrameAssembler.h"

#include <chrono>

#include "SSAssert.h"

static constexpr qint64 kOpcUaNsPerMs   = 1000000LL;
static constexpr qint64 kMaxClockSkewMs = 5000;

/**
 * @brief Constructs an empty assembler; the layout is sized by reserve() at subscribe time.
 */
IO::Drivers::OpcUaFrameAssembler::OpcUaFrameAssembler(QObject* parent)
  : QObject(parent)
  , m_clockValid(false)
  , m_frameCursor(0)
  , m_valuesReceived(0)
  , m_badStatusCount(0)
  , m_unstampedCount(0)
  , m_frameBytes(0)
  , m_lastStampNs(0)
  , m_serverOffsetMs(0)
  , m_clockOffsetNs(0)
{}

/**
 * @brief Drops the value cache when the session goes down. The lifetime counters survive: they are
 *        pulled diagnostics for the whole driver, not for one session.
 */
void IO::Drivers::OpcUaFrameAssembler::reset()
{
  m_slots.clear();
  m_frameCursor    = 0;
  m_clockValid     = false;
  m_serverOffsetMs = 0;
}

/**
 * @brief Samples the steady-to-wall offset a fresh subscription's timestamps are mapped through.
 */
void IO::Drivers::OpcUaFrameAssembler::beginSession()
{
  const auto steady     = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 steadyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(steady).count();
  m_clockOffsetNs       = steadyNs - QDateTime::currentMSecsSinceEpoch() * kOpcUaNsPerMs;
  m_clockValid          = true;
  m_lastStampNs         = 0;
  m_serverOffsetMs      = 0;
}

/**
 * @brief Sizes the slot cache from the tag layout and reserves the worst-case frame once.
 */
void IO::Drivers::OpcUaFrameAssembler::reserve(const QList<OpcUaTag>& tags)
{
  m_firstIndex.clear();
  m_slots.clear();

  m_slotCount.clear();

  qsizetype bytes = OpcUaWire::kHeaderBytes;
  for (const auto& tag : tags) {
    const auto type = wireTypeFor(tag);
    const int count = qMax(1, tag.arrayLen);
    m_firstIndex.append(m_slots.size());
    m_slotCount.append(count);
    bytes += static_cast<qsizetype>(count) * OpcUaWire::maxEntryBytes(type);
    for (int i = 0; i < count; ++i) {
      Slot slot;
      slot.type = type;
      m_slots.append(slot);
    }
  }

  SS_ASSERT_LOG(m_slots.size() <= OpcUaWire::kMaxTags);
  m_frameBytes = qMin<qsizetype>(bytes, OpcUaWire::kMaxFrameBytes);
  m_frame      = QByteArray();
  m_frame.reserve(m_frameBytes);
}

/**
 * @brief Flags a tag's slots as stale after a Bad status; the dashboard keeps the last good value
 *        and the quality is reported through the diagnostics snapshot instead of vanishing.
 */
void IO::Drivers::OpcUaFrameAssembler::markBad(const int tag)
{
  SS_ASSERT(tag >= 0 && tag < m_firstIndex.size(), return);

  const int first = m_firstIndex.at(tag);
  const int count = m_slotCount.at(tag);
  for (int i = 0; i < count && first + i < m_slots.size(); ++i)
    m_slots[first + i].bad = true;
}

/**
 * @brief Adopts the server-to-local clock offset from the first stamped notification of a session;
 *        an un-NTP'd PLC is followed rather than rejected.
 */
void IO::Drivers::OpcUaFrameAssembler::noteServerTimestamp(const QDateTime& serverTs)
{
  if (m_serverOffsetMs || !serverTs.isValid())
    return;

  m_serverOffsetMs = serverTs.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
}

/**
 * @brief Writes a value into its slot(s); a bad status keeps the last good value and counts.
 *        Arrays fan out element-wise, extra elements are dropped, missing ones left latched.
 */
void IO::Drivers::OpcUaFrameAssembler::storeValue(const int tag,
                                                  const QVariant& value,
                                                  OpcUaTypes::StatusCode status,
                                                  const QDateTime& sourceTs)
{
  SS_ASSERT(tag >= 0 && tag < m_firstIndex.size(), return);
  ++m_valuesReceived;

  if (OpcUaTypes::isBad(status)) {
    ++m_badStatusCount;
    markBad(tag);
    return;
  }

  const int first = m_firstIndex.at(tag);
  const int count = m_slotCount.at(tag);
  if (value.typeId() == QMetaType::QVariantList) {
    const auto list = value.toList();
    for (int i = 0; i < count && i < list.size() && first + i < m_slots.size(); ++i) {
      auto& slot    = m_slots[first + i];
      slot.value    = list.at(i);
      slot.sourceTs = sourceTs;
      slot.dirty    = true;
      slot.bad      = false;
    }

    return;
  }

  SS_ASSERT(first < m_slots.size(), return);
  auto& slot    = m_slots[first];
  slot.value    = value;
  slot.sourceTs = sourceTs;
  slot.dirty    = true;
  slot.bad      = false;
}

/**
 * @brief True while no wire layout is held, which is what tells the driver a disconnect arrived
 *        before the subscription ever came up.
 */
bool IO::Drivers::OpcUaFrameAssembler::idle() const noexcept
{
  return m_slots.isEmpty();
}

/**
 * @brief Encodes every dirty slot into one delta frame stamped with the earliest source time it
 *        carries, and hands the buffer over. No dirty slot, no frame; slots that do not fit under
 *        the decoder cap stay dirty for the next tick. The buffer is re-reserved on the way out,
 *        which keeps a publishing tick at one allocation.
 */
bool IO::Drivers::OpcUaFrameAssembler::assemble(QByteArray& frame,
                                                CapturedData::SteadyTimePoint& timestamp)
{
  using namespace OpcUaWire;
  beginFrame(m_frame);

  QDateTime earliest;
  const int slotCount = m_slots.size();
  if (m_frameCursor >= slotCount)
    m_frameCursor = 0;

  for (int step = 0; step < slotCount; ++step) {
    const int index = (m_frameCursor + step) % slotCount;
    auto& slot      = m_slots[index];
    if (!slot.dirty)
      continue;

    if (m_frame.size() + maxEntryBytes(slot.type) > kMaxFrameBytes) {
      m_frameCursor = index;
      break;
    }

    if (!slot.warned && !valueFitsType(slot.value, slot.type)) {
      slot.warned = true;
      Q_EMIT typeMismatch(
        index, codeFromType(slot.type), QString::fromLatin1(slot.value.typeName()));
    }

    appendEntry(m_frame, index, slot.type, slot.value);
    slot.dirty = false;
    if (!earliest.isValid() || (slot.sourceTs.isValid() && slot.sourceTs < earliest))
      earliest = slot.sourceTs;
  }

  if (m_frame.size() <= kHeaderBytes)
    return false;

  timestamp = toSteady(earliest);
  frame     = std::move(m_frame);
  m_frame   = QByteArray();
  m_frame.reserve(m_frameBytes);
  return true;
}

/**
 * @brief The node ids whose newest value carried a Bad status (R11 diagnostics).
 */
QStringList IO::Drivers::OpcUaFrameAssembler::badTags(const QList<OpcUaTag>& tags) const
{
  QStringList out;
  for (int tag = 0; tag < m_firstIndex.size() && tag < tags.size(); ++tag) {
    const int first = m_firstIndex.at(tag);
    if (first < m_slots.size() && m_slots.at(first).bad)
      out.append(tags.at(tag).nodeId);
  }

  return out;
}

/**
 * @brief How many values have been stored since the driver was created.
 */
quint64 IO::Drivers::OpcUaFrameAssembler::valuesReceived() const noexcept
{
  return m_valuesReceived;
}

/**
 * @brief How many values arrived with a Bad status.
 */
quint64 IO::Drivers::OpcUaFrameAssembler::badStatusCount() const noexcept
{
  return m_badStatusCount;
}

/**
 * @brief How many values could not be stamped from the server's own clock.
 */
quint64 IO::Drivers::OpcUaFrameAssembler::unstampedCount() const noexcept
{
  return m_unstampedCount;
}

/**
 * @brief Maps a server source timestamp onto the steady clock through the per-connect offset;
 *        skew is measured against the server-to-local offset sampled at connect, so an un-NTP'd
 *        PLC is followed rather than rejected; a missing or wildly skewed stamp falls back to now
 *        and counts as unstamped. The result never goes backwards (previous stamp plus 1 ns).
 */
IO::CapturedData::SteadyTimePoint IO::Drivers::OpcUaFrameAssembler::toSteady(
  const QDateTime& sourceTs)
{
  const auto now = CapturedData::SteadyClock::now();
  const qint64 nowNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

  qint64 stamp = nowNs;
  if (!m_clockValid || !sourceTs.isValid())
    ++m_unstampedCount;
  else {
    const qint64 skewMs =
      sourceTs.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch() - m_serverOffsetMs;
    if (skewMs > kMaxClockSkewMs || skewMs < -kMaxClockSkewMs)
      ++m_unstampedCount;
    else
      stamp = (sourceTs.toMSecsSinceEpoch() - m_serverOffsetMs) * kOpcUaNsPerMs + m_clockOffsetNs;
  }

  stamp         = qMax(stamp, m_lastStampNs + 1);
  m_lastStampNs = stamp;
  return CapturedData::SteadyTimePoint(std::chrono::nanoseconds(stamp));
}
