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

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTest>

#include "IO/Drivers/Modbus/ModbusRegisterGroups.h"
#include "IO/Drivers/Modbus/ModbusRtuCodec.h"

using IO::Drivers::ModbusRegisterGroups;

namespace Rtu = IO::Drivers::ModbusRtu;

inline constexpr quint8 kHolding  = 0;
inline constexpr quint8 kInput    = 1;
inline constexpr quint8 kCoils    = 2;
inline constexpr quint8 kDiscrete = 3;

/**
 * @brief The poll list a Modbus session is planned from and the RTU framing its replies are
 *        published as. A group the list refuses never reaches the wire, and a frame without a
 *        checksum is not an RTU frame at all: both are decisions a device would otherwise make
 *        for us, on a live PLC (spec 0075 E12).
 */
class TstModbusRegisterGroups : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void registerReadsStopAtTheWordCap();
  void bitReadsGetTheirOwnCap();
  void duplicatesAndEmptyGroupsAreRefused();
  void groupsSurviveARestore();
  void aRestoreDropsAnOutOfRangeEntry();
  void removalAndClearTruncateThePersistedArray();
  void jsonCarriesEveryFieldOfEveryGroup();
  void functionCodesFollowTheRegisterType();
  void theChecksumIsTheModbusCrc();

private:
  [[nodiscard]] QSettings& settings();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Redirects QSettings into the test sandbox so the suite never touches a real installation.
 */
void TstModbusRegisterGroups::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_modbus_register_groups"));
}

/**
 * @brief Starts every case from an empty persisted array.
 */
void TstModbusRegisterGroups::init()
{
  ModbusRegisterGroups groups(settings());
  groups.clear();
}

/**
 * @brief The one settings store the suite shares, so a persist in one object is visible to the
 *        next one; that round trip is the behaviour under test.
 */
QSettings& TstModbusRegisterGroups::settings()
{
  static QSettings store;
  return store;
}

//--------------------------------------------------------------------------------------------------
// Request caps
//--------------------------------------------------------------------------------------------------

/**
 * @brief FC03/FC04 answer at most 125 sixteen-bit registers, because the reply's byte count is a
 *        single octet. A group past that is refused rather than sent and truncated by the device.
 */
void TstModbusRegisterGroups::registerReadsStopAtTheWordCap()
{
  ModbusRegisterGroups groups(settings());

  QVERIFY(groups.add(kHolding, 0, 125));
  QVERIFY(groups.add(kInput, 0, 1));
  QVERIFY(!groups.add(kHolding, 200, 126));
  QVERIFY(!groups.add(kInput, 200, 2000));
  QCOMPARE(groups.count(), 2);
  QCOMPARE(groups.totalDatasets(), 126);
}

/**
 * @brief FC01/FC02 answer bits, eight to the octet, so 2000 of them fit the same single-octet byte
 *        count. Sharing the register ceiling refused four fifths of a legal coil read.
 */
void TstModbusRegisterGroups::bitReadsGetTheirOwnCap()
{
  ModbusRegisterGroups groups(settings());

  QVERIFY(groups.add(kCoils, 0, 2000));
  QVERIFY(groups.add(kDiscrete, 0, 126));
  QVERIFY(!groups.add(kCoils, 5000, 2001));
  QCOMPARE(groups.count(), 2);
  QCOMPARE(groups.at(0).count, quint16(2000));
  QCOMPARE(groups.at(1).registerType, kDiscrete);
}

/**
 * @brief An exact duplicate and a zero-length group are both refused, so the caller only announces
 *        a real edit and the poll cycle never contains a request that reads nothing.
 */
void TstModbusRegisterGroups::duplicatesAndEmptyGroupsAreRefused()
{
  ModbusRegisterGroups groups(settings());

  QVERIFY(groups.add(kHolding, 40, 2));
  QVERIFY(!groups.add(kHolding, 40, 2));
  QVERIFY(!groups.add(kHolding, 40, 0));
  QVERIFY(groups.add(kInput, 40, 2));
  QCOMPARE(groups.count(), 2);
}

//--------------------------------------------------------------------------------------------------
// Persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief The poll ORDER is the generated parser's group cursor, so a restore has to reproduce the
 *        list exactly: a reordered restore attributes every frame to the wrong group.
 */
void TstModbusRegisterGroups::groupsSurviveARestore()
{
  ModbusRegisterGroups groups(settings());
  QVERIFY(groups.add(kCoils, 100, 9));
  QVERIFY(groups.add(kHolding, 40, 3));
  QVERIFY(groups.add(kInput, 7, 1));

  ModbusRegisterGroups restored(settings());
  restored.restore();
  QCOMPARE(restored.count(), 3);

  for (int i = 0; i < groups.count(); ++i) {
    QCOMPARE(restored.at(i).registerType, groups.at(i).registerType);
    QCOMPARE(restored.at(i).startAddress, groups.at(i).startAddress);
    QCOMPARE(restored.at(i).count, groups.at(i).count);
  }

  QCOMPARE(restored.totalDatasets(), groups.totalDatasets());
}

/**
 * @brief A settings file edited by hand (or written by a version with a different cap) can name a
 *        group no single read can carry; it is dropped on restore instead of planning a request
 *        the controller answers by truncating.
 */
void TstModbusRegisterGroups::aRestoreDropsAnOutOfRangeEntry()
{
  auto& store = settings();
  store.beginWriteArray(QStringLiteral("ModbusDriver/registerGroups"));
  const quint16 counts[] = {2, 4000, 3};
  for (int i = 0; i < 3; ++i) {
    store.setArrayIndex(i);
    store.setValue(QStringLiteral("type"), kCoils);
    store.setValue(QStringLiteral("start"), quint16(i * 10));
    store.setValue(QStringLiteral("count"), counts[i]);
  }

  store.endArray();

  ModbusRegisterGroups groups(store);
  groups.restore();
  QCOMPARE(groups.count(), 2);
  QCOMPARE(groups.at(0).count, quint16(2));
  QCOMPARE(groups.at(1).count, quint16(3));
}

/**
 * @brief A shrunk list must not resurrect a removed group: the array size endArray() stamps is
 *        what the next restore reads.
 */
void TstModbusRegisterGroups::removalAndClearTruncateThePersistedArray()
{
  ModbusRegisterGroups groups(settings());
  QVERIFY(groups.add(kHolding, 0, 1));
  QVERIFY(groups.add(kHolding, 10, 1));
  QVERIFY(groups.add(kHolding, 20, 1));

  QVERIFY(groups.remove(1));
  QVERIFY(!groups.remove(9));
  QVERIFY(!groups.remove(-1));

  ModbusRegisterGroups restored(settings());
  restored.restore();
  QCOMPARE(restored.count(), 2);
  QCOMPARE(restored.at(1).startAddress, quint16(20));

  groups.clear();
  QVERIFY(groups.isEmpty());

  ModbusRegisterGroups empty(settings());
  empty.restore();
  QVERIFY(empty.isEmpty());
  QCOMPARE(empty.totalDatasets(), 0);
}

/**
 * @brief The JSON shape is the driver property a project stores, so every field a restore needs
 *        has to be in it.
 */
void TstModbusRegisterGroups::jsonCarriesEveryFieldOfEveryGroup()
{
  ModbusRegisterGroups groups(settings());
  QVERIFY(groups.add(kDiscrete, 300, 16));

  const auto array = groups.toJson();
  QCOMPARE(array.size(), qsizetype(1));

  const auto object = array.at(0).toObject();
  QCOMPARE(object.value(QStringLiteral("type")).toInt(), int(kDiscrete));
  QCOMPARE(object.value(QStringLiteral("start")).toInt(), 300);
  QCOMPARE(object.value(QStringLiteral("count")).toInt(), 16);
}

//--------------------------------------------------------------------------------------------------
// RTU framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief The function code a group's reply carries is what the generated parser resynchronises on,
 *        so it has to follow the register type and nothing else.
 */
void TstModbusRegisterGroups::functionCodesFollowTheRegisterType()
{
  QCOMPARE(Rtu::functionCodeForType(kHolding), quint8(0x03));
  QCOMPARE(Rtu::functionCodeForType(kInput), quint8(0x04));
  QCOMPARE(Rtu::functionCodeForType(kCoils), quint8(0x01));
  QCOMPARE(Rtu::functionCodeForType(kDiscrete), quint8(0x02));
  QCOMPARE(Rtu::functionCodeForType(9), quint8(0x03));
}

/**
 * @brief CRC-16/Modbus, low octet first. The golden value is the one every Modbus stack computes
 *        for this frame; without it the published bytes are a header-shaped fragment that a
 *        checksum-validating consumer rejects.
 */
void TstModbusRegisterGroups::theChecksumIsTheModbusCrc()
{
  QByteArray frame;
  frame.append(static_cast<char>(0x01));
  frame.append(static_cast<char>(0x03));
  frame.append(static_cast<char>(0x02));
  frame.append(static_cast<char>(0x00));
  frame.append(static_cast<char>(0x0A));

  Rtu::appendCrc(frame);
  QCOMPARE(frame.size(), qsizetype(7));
  QCOMPARE(static_cast<quint8>(frame.at(5)), quint8(0x38));
  QCOMPARE(static_cast<quint8>(frame.at(6)), quint8(0x43));

  QByteArray placeholder;
  placeholder.append(static_cast<char>(0x01));
  placeholder.append(static_cast<char>(0x03));
  placeholder.append(static_cast<char>(0x00));

  Rtu::appendCrc(placeholder);
  QCOMPARE(placeholder.size(), qsizetype(5));
  QCOMPARE(static_cast<quint8>(placeholder.at(2)), quint8(0x00));
}

QTEST_GUILESS_MAIN(TstModbusRegisterGroups)

#include "tst_modbus_register_groups.moc"
