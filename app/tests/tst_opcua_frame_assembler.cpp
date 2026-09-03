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

#include <QDateTime>
#include <QSet>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QVariant>

#include "IO/Drivers/OpcUa/OpcUaFrameAssembler.h"

using IO::Drivers::OpcUaFrameAssembler;
using IO::Drivers::OpcUaTag;

namespace Wire = IO::Drivers::OpcUaWire;

inline constexpr auto kGood = IO::Drivers::OpcUaTypes::kStatusGood;
inline constexpr auto kBad  = IO::Drivers::OpcUaTypes::kStatusBadInternal;

/**
 * @brief One subscribed tag of @p type, @p length elements wide.
 */
[[nodiscard]] static OpcUaTag tagOf(Wire::Type type, int length = 1)
{
  OpcUaTag tag;
  tag.nodeId   = QStringLiteral("ns=2;s=Tag%1").arg(static_cast<int>(type));
  tag.name     = tag.nodeId;
  tag.type     = type;
  tag.arrayLen = length;
  return tag;
}

/**
 * @brief Walks a delta frame into its entries, failing the walk rather than the caller when the
 *        encoder produced something the decoder cannot read.
 */
[[nodiscard]] static QList<Wire::Entry> entriesOf(const QByteArray& frame)
{
  QList<Wire::Entry> out;
  if (frame.isEmpty() || static_cast<std::uint8_t>(frame.at(0)) != Wire::kWireVersion)
    return out;

  qsizetype pos = Wire::kHeaderBytes;
  Wire::Entry entry;
  for (int i = 0; i < Wire::kMaxTags && Wire::readEntry(frame, pos, entry); ++i)
    out.append(entry);

  return out;
}

/**
 * @brief The OPC UA value cache and delta encoder: which slots a tag layout owns, which of them a
 *        tick encodes, what a Bad status does to the last good value, and how a frame that cannot
 *        hold every changed slot splits across ticks without starving the high indices.
 */
class TstOpcUaFrameAssembler : public QObject {
  Q_OBJECT

private slots:
  void reserveSizesOneSlotPerArrayElement();
  void onlyChangedSlotsAreEncoded();
  void arraysFanOutElementWise();
  void aBadStatusKeepsTheLastGoodValue();
  void anOversizedTickSplitsAndRotates();
  void stampsNeverGoBackwards();
  void aTypeMismatchIsReportedOnce();
};

//--------------------------------------------------------------------------------------------------
// Layout
//--------------------------------------------------------------------------------------------------

/**
 * @brief Each array element owns its own wire index, so the layout is the sum of the element
 *        counts and not the tag count; a layout sized per tag would collapse an array onto one
 *        dataset and silently publish only its first element.
 */
void TstOpcUaFrameAssembler::reserveSizesOneSlotPerArrayElement()
{
  OpcUaFrameAssembler assembler;
  QVERIFY(assembler.idle());

  const QList<OpcUaTag> tags{tagOf(Wire::Type::F64), tagOf(Wire::Type::I32, 3)};
  assembler.reserve(tags);
  assembler.beginSession();
  QVERIFY(!assembler.idle());

  assembler.storeValue(1, QVariantList{1, 2, 3}, kGood, QDateTime::currentDateTimeUtc());

  QByteArray frame;
  IO::CapturedData::SteadyTimePoint stamp;
  QVERIFY(assembler.assemble(frame, stamp));

  const auto entries = entriesOf(frame);
  QCOMPARE(entries.size(), qsizetype(3));
  QCOMPARE(entries.at(0).index, 1);
  QCOMPARE(entries.at(2).index, 3);

  assembler.reset();
  QVERIFY(assembler.idle());
}

//--------------------------------------------------------------------------------------------------
// Delta encoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief A tick carries only what moved since the previous one, and a tick with nothing to say
 *        produces no frame at all: the decoder latches everything it is not sent.
 */
void TstOpcUaFrameAssembler::onlyChangedSlotsAreEncoded()
{
  OpcUaFrameAssembler assembler;
  assembler.reserve({tagOf(Wire::Type::F64), tagOf(Wire::Type::Bool)});
  assembler.beginSession();

  QByteArray frame;
  IO::CapturedData::SteadyTimePoint stamp;
  QVERIFY(!assembler.assemble(frame, stamp));

  assembler.storeValue(1, QVariant(true), kGood, QDateTime::currentDateTimeUtc());
  QVERIFY(assembler.assemble(frame, stamp));

  auto entries = entriesOf(frame);
  QCOMPARE(entries.size(), qsizetype(1));
  QCOMPARE(entries.at(0).index, 1);
  QCOMPARE(entries.at(0).type, Wire::Type::Bool);

  QVERIFY(!assembler.assemble(frame, stamp));
  QCOMPARE(assembler.valuesReceived(), quint64(1));
}

/**
 * @brief An array value fans out element-wise. Extra elements are dropped rather than written past
 *        the tag's slots, and a short list leaves the elements it did not name latched.
 */
void TstOpcUaFrameAssembler::arraysFanOutElementWise()
{
  OpcUaFrameAssembler assembler;
  assembler.reserve({tagOf(Wire::Type::I32, 3)});
  assembler.beginSession();

  const auto now = QDateTime::currentDateTimeUtc();
  assembler.storeValue(0, QVariantList{10, 20, 30, 40}, kGood, now);

  QByteArray frame;
  IO::CapturedData::SteadyTimePoint stamp;
  QVERIFY(assembler.assemble(frame, stamp));
  QCOMPARE(entriesOf(frame).size(), qsizetype(3));

  assembler.storeValue(0, QVariantList{99}, kGood, now);
  QVERIFY(assembler.assemble(frame, stamp));

  const auto entries = entriesOf(frame);
  QCOMPARE(entries.size(), qsizetype(1));
  QCOMPARE(entries.at(0).index, 0);
  QCOMPARE(entries.at(0).text, QStringLiteral("99"));
}

//--------------------------------------------------------------------------------------------------
// Quality
//--------------------------------------------------------------------------------------------------

/**
 * @brief A Bad status is the server saying its own reading is worthless, so the slot keeps the last
 *        good value, the tag is named in the diagnostics, and nothing is published: overwriting
 *        with the bad reading would present the failure as data.
 */
void TstOpcUaFrameAssembler::aBadStatusKeepsTheLastGoodValue()
{
  const QList<OpcUaTag> tags{tagOf(Wire::Type::F64)};

  OpcUaFrameAssembler assembler;
  assembler.reserve(tags);
  assembler.beginSession();

  const auto now = QDateTime::currentDateTimeUtc();
  assembler.storeValue(0, QVariant(21.5), kGood, now);

  QByteArray frame;
  IO::CapturedData::SteadyTimePoint stamp;
  QVERIFY(assembler.assemble(frame, stamp));
  QCOMPARE(entriesOf(frame).at(0).text, QStringLiteral("21.5"));
  QVERIFY(assembler.badTags(tags).isEmpty());

  assembler.storeValue(0, QVariant(999.0), kBad, now);
  QVERIFY(!assembler.assemble(frame, stamp));
  QCOMPARE(assembler.badStatusCount(), quint64(1));
  QCOMPARE(assembler.badTags(tags), QStringList{tags.at(0).nodeId});

  assembler.storeValue(0, QVariant(22.5), kGood, now);
  QVERIFY(assembler.assemble(frame, stamp));
  QCOMPARE(entriesOf(frame).at(0).text, QStringLiteral("22.5"));
  QVERIFY(assembler.badTags(tags).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Frame splitting
//--------------------------------------------------------------------------------------------------

/**
 * @brief More changed slots than one frame can carry split across ticks, and the cursor resumes
 *        where it stopped: restarting at zero every tick would publish the low indices forever and
 *        starve everything past the cap.
 */
void TstOpcUaFrameAssembler::anOversizedTickSplitsAndRotates()
{
  const int perEntry = Wire::maxEntryBytes(Wire::Type::Str);
  const int count    = (Wire::kMaxFrameBytes / perEntry) + 8;
  QVERIFY(count < Wire::kMaxTags);

  QList<OpcUaTag> tags;
  tags.reserve(count);
  for (int i = 0; i < count; ++i)
    tags.append(tagOf(Wire::Type::Str));

  OpcUaFrameAssembler assembler;
  assembler.reserve(tags);
  assembler.beginSession();

  const auto now = QDateTime::currentDateTimeUtc();
  const QString payload(Wire::kMaxStringBytes, QLatin1Char('x'));
  for (int i = 0; i < count; ++i)
    assembler.storeValue(i, QVariant(payload), kGood, now);

  QSet<int> seen;
  QByteArray frame;
  IO::CapturedData::SteadyTimePoint stamp;

  int firstFrameEntries = 0;
  for (int tick = 0; tick < 4 && assembler.assemble(frame, stamp); ++tick) {
    QVERIFY(frame.size() <= Wire::kMaxFrameBytes);

    const auto entries = entriesOf(frame);
    QVERIFY(!entries.isEmpty());
    if (tick == 0)
      firstFrameEntries = static_cast<int>(entries.size());

    for (const auto& entry : entries) {
      QVERIFY2(!seen.contains(entry.index), "one slot was encoded twice across ticks");
      seen.insert(entry.index);
    }
  }

  QVERIFY(firstFrameEntries > 0 && firstFrameEntries < count);
  QCOMPARE(seen.size(), count);
  QVERIFY(!assembler.assemble(frame, stamp));
}

//--------------------------------------------------------------------------------------------------
// Time and typing
//--------------------------------------------------------------------------------------------------

/**
 * @brief The stamp is the source's, mapped once per session, and it never rewinds: two frames with
 *        the same instant would hand the pipeline two rows it cannot order.
 */
void TstOpcUaFrameAssembler::stampsNeverGoBackwards()
{
  OpcUaFrameAssembler assembler;
  assembler.reserve({tagOf(Wire::Type::F64)});
  assembler.beginSession();

  const auto later   = QDateTime::currentDateTimeUtc();
  const auto earlier = later.addMSecs(-1000);

  QByteArray frame;
  IO::CapturedData::SteadyTimePoint first;
  IO::CapturedData::SteadyTimePoint second;

  assembler.storeValue(0, QVariant(1.0), kGood, later);
  QVERIFY(assembler.assemble(frame, first));

  assembler.storeValue(0, QVariant(2.0), kGood, earlier);
  QVERIFY(assembler.assemble(frame, second));

  QVERIFY(second > first);
}

/**
 * @brief A value the declared type cannot carry is reported ONCE per slot: a mis-declared schema
 *        would otherwise emit one warning per sample for as long as the session runs. A double on
 *        an integer tag is the case that reaches a dashboard, silently truncated on every tick.
 */
void TstOpcUaFrameAssembler::aTypeMismatchIsReportedOnce()
{
  OpcUaFrameAssembler assembler;
  assembler.reserve({tagOf(Wire::Type::I32)});
  assembler.beginSession();

  QSignalSpy spy(&assembler, &OpcUaFrameAssembler::typeMismatch);
  QVERIFY(spy.isValid());

  const auto now = QDateTime::currentDateTimeUtc();

  QByteArray frame;
  IO::CapturedData::SteadyTimePoint stamp;
  for (int i = 0; i < 3; ++i) {
    assembler.storeValue(0, QVariant(1.5 + i), kGood, now);
    QVERIFY(assembler.assemble(frame, stamp));
  }

  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toInt(), 0);
}

QTEST_APPLESS_MAIN(TstOpcUaFrameAssembler)

#include "tst_opcua_frame_assembler.moc"
