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

#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QTest>
#include <QVector>
#include <vector>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing. CJK/emoji fixtures are spelled with \u escapes
// (never raw non-ASCII source bytes) so the file stays ASCII-only under code-verify.

using namespace DataModel;

/**
 * @brief Byte-level contract of the pure SerialStudio helpers in SerialStudioFrameSupport.cpp.
 */
class TstFrameSupport : public QObject {
  Q_OBJECT

private slots:
  void hexToBytes_data();
  void hexToBytes();
  void hexToBytesIsCaseInsensitive();

  void resolveEscapeSequences_data();
  void resolveEscapeSequences();
  void resolveEscapeSequencesKeepsATrailingLoneBackslash();
  void resolveEscapeSequencesPassesAnUnknownEscapeThrough();

  void commercialCfgIsFalseForAnEmptyProject();
  void commercialCfgIsFalseForAPlainInputGroup();
  void commercialCfgIsTrueForAnOutputGroup();
  void commercialCfgFlagsCommercialWidgets_data();
  void commercialCfgFlagsCommercialWidgets();
  void commercialCfgIsTrueForAWaterfallDataset();
  void commercialCfgIgnoresATransformWithoutNotify();
  void commercialCfgSkipsAnEmptyTransformCode();
  void commercialCfgFlagsANotifyFamilyCall_data();
  void commercialCfgFlagsANotifyFamilyCall();
  void commercialCfgMatchesOnANotifySubstring();
  void commercialCfgTriggersOnTheLastGroupOnly();
  void commercialCfgQVectorAndStdVectorOverloadsAgree();

  void encodeTextOfEmptyStringIsEmpty_data();
  void encodeTextOfEmptyStringIsEmpty();
  void decodeTextOfEmptyBytesIsEmpty_data();
  void decodeTextOfEmptyBytesIsEmpty();
  void utf8RoundTripsAsciiAndMultiByteCodepoints();
  void latin1SubstitutesAnOutOfRangeCharacter();
  void utf16LEAndBEDifferInByteOrder();
  void utf16RoundTripsBothByteOrders();
  void cjkCodecsRoundTrip_data();
  void cjkCodecsRoundTrip();
  void outOfRangeEncodingFallsBackToUtf8();
  void encSystemRoundTripsAscii();
};

//--------------------------------------------------------------------------------------------------
// hexToBytes
//--------------------------------------------------------------------------------------------------

void TstFrameSupport::hexToBytes_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("empty") << QString() << QByteArray();
  QTest::newRow("single-byte") << QStringLiteral("41") << QByteArray::fromHex("41");
  QTest::newRow("two-bytes-with-space") << QStringLiteral("41 42") << QByteArray("AB");
  QTest::newRow("space-separated") << QStringLiteral("41 42 43") << QByteArray("ABC");
  QTest::newRow("odd-length") << QStringLiteral("414") << QByteArray();
  QTest::newRow("non-hex-digit") << QStringLiteral("4G") << QByteArray();
  QTest::newRow("zero-byte") << QStringLiteral("00") << QByteArray(1, '\0');
  QTest::newRow("spaces-only") << QStringLiteral("   ") << QByteArray();
}

/**
 * @brief hexToBytes() strips spaces, rejects odd length and non-hex digits, and packs pairs.
 */
void TstFrameSupport::hexToBytes()
{
  QFETCH(QString, input);
  QFETCH(QByteArray, expected);

  QCOMPARE(SerialStudio::hexToBytes(input), expected);
}

/**
 * @brief Hex parsing is case-insensitive: the same byte is produced either way.
 */
void TstFrameSupport::hexToBytesIsCaseInsensitive()
{
  QCOMPARE(SerialStudio::hexToBytes(QStringLiteral("4a")),
           SerialStudio::hexToBytes(QStringLiteral("4A")));
  QCOMPARE(SerialStudio::hexToBytes(QStringLiteral("deadbeef")),
           SerialStudio::hexToBytes(QStringLiteral("DEADBEEF")));
}

//--------------------------------------------------------------------------------------------------
// resolveEscapeSequences
//--------------------------------------------------------------------------------------------------

void TstFrameSupport::resolveEscapeSequences_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("newline") << QStringLiteral("\\n") << QString(QChar(u'\n'));
  QTest::newRow("tab-cr-lf") << QStringLiteral("\\t\\r\\n")
                             << QString(QChar(u'\t')) + QChar(u'\r') + QChar(u'\n');
  QTest::newRow("escaped-backslash") << QStringLiteral("\\\\") << QString(QChar(u'\\'));
  QTest::newRow("identity") << QStringLiteral("hello world") << QStringLiteral("hello world");
  QTest::newRow("empty") << QString() << QString();
  QTest::newRow("consecutive-escapes")
    << QStringLiteral("\\n\\t") << QString(QChar(u'\n')) + QChar(u'\t');
}

/**
 * @brief resolveEscapeSequences() turns C-style two-character escapes into control characters.
 */
void TstFrameSupport::resolveEscapeSequences()
{
  QFETCH(QString, input);
  QFETCH(QString, expected);

  QCOMPARE(SerialStudio::resolveEscapeSequences(input), expected);
}

/**
 * @brief A backslash with nothing after it is appended literally instead of being dropped.
 */
void TstFrameSupport::resolveEscapeSequencesKeepsATrailingLoneBackslash()
{
  const QString input = QStringLiteral("abc") + QChar(u'\\');
  QCOMPARE(SerialStudio::resolveEscapeSequences(input), input);
}

/**
 * @brief An escape letter the switch does not recognize passes both characters through unchanged.
 */
void TstFrameSupport::resolveEscapeSequencesPassesAnUnknownEscapeThrough()
{
  QCOMPARE(SerialStudio::resolveEscapeSequences(QStringLiteral("\\q")), QStringLiteral("\\q"));
}

//--------------------------------------------------------------------------------------------------
// commercialCfg fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a one-group project with the given type and widget name, no datasets.
 */
static std::vector<Group> singleGroupProject(GroupType type, const QString& widget)
{
  Group group;
  group.groupType = type;
  group.widget    = widget;
  return {group};
}

/**
 * @brief Builds a one-group, one-dataset project with a non-commercial widget.
 */
static std::vector<Group> singleDatasetProject(const Dataset& dataset)
{
  Group group;
  group.widget = QStringLiteral("bar");
  group.datasets.push_back(dataset);
  return {group};
}

//--------------------------------------------------------------------------------------------------
// commercialCfg
//--------------------------------------------------------------------------------------------------

/**
 * @brief No groups at all cannot require commercial features.
 */
void TstFrameSupport::commercialCfgIsFalseForAnEmptyProject()
{
  QVERIFY(!SerialStudio::commercialCfg(std::vector<Group>()));
  QVERIFY(!SerialStudio::commercialCfg(QVector<Group>()));
}

/**
 * @brief A plain input group with a non-commercial widget and no datasets is free.
 */
void TstFrameSupport::commercialCfgIsFalseForAPlainInputGroup()
{
  const auto project = singleGroupProject(GroupType::Input, QStringLiteral("bar"));
  QVERIFY(!SerialStudio::commercialCfg(project));
}

/**
 * @brief Output (control) groups are Pro regardless of widget.
 */
void TstFrameSupport::commercialCfgIsTrueForAnOutputGroup()
{
  const auto project = singleGroupProject(GroupType::Output, QStringLiteral("bar"));
  QVERIFY(SerialStudio::commercialCfg(project));
}

void TstFrameSupport::commercialCfgFlagsCommercialWidgets_data()
{
  QTest::addColumn<QString>("widget");
  QTest::addColumn<bool>("expected");

  QTest::newRow("plot3d") << QStringLiteral("plot3d") << true;
  QTest::newRow("image") << QStringLiteral("image") << true;
  QTest::newRow("painter") << QStringLiteral("painter") << true;
  QTest::newRow("gauge") << QStringLiteral("gauge") << false;
  QTest::newRow("bar") << QStringLiteral("bar") << false;
}

/**
 * @brief plot3d, image and painter are Pro-only group widgets; gauge and bar are not.
 */
void TstFrameSupport::commercialCfgFlagsCommercialWidgets()
{
  QFETCH(QString, widget);
  QFETCH(bool, expected);

  const auto project = singleGroupProject(GroupType::Input, widget);
  QCOMPARE(SerialStudio::commercialCfg(project), expected);
}

/**
 * @brief A single waterfall-enabled dataset is enough to flag the whole project as Pro.
 */
void TstFrameSupport::commercialCfgIsTrueForAWaterfallDataset()
{
  Dataset dataset;
  dataset.waterfall = true;

  QVERIFY(SerialStudio::commercialCfg(singleDatasetProject(dataset)));
}

/**
 * @brief A transform script that never calls the notify() family does not require Pro.
 */
void TstFrameSupport::commercialCfgIgnoresATransformWithoutNotify()
{
  Dataset dataset;
  dataset.transformCode = QStringLiteral("return value * 2;");

  QVERIFY(!SerialStudio::commercialCfg(singleDatasetProject(dataset)));
}

/**
 * @brief An empty transform script is skipped before the notify() scan even runs.
 */
void TstFrameSupport::commercialCfgSkipsAnEmptyTransformCode()
{
  Dataset dataset;
  dataset.transformCode = QString();

  QVERIFY(!SerialStudio::commercialCfg(singleDatasetProject(dataset)));
}

void TstFrameSupport::commercialCfgFlagsANotifyFamilyCall_data()
{
  QTest::addColumn<QString>("code");

  QTest::newRow("notify") << QStringLiteral("notify('hi');");
  QTest::newRow("notifyInfo") << QStringLiteral("notifyInfo('hi');");
  QTest::newRow("notifyWarning") << QStringLiteral("notifyWarning('hi');");
  QTest::newRow("notifyCritical") << QStringLiteral("notifyCritical('hi');");
  QTest::newRow("notifyClear") << QStringLiteral("notifyClear();");
}

/**
 * @brief Every member of the notify() API family flags the dataset's transform as commercial.
 */
void TstFrameSupport::commercialCfgFlagsANotifyFamilyCall()
{
  QFETCH(QString, code);

  Dataset dataset;
  dataset.transformCode = code;

  QVERIFY(SerialStudio::commercialCfg(singleDatasetProject(dataset)));
}

/**
 * @brief transformUsesNotifications() is a plain substring scan: "xnotify(" also matches "notify(",
 *        which is the actual shipped behavior rather than a strict function-call match.
 */
void TstFrameSupport::commercialCfgMatchesOnANotifySubstring()
{
  Dataset dataset;
  dataset.transformCode = QStringLiteral("xnotify(42);");

  QVERIFY(SerialStudio::commercialCfg(singleDatasetProject(dataset)));
}

/**
 * @brief The scan checks every group, so a trigger on the last group is not shadowed by earlier
 *        non-commercial groups.
 */
void TstFrameSupport::commercialCfgTriggersOnTheLastGroupOnly()
{
  Group plain;
  plain.groupType = GroupType::Input;
  plain.widget    = QStringLiteral("bar");

  Group commercial;
  commercial.groupType = GroupType::Output;
  commercial.widget    = QStringLiteral("bar");

  const std::vector<Group> project{plain, commercial};
  QVERIFY(SerialStudio::commercialCfg(project));
}

/**
 * @brief The QVector and std::vector overloads run the exact same logic; a drift regression would
 *        show up as this test disagreeing on an identical fixture.
 */
void TstFrameSupport::commercialCfgQVectorAndStdVectorOverloadsAgree()
{
  const QString widget = QStringLiteral("bar");
  const std::vector<Group> emptyProject;
  const std::vector<Group> plainProject  = singleGroupProject(GroupType::Input, widget);
  const std::vector<Group> outputProject = singleGroupProject(GroupType::Output, widget);

  QVector<Group> emptyProjectQ;
  QVector<Group> plainProjectQ{plainProject.begin(), plainProject.end()};
  QVector<Group> outputProjectQ{outputProject.begin(), outputProject.end()};

  QCOMPARE(SerialStudio::commercialCfg(emptyProject), SerialStudio::commercialCfg(emptyProjectQ));
  QCOMPARE(SerialStudio::commercialCfg(plainProject), SerialStudio::commercialCfg(plainProjectQ));
  QCOMPARE(SerialStudio::commercialCfg(outputProject), SerialStudio::commercialCfg(outputProjectQ));
}

//--------------------------------------------------------------------------------------------------
// encodeText / decodeText -- empty input
//--------------------------------------------------------------------------------------------------

void TstFrameSupport::encodeTextOfEmptyStringIsEmpty_data()
{
  QTest::addColumn<int>("enc");

  QTest::newRow("Utf8") << int(SerialStudio::EncUtf8);
  QTest::newRow("Utf16LE") << int(SerialStudio::EncUtf16LE);
  QTest::newRow("Utf16BE") << int(SerialStudio::EncUtf16BE);
  QTest::newRow("Latin1") << int(SerialStudio::EncLatin1);
  QTest::newRow("System") << int(SerialStudio::EncSystem);
  QTest::newRow("Gbk") << int(SerialStudio::EncGbk);
  QTest::newRow("Gb18030") << int(SerialStudio::EncGb18030);
  QTest::newRow("Big5") << int(SerialStudio::EncBig5);
  QTest::newRow("ShiftJis") << int(SerialStudio::EncShiftJis);
  QTest::newRow("EucJp") << int(SerialStudio::EncEucJp);
  QTest::newRow("EucKr") << int(SerialStudio::EncEucKr);
}

/**
 * @brief encodeText() short-circuits on an empty string for every one of the eleven encodings.
 */
void TstFrameSupport::encodeTextOfEmptyStringIsEmpty()
{
  QFETCH(int, enc);

  const auto encoding = static_cast<SerialStudio::TextEncoding>(enc);
  QVERIFY(SerialStudio::encodeText(QString(), encoding).isEmpty());
}

void TstFrameSupport::decodeTextOfEmptyBytesIsEmpty_data()
{
  encodeTextOfEmptyStringIsEmpty_data();
}

/**
 * @brief decodeText() short-circuits on an empty byte view for every one of the eleven encodings.
 */
void TstFrameSupport::decodeTextOfEmptyBytesIsEmpty()
{
  QFETCH(int, enc);

  const auto encoding = static_cast<SerialStudio::TextEncoding>(enc);
  QVERIFY(SerialStudio::decodeText(QByteArray(), encoding).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// encodeText / decodeText -- native QStringConverter encodings
//--------------------------------------------------------------------------------------------------

/**
 * @brief UTF-8 round-trips plain ASCII together with a multi-byte BMP codepoint.
 */
void TstFrameSupport::utf8RoundTripsAsciiAndMultiByteCodepoints()
{
  const QString text = QStringLiteral("Hello \u4E2D\u6587 Test");

  const auto bytes = SerialStudio::encodeText(text, SerialStudio::EncUtf8);
  QVERIFY(bytes.size() > text.size());
  QCOMPARE(SerialStudio::decodeText(bytes, SerialStudio::EncUtf8), text);
}

/**
 * @brief A codepoint outside Latin-1 is replaced rather than dropped -- lossy but stable.
 */
void TstFrameSupport::latin1SubstitutesAnOutOfRangeCharacter()
{
  const QString text = QStringLiteral("A\u4E2DB");

  const auto bytes = SerialStudio::encodeText(text, SerialStudio::EncLatin1);
  QCOMPARE(bytes.size(), qsizetype(3));
  QCOMPARE(static_cast<quint8>(bytes.at(1)), quint8('?'));
  QCOMPARE(static_cast<quint8>(bytes.at(0)), quint8('A'));
  QCOMPARE(static_cast<quint8>(bytes.at(2)), quint8('B'));

  const auto roundTrip = SerialStudio::decodeText(bytes, SerialStudio::EncLatin1);
  QVERIFY(roundTrip != text);
  QCOMPARE(roundTrip.size(), text.size());
}

/**
 * @brief UTF-16LE and UTF-16BE pack the exact same string with the two bytes per unit swapped.
 */
void TstFrameSupport::utf16LEAndBEDifferInByteOrder()
{
  const QString text = QStringLiteral("AB");

  const auto le = SerialStudio::encodeText(text, SerialStudio::EncUtf16LE);
  const auto be = SerialStudio::encodeText(text, SerialStudio::EncUtf16BE);

  QCOMPARE(le, QByteArray::fromHex("41004200"));
  QCOMPARE(be, QByteArray::fromHex("00410042"));
  QVERIFY(le != be);
}

/**
 * @brief Each UTF-16 byte order round-trips through its own encoder/decoder pair.
 */
void TstFrameSupport::utf16RoundTripsBothByteOrders()
{
  const QString text = QStringLiteral("Hello \u4E2D\u6587");

  const auto le = SerialStudio::encodeText(text, SerialStudio::EncUtf16LE);
  const auto be = SerialStudio::encodeText(text, SerialStudio::EncUtf16BE);

  QCOMPARE(SerialStudio::decodeText(le, SerialStudio::EncUtf16LE), text);
  QCOMPARE(SerialStudio::decodeText(be, SerialStudio::EncUtf16BE), text);
}

//--------------------------------------------------------------------------------------------------
// encodeText / decodeText -- legacy East-Asian codecs (QTextCodec)
//--------------------------------------------------------------------------------------------------

void TstFrameSupport::cjkCodecsRoundTrip_data()
{
  QTest::addColumn<int>("enc");
  QTest::addColumn<QString>("text");

  QTest::newRow("Gbk-simplified-chinese")
    << int(SerialStudio::EncGbk) << QStringLiteral("\u4E2D\u6587");
  QTest::newRow("Gb18030-simplified-chinese")
    << int(SerialStudio::EncGb18030) << QStringLiteral("\u4E2D\u6587");
  QTest::newRow("Big5-chinese") << int(SerialStudio::EncBig5) << QStringLiteral("\u4E2D\u6587");
  QTest::newRow("ShiftJis-japanese")
    << int(SerialStudio::EncShiftJis) << QStringLiteral("\u3042\u3044\u3046");
  QTest::newRow("EucJp-japanese") << int(SerialStudio::EncEucJp)
                                  << QStringLiteral("\u3042\u3044\u3046");
  QTest::newRow("EucKr-korean") << int(SerialStudio::EncEucKr)
                                << QStringLiteral("\uD55C\uAD6D\uC5B4");
}

/**
 * @brief Every legacy East-Asian codec round-trips its own representative script through
 *        QTextCodec::fromUnicode()/toUnicode().
 */
void TstFrameSupport::cjkCodecsRoundTrip()
{
  QFETCH(int, enc);
  QFETCH(QString, text);

  const auto encoding = static_cast<SerialStudio::TextEncoding>(enc);
  const auto bytes    = SerialStudio::encodeText(text, encoding);

  QVERIFY(!bytes.isEmpty());
  QCOMPARE(SerialStudio::decodeText(bytes, encoding), text);
}

//--------------------------------------------------------------------------------------------------
// encodeText / decodeText -- fallback and locale-dependent paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief An out-of-range enum value falls back to the legacy UTF-8 codec instead of crashing.
 */
void TstFrameSupport::outOfRangeEncodingFallsBackToUtf8()
{
  const auto encoding = static_cast<SerialStudio::TextEncoding>(99);
  const QString text  = QStringLiteral("Hello");

  const auto bytes = SerialStudio::encodeText(text, encoding);
  QCOMPARE(bytes, text.toUtf8());
  QCOMPARE(SerialStudio::decodeText(bytes, encoding), text);
}

/**
 * @brief EncSystem is locale-dependent, so only plain ASCII is checked -- every locale round-trips
 *        ASCII identically.
 */
void TstFrameSupport::encSystemRoundTripsAscii()
{
  const QString text = QStringLiteral("Hello System");

  const auto bytes = SerialStudio::encodeText(text, SerialStudio::EncSystem);
  QCOMPARE(SerialStudio::decodeText(bytes, SerialStudio::EncSystem), text);
}

QTEST_APPLESS_MAIN(TstFrameSupport)

#include "tst_frame_support.moc"
