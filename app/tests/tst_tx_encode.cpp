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

#include <QByteArray>
#include <QString>
#include <QTest>

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "DataModel/ActionBytes.h"

// Spec 0091 AC6: the unified TX encoder is pinned here — payload bytes, EOL placement, and the
// invariant that a named checksum covers the payload INCLUDING the EOL (the console's historical
// semantics). get_tx_bytes(Action) must stay a pure wrapper over encode_tx(), so console sends
// and project Actions can never disagree on the wire.

/**
 * @brief Byte-level contract of DataModel::encode_tx() and its Action wrapper.
 */
class TstTxEncode : public QObject {
  Q_OBJECT

private slots:
  void registryHoldsTheNamesUsedHere();

  void textWithEscapesAndCrlf();
  void hexWithCrc16Modbus();
  void utf8TextNoEolCrc32();

  void checksumCoversTheEol();
  void emptyChecksumAppendsNothing();

  void actionParityTextNoChecksum();
  void actionParityHexWithChecksum();
};

/**
 * @brief A renamed registry entry must fail here, not silently downgrade every case below to
 *        "no checksum appended".
 */
void TstTxEncode::registryHoldsTheNamesUsedHere()
{
  const auto names = IO::availableChecksums();
  QVERIFY(names.contains(QStringLiteral("CRC-16-MODBUS")));
  QVERIFY(names.contains(QStringLiteral("CRC-32")));
}

/**
 * @brief Text payload with an escape sequence plus CRLF EOL bytes, no checksum.
 */
void TstTxEncode::textWithEscapesAndCrlf()
{
  DataModel::TxPayload spec;
  spec.encoding = SerialStudio::EncUtf8;
  spec.payload  = QStringLiteral("PING\\t1");
  spec.eolBytes = QByteArrayLiteral("\r\n");

  QCOMPARE(DataModel::encode_tx(spec), QByteArrayLiteral("PING\t1\r\n"));
}

/**
 * @brief Hex payload with a CRC-16-MODBUS appended over the payload bytes.
 */
void TstTxEncode::hexWithCrc16Modbus()
{
  DataModel::TxPayload spec;
  spec.hex      = true;
  spec.payload  = QStringLiteral("01 03 00 00 00 0A");
  spec.checksum = QStringLiteral("CRC-16-MODBUS");

  const QByteArray base = QByteArrayLiteral("\x01\x03\x00\x00\x00\x0A");
  const QByteArray crc  = IO::checksum(QStringLiteral("CRC-16-MODBUS"), base);
  QCOMPARE(crc.size(), 2);
  QCOMPARE(DataModel::encode_tx(spec), base + crc);
}

/**
 * @brief Multi-byte UTF-8 text, no EOL, CRC-32 appended over the encoded payload.
 */
void TstTxEncode::utf8TextNoEolCrc32()
{
  DataModel::TxPayload spec;
  spec.encoding = SerialStudio::EncUtf8;
  spec.payload  = QString::fromUtf8("h\xC3\xA9llo\xC2\xB0");
  spec.checksum = QStringLiteral("CRC-32");

  const QByteArray base = QByteArrayLiteral("h\xC3\xA9llo\xC2\xB0");
  const QByteArray crc  = IO::checksum(QStringLiteral("CRC-32"), base);
  QCOMPARE(crc.size(), 4);
  QCOMPARE(DataModel::encode_tx(spec), base + crc);
}

/**
 * @brief The checksum covers payload + EOL, never the payload alone.
 */
void TstTxEncode::checksumCoversTheEol()
{
  DataModel::TxPayload spec;
  spec.encoding = SerialStudio::EncUtf8;
  spec.payload  = QStringLiteral("AB");
  spec.eolBytes = QByteArrayLiteral("\r\n");
  spec.checksum = QStringLiteral("CRC-32");

  const QByteArray covered  = QByteArrayLiteral("AB\r\n");
  const QByteArray expected = covered + IO::checksum(QStringLiteral("CRC-32"), covered);
  const QByteArray wrong =
    covered + IO::checksum(QStringLiteral("CRC-32"), QByteArrayLiteral("AB"));

  QCOMPARE(DataModel::encode_tx(spec), expected);
  QVERIFY(DataModel::encode_tx(spec) != wrong);
}

/**
 * @brief An empty checksum name appends nothing (the "" = none contract).
 */
void TstTxEncode::emptyChecksumAppendsNothing()
{
  DataModel::TxPayload spec;
  spec.encoding = SerialStudio::EncUtf8;
  spec.payload  = QStringLiteral("data");
  spec.eolBytes = QByteArrayLiteral("\n");

  QCOMPARE(DataModel::encode_tx(spec), QByteArrayLiteral("data\n"));
}

/**
 * @brief get_tx_bytes() equals encode_tx() on the equivalent spec for a text Action whose
 *        escape-sequence EOL resolves to a newline.
 */
void TstTxEncode::actionParityTextNoChecksum()
{
  DataModel::Action action;
  action.txData      = QStringLiteral("hello");
  action.eolSequence = QStringLiteral("\\n");
  action.txEncoding  = SerialStudio::EncUtf8;

  DataModel::TxPayload spec;
  spec.encoding = SerialStudio::EncUtf8;
  spec.payload  = QStringLiteral("hello");
  spec.eolBytes = QByteArrayLiteral("\n");

  QCOMPARE(DataModel::get_tx_bytes(action), DataModel::encode_tx(spec));
  QCOMPARE(DataModel::get_tx_bytes(action), QByteArrayLiteral("hello\n"));
}

/**
 * @brief get_tx_bytes() equals encode_tx() on the equivalent spec for a hex Action carrying a
 *        checksum name.
 */
void TstTxEncode::actionParityHexWithChecksum()
{
  DataModel::Action action;
  action.binaryData = true;
  action.txData     = QStringLiteral("DE AD");
  action.checksum   = QStringLiteral("CRC-16-MODBUS");

  DataModel::TxPayload spec;
  spec.hex      = true;
  spec.payload  = QStringLiteral("DE AD");
  spec.checksum = QStringLiteral("CRC-16-MODBUS");

  const QByteArray base = QByteArrayLiteral("\xDE\xAD");
  const QByteArray out  = DataModel::get_tx_bytes(action);
  QCOMPARE(out, DataModel::encode_tx(spec));
  QCOMPARE(out, base + IO::checksum(QStringLiteral("CRC-16-MODBUS"), base));
}

QTEST_APPLESS_MAIN(TstTxEncode)

#include "tst_tx_encode.moc"
