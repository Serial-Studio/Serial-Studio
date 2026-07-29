/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
#include <optional>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QTest>
#include <vector>

#include "DataModel/Frame.h"

// Companion to tst_frame_serialization: that suite pins the happy-path round trip of every entity,
// this one pins the legacy, clamping and repair branches a saved project can still reach -- the TX
// encoder, the hex/escape delimiter reader, the table folder walk, the commercial-feature flag and
// the entity readers' rejection paths. No case is duplicated between the two files.
//
// Every field name comes from Keys:: rather than a string literal, so renaming a key breaks the
// compile instead of silently changing the on-disk project format. Fixtures are built in code --
// no data files, so the suite has no working-directory dependence.

using namespace DataModel;

// Text-encoding ids mirrored from SerialStudio::TextEncoding. The suite deliberately does not
// include SerialStudio.h: Action::txEncoding is a plain int on the wire, and the header carries a
// QObject singleton this link set has no business pulling in.
static constexpr int kEncUtf8   = 0;
static constexpr int kEncLatin1 = 3;

/**
 * @brief Legacy, clamping and repair branches of Frame.cpp and the inline Frame.h readers.
 */
class TstFrameJsonLegacy : public QObject {
  Q_OBJECT

private slots:
  void txBytesResolveEscapesAndAppendTheEol();
  void txBytesDecodeAHexPayloadWhenBinary();
  void txBytesHonourTheConfiguredTextEncoding();
  void txBytesSendABareEolAfterAnUnparsableHexPayload();

  void ioSettingsResolveEscapesOnTheTextBranch();
  void ioSettingsDecodeHexDelimiters();
  void ioSettingsDropAnEscapeSequenceOnTheHexBranch();
  void ioSettingsPreferTheCanonicalChecksumKey();

  void tableFolderPathWalksTheParentChain();
  void tableFolderPathShortCircuitsAtTheTopLevel();
  void tableFolderPathStopsAtAnUnknownParent();
  void tableFolderPathTerminatesOnACycle();

  void commercialFlagStaysOffForAGplProject();
  void commercialFlagFollowsAnOutputGroup();
  void commercialFlagFollowsAProWidget();
  void commercialFlagFollowsAWaterfallDataset();
  void commercialFlagFollowsANotifyingTransform();

  void legacyAlarmFieldsSynthesiseBands();
  void legacyAlarmFieldsAcceptTheBareAlarmKey();
  void legacyAlarmFieldsAreIgnoredWhenDisabled();
  void legacyAlarmFieldsOutsideTheWidgetRangeAreDropped();
  void canonicalAlarmBandsWinOverTheLegacyFields();
  void alarmBandClampsAnOutOfRangeSeverity();

  void frequencyMarkerClampsTheBandCeiling();
  void frequencyMarkerRejectsAnAbsurdFrequency();
  void frequencyMarkerSwapsReversedLevels();
  void frequencyMarkerClearsAnUnparsableColour();

  void imageGroupIgnoresItsDatasetArray();
  void groupSkipsAnEmptyDatasetObject();
  void groupSkipsAnUnreadableOutputWidget();
  void groupClampsItsColumnCount();
  void groupClampsAnOutOfRangeGroupType();

  void frameAbortsOnAMalformedAction();
  void frameSkipsAMalformedSource();

  void datasetClearsAnUnparsableColour();
  void tableDefSkipsANamelessRegister();
};

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief A readable group of the requested widget type, carrying one plain dataset.
 */
static Group widgetGroup(const QString& widget)
{
  Dataset d;
  d.title = QStringLiteral("Voltage");
  d.value = QStringLiteral("3.3");

  Group g;
  g.title  = QStringLiteral("Sensors");
  g.widget = widget;
  g.datasets.push_back(d);
  return g;
}

/**
 * @brief Round-trips a single-group frame through serialize()/read() so finalize_frame() runs.
 */
static std::optional<Frame> roundTripFrameWith(const Group& g)
{
  Frame f;
  f.title = QStringLiteral("Telemetry");
  f.groups.push_back(g);
  return fromJson<Frame>(toJson(f));
}

/**
 * @brief A dataset JSON object with an explicit widget range, ready for legacy alarm keys.
 */
static QJsonObject rangedDatasetJson()
{
  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Temperature"));
  json.insert(Keys::WgtMin, 0);
  json.insert(Keys::WgtMax, 100);
  return json;
}

//--------------------------------------------------------------------------------------------------
// get_tx_bytes()
//--------------------------------------------------------------------------------------------------

void TstFrameJsonLegacy::txBytesResolveEscapesAndAppendTheEol()
{
  Action a;
  a.txData      = QStringLiteral("A\\tB");
  a.eolSequence = QStringLiteral("\\r\\n");

  QCOMPARE(get_tx_bytes(a), QByteArray("A\tB\r\n"));

  a.eolSequence.clear();
  QCOMPARE(get_tx_bytes(a), QByteArray("A\tB"));
}

/**
 * @brief A binary action reads txData as hex digits; the EOL is still resolved and appended.
 */
void TstFrameJsonLegacy::txBytesDecodeAHexPayloadWhenBinary()
{
  Action a;
  a.binaryData  = true;
  a.txData      = QStringLiteral("AA 55 0F");
  a.eolSequence = QStringLiteral("\\n");

  QCOMPARE(get_tx_bytes(a), QByteArray::fromHex("aa550f0a"));
}

/**
 * @brief The text branch encodes both payload and EOL with the action's encoding; the binary branch
 *        always writes the EOL as UTF-8, so a non-UTF-8 EOL widens there. Pinned as current
 *        behaviour, not endorsed.
 */
void TstFrameJsonLegacy::txBytesHonourTheConfiguredTextEncoding()
{
  const QString degree = QString(QChar(0x00B0));

  Action text;
  text.txData      = QStringLiteral("A") + degree;
  text.eolSequence = QStringLiteral("\\n");
  text.txEncoding  = kEncUtf8;
  QCOMPARE(get_tx_bytes(text), QByteArray::fromHex("41c2b00a"));

  text.txEncoding = kEncLatin1;
  QCOMPARE(get_tx_bytes(text), QByteArray::fromHex("41b00a"));

  Action binary;
  binary.binaryData  = true;
  binary.txData      = QStringLiteral("41");
  binary.eolSequence = degree;
  binary.txEncoding  = kEncLatin1;
  QCOMPARE(get_tx_bytes(binary), QByteArray::fromHex("41c2b0"));
}

/**
 * @brief hexToBytes() drops an odd-length payload wholesale, and get_tx_bytes() still appends the
 *        EOL: the device receives a terminator with no command in front of it.
 */
void TstFrameJsonLegacy::txBytesSendABareEolAfterAnUnparsableHexPayload()
{
  Action a;
  a.binaryData  = true;
  a.txData      = QStringLiteral("ABC");
  a.eolSequence = QStringLiteral("\\n");

  QTest::ignoreMessage(QtWarningMsg, "\"ABC\" is not a valid hexadecimal array");
  QCOMPARE(get_tx_bytes(a), QByteArray("\n"));

  a.txData = QStringLiteral("ZZ");
  QTest::ignoreMessage(QtWarningMsg, "\"ZZ\" is not a valid hexadecimal array");
  QCOMPARE(get_tx_bytes(a), QByteArray("\n"));
}

//--------------------------------------------------------------------------------------------------
// read_io_settings()
//--------------------------------------------------------------------------------------------------

void TstFrameJsonLegacy::ioSettingsResolveEscapesOnTheTextBranch()
{
  QJsonObject json;
  json.insert(Keys::FrameStart, QStringLiteral("/*"));
  json.insert(Keys::FrameEnd, QStringLiteral("\\r\\n"));

  QByteArray start;
  QByteArray end;
  QString checksum;
  read_io_settings(start, end, checksum, json);

  QCOMPARE(start, QByteArray("/*"));
  QCOMPARE(end, QByteArray("\r\n"));
  QVERIFY(checksum.isEmpty());

  QByteArray emptyStart;
  QByteArray emptyEnd;
  read_io_settings(emptyStart, emptyEnd, checksum, QJsonObject());
  QVERIFY(emptyStart.isEmpty());
  QVERIFY(emptyEnd.isEmpty());
}

void TstFrameJsonLegacy::ioSettingsDecodeHexDelimiters()
{
  QJsonObject json;
  json.insert(Keys::FrameStart, QStringLiteral("AA55"));
  json.insert(Keys::FrameEnd, QStringLiteral("0D 0A"));
  json.insert(Keys::HexadecimalDelimiters, true);

  QByteArray start;
  QByteArray end;
  QString checksum;
  read_io_settings(start, end, checksum, json);

  QCOMPARE(start, QByteArray::fromHex("aa55"));
  QCOMPARE(end, QByteArray("\r\n"));
}

/**
 * @brief Escape resolution runs before the hex decode, so a hex-delimiter project that stores
 *        "\n" loses its end delimiter entirely: fromHex() skips the resolved control character.
 */
void TstFrameJsonLegacy::ioSettingsDropAnEscapeSequenceOnTheHexBranch()
{
  QJsonObject json;
  json.insert(Keys::FrameStart, QStringLiteral("41"));
  json.insert(Keys::FrameEnd, QStringLiteral("\\n"));
  json.insert(Keys::HexadecimalDelimiters, true);

  QByteArray start;
  QByteArray end;
  QString checksum;
  read_io_settings(start, end, checksum, json);

  QCOMPARE(start, QByteArray("A"));
  QVERIFY(end.isEmpty());
}

/**
 * @brief "checksumAlgorithm" wins whenever it is present, even when it holds an empty string.
 */
void TstFrameJsonLegacy::ioSettingsPreferTheCanonicalChecksumKey()
{
  QByteArray start;
  QByteArray end;
  QString checksum;

  QJsonObject legacy;
  legacy.insert(Keys::Checksum, QStringLiteral("CRC-8"));
  read_io_settings(start, end, checksum, legacy);
  QCOMPARE(checksum, QStringLiteral("CRC-8"));

  QJsonObject both = legacy;
  both.insert(Keys::ChecksumAlgorithm, QStringLiteral("CRC-32"));
  read_io_settings(start, end, checksum, both);
  QCOMPARE(checksum, QStringLiteral("CRC-32"));

  QJsonObject blanked = legacy;
  blanked.insert(Keys::ChecksumAlgorithm, QString());
  read_io_settings(start, end, checksum, blanked);
  QVERIFY(checksum.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// tableFolderPath() / tableFullPath()
//--------------------------------------------------------------------------------------------------

void TstFrameJsonLegacy::tableFolderPathWalksTheParentChain()
{
  TableFolder root;
  root.folderId = 1;
  root.title    = QStringLiteral("Calibration");

  TableFolder leaf;
  leaf.folderId       = 2;
  leaf.parentFolderId = 1;
  leaf.title          = QStringLiteral("Sensors");

  const std::vector<TableFolder> folders{root, leaf};

  QCOMPARE(tableFolderPath(folders, 1), QStringLiteral("Calibration"));
  QCOMPARE(tableFolderPath(folders, 2), QStringLiteral("Calibration/Sensors"));
  QCOMPARE(tableFullPath(folders, 2, QStringLiteral("Gain")),
           QStringLiteral("Calibration/Sensors/Gain"));
}

void TstFrameJsonLegacy::tableFolderPathShortCircuitsAtTheTopLevel()
{
  TableFolder root;
  root.folderId = 1;
  root.title    = QStringLiteral("Calibration");

  const std::vector<TableFolder> folders{root};

  QVERIFY(tableFolderPath(folders, -1).isEmpty());
  QCOMPARE(tableFullPath(folders, -1, QStringLiteral("Gain")), QStringLiteral("Gain"));
  QVERIFY(tableFolderPath({}, -1).isEmpty());
}

void TstFrameJsonLegacy::tableFolderPathStopsAtAnUnknownParent()
{
  TableFolder orphan;
  orphan.folderId       = 2;
  orphan.parentFolderId = 99;
  orphan.title          = QStringLiteral("Sensors");

  const std::vector<TableFolder> folders{orphan};

  QVERIFY(tableFolderPath(folders, 99).isEmpty());
  QCOMPARE(tableFolderPath(folders, 2), QStringLiteral("Sensors"));
  QCOMPARE(tableFullPath(folders, 99, QStringLiteral("Gain")), QStringLiteral("Gain"));
}

/**
 * @brief The walk is bounded by the folder count, so a corrupted file whose folders point at each
 *        other terminates with a repeated-but-finite path instead of hanging the loader.
 */
void TstFrameJsonLegacy::tableFolderPathTerminatesOnACycle()
{
  TableFolder a;
  a.folderId       = 1;
  a.parentFolderId = 2;
  a.title          = QStringLiteral("A");

  TableFolder b;
  b.folderId       = 2;
  b.parentFolderId = 1;
  b.title          = QStringLiteral("B");

  const std::vector<TableFolder> folders{a, b};
  QCOMPARE(tableFolderPath(folders, 1), QStringLiteral("A/B/A"));
}

//--------------------------------------------------------------------------------------------------
// finalize_frame(): containsCommercialFeatures
//--------------------------------------------------------------------------------------------------

void TstFrameJsonLegacy::commercialFlagStaysOffForAGplProject()
{
  const auto restored = roundTripFrameWith(widgetGroup(QStringLiteral("datagrid")));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->containsCommercialFeatures, false);
}

void TstFrameJsonLegacy::commercialFlagFollowsAnOutputGroup()
{
  OutputWidget button;
  button.title = QStringLiteral("Arm");

  Group g;
  g.title     = QStringLiteral("Controls");
  g.groupType = GroupType::Output;
  g.outputWidgets.push_back(button);

  const auto restored = roundTripFrameWith(g);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->groups[0].groupType, GroupType::Output);
  QCOMPARE(restored->containsCommercialFeatures, true);
}

void TstFrameJsonLegacy::commercialFlagFollowsAProWidget()
{
  const auto plot3d = roundTripFrameWith(widgetGroup(QStringLiteral("plot3d")));
  QVERIFY(plot3d.has_value());
  QCOMPARE(plot3d->containsCommercialFeatures, true);

  Group image;
  image.title  = QStringLiteral("Camera");
  image.widget = QStringLiteral("image");

  const auto restoredImage = roundTripFrameWith(image);
  QVERIFY(restoredImage.has_value());
  QCOMPARE(restoredImage->containsCommercialFeatures, true);
}

void TstFrameJsonLegacy::commercialFlagFollowsAWaterfallDataset()
{
  Group g                 = widgetGroup(QStringLiteral("datagrid"));
  g.datasets[0].fft       = true;
  g.datasets[0].waterfall = true;

  const auto restored = roundTripFrameWith(g);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->groups[0].datasets[0].waterfall, true);
  QCOMPARE(restored->containsCommercialFeatures, true);
}

/**
 * @brief A transform that only computes stays GPL; one that calls into the notify() family does
 *        not, and the detection survives the round trip through transformCode.
 */
void TstFrameJsonLegacy::commercialFlagFollowsANotifyingTransform()
{
  Group plain                     = widgetGroup(QStringLiteral("datagrid"));
  plain.datasets[0].transformCode = QStringLiteral("return x * 2");

  const auto restoredPlain = roundTripFrameWith(plain);
  QVERIFY(restoredPlain.has_value());
  QCOMPARE(restoredPlain->containsCommercialFeatures, false);

  Group notifying                     = widgetGroup(QStringLiteral("datagrid"));
  notifying.datasets[0].transformCode = QStringLiteral("notifyWarning('hot'); return x");

  const auto restoredNotifying = roundTripFrameWith(notifying);
  QVERIFY(restoredNotifying.has_value());
  QCOMPARE(restoredNotifying->containsCommercialFeatures, true);
}

//--------------------------------------------------------------------------------------------------
// readDatasetAlarmBands(): v3.3 legacy fields
//--------------------------------------------------------------------------------------------------

/**
 * @brief A v3.3 project stores one low and one high threshold; the reader synthesises the two
 *        Warning bands that reach from the widget range to each threshold.
 */
void TstFrameJsonLegacy::legacyAlarmFieldsSynthesiseBands()
{
  auto json = rangedDatasetJson();
  json.insert(Keys::AlarmEnabled, true);
  json.insert(Keys::AlarmLow, 20);
  json.insert(Keys::AlarmHigh, 80);

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->alarmBands.size(), size_t(2));
  QCOMPARE(restored->alarmBands[0].min, 0.0);
  QCOMPARE(restored->alarmBands[0].max, 20.0);
  QCOMPARE(restored->alarmBands[0].severity, AlarmSeverity::Warning);
  QCOMPARE(restored->alarmBands[1].min, 80.0);
  QCOMPARE(restored->alarmBands[1].max, 100.0);
  QCOMPARE(restored->alarmBands[1].severity, AlarmSeverity::Warning);
}

/**
 * @brief Older still: a single "alarm" key stands in for the missing high threshold.
 */
void TstFrameJsonLegacy::legacyAlarmFieldsAcceptTheBareAlarmKey()
{
  auto json = rangedDatasetJson();
  json.insert(Keys::AlarmEnabled, true);
  json.insert(Keys::Alarm, 70);

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->alarmBands.size(), size_t(1));
  QCOMPARE(restored->alarmBands[0].min, 70.0);
  QCOMPARE(restored->alarmBands[0].max, 100.0);

  auto explicitHigh = json;
  explicitHigh.insert(Keys::AlarmHigh, 90);

  const auto preferred = fromJson<Dataset>(explicitHigh);
  QVERIFY(preferred.has_value());
  QCOMPARE(preferred->alarmBands.size(), size_t(1));
  QCOMPARE(preferred->alarmBands[0].min, 90.0);
}

void TstFrameJsonLegacy::legacyAlarmFieldsAreIgnoredWhenDisabled()
{
  auto json = rangedDatasetJson();
  json.insert(Keys::AlarmLow, 20);
  json.insert(Keys::AlarmHigh, 80);

  const auto absent = fromJson<Dataset>(json);
  QVERIFY(absent.has_value());
  QVERIFY(absent->alarmBands.empty());

  json.insert(Keys::AlarmEnabled, false);

  const auto disabled = fromJson<Dataset>(json);
  QVERIFY(disabled.has_value());
  QVERIFY(disabled->alarmBands.empty());
}

/**
 * @brief A threshold outside the widget range would describe a band that can never be entered, so
 *        it is dropped rather than clamped.
 */
void TstFrameJsonLegacy::legacyAlarmFieldsOutsideTheWidgetRangeAreDropped()
{
  auto json = rangedDatasetJson();
  json.insert(Keys::AlarmEnabled, true);
  json.insert(Keys::AlarmLow, -5);
  json.insert(Keys::AlarmHigh, 150);

  const auto outside = fromJson<Dataset>(json);
  QVERIFY(outside.has_value());
  QVERIFY(outside->alarmBands.empty());

  auto onTheEdge = rangedDatasetJson();
  onTheEdge.insert(Keys::AlarmEnabled, true);
  onTheEdge.insert(Keys::AlarmLow, 0);
  onTheEdge.insert(Keys::AlarmHigh, 100);

  const auto edges = fromJson<Dataset>(onTheEdge);
  QVERIFY(edges.has_value());
  QVERIFY(edges->alarmBands.empty());
}

void TstFrameJsonLegacy::canonicalAlarmBandsWinOverTheLegacyFields()
{
  AlarmBand band;
  band.min      = 40;
  band.max      = 60;
  band.severity = AlarmSeverity::Critical;

  QJsonArray bands;
  bands.append(toJson(band));

  auto json = rangedDatasetJson();
  json.insert(Keys::AlarmBands, bands);
  json.insert(Keys::AlarmEnabled, true);
  json.insert(Keys::AlarmLow, 20);
  json.insert(Keys::AlarmHigh, 80);

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->alarmBands.size(), size_t(1));
  QCOMPARE(restored->alarmBands[0].min, 40.0);
  QCOMPARE(restored->alarmBands[0].severity, AlarmSeverity::Critical);

  QJsonObject unreadable;
  unreadable.insert(Keys::Min, 10);
  unreadable.insert(Keys::Max, 10);

  QJsonArray mixed;
  mixed.append(unreadable);
  mixed.append(toJson(band));

  auto mixedJson = rangedDatasetJson();
  mixedJson.insert(Keys::AlarmBands, mixed);

  const auto filtered = fromJson<Dataset>(mixedJson);
  QVERIFY(filtered.has_value());
  QCOMPARE(filtered->alarmBands.size(), size_t(1));
  QCOMPARE(filtered->alarmBands[0].max, 60.0);
}

void TstFrameJsonLegacy::alarmBandClampsAnOutOfRangeSeverity()
{
  QJsonObject json;
  json.insert(Keys::Min, 0);
  json.insert(Keys::Max, 10);
  json.insert(Keys::Severity, 99);

  const auto above = fromJson<AlarmBand>(json);
  QVERIFY(above.has_value());
  QCOMPARE(above->severity, AlarmSeverity::Critical);

  json.insert(Keys::Severity, -5);

  const auto below = fromJson<AlarmBand>(json);
  QVERIFY(below.has_value());
  QCOMPARE(below->severity, AlarmSeverity::Info);
}

//--------------------------------------------------------------------------------------------------
// read(FrequencyMarker)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The band end is capped at the largest Nyquist an int sampling rate can produce.
 */
void TstFrameJsonLegacy::frequencyMarkerClampsTheBandCeiling()
{
  QJsonObject json;
  json.insert(Keys::Frequency, 1000);
  json.insert(Keys::EndFrequency, 1.0e12);

  const auto restored = fromJson<FrequencyMarker>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->frequency, 1000.0);
  QCOMPARE(restored->endFrequency, 2147483648.0);
}

void TstFrameJsonLegacy::frequencyMarkerRejectsAnAbsurdFrequency()
{
  QJsonObject beyondCeiling;
  beyondCeiling.insert(Keys::Frequency, 1.0e12);
  QVERIFY(!fromJson<FrequencyMarker>(beyondCeiling).has_value());

  QJsonObject atCeiling;
  atCeiling.insert(Keys::Frequency, 2147483648.0);
  QVERIFY(fromJson<FrequencyMarker>(atCeiling).has_value());
}

void TstFrameJsonLegacy::frequencyMarkerSwapsReversedLevels()
{
  QJsonObject json;
  json.insert(Keys::Frequency, 120);
  json.insert(Keys::WarningDb, -5);
  json.insert(Keys::AlarmDb, -40);

  const auto restored = fromJson<FrequencyMarker>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->warningDb, -40.0);
  QCOMPARE(restored->alarmDb, -5.0);

  QJsonObject onlyWarning;
  onlyWarning.insert(Keys::Frequency, 120);
  onlyWarning.insert(Keys::WarningDb, -5);

  const auto untouched = fromJson<FrequencyMarker>(onlyWarning);
  QVERIFY(untouched.has_value());
  QCOMPARE(untouched->warningDb, -5.0);
  QVERIFY(std::isnan(untouched->alarmDb));
}

void TstFrameJsonLegacy::frequencyMarkerClearsAnUnparsableColour()
{
  QJsonObject json;
  json.insert(Keys::Frequency, 120);
  json.insert(Keys::Color, QStringLiteral("#gg0011"));

  const auto invalid = fromJson<FrequencyMarker>(json);
  QVERIFY(invalid.has_value());
  QVERIFY(invalid->color.isEmpty());

  json.insert(Keys::Color, QStringLiteral("#00ff00"));

  const auto valid = fromJson<FrequencyMarker>(json);
  QVERIFY(valid.has_value());
  QCOMPARE(valid->color, QStringLiteral("#00ff00"));
}

//--------------------------------------------------------------------------------------------------
// read(Group)
//--------------------------------------------------------------------------------------------------

/**
 * @brief An image group returns as soon as its detection fields are read, so datasets left in the
 *        file by an earlier widget type are silently dropped instead of loaded.
 */
void TstFrameJsonLegacy::imageGroupIgnoresItsDatasetArray()
{
  auto json = toJson(widgetGroup(QStringLiteral("datagrid")));
  json.insert(Keys::Widget, QStringLiteral("image"));

  const auto restored = fromJson<Group>(json);
  QVERIFY(restored.has_value());
  QVERIFY(restored->datasets.empty());
  QCOMPARE(restored->imgDetectionMode, QStringLiteral("autodetect"));
  QVERIFY(restored->imgStartSequence.isEmpty());
  QVERIFY(restored->imgEndSequence.isEmpty());
}

/**
 * @brief An empty dataset object is skipped, but the surviving datasets keep their array index as
 *        the datasetId, so the numbering has a hole where the skipped entry was.
 */
void TstFrameJsonLegacy::groupSkipsAnEmptyDatasetObject()
{
  Dataset d;
  d.title = QStringLiteral("Voltage");

  QJsonArray datasets;
  datasets.append(QJsonObject());
  datasets.append(toJson(d));

  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Sensors"));
  json.insert(Keys::Widget, QStringLiteral("datagrid"));
  json.insert(Keys::Datasets, datasets);

  const auto restored = fromJson<Group>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->datasets.size(), size_t(1));
  QCOMPARE(restored->datasets[0].title, QStringLiteral("Voltage"));
  QCOMPARE(restored->datasets[0].datasetId, 1);
}

/**
 * @brief An output widget the reader rejects is skipped without failing the group, and the kept
 *        widgets keep their array index as the widgetId.
 */
void TstFrameJsonLegacy::groupSkipsAnUnreadableOutputWidget()
{
  QJsonObject untitled;
  untitled.insert(Keys::Icon, QStringLiteral("Send Property"));

  OutputWidget good;
  good.title = QStringLiteral("Arm");

  QJsonArray widgets;
  widgets.append(untitled);
  widgets.append(toJson(good));

  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Controls"));
  json.insert(Keys::GroupType, static_cast<int>(GroupType::Output));
  json.insert(Keys::OutputWidgets, widgets);

  const auto restored = fromJson<Group>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->outputWidgets.size(), size_t(1));
  QCOMPARE(restored->outputWidgets[0].title, QStringLiteral("Arm"));
  QCOMPARE(restored->outputWidgets[0].widgetId, 1);
}

void TstFrameJsonLegacy::groupClampsItsColumnCount()
{
  auto json = toJson(widgetGroup(QStringLiteral("datagrid")));

  json.insert(Keys::OutputColumns, 50);
  const auto tooMany = fromJson<Group>(json);
  QVERIFY(tooMany.has_value());
  QCOMPARE(tooMany->columns, 10);

  json.insert(Keys::OutputColumns, 0);
  const auto tooFew = fromJson<Group>(json);
  QVERIFY(tooFew.has_value());
  QCOMPARE(tooFew->columns, 1);
}

void TstFrameJsonLegacy::groupClampsAnOutOfRangeGroupType()
{
  auto json = toJson(widgetGroup(QStringLiteral("datagrid")));

  json.insert(Keys::GroupType, 7);
  const auto tooHigh = fromJson<Group>(json);
  QVERIFY(tooHigh.has_value());
  QCOMPARE(tooHigh->groupType, GroupType::Output);

  json.insert(Keys::GroupType, -3);
  const auto tooLow = fromJson<Group>(json);
  QVERIFY(tooLow.has_value());
  QCOMPARE(tooLow->groupType, GroupType::Input);
}

//--------------------------------------------------------------------------------------------------
// read(Frame)
//--------------------------------------------------------------------------------------------------

/**
 * @brief One unreadable action aborts the whole frame, leaving the already-read groups behind and
 *        the action list empty. Pinned so a future tightening is a deliberate change.
 */
void TstFrameJsonLegacy::frameAbortsOnAMalformedAction()
{
  QJsonArray groups;
  groups.append(toJson(widgetGroup(QStringLiteral("datagrid"))));

  Action good;
  good.title = QStringLiteral("Reboot");

  QJsonArray actions;
  actions.append(QJsonObject());
  actions.append(toJson(good));

  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Telemetry"));
  json.insert(Keys::Groups, groups);
  json.insert(Keys::Actions, actions);

  Frame f;
  QVERIFY(!read(f, json));
  QCOMPARE(f.title, QStringLiteral("Telemetry"));
  QCOMPARE(f.groups.size(), size_t(1));
  QVERIFY(f.actions.empty());
  QCOMPARE(f.containsCommercialFeatures, false);
}

/**
 * @brief A malformed source is dropped on its own: unlike actions, the source list is filtered.
 */
void TstFrameJsonLegacy::frameSkipsAMalformedSource()
{
  Source good;
  good.sourceId = 1;
  good.title    = QStringLiteral("Ground Station");

  QJsonArray groups;
  groups.append(toJson(widgetGroup(QStringLiteral("datagrid"))));

  QJsonArray sources;
  sources.append(QJsonObject());
  sources.append(toJson(good));

  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Telemetry"));
  json.insert(Keys::Groups, groups);
  json.insert(Keys::Sources, sources);

  const auto restored = fromJson<Frame>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->sources.size(), size_t(1));
  QCOMPARE(restored->sources[0].title, good.title);
}

//--------------------------------------------------------------------------------------------------
// Generated dataset reader and TableDef
//--------------------------------------------------------------------------------------------------

/**
 * @brief finalizeDatasetRead() drops a colour Qt cannot parse so the widget falls back to the
 *        theme palette instead of painting with an undefined QColor.
 */
void TstFrameJsonLegacy::datasetClearsAnUnparsableColour()
{
  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Altitude"));
  json.insert(Keys::Color, QStringLiteral("#gg0011"));

  const auto invalid = fromJson<Dataset>(json);
  QVERIFY(invalid.has_value());
  QVERIFY(invalid->color.isEmpty());

  json.insert(Keys::Color, QStringLiteral("chartreuse"));

  const auto named = fromJson<Dataset>(json);
  QVERIFY(named.has_value());
  QCOMPARE(named->color, QStringLiteral("chartreuse"));
}

void TstFrameJsonLegacy::tableDefSkipsANamelessRegister()
{
  QJsonObject nameless;
  nameless.insert(Keys::RegisterTypeName, QStringLiteral("computed"));

  RegisterDef good;
  good.name = QStringLiteral("gain");

  QJsonArray registers;
  registers.append(nameless);
  registers.append(QJsonObject());
  registers.append(toJson(good));

  QJsonObject json;
  json.insert(Keys::Name, QStringLiteral("Calibration"));
  json.insert(Keys::Registers, registers);

  const auto restored = fromJson<TableDef>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->registers.size(), size_t(1));
  QCOMPARE(restored->registers[0].name, QStringLiteral("gain"));
}

QTEST_APPLESS_MAIN(TstFrameJsonLegacy)

#include "tst_frame_json_legacy.moc"
