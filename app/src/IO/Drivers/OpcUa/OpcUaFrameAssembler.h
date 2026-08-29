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

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "IO/Drivers/OpcUa/OpcUaTag.h"
#include "IO/Drivers/OpcUaTypes.h"
#include "IO/Drivers/OpcUaWire.h"
#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief The value cache and delta-frame encoder behind the OPC UA driver's publishing tick: one
 *        slot per wire index, the newest value latched into it, one worst-case buffer reserved at
 *        subscribe time. SOURCE OWNS TIME: the server's source timestamp is mapped onto the steady
 *        clock at the driver boundary here, and nothing downstream may re-stamp assemble()'s rows.
 */
class OpcUaFrameAssembler : public QObject {
  Q_OBJECT

signals:
  void typeMismatch(int index, const QString& declared, const QString& actual);

public:
  explicit OpcUaFrameAssembler(QObject* parent = nullptr);

  OpcUaFrameAssembler(OpcUaFrameAssembler&&)                 = delete;
  OpcUaFrameAssembler(const OpcUaFrameAssembler&)            = delete;
  OpcUaFrameAssembler& operator=(OpcUaFrameAssembler&&)      = delete;
  OpcUaFrameAssembler& operator=(const OpcUaFrameAssembler&) = delete;

  void reset();
  void beginSession();
  void reserve(const QList<OpcUaTag>& tags);
  void markBad(const int tag);
  void noteServerTimestamp(const QDateTime& serverTs);
  void storeValue(const int tag,
                  const QVariant& value,
                  OpcUaTypes::StatusCode status,
                  const QDateTime& sourceTs);

  [[nodiscard]] bool idle() const noexcept;
  [[nodiscard]] bool assemble(QByteArray& frame, CapturedData::SteadyTimePoint& timestamp);
  [[nodiscard]] QStringList badTags(const QList<OpcUaTag>& tags) const;

  [[nodiscard]] quint64 valuesReceived() const noexcept;
  [[nodiscard]] quint64 badStatusCount() const noexcept;
  [[nodiscard]] quint64 unstampedCount() const noexcept;

private:
  [[nodiscard]] CapturedData::SteadyTimePoint toSteady(const QDateTime& sourceTs);

  /**
   * @brief Per-wire-index cache slot: the newest value, its status and source time.
   */
  struct Slot {
    QVariant value;
    QDateTime sourceTs;
    OpcUaWire::Type type;
    bool dirty;
    bool bad;
    bool warned;

    Slot() : type(OpcUaWire::Type::Str), dirty(false), bad(false), warned(false) {}
  };

  bool m_clockValid;
  int m_frameCursor;
  quint64 m_valuesReceived;
  quint64 m_badStatusCount;
  quint64 m_unstampedCount;
  QByteArray m_frame;
  QList<Slot> m_slots;
  QList<int> m_firstIndex;
  QList<int> m_slotCount;
  qsizetype m_frameBytes;
  qint64 m_lastStampNs;
  qint64 m_serverOffsetMs;
  qint64 m_clockOffsetNs;
};

}  // namespace Drivers
}  // namespace IO
