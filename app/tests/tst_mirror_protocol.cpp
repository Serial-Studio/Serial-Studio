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

#include <cmath>
#include <limits>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <QTest>
#include <utility>
#include <vector>

#include "API/Mirror/MirrorProtocol.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Fixture helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The six-entry identity list from the first (structure) line of
 *        tests/fixtures/mirror/small.ndjson, reproduced inline per this suite's no-file-fixture
 *        rule. Its recorded layoutHash in tests/fixtures/mirror/manifest.json is 3af81b130b65f883.
 */
static std::vector<API::Mirror::DatasetId> smallFixtureDatasets()
{
  return {
    {0,     0},
    {0,     1},
    {0,     2},
    {0, 10000},
    {0, 10001},
    {0, 10002}
  };
}

/**
 * @brief A structure payload built over the small-fixture identity list, reused by the chunking
 *        and JSON-validity tests below.
 */
static QJsonObject sampleStructurePayload()
{
  QJsonObject project;
  project.insert(QLatin1String("title"), QLatin1String("Lorenz Attractor"));

  return API::Mirror::encodeStructure(
    7, smallFixtureDatasets(), {0}, project, 0, 10.0, false, 1753459200000);
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

/**
 * @brief Byte-level contract of the spec-0040 mirror wire codec in MirrorProtocol.h: the layout
 *        hash, the significant-digit rounding, and the structure/snapshot/heartbeat/chunk encoders.
 */
class TstMirrorProtocol : public QObject {
  Q_OBJECT

private slots:
  void layoutHashEmptySeedIsPinned();
  void layoutHashDeterministicForRepeatedCalls();
  void layoutHashOrderSensitive();
  void layoutHashMembershipSensitive();

  void fixtureSmallReconstructsManifestHash();

  void roundSignificantOutOfRangeDigitsReturnInputUnchanged();
  void roundSignificantSpecialValuesReturnInputUnchanged();
  void roundSignificantRoundsToRequestedDigits();

  void encodeLineWrapsPayloadInMirrorEnvelope();
  void encodeLineEndsWithSingleNewline();

  void encodeStructureLayoutHashMatchesStandalone();
  void encodeStructureFieldsAndPassthroughValues();

  void encodeSnapshotPositionalValuesWithSideMaps();
  void encodeSnapshotAllNumericOmitsSideMaps();

  void encodeHeartbeatShape();

  void chunkStructureSinglePartForSmallPayload();
  void chunkStructureMultiPartReassemblesOriginal();
  void chunkStructureEmptyVectorWhenPartsExceedCap();

  void encodedLinesAreValidJsonPerLine();
};

//--------------------------------------------------------------------------------------------------
// Layout hash
//--------------------------------------------------------------------------------------------------

/**
 * @brief The empty-list hash is the FNV-1a-64 seed "mirror-v1\n" alone, as 16 lowercase hex
 *        digits; this is the deterministic floor every other hash in this suite builds on.
 */
void TstMirrorProtocol::layoutHashEmptySeedIsPinned()
{
  const QString hash = API::Mirror::layoutHash({});

  QCOMPARE(hash.size(), 16);
  QCOMPARE(hash, hash.toLower());
  QCOMPARE(hash, QStringLiteral("85982a3b56d4965e"));
}

/**
 * @brief The same identity list hashes identically on every call: the layout hash is a pure
 *        function of its input, never seeded from wall-clock or process state.
 */
void TstMirrorProtocol::layoutHashDeterministicForRepeatedCalls()
{
  const auto datasets = smallFixtureDatasets();

  QCOMPARE(API::Mirror::layoutHash(datasets), API::Mirror::layoutHash(datasets));
}

/**
 * @brief Swapping two entries changes the hash, since the format is positional and the hash is
 *        the only thing that makes reusing a stale layout unsafe.
 */
void TstMirrorProtocol::layoutHashOrderSensitive()
{
  auto datasets       = smallFixtureDatasets();
  const auto original = API::Mirror::layoutHash(datasets);

  std::swap(datasets[0], datasets[1]);
  const auto swapped = API::Mirror::layoutHash(datasets);

  QVERIFY(original != swapped);
  QCOMPARE(swapped, QStringLiteral("86b2833ecaa03fef"));
}

/**
 * @brief Removing or adding one entry changes the hash from both directions of the same list.
 */
void TstMirrorProtocol::layoutHashMembershipSensitive()
{
  auto datasets       = smallFixtureDatasets();
  const auto original = API::Mirror::layoutHash(datasets);

  datasets.pop_back();
  const auto removed = API::Mirror::layoutHash(datasets);
  QVERIFY(removed != original);
  QCOMPARE(removed, QStringLiteral("e5305d77ccb34407"));

  datasets.push_back({0, 10002});
  datasets.push_back({0, 10003});
  const auto added = API::Mirror::layoutHash(datasets);
  QVERIFY(added != original);
  QCOMPARE(added, QStringLiteral("0f6d3353cd857f0e"));
}

//--------------------------------------------------------------------------------------------------
// Fixture cross-check
//--------------------------------------------------------------------------------------------------

/**
 * @brief The identity list reconstructed from tests/fixtures/mirror/small.ndjson reproduces the
 *        layoutHash recorded for the "small" entry in tests/fixtures/mirror/manifest.json.
 */
void TstMirrorProtocol::fixtureSmallReconstructsManifestHash()
{
  const QString manifestHash = QStringLiteral("3af81b130b65f883");

  QCOMPARE(API::Mirror::layoutHash(smallFixtureDatasets()), manifestHash);
}

//--------------------------------------------------------------------------------------------------
// roundSignificant
//--------------------------------------------------------------------------------------------------

/**
 * @brief digits outside [1, 16] returns the input value unchanged, on either side of the band.
 */
void TstMirrorProtocol::roundSignificantOutOfRangeDigitsReturnInputUnchanged()
{
  QCOMPARE(API::Mirror::roundSignificant(1.23456789, 0), 1.23456789);
  QCOMPARE(API::Mirror::roundSignificant(1.23456789, 17), 1.23456789);
  QCOMPARE(API::Mirror::roundSignificant(1.23456789, -5), 1.23456789);
}

/**
 * @brief Zero and every non-finite double pass through unchanged, regardless of digits: a
 *        significant-digit rounding has nothing meaningful to say about them.
 */
void TstMirrorProtocol::roundSignificantSpecialValuesReturnInputUnchanged()
{
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();

  QVERIFY(API::Mirror::roundSignificant(0.0, 3) == 0.0);
  QVERIFY(std::isnan(API::Mirror::roundSignificant(nan, 3)));
  QVERIFY(std::isinf(API::Mirror::roundSignificant(inf, 3)));
  QCOMPARE(API::Mirror::roundSignificant(-inf, 3), -inf);
}

/**
 * @brief In-range digits round through the 'g' formatter to the requested decimal, for both
 *        positive and negative values.
 */
void TstMirrorProtocol::roundSignificantRoundsToRequestedDigits()
{
  QCOMPARE(API::Mirror::roundSignificant(1.23456789, 3), 1.23);
  QCOMPARE(API::Mirror::roundSignificant(-1.23456789, 3), -1.23);
}

//--------------------------------------------------------------------------------------------------
// encodeLine
//--------------------------------------------------------------------------------------------------

/**
 * @brief encodeLine() wraps the payload once, under the top-level "mirror" key, with nothing else
 *        at that level.
 */
void TstMirrorProtocol::encodeLineWrapsPayloadInMirrorEnvelope()
{
  QJsonObject payload;
  payload.insert(QLatin1String("kind"), QLatin1String("heartbeat"));
  payload.insert(QLatin1String("epoch"), 1);

  const auto line = API::Mirror::encodeLine(payload);
  const auto doc  = QJsonDocument::fromJson(line);

  QVERIFY(doc.isObject());
  QCOMPARE(doc.object().keys().size(), 1);
  QVERIFY(doc.object().contains(QLatin1String("mirror")));
  QCOMPARE(doc.object().value(QLatin1String("mirror")).toObject(), payload);
}

/**
 * @brief The encoded line is single-line NDJSON: exactly one trailing newline, nowhere else.
 */
void TstMirrorProtocol::encodeLineEndsWithSingleNewline()
{
  QJsonObject payload;
  payload.insert(QLatin1String("kind"), QLatin1String("heartbeat"));

  const auto line = API::Mirror::encodeLine(payload);

  QVERIFY(line.endsWith('\n'));
  QCOMPARE(line.count('\n'), 1);
}

//--------------------------------------------------------------------------------------------------
// encodeStructure
//--------------------------------------------------------------------------------------------------

/**
 * @brief The layoutHash field embedded in a structure payload is exactly the standalone
 *        layoutHash() over the same identity list, never a separately derived value.
 */
void TstMirrorProtocol::encodeStructureLayoutHashMatchesStandalone()
{
  const auto datasets = smallFixtureDatasets();
  const auto payload  = sampleStructurePayload();

  QCOMPARE(payload.value(QLatin1String("layoutHash")).toString(),
           API::Mirror::layoutHash(datasets));
}

/**
 * @brief Every other structure field carries what was passed in: kind, wire version, epoch,
 *        the ordered dataset id pairs, source ids, display state and the clock/project blobs.
 */
void TstMirrorProtocol::encodeStructureFieldsAndPassthroughValues()
{
  const auto payload = sampleStructurePayload();

  QCOMPARE(payload.value(QLatin1String("kind")).toString(), QStringLiteral("structure"));
  QCOMPARE(payload.value(QLatin1String("wireVersion")).toInt(), API::Mirror::kWireVersion);
  QCOMPARE(payload.value(QLatin1String("epoch")).toInteger(), qint64(7));
  QCOMPARE(payload.value(QLatin1String("operationMode")).toInt(), 0);
  QCOMPARE(payload.value(QLatin1String("plotTimeRange")).toDouble(), 10.0);
  QCOMPARE(payload.value(QLatin1String("frozen")).toBool(), false);

  const auto sources = payload.value(QLatin1String("sourceIds")).toArray();
  QCOMPARE(sources.size(), 1);
  QCOMPARE(sources.at(0).toInt(), 0);

  const auto ids = payload.value(QLatin1String("datasets")).toArray();
  QCOMPARE(ids.size(), 6);
  QCOMPARE(ids.at(0).toArray().at(0).toInt(), 0);
  QCOMPARE(ids.at(0).toArray().at(1).toInt(), 0);
  QCOMPARE(ids.at(3).toArray().at(0).toInt(), 0);
  QCOMPARE(ids.at(3).toArray().at(1).toInt(), 10000);

  const auto clock = payload.value(QLatin1String("clock")).toObject();
  QCOMPARE(clock.value(QLatin1String("domain")).toString(), QStringLiteral("monotonic-relative"));
  QCOMPARE(clock.value(QLatin1String("originUnixMs")).toInteger(), qint64(1753459200000));

  const auto project = payload.value(QLatin1String("project")).toObject();
  QCOMPARE(project.value(QLatin1String("title")).toString(), QStringLiteral("Lorenz Attractor"));
}

//--------------------------------------------------------------------------------------------------
// encodeSnapshot
//--------------------------------------------------------------------------------------------------

/**
 * @brief Number slots carry a rounded value at their position; Text and NonFinite slots hold a
 *        null in the positional array and their real value in the matching sparse side map.
 */
void TstMirrorProtocol::encodeSnapshotPositionalValuesWithSideMaps()
{
  using API::Mirror::SnapshotValue;

  std::vector<SnapshotValue> values(4);
  values[0].kind   = SnapshotValue::Kind::Number;
  values[0].number = 3.14159;
  values[1].kind   = SnapshotValue::Kind::Text;
  values[1].text   = QStringLiteral("OK");
  values[2].kind   = SnapshotValue::Kind::NonFinite;
  values[2].text   = QLatin1String(API::Mirror::NonFinite::NaN);
  values[3].kind   = SnapshotValue::Kind::Empty;

  const std::vector<qint64> tNs{111111, 222222};
  const auto payload = API::Mirror::encodeSnapshot(9, 5, tNs, values, 3);

  QCOMPARE(payload.value(QLatin1String("kind")).toString(), QStringLiteral("snapshot"));
  QCOMPARE(payload.value(QLatin1String("epoch")).toInteger(), qint64(9));
  QCOMPARE(payload.value(QLatin1String("seq")).toInteger(), qint64(5));

  const auto times = payload.value(QLatin1String("tNs")).toArray();
  QCOMPARE(times.size(), 2);
  QCOMPARE(times.at(0).toInteger(), qint64(111111));
  QCOMPARE(times.at(1).toInteger(), qint64(222222));

  const auto valueSlots = payload.value(QLatin1String("values")).toArray();
  QCOMPARE(valueSlots.size(), 4);
  QCOMPARE(valueSlots.at(0).toDouble(), 3.14);
  QVERIFY(valueSlots.at(1).isNull());
  QVERIFY(valueSlots.at(2).isNull());
  QVERIFY(valueSlots.at(3).isNull());

  const auto strings = payload.value(QLatin1String("strings")).toObject();
  QCOMPARE(strings.size(), 1);
  QCOMPARE(strings.value(QLatin1String("1")).toString(), QStringLiteral("OK"));

  const auto nonFinite = payload.value(QLatin1String("nonFinite")).toObject();
  QCOMPARE(nonFinite.size(), 1);
  QCOMPARE(nonFinite.value(QLatin1String("2")).toString(), QStringLiteral("nan"));
}

/**
 * @brief A snapshot with only Number slots carries neither the "strings" nor the "nonFinite" key
 *        at all, not an empty object: the two side maps are opt-in per snapshot.
 */
void TstMirrorProtocol::encodeSnapshotAllNumericOmitsSideMaps()
{
  using API::Mirror::SnapshotValue;

  std::vector<SnapshotValue> values(2);
  values[0].kind   = SnapshotValue::Kind::Number;
  values[0].number = 1.0;
  values[1].kind   = SnapshotValue::Kind::Number;
  values[1].number = 2.0;

  const std::vector<qint64> tNs{1000};
  const auto payload = API::Mirror::encodeSnapshot(1, 1, tNs, values, 6);

  QVERIFY(!payload.contains(QLatin1String("strings")));
  QVERIFY(!payload.contains(QLatin1String("nonFinite")));
}

//--------------------------------------------------------------------------------------------------
// encodeHeartbeat
//--------------------------------------------------------------------------------------------------

/**
 * @brief A heartbeat carries exactly kind, epoch and the repeated last sequence, nothing else.
 */
void TstMirrorProtocol::encodeHeartbeatShape()
{
  const auto payload = API::Mirror::encodeHeartbeat(3, 42);

  QCOMPARE(payload.size(), 3);
  QCOMPARE(payload.value(QLatin1String("kind")).toString(), QStringLiteral("heartbeat"));
  QCOMPARE(payload.value(QLatin1String("epoch")).toInteger(), qint64(3));
  QCOMPARE(payload.value(QLatin1String("seq")).toInteger(), qint64(42));
}

//--------------------------------------------------------------------------------------------------
// chunkStructure
//--------------------------------------------------------------------------------------------------

/**
 * @brief A structure well under chunkBytes yields exactly one chunk, and that chunk's data
 *        decodes back to the structure's exact compact-JSON bytes.
 */
void TstMirrorProtocol::chunkStructureSinglePartForSmallPayload()
{
  const auto structure = sampleStructurePayload();
  const auto chunks    = API::Mirror::chunkStructure(structure);

  QCOMPARE(chunks.size(), std::size_t(1));
  QCOMPARE(chunks[0].value(QLatin1String("kind")).toString(), QStringLiteral("structureChunk"));
  QCOMPARE(chunks[0].value(QLatin1String("epoch")).toInteger(), qint64(7));
  QCOMPARE(chunks[0].value(QLatin1String("part")).toInt(), 0);
  QCOMPARE(chunks[0].value(QLatin1String("parts")).toInt(), 1);

  const auto decoded =
    QByteArray::fromBase64(chunks[0].value(QLatin1String("data")).toString().toLatin1());
  QCOMPARE(decoded, QJsonDocument(structure).toJson(QJsonDocument::Compact));
}

/**
 * @brief A chunkBytes small enough to force multiple parts still reassembles, in part order,
 *        into the exact original base64 blob and the exact original compact JSON.
 */
void TstMirrorProtocol::chunkStructureMultiPartReassemblesOriginal()
{
  const auto structure    = sampleStructurePayload();
  const auto blob         = QJsonDocument(structure).toJson(QJsonDocument::Compact).toBase64();
  const int chunkBytes    = qMax(1, static_cast<int>(blob.size()) / 5);
  const int expectedParts = static_cast<int>((blob.size() + chunkBytes - 1) / chunkBytes);

  const auto chunks = API::Mirror::chunkStructure(structure, chunkBytes);

  QCOMPARE(chunks.size(), std::size_t(qMax(1, expectedParts)));
  QVERIFY(chunks.size() > std::size_t(1));

  QByteArray reassembled;
  for (std::size_t i = 0; i < chunks.size(); ++i) {
    QCOMPARE(chunks[i].value(QLatin1String("part")).toInt(), static_cast<int>(i));
    QCOMPARE(chunks[i].value(QLatin1String("parts")).toInt(), static_cast<int>(chunks.size()));
    reassembled += chunks[i].value(QLatin1String("data")).toString().toLatin1();
  }

  QCOMPARE(reassembled, blob);
  QCOMPARE(QByteArray::fromBase64(reassembled),
           QJsonDocument(structure).toJson(QJsonDocument::Compact));
}

/**
 * @brief A chunkBytes of 1 forces more parts than kMaxStructureParts allows, so chunkStructure()
 *        refuses the split rather than returning a partial or over-cap vector.
 */
void TstMirrorProtocol::chunkStructureEmptyVectorWhenPartsExceedCap()
{
  const auto structure = sampleStructurePayload();
  const auto chunks    = API::Mirror::chunkStructure(structure, 1);

  QVERIFY(chunks.empty());
}

//--------------------------------------------------------------------------------------------------
// NDJSON validity
//--------------------------------------------------------------------------------------------------

/**
 * @brief Both a structure and a snapshot payload, once wrapped by encodeLine(), parse as one
 *        valid JSON object per line with the original payload intact under "mirror".
 */
void TstMirrorProtocol::encodedLinesAreValidJsonPerLine()
{
  const auto structure = sampleStructurePayload();
  const auto snapshot  = API::Mirror::encodeSnapshot(7, 1, {1000}, {}, 3);

  for (const auto& payload : {structure, snapshot}) {
    const auto line = API::Mirror::encodeLine(payload);

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(line, &error);

    QCOMPARE(error.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());
    QVERIFY(doc.object().contains(QLatin1String("mirror")));
    QCOMPARE(doc.object().value(QLatin1String("mirror")).toObject(), payload);
  }
}

QTEST_APPLESS_MAIN(TstMirrorProtocol)

#include "tst_mirror_protocol.moc"
