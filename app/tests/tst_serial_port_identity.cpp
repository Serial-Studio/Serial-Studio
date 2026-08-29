/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QJsonObject>
#include <QTest>

#include "IO/Drivers/SerialPortIdentity.h"

// Every test function builds its own identities: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Scoring contract of the serial-port identity matcher shared by UART and Modbus.
 */
class TstSerialPortIdentity : public QObject {
  Q_OBJECT

private slots:
  void scoreIdentityMatch_data();
  void scoreIdentityMatch();

  void serialOutranksDescription();
  void differentUnitLosesToSameUnit();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an identity object from the fields a port may report; empty fields are omitted,
 *        exactly as SerialPorts::identity() omits what the device does not expose.
 */
[[nodiscard]] static QJsonObject makeIdentity(const QString& vid,
                                              const QString& pid,
                                              const QString& serial,
                                              const QString& portName,
                                              const QString& description)
{
  QJsonObject id;

  if (!vid.isEmpty())
    id.insert(QStringLiteral("vid"), vid);

  if (!pid.isEmpty())
    id.insert(QStringLiteral("pid"), pid);

  if (!serial.isEmpty())
    id.insert(QStringLiteral("serial"), serial);

  if (!portName.isEmpty())
    id.insert(QStringLiteral("portName"), portName);

  if (!description.isEmpty())
    id.insert(QStringLiteral("description"), description);

  return id;
}

//--------------------------------------------------------------------------------------------------
// scoreIdentityMatch
//--------------------------------------------------------------------------------------------------

void TstSerialPortIdentity::scoreIdentityMatch_data()
{
  QTest::addColumn<QJsonObject>("candidate");
  QTest::addColumn<QJsonObject>("saved");
  QTest::addColumn<int>("expected");

  const auto full = makeIdentity(QStringLiteral("2341"),
                                 QStringLiteral("0043"),
                                 QStringLiteral("A9007M1L"),
                                 QStringLiteral("ttyACM0"),
                                 QStringLiteral("Arduino Uno"));

  QTest::newRow("identical identity") << full << full << 165;

  QTest::newRow("empty saved matches nothing") << full << QJsonObject() << 0;

  QTest::newRow("empty candidate matches nothing") << QJsonObject() << full << 0;

  QTest::newRow("same model, other unit") << full
                                          << makeIdentity(QStringLiteral("2341"),
                                                          QStringLiteral("0043"),
                                                          QStringLiteral("OTHERUNIT"),
                                                          QString(),
                                                          QString())
                                          << 100;

  QTest::newRow("same vid, other pid")
    << full
    << makeIdentity(QStringLiteral("2341"), QStringLiteral("9999"), QString(), QString(), QString())
    << 0;

  QTest::newRow("port name only")
    << full << makeIdentity(QString(), QString(), QString(), QStringLiteral("ttyACM0"), QString())
    << 5;

  QTest::newRow("description only")
    << full
    << makeIdentity(QString(), QString(), QString(), QString(), QStringLiteral("Arduino Uno"))
    << 10;

  QTest::newRow("port name and description")
    << full
    << makeIdentity(
         QString(), QString(), QString(), QStringLiteral("ttyACM0"), QStringLiteral("Arduino Uno"))
    << 15;

  QTest::newRow("moved to another port node") << makeIdentity(QStringLiteral("2341"),
                                                              QStringLiteral("0043"),
                                                              QStringLiteral("A9007M1L"),
                                                              QStringLiteral("ttyACM3"),
                                                              QStringLiteral("Arduino Uno"))
                                              << full << 160;
}

/**
 * @brief The score is the sum of the fields that agree: VID+PID 100, serial 50, description 10,
 *        port name 5. A device that moved to another node still outscores a same-node stranger.
 */
void TstSerialPortIdentity::scoreIdentityMatch()
{
  QFETCH(QJsonObject, candidate);
  QFETCH(QJsonObject, saved);
  QFETCH(int, expected);

  QCOMPARE(IO::Drivers::SerialPorts::scoreIdentityMatch(candidate, saved), expected);
}

/**
 * @brief The unit's serial number is what tells two identical adapters apart, so it must weigh
 *        more than the model description they share.
 */
void TstSerialPortIdentity::serialOutranksDescription()
{
  const auto saved = makeIdentity(QStringLiteral("0403"),
                                  QStringLiteral("6001"),
                                  QStringLiteral("FTABCDEF"),
                                  QStringLiteral("ttyUSB0"),
                                  QStringLiteral("FT232R USB UART"));

  const auto sameUnit = makeIdentity(QStringLiteral("0403"),
                                     QStringLiteral("6001"),
                                     QStringLiteral("FTABCDEF"),
                                     QStringLiteral("ttyUSB9"),
                                     QString());

  const auto twinUnit = makeIdentity(QStringLiteral("0403"),
                                     QStringLiteral("6001"),
                                     QStringLiteral("FT123456"),
                                     QStringLiteral("ttyUSB0"),
                                     QStringLiteral("FT232R USB UART"));

  const int sameScore = IO::Drivers::SerialPorts::scoreIdentityMatch(sameUnit, saved);
  const int twinScore = IO::Drivers::SerialPorts::scoreIdentityMatch(twinUnit, saved);

  QCOMPARE(sameScore, 150);
  QCOMPARE(twinScore, 115);
  QVERIFY(sameScore > twinScore);
}

/**
 * @brief A port of a different model never reaches the hardware score, whatever its name.
 */
void TstSerialPortIdentity::differentUnitLosesToSameUnit()
{
  const auto saved = makeIdentity(QStringLiteral("0403"),
                                  QStringLiteral("6001"),
                                  QString(),
                                  QStringLiteral("ttyUSB0"),
                                  QString());

  const auto stranger = makeIdentity(QStringLiteral("10C4"),
                                     QStringLiteral("EA60"),
                                     QString(),
                                     QStringLiteral("ttyUSB0"),
                                     QString());

  QCOMPARE(IO::Drivers::SerialPorts::scoreIdentityMatch(stranger, saved), 5);
}

QTEST_APPLESS_MAIN(TstSerialPortIdentity)

#include "tst_serial_port_identity.moc"
