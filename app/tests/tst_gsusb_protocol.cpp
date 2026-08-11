/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
#include <QTest>

#include "IO/Drivers/CANBus/GsUsbProtocol.h"

using namespace IO::Drivers::GsUsb;

// Classic CANable limits (STM32F072 candleLight): 48 MHz CAN clock, tseg1 1-16, tseg2 1-8,
// sjw <= 4, brp 1-1024. The classic vectors below pin the solver's exact output for the nine
// standard rates the app offers -- any diff here is a wire-visible behavior change (spec 0049 R5).
static constexpr GsDeviceBtConst kClassicLimits = {0, 48000000, 1, 16, 1, 8, 4, 1, 1024, 1};

// FD data-phase limits in the shape a CANable 2.0-class device (STM32G431 FDCAN) reports through
// BT_CONST_EXT: 80 MHz clock, dtseg1 1-16, dtseg2 1-8, dsjw <= 4, dbrp 1-32.
static constexpr GsDeviceBtConstExt kFdLimits = {
  kFeatureFd | kFeatureBtConstExt,
  80000000,
  1,
  255,
  1,
  128,
  128,
  1,
  512,
  1,
  1,
  16,
  1,
  8,
  4,
  1,
  32,
  1,
};

/**
 * @brief Pure gs_usb protocol helpers: DLC code mapping and the bit-timing solver (spec 0049).
 */
class TstGsUsbProtocol : public QObject {
  Q_OBJECT

private slots:
  void dlcToLengthTable();
  void lengthToDlcRoundsUp();
  void dlcRoundTrip();
  void classicTimingIsPinned_data();
  void classicTimingIsPinned();
  void fdDataTimingSolves_data();
  void fdDataTimingSolves();
  void timingLimitsSelectsPhase();
  void solverRejectsImpossibleRates();
};

/**
 * @brief dlc2len() reproduces the CAN FD DLC table for all 16 codes.
 */
void TstGsUsbProtocol::dlcToLengthTable()
{
  const int expected[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
  for (int dlc = 0; dlc < 16; ++dlc)
    QCOMPARE(dlc2len(static_cast<std::uint8_t>(dlc)), expected[dlc]);

  QCOMPARE(dlc2len(static_cast<std::uint8_t>(0xFF)), 64);
}

/**
 * @brief len2dlc() rounds odd payload lengths up to the next valid CAN FD size (spec 0049 R4).
 */
void TstGsUsbProtocol::lengthToDlcRoundsUp()
{
  QCOMPARE(dlc2len(len2dlc(0)), 0);
  QCOMPARE(dlc2len(len2dlc(8)), 8);
  QCOMPARE(dlc2len(len2dlc(9)), 12);
  QCOMPARE(dlc2len(len2dlc(13)), 16);
  QCOMPARE(dlc2len(len2dlc(20)), 20);
  QCOMPARE(dlc2len(len2dlc(21)), 24);
  QCOMPARE(dlc2len(len2dlc(49)), 64);
  QCOMPARE(dlc2len(len2dlc(64)), 64);
  QCOMPARE(dlc2len(len2dlc(-3)), 0);
  QCOMPARE(dlc2len(len2dlc(1000)), 64);
}

/**
 * @brief Every valid DLC survives the len -> dlc -> len round trip unchanged.
 */
void TstGsUsbProtocol::dlcRoundTrip()
{
  for (int dlc = 0; dlc < 16; ++dlc) {
    const int length = dlc2len(static_cast<std::uint8_t>(dlc));
    QCOMPARE(len2dlc(length), static_cast<std::uint8_t>(dlc));
  }
}

/**
 * @brief Solver output for the nine standard classic rates at 48 MHz, pinned bit-for-bit.
 */
void TstGsUsbProtocol::classicTimingIsPinned_data()
{
  QTest::addColumn<quint32>("bitrate");
  QTest::addColumn<quint32>("brp");
  QTest::addColumn<quint32>("propSeg");
  QTest::addColumn<quint32>("phaseSeg1");
  QTest::addColumn<quint32>("phaseSeg2");
  QTest::addColumn<quint32>("sjw");

  QTest::newRow("10k") << 10000u << 240u << 8u << 8u << 3u << 3u;
  QTest::newRow("20k") << 20000u << 120u << 8u << 8u << 3u << 3u;
  QTest::newRow("50k") << 50000u << 48u << 8u << 8u << 3u << 3u;
  QTest::newRow("100k") << 100000u << 24u << 8u << 8u << 3u << 3u;
  QTest::newRow("125k") << 125000u << 24u << 7u << 6u << 2u << 2u;
  QTest::newRow("250k") << 250000u << 12u << 7u << 6u << 2u << 2u;
  QTest::newRow("500k") << 500000u << 6u << 7u << 6u << 2u << 2u;
  QTest::newRow("800k") << 800000u << 3u << 8u << 8u << 3u << 3u;
  QTest::newRow("1M") << 1000000u << 3u << 7u << 6u << 2u << 2u;
}

void TstGsUsbProtocol::classicTimingIsPinned()
{
  QFETCH(quint32, bitrate);
  QFETCH(quint32, brp);
  QFETCH(quint32, propSeg);
  QFETCH(quint32, phaseSeg1);
  QFETCH(quint32, phaseSeg2);
  QFETCH(quint32, sjw);

  GsDeviceBitTiming timing{};
  QVERIFY(solveBitTiming(kClassicLimits, bitrate, timing));
  QCOMPARE(timing.brp, brp);
  QCOMPARE(timing.propSeg, propSeg);
  QCOMPARE(timing.phaseSeg1, phaseSeg1);
  QCOMPARE(timing.phaseSeg2, phaseSeg2);
  QCOMPARE(timing.sjw, sjw);
}

/**
 * @brief FD data-phase rates solve within the G431-class limits and reconstruct the exact rate.
 */
void TstGsUsbProtocol::fdDataTimingSolves_data()
{
  QTest::addColumn<quint32>("bitrate");

  QTest::newRow("1M") << 1000000u;
  QTest::newRow("2M") << 2000000u;
  QTest::newRow("4M") << 4000000u;
  QTest::newRow("5M") << 5000000u;
  QTest::newRow("8M") << 8000000u;
}

void TstGsUsbProtocol::fdDataTimingSolves()
{
  QFETCH(quint32, bitrate);

  const GsDeviceBtConst limits = timingLimits(kFdLimits, true);
  GsDeviceBitTiming timing{};
  QVERIFY(solveBitTiming(limits, bitrate, timing));

  QVERIFY(timing.brp >= limits.brpMin && timing.brp <= limits.brpMax);
  const quint32 tseg1 = timing.propSeg + timing.phaseSeg1;
  QVERIFY(tseg1 >= limits.tseg1Min && tseg1 <= limits.tseg1Max);
  QVERIFY(timing.phaseSeg2 >= limits.tseg2Min && timing.phaseSeg2 <= limits.tseg2Max);
  QVERIFY(timing.sjw >= 1 && timing.sjw <= limits.sjwMax);

  const quint32 total = 1 + tseg1 + timing.phaseSeg2;
  QCOMPARE(limits.fclkCan / (timing.brp * total), bitrate);
  QCOMPARE(limits.fclkCan % (timing.brp * total), 0u);
}

/**
 * @brief timingLimits() carves the arbitration and data-phase views out of BT_CONST_EXT.
 */
void TstGsUsbProtocol::timingLimitsSelectsPhase()
{
  const GsDeviceBtConst nominal = timingLimits(kFdLimits, false);
  QCOMPARE(nominal.fclkCan, kFdLimits.fclkCan);
  QCOMPARE(nominal.tseg1Max, kFdLimits.tseg1Max);
  QCOMPARE(nominal.brpMax, kFdLimits.brpMax);

  const GsDeviceBtConst data = timingLimits(kFdLimits, true);
  QCOMPARE(data.fclkCan, kFdLimits.fclkCan);
  QCOMPARE(data.tseg1Max, kFdLimits.dtseg1Max);
  QCOMPARE(data.tseg2Max, kFdLimits.dtseg2Max);
  QCOMPARE(data.sjwMax, kFdLimits.dsjwMax);
  QCOMPARE(data.brpMax, kFdLimits.dbrpMax);
}

/**
 * @brief Zero rates, zero clocks, unreachable rates, and hostile device-supplied limits are
 *        refused in bounded time (the limits come off the wire from untrusted firmware).
 */
void TstGsUsbProtocol::solverRejectsImpossibleRates()
{
  GsDeviceBitTiming timing{};
  QVERIFY(!solveBitTiming(kClassicLimits, 0, timing));

  GsDeviceBtConst dead = kClassicLimits;
  dead.fclkCan         = 0;
  QVERIFY(!solveBitTiming(dead, 500000, timing));

  QVERIFY(!solveBitTiming(kClassicLimits, 48000000, timing));

  GsDeviceBtConst hostile = kClassicLimits;
  hostile.brpMax          = 0xFFFFFFFFu;
  hostile.brpInc          = 0;
  QVERIFY(!solveBitTiming(hostile, 999983, timing));

  GsDeviceBtConst inverted = kClassicLimits;
  inverted.brpMin          = 10;
  inverted.brpMax          = 5;
  QVERIFY(!solveBitTiming(inverted, 500000, timing));

  GsDeviceBtConst wrap = kClassicLimits;
  QVERIFY(!solveBitTiming(wrap, 0xFFFFFFF0u, timing));
}

QTEST_APPLESS_MAIN(TstGsUsbProtocol)

#include "tst_gsusb_protocol.moc"
