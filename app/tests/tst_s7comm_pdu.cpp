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

#include <QTest>

#include "IO/Drivers/S7/S7Pdu.h"

using namespace IO::Drivers;
using namespace IO::Drivers::S7Comm;

/**
 * @brief One answered item as a controller spells it on the wire.
 */
struct Answer {
  int returnCode = 0;
  int transport  = 0;
  QByteArray data;
};

/**
 * @brief Builds a byte array out of an initializer list, which is how every golden packet in this
 *        suite is spelled.
 */
[[nodiscard]] static QByteArray bytes(std::initializer_list<int> values)
{
  QByteArray out;
  out.reserve(static_cast<qsizetype>(values.size()));
  for (const int value : values)
    out.append(static_cast<char>(value & 0xFF));

  return out;
}

/**
 * @brief Resolves an absolute address into the read item the request builder consumes.
 */
[[nodiscard]] static ReadItem parsedItem(const QString& text)
{
  QString error;
  const auto address = S7Address::parse(text, error);
  return itemForAddress(address);
}

/**
 * @brief Renders the data section of a read acknowledgement, padding an odd item that is followed
 *        by another one exactly as a controller does.
 */
[[nodiscard]] static QByteArray answerSection(const QList<Answer>& answers)
{
  QByteArray data;
  for (int i = 0; i < answers.size(); ++i) {
    const auto& answer = answers.at(i);
    const bool counted = answer.transport == kResultBit || answer.transport == kResultBits;
    const int declared =
      counted ? static_cast<int>(answer.data.size()) * 8 : static_cast<int>(answer.data.size());

    data.append(static_cast<char>(answer.returnCode));
    data.append(static_cast<char>(answer.transport));
    data.append(static_cast<char>((declared >> 8) & 0xFF));
    data.append(static_cast<char>(declared & 0xFF));
    data.append(answer.data);
    if (i + 1 < answers.size() && (answer.data.size() % 2) != 0)
      data.append(static_cast<char>(0));
  }

  return data;
}

/**
 * @brief Builds an acknowledgement carrying @p parameters and @p data, with the header error pair
 *        every response reserves two octets for.
 */
[[nodiscard]] static QByteArray acknowledgement(quint16 reference,
                                                const QByteArray& parameters,
                                                const QByteArray& data,
                                                int errorClass,
                                                int errorCode)
{
  QByteArray pdu;
  pdu.append(static_cast<char>(kProtocolId));
  pdu.append(static_cast<char>(kRosctrAckData));
  pdu.append(bytes({0x00, 0x00}));
  pdu.append(static_cast<char>((reference >> 8) & 0xFF));
  pdu.append(static_cast<char>(reference & 0xFF));
  pdu.append(static_cast<char>((parameters.size() >> 8) & 0xFF));
  pdu.append(static_cast<char>(parameters.size() & 0xFF));
  pdu.append(static_cast<char>((data.size() >> 8) & 0xFF));
  pdu.append(static_cast<char>(data.size() & 0xFF));
  pdu.append(static_cast<char>(errorClass));
  pdu.append(static_cast<char>(errorCode));
  pdu.append(parameters);
  pdu.append(data);
  return pdu;
}

/**
 * @brief Builds a read acknowledgement whose parameter section declares @p count items.
 */
[[nodiscard]] static QByteArray readAck(quint16 reference, int count, const QByteArray& data)
{
  const auto parameters = bytes({kFunctionReadVar, count});
  return acknowledgement(reference, parameters, data, 0, 0);
}

/**
 * @brief Pins the S7 application layer: the setup negotiation that fixes the message budget, the
 *        chunking that budget imposes on a variable list, the golden request bytes for every area
 *        and access width, and the response walk that turns controller octets back into values.
 *        A misplaced octet here reads the wrong memory on a live machine, so every field is
 *        checked against bytes rather than against the encoder's own idea of them.
 */
class TstS7CommPdu : public QObject {
  Q_OBJECT

private slots:
  void setupRequestIsTheGoldenJob();
  void setupResponseRecordsTheNegotiatedLength();
  void setupResponseClampsAnImpossibleLength_data();
  void setupResponseClampsAnImpossibleLength();
  void setupRefusalCarriesItsReason();
  void referencesAdvanceAndSkipZero();
  void readRequestIsGoldenForEveryArea_data();
  void readRequestIsGoldenForEveryArea();
  void chunkingFitsBothHalvesOfTheBudget();
  void anOversizedItemStillMakesProgress();
  void responseYieldsOneResultPerItem();
  void refusedItemsAreCountedAndKeepTheirNeighbours();
  void oddLengthItemsCarryAFillOctet();
  void truncatedResponsesAppendNothing_data();
  void truncatedResponsesAppendNothing();
  void aRefusedJobIsNotAMalformedOne();
  void valuesDecodePerDeclaredType_data();
  void valuesDecodePerDeclaredType();
};

//--------------------------------------------------------------------------------------------------
// Setup negotiation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The setup job asks for one outstanding message in each direction and the 960-octet
 *        length every S7 client proposes.
 */
void TstS7CommPdu::setupRequestIsTheGoldenJob()
{
  PduCodec codec;

  const auto request = codec.buildSetupRequest(1);
  QCOMPARE(request,
           bytes({0x32,
                  0x01,
                  0x00,
                  0x00,
                  0x00,
                  0x01,
                  0x00,
                  0x08,
                  0x00,
                  0x00,
                  0xF0,
                  0x00,
                  0x00,
                  0x01,
                  0x00,
                  0x01,
                  0x03,
                  0xC0}));
  QCOMPARE(request.size(), qsizetype(kJobHeaderBytes + kSetupParamBytes));
}

/**
 * @brief The acknowledgement's length is the ceiling every later exchange is planned against, so
 *        it is stored rather than the value the request proposed.
 */
void TstS7CommPdu::setupResponseRecordsTheNegotiatedLength()
{
  PduCodec codec;
  QCOMPARE(codec.pduBytes(), kMinPduBytes);

  const auto parameters = bytes({0xF0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0xE0});
  const auto response   = acknowledgement(1, parameters, QByteArray(), 0, 0);
  QCOMPARE(codec.parseSetupResponse(response), PduResult::Ok);
  QCOMPARE(codec.pduBytes(), 480);
  QCOMPARE(codec.malformedPdus(), quint64(0));
  QVERIFY(codec.lastError().isEmpty());

  codec.reset();
  QCOMPARE(codec.pduBytes(), kMinPduBytes);
}

/**
 * @brief A length outside what the protocol admits is clamped: planning chunks against a bogus
 *        ceiling produces requests the controller answers by truncating them.
 */
void TstS7CommPdu::setupResponseClampsAnImpossibleLength_data()
{
  QTest::addColumn<int>("declared");
  QTest::addColumn<int>("expected");

  QTest::newRow("zero") << 0 << kMinPduBytes;
  QTest::newRow("below the floor") << 100 << kMinPduBytes;
  QTest::newRow("the floor") << 240 << 240;
  QTest::newRow("typical") << 960 << 960;
  QTest::newRow("above the ceiling") << 65000 << kMaxPduBytes;
}

/**
 * @brief Drives the clamping table.
 */
void TstS7CommPdu::setupResponseClampsAnImpossibleLength()
{
  QFETCH(int, declared);
  QFETCH(int, expected);

  const auto parameters =
    bytes({0xF0, 0x00, 0x00, 0x01, 0x00, 0x01, (declared >> 8) & 0xFF, declared & 0xFF});

  PduCodec codec;
  QCOMPARE(codec.parseSetupResponse(acknowledgement(1, parameters, QByteArray(), 0, 0)),
           PduResult::Ok);
  QCOMPARE(codec.pduBytes(), expected);
}

/**
 * @brief A refused setup is a configuration answer, not a broken message: it carries a reason and
 *        does not count as malformed.
 */
void TstS7CommPdu::setupRefusalCarriesItsReason()
{
  const auto parameters = bytes({0xF0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x03, 0xC0});

  PduCodec codec;
  QCOMPARE(codec.parseSetupResponse(acknowledgement(1, parameters, QByteArray(), 0x87, 0x04)),
           PduResult::Refused);
  QVERIFY(!codec.lastError().isEmpty());
  QCOMPARE(codec.malformedPdus(), quint64(0));
  QCOMPARE(codec.pduBytes(), kMinPduBytes);

  QCOMPARE(codec.parseSetupResponse(bytes({0x32, 0x03})), PduResult::Truncated);
  QCOMPARE(codec.malformedPdus(), quint64(1));
}

/**
 * @brief The request reference is what pairs an answer with its question; it never returns to
 *        zero, which is the value a controller sends when it is answering nothing in particular.
 */
void TstS7CommPdu::referencesAdvanceAndSkipZero()
{
  PduCodec codec;
  QCOMPARE(codec.nextReference(), quint16(1));
  QCOMPARE(codec.nextReference(), quint16(2));

  quint16 last = 0;
  for (int i = 0; i < 70000; ++i)
    last = codec.nextReference();

  QVERIFY(last != 0);
}

//--------------------------------------------------------------------------------------------------
// Read requests
//--------------------------------------------------------------------------------------------------

/**
 * @brief One row per area and access width. The start address is counted in BITS, which is the
 *        field a byte-oriented reading of the protocol gets wrong by a factor of eight.
 */
void TstS7CommPdu::readRequestIsGoldenForEveryArea_data()
{
  QTest::addColumn<QString>("address");
  QTest::addColumn<QByteArray>("item");

  QTest::newRow("data block, double word")
    << QStringLiteral("DB5.DBD20")
    << bytes({0x12, 0x0A, 0x10, 0x02, 0x00, 0x04, 0x00, 0x05, 0x84, 0x00, 0x00, 0xA0});
  QTest::newRow("data block, bit")
    << QStringLiteral("DB1.DBX0.3")
    << bytes({0x12, 0x0A, 0x10, 0x01, 0x00, 0x01, 0x00, 0x01, 0x84, 0x00, 0x00, 0x03});
  QTest::newRow("flag memory, word")
    << QStringLiteral("MW10")
    << bytes({0x12, 0x0A, 0x10, 0x02, 0x00, 0x02, 0x00, 0x00, 0x83, 0x00, 0x00, 0x50});
  QTest::newRow("inputs, byte") << QStringLiteral(
    "IB0") << bytes({0x12, 0x0A, 0x10, 0x02, 0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00});
  QTest::newRow("outputs, bit") << QStringLiteral(
    "Q0.1") << bytes({0x12, 0x0A, 0x10, 0x01, 0x00, 0x01, 0x00, 0x00, 0x82, 0x00, 0x00, 0x01});
  QTest::newRow("data block, string")
    << QStringLiteral("DB4.DBB10:STRING[8]")
    << bytes({0x12, 0x0A, 0x10, 0x02, 0x00, 0x0A, 0x00, 0x04, 0x84, 0x00, 0x00, 0x50});
}

/**
 * @brief Builds a one-item job around each row and checks the whole packet, header included.
 */
void TstS7CommPdu::readRequestIsGoldenForEveryArea()
{
  QFETCH(QString, address);
  QFETCH(QByteArray, item);

  QByteArray expected =
    bytes({0x32, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x01});
  expected.append(item);

  PduCodec codec;
  const QList<ReadItem> items{parsedItem(address)};
  const auto request = codec.buildReadRequest(7, items, Chunk{0, 1});
  QCOMPARE(request, expected);
  QCOMPARE(request.size(), qsizetype(kJobHeaderBytes + kReadParamBytes + kRequestItemBytes));
}

/**
 * @brief Chunking spends two budgets at once: twelve octets per item on the way out and four plus
 *        the payload on the way back. Every planned chunk has to fit both, and together the chunks
 *        have to cover the list exactly once.
 */
void TstS7CommPdu::chunkingFitsBothHalvesOfTheBudget()
{
  PduCodec codec;

  QList<ReadItem> items;
  for (int i = 0; i < 40; ++i)
    items.append(parsedItem(QStringLiteral("DB1.DBD%1").arg(i * 4)));

  const auto chunks = codec.planChunks(items);
  QVERIFY(!chunks.isEmpty());

  int covered = 0;
  for (const auto& chunk : chunks) {
    QCOMPARE(chunk.first, covered);
    QVERIFY(chunk.count > 0 && chunk.count <= kMaxItemsPerRequest);

    const auto request = codec.buildReadRequest(1, items, chunk);
    QVERIFY(request.size() <= codec.pduBytes());

    int answered = kAckHeaderBytes + kReadParamBytes;
    for (int i = 0; i < chunk.count; ++i)
      answered += kResultItemBytes + resultBytes(items.at(chunk.first + i));

    QVERIFY(answered <= codec.pduBytes());
    covered += chunk.count;
  }

  QCOMPARE(qsizetype(covered), items.size());
  QCOMPARE(chunks.first().count, 19);
  QCOMPARE(codec.oversizedItems(), quint64(0));
}

/**
 * @brief An item whose own answer cannot fit the budget is still requested, alone in its chunk and
 *        counted: refusing to send it would stall the whole poll on one configuration line.
 */
void TstS7CommPdu::anOversizedItemStillMakesProgress()
{
  PduCodec codec;

  const QList<ReadItem> items{parsedItem(QStringLiteral("DB4.DBB0:STRING[254]")),
                              parsedItem(QStringLiteral("MW0"))};

  const auto chunks = codec.planChunks(items);
  QCOMPARE(chunks.size(), qsizetype(2));
  QCOMPARE(chunks.at(0).count, 1);
  QCOMPARE(chunks.at(1).count, 1);
  QCOMPARE(codec.oversizedItems(), quint64(1));
}

//--------------------------------------------------------------------------------------------------
// Read responses
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every item is located inside the response buffer rather than copied out of it, so the
 *        offsets themselves are what has to be right.
 */
void TstS7CommPdu::responseYieldsOneResultPerItem()
{
  const QList<Answer> answers{
    {kReturnSuccess,   kResultBits, bytes({0x41, 0x20, 0x00, 0x00})},
    {kReturnSuccess,    kResultBit,                   bytes({0x01})},
    {kReturnSuccess, kResultOctets, bytes({0x08, 0x02, 0x41, 0x42})},
  };

  const auto response = readAck(3, answers.size(), answerSection(answers));

  PduCodec codec;
  QList<ReadResult> results;
  QCOMPARE(codec.parseReadResponse(response, answers.size(), results), PduResult::Ok);
  QCOMPARE(results.size(), qsizetype(3));

  QCOMPARE(results.at(0).returnCode, static_cast<std::uint8_t>(kReturnSuccess));
  QCOMPARE(results.at(0).size, qsizetype(4));
  QCOMPARE(response.mid(results.at(0).offset, results.at(0).size), bytes({0x41, 0x20, 0x00, 0x00}));

  QCOMPARE(results.at(1).size, qsizetype(1));
  QCOMPARE(response.mid(results.at(1).offset, results.at(1).size), bytes({0x01}));

  QCOMPARE(results.at(2).size, qsizetype(4));
  QCOMPARE(response.mid(results.at(2).offset, results.at(2).size), bytes({0x08, 0x02, 0x41, 0x42}));
  QCOMPARE(codec.refusedItems(), quint64(0));
}

/**
 * @brief A refused item is counted and reported with its code, and the items around it still
 *        decode: a bad address is a configuration error, not a broken session.
 */
void TstS7CommPdu::refusedItemsAreCountedAndKeepTheirNeighbours()
{
  const QList<Answer> answers{
    {kReturnSuccess, kResultBits, bytes({0x00, 0x0A})},
    {          0x05,        0x00,        QByteArray()},
    {          0x0A,        0x00,        QByteArray()},
    {kReturnSuccess, kResultBits, bytes({0x00, 0x0B})},
  };

  const auto response = readAck(4, answers.size(), answerSection(answers));

  PduCodec codec;
  QList<ReadResult> results;
  QCOMPARE(codec.parseReadResponse(response, answers.size(), results), PduResult::Ok);
  QCOMPARE(results.size(), qsizetype(4));
  QCOMPARE(results.at(1).returnCode, static_cast<std::uint8_t>(0x05));
  QCOMPARE(results.at(1).size, qsizetype(0));
  QCOMPARE(results.at(2).returnCode, static_cast<std::uint8_t>(0x0A));
  QCOMPARE(response.mid(results.at(3).offset, results.at(3).size), bytes({0x00, 0x0B}));
  QCOMPARE(codec.refusedItems(), quint64(2));

  QVERIFY(!returnCodeText(0x05).isEmpty());
  QVERIFY(!returnCodeText(0x0A).isEmpty());
  QVERIFY(returnCodeText(0x05) != returnCodeText(0x0A));
}

/**
 * @brief An odd payload is followed by a fill octet whenever another item follows it. Missing the
 *        fill shifts every later item by one, which decodes as data rather than as an error.
 */
void TstS7CommPdu::oddLengthItemsCarryAFillOctet()
{
  const QList<Answer> answers{
    {kReturnSuccess,    kResultBit,       bytes({0x01})},
    {kReturnSuccess, kResultOctets,       bytes({0x2A})},
    {kReturnSuccess,   kResultBits, bytes({0x12, 0x34})},
  };

  const auto data = answerSection(answers);
  QCOMPARE(data.size(), qsizetype(4 + 1 + 1 + 4 + 1 + 1 + 4 + 2));

  PduCodec codec;
  QList<ReadResult> results;
  QCOMPARE(codec.parseReadResponse(readAck(5, answers.size(), data), answers.size(), results),
           PduResult::Ok);
  QCOMPARE(results.size(), qsizetype(3));

  const auto response = readAck(5, answers.size(), data);
  QCOMPARE(response.mid(results.at(1).offset, results.at(1).size), bytes({0x2A}));
  QCOMPARE(response.mid(results.at(2).offset, results.at(2).size), bytes({0x12, 0x34}));
}

/**
 * @brief The shapes a short or hostile answer arrives in: a header that stops mid-field, a
 *        declared count no data backs, and a length that walks past the message.
 */
void TstS7CommPdu::truncatedResponsesAppendNothing_data()
{
  QTest::addColumn<QByteArray>("response");
  QTest::addColumn<int>("expected");

  QTest::newRow("header stops short") << bytes({0x32, 0x03, 0x00, 0x00, 0x00, 0x01}) << 1;

  const QList<Answer> one{
    {kReturnSuccess, kResultBits, bytes({0x00, 0x01})}
  };
  QTest::newRow("second item never arrives") << readAck(6, 2, answerSection(one)) << 2;

  auto runaway = answerSection(one);
  runaway[2]   = static_cast<char>(0x7F);
  QTest::newRow("length walks past the message") << readAck(6, 1, runaway) << 1;
}

/**
 * @brief A refused response appends NOTHING and leaves whatever the caller already collected: an
 *        item assembled out of the next item's octets carries no marker downstream.
 */
void TstS7CommPdu::truncatedResponsesAppendNothing()
{
  QFETCH(QByteArray, response);
  QFETCH(int, expected);

  PduCodec codec;
  QList<ReadResult> results;
  results.append(ReadResult{kReturnSuccess, 0, 0});

  QCOMPARE(codec.parseReadResponse(response, expected, results), PduResult::Truncated);
  QCOMPARE(results.size(), qsizetype(1));
  QCOMPARE(codec.malformedPdus(), quint64(1));
}

/**
 * @brief A job the controller refuses outright reports its reason and is NOT counted as malformed;
 *        an answer to a service nobody asked for is.
 */
void TstS7CommPdu::aRefusedJobIsNotAMalformedOne()
{
  const auto parameters = bytes({kFunctionReadVar, 0x01});

  PduCodec codec;
  QList<ReadResult> results;
  QCOMPARE(
    codec.parseReadResponse(acknowledgement(7, parameters, QByteArray(), 0x85, 0x00), 1, results),
    PduResult::Refused);
  QVERIFY(!codec.lastError().isEmpty());
  QVERIFY(results.isEmpty());
  QCOMPARE(codec.malformedPdus(), quint64(0));

  const auto wrongService = bytes({kFunctionSetup, 0x01});
  QCOMPARE(
    codec.parseReadResponse(acknowledgement(7, wrongService, QByteArray(), 0, 0), 1, results),
    PduResult::Malformed);
  QCOMPARE(codec.malformedPdus(), quint64(1));

  const auto wrongCount = bytes({kFunctionReadVar, 0x04});
  QCOMPARE(codec.parseReadResponse(acknowledgement(7, wrongCount, QByteArray(), 0, 0), 1, results),
           PduResult::Malformed);
  QCOMPARE(codec.malformedPdus(), quint64(2));
}

//--------------------------------------------------------------------------------------------------
// Value decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief One row per declared type. S7 controllers are big-endian, so a byte order taken from the
 *        host would read every multi-octet value as a different number.
 */
void TstS7CommPdu::valuesDecodePerDeclaredType_data()
{
  QTest::addColumn<QString>("address");
  QTest::addColumn<QByteArray>("raw");
  QTest::addColumn<QVariant>("expected");

  QTest::newRow("bool set") << QStringLiteral("DB1.DBX0.3") << bytes({0x01}) << QVariant(true);
  QTest::newRow("bool clear") << QStringLiteral("DB1.DBX0.3") << bytes({0x00}) << QVariant(false);
  QTest::newRow("byte") << QStringLiteral("MB0") << bytes({0x7F})
                        << QVariant(static_cast<uint>(127));
  QTest::newRow("word") << QStringLiteral("MW0") << bytes({0x12, 0x34})
                        << QVariant(static_cast<uint>(4660));
  QTest::newRow("double word") << QStringLiteral("MD0") << bytes({0x12, 0x34, 0x56, 0x78})
                               << QVariant(static_cast<qulonglong>(305419896));
  QTest::newRow("signed int") << QStringLiteral("MW0:INT") << bytes({0xFF, 0xFE}) << QVariant(-2);
  QTest::newRow("signed double int")
    << QStringLiteral("MD0:DINT") << bytes({0xFF, 0xFF, 0xFF, 0xFE}) << QVariant(-2);
  QTest::newRow("real") << QStringLiteral("MD0:REAL") << bytes({0x41, 0x20, 0x00, 0x00})
                        << QVariant(10.0);
  QTest::newRow("negative real") << QStringLiteral("MD0:REAL") << bytes({0xC0, 0x00, 0x00, 0x00})
                                 << QVariant(-2.0);
  QTest::newRow("string") << QStringLiteral("DB4.DBB10:STRING[8]")
                          << bytes({0x08, 0x03, 0x41, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00})
                          << QVariant(QStringLiteral("ABC"));
  QTest::newRow("string longer than it declares")
    << QStringLiteral("DB4.DBB10:STRING[4]") << bytes({0x04, 0x40, 0x41, 0x42})
    << QVariant(QStringLiteral("AB"));
}

/**
 * @brief Drives the decode table.
 */
void TstS7CommPdu::valuesDecodePerDeclaredType()
{
  QFETCH(QString, address);
  QFETCH(QByteArray, raw);
  QFETCH(QVariant, expected);

  QString error;
  const auto parsed = S7Address::parse(address, error);
  QVERIFY(S7Address::isValid(parsed));

  const auto value = decodeValue(parsed, raw);
  QVERIFY(value.isValid());
  QCOMPARE(value, expected);
}

QTEST_APPLESS_MAIN(TstS7CommPdu)

#include "tst_s7comm_pdu.moc"
