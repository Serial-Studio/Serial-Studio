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
#include <optional>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QTest>
#include <QVariant>

#include "DataModel/Frame.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.
//
// Every field name comes from Keys:: rather than a string literal, so renaming a key breaks the
// compile instead of silently changing the on-disk project format. Fixtures are built in code --
// no data files, so the suite has no working-directory dependence.

using namespace DataModel;

/**
 * @brief serialize()/read() round-trips for every project entity that reaches a saved file.
 */
class TstFrameSerialization : public QObject {
  Q_OBJECT

private slots:
  void actionRoundTripsEveryField();
  void actionDefaultsSurviveTheRoundTrip();
  void actionRejectsAnEmptyObject();
  void actionToleratesUnknownKeys();
  void actionClampsAnOutOfRangeTimerMode();

  void alarmBandRoundTripsAndNormalisesTheRange();
  void alarmBandWithoutARangeIsRejected();

  void frequencyMarkerRoundTripsOptionalFields();
  void frequencyMarkerOmitsUnsetLevels();
  void frequencyMarkerRejectsANonPositiveFrequency();
  void frequencyMarkerDemotesAnInvertedBand();

  void outputWidgetRoundTripsEveryField();
  void outputWidgetWithoutATitleIsRejected();
  void outputWidgetIdentityIsAssignedByTheGroupReader();

  void sourceRoundTripsEveryField();
  void sourceFallsBackToLegacyChecksumAndDecoderKeys();
  void sourceDefaultsAMissingFrameEnd();
  void sourceRejectsAnEmptyObject();

  void registerDefRoundTripsNumericAndStringDefaults();
  void registerDefSystemTypeDegradesToConstant();
  void registerDefRequiresAName();
  void tableDefRoundTripsItsRegisters();
  void tableDefOmitsATopLevelParentFolder();

  void workspaceRoundTripsWidgetReferences();
  void workspaceRequiresAnIdAndATitle();
  void folderStructsRoundTrip();

  void datasetRoundTripsConfiguration();
  void defaultDatasetRoundTripsToItsDefaults();
  void datasetOverviewDisplayRoundTrips();
  void datasetOmittedKeysLoadTheStructDefaults();
  void datasetSourceIdIsNotSerialized();
  void datasetIdentityIsAssignedByTheGroupReader();
  void datasetNestedAlarmBandsAndMarkersRoundTrip();
  void datasetFallsBackToLegacyMinMaxKeys();
  void datasetNormalisesInvertedRanges();
  void datasetRepairsAnOutOfRangeFftWindow();
  void datasetRejectsAnEmptyObjectWithoutMutating();
  void datasetToleratesUnknownKeys();

  void groupRoundTripsItsDatasets();
  void groupRequiresATitle();
  void groupWithoutDatasetsRoundTrips();
  void groupRoundTripsOutputWidgets();
  void groupRoundTripsImagePainterAndWebViewFields();

  void frameRoundTripsGroupsAndActions();
  void frameRequiresATitleAndAtLeastOneGroup();
  void frameSourcesAreNotSerialized();
  void frameWriterStampForcesASchemaVersion();
  void frameControlScriptRoundTrips();
  void frameWithAMalformedGroupReturnsFalseAfterMutating();
  void frameFinalizationAssignsDatasetUniqueIds();
};

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief A dataset with every serialized field set away from its default.
 */
static Dataset populatedDataset()
{
  Dataset d;
  d.index                = 7;
  d.xAxisId              = 12345;
  d.waterfall            = true;
  d.waterfallYAxis       = 54321;
  d.fft                  = true;
  d.fftLogX              = true;
  d.fftBallistics        = true;
  d.fftBallisticsRelease = 450;
  d.pltLogX              = true;
  d.pltLogY              = true;
  d.led                  = true;
  d.log                  = true;
  d.plt                  = true;
  d.ledHigh              = 75;
  d.fftSamples           = 512;
  d.fftSamplingRate      = 250;
  d.fftWindow            = 2;
  d.fftMin               = -1;
  d.fftMax               = 1;
  d.pltMin               = -2;
  d.pltMax               = 2;
  d.wgtMin               = -3;
  d.wgtMax               = 3;
  d.displayTickCount     = 8;
  d.displayFormat        = QStringLiteral("2f");
  d.decimalPoints        = 3;
  d.title                = QStringLiteral("Altitude");
  d.value                = QStringLiteral("42.5");
  d.units                = QStringLiteral("m");
  d.widget               = QStringLiteral("gauge");
  d.color                = QStringLiteral("#ff8800");
  d.alias                = QStringLiteral("alt");
  d.uniqueId             = 2000042;
  d.transformCode        = QStringLiteral("return x * 2");
  d.transformLanguage    = 1;
  d.virtual_             = true;
  d.hideOnDashboard      = true;
  d.extremeHold          = true;
  d.overviewDisplay      = true;
  d.enabled              = false;
  return d;
}

/**
 * @brief The smallest group read() accepts: a title plus one dataset.
 */
static Group minimalGroup(const QString& title)
{
  Dataset d;
  d.title = QStringLiteral("Voltage");
  d.value = QStringLiteral("3.3");

  Group g;
  g.title  = title;
  g.widget = QStringLiteral("datagrid");
  g.datasets.push_back(d);
  return g;
}

//--------------------------------------------------------------------------------------------------
// Action
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::actionRoundTripsEveryField()
{
  Action a;
  a.actionId             = 4;
  a.sourceId             = 2;
  a.repeatCount          = 9;
  a.timerIntervalMs      = 750;
  a.txEncoding           = 1;
  a.timerMode            = TimerMode::RepeatNTimes;
  a.binaryData           = true;
  a.autoExecuteOnConnect = true;
  a.icon                 = QStringLiteral("Stop Property");
  a.title                = QStringLiteral("Reboot");
  a.txData               = QStringLiteral("AA55");
  a.eolSequence          = QStringLiteral("\\r\\n");

  const auto restored = fromJson<Action>(toJson(a));
  QVERIFY(restored.has_value());

  QCOMPARE(restored->sourceId, a.sourceId);
  QCOMPARE(restored->repeatCount, a.repeatCount);
  QCOMPARE(restored->timerIntervalMs, a.timerIntervalMs);
  QCOMPARE(restored->txEncoding, a.txEncoding);
  QCOMPARE(restored->timerMode, a.timerMode);
  QCOMPARE(restored->binaryData, a.binaryData);
  QCOMPARE(restored->autoExecuteOnConnect, a.autoExecuteOnConnect);
  QCOMPARE(restored->icon, a.icon);
  QCOMPARE(restored->title, a.title);
  QCOMPARE(restored->txData, a.txData);
  QCOMPARE(restored->eolSequence, a.eolSequence);
}

/**
 * @brief actionId is positional state the project model owns, so it never reaches the file.
 */
void TstFrameSerialization::actionDefaultsSurviveTheRoundTrip()
{
  const Action original;
  const auto restored = fromJson<Action>(toJson(original));
  QVERIFY(restored.has_value());

  QCOMPARE(restored->icon, original.icon);
  QCOMPARE(restored->repeatCount, original.repeatCount);
  QCOMPARE(restored->timerIntervalMs, original.timerIntervalMs);
  QCOMPARE(restored->timerMode, original.timerMode);
  QCOMPARE(restored->binaryData, original.binaryData);
  QCOMPARE(restored->autoExecuteOnConnect, original.autoExecuteOnConnect);
  QCOMPARE(restored->actionId, -1);
}

void TstFrameSerialization::actionRejectsAnEmptyObject()
{
  Action a;
  a.title = QStringLiteral("kept");

  QVERIFY(!read(a, QJsonObject()));
  QCOMPARE(a.title, QStringLiteral("kept"));
}

void TstFrameSerialization::actionToleratesUnknownKeys()
{
  Action a;
  a.title = QStringLiteral("Reboot");

  auto json = toJson(a);
  json.insert(QStringLiteral("aFieldFromTheFuture"), 42);

  const auto restored = fromJson<Action>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->title, a.title);
}

void TstFrameSerialization::actionClampsAnOutOfRangeTimerMode()
{
  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Reboot"));
  json.insert(Keys::TimerMode, 99);

  const auto restored = fromJson<Action>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->timerMode, TimerMode::Off);
}

//--------------------------------------------------------------------------------------------------
// AlarmBand
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::alarmBandRoundTripsAndNormalisesTheRange()
{
  AlarmBand band;
  band.min      = 90;
  band.max      = 10;
  band.severity = AlarmSeverity::Critical;
  band.blink    = true;
  band.color    = QStringLiteral("#ff0000");
  band.label    = QStringLiteral("Overheat");

  const auto json = toJson(band);
  QCOMPARE(json.value(Keys::Min).toDouble(), 10.0);
  QCOMPARE(json.value(Keys::Max).toDouble(), 90.0);

  const auto restored = fromJson<AlarmBand>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->min, 10.0);
  QCOMPARE(restored->max, 90.0);
  QCOMPARE(restored->severity, AlarmSeverity::Critical);
  QCOMPARE(restored->blink, true);
  QCOMPARE(restored->color, band.color);
  QCOMPARE(restored->label, band.label);
}

/**
 * @brief A zero-width band carries no information, so read() rejects it rather than storing a
 *        zone that can never be entered.
 */
void TstFrameSerialization::alarmBandWithoutARangeIsRejected()
{
  const AlarmBand band;
  QVERIFY(!fromJson<AlarmBand>(toJson(band)).has_value());
}

//--------------------------------------------------------------------------------------------------
// FrequencyMarker
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::frequencyMarkerRoundTripsOptionalFields()
{
  FrequencyMarker marker;
  marker.frequency    = 100;
  marker.endFrequency = 250;
  marker.warningDb    = -30;
  marker.alarmDb      = -10;
  marker.color        = QStringLiteral("#00ff00");
  marker.label        = QStringLiteral("Rotor");

  const auto restored = fromJson<FrequencyMarker>(toJson(marker));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->frequency, marker.frequency);
  QCOMPARE(restored->endFrequency, marker.endFrequency);
  QCOMPARE(restored->warningDb, marker.warningDb);
  QCOMPARE(restored->alarmDb, marker.alarmDb);
  QCOMPARE(restored->color, marker.color);
  QCOMPARE(restored->label, marker.label);
}

/**
 * @brief Unset dB levels are NaN and must stay absent from the file, not serialize as zero.
 */
void TstFrameSerialization::frequencyMarkerOmitsUnsetLevels()
{
  FrequencyMarker marker;
  marker.frequency = 50;

  const auto json = toJson(marker);
  QVERIFY(!json.contains(Keys::WarningDb));
  QVERIFY(!json.contains(Keys::AlarmDb));
  QVERIFY(!json.contains(Keys::EndFrequency));
  QVERIFY(!json.contains(Keys::Color));
  QVERIFY(!json.contains(Keys::Label));

  const auto restored = fromJson<FrequencyMarker>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->frequency, 50.0);
  QCOMPARE(restored->endFrequency, 0.0);
  QVERIFY(std::isnan(restored->warningDb));
  QVERIFY(std::isnan(restored->alarmDb));
}

void TstFrameSerialization::frequencyMarkerRejectsANonPositiveFrequency()
{
  QJsonObject zero;
  zero.insert(Keys::Frequency, 0);
  QVERIFY(!fromJson<FrequencyMarker>(zero).has_value());

  QJsonObject negative;
  negative.insert(Keys::Frequency, -5);
  QVERIFY(!fromJson<FrequencyMarker>(negative).has_value());
}

void TstFrameSerialization::frequencyMarkerDemotesAnInvertedBand()
{
  QJsonObject json;
  json.insert(Keys::Frequency, 100);
  json.insert(Keys::EndFrequency, 50);

  const auto restored = fromJson<FrequencyMarker>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->frequency, 100.0);
  QCOMPARE(restored->endFrequency, 0.0);
}

//--------------------------------------------------------------------------------------------------
// OutputWidget
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::outputWidgetRoundTripsEveryField()
{
  OutputWidget w;
  w.sourceId         = 2;
  w.txEncoding       = 1;
  w.type             = OutputWidgetType::Slider;
  w.monoIcon         = true;
  w.minValue         = -5;
  w.maxValue         = 5;
  w.stepSize         = 0.5;
  w.initialValue     = 1;
  w.icon             = QStringLiteral("Send Property");
  w.title            = QStringLiteral("Throttle");
  w.transmitFunction = QStringLiteral("return String(value)");

  const auto restored = fromJson<OutputWidget>(toJson(w));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->sourceId, w.sourceId);
  QCOMPARE(restored->txEncoding, w.txEncoding);
  QCOMPARE(restored->type, w.type);
  QCOMPARE(restored->monoIcon, w.monoIcon);
  QCOMPARE(restored->minValue, w.minValue);
  QCOMPARE(restored->maxValue, w.maxValue);
  QCOMPARE(restored->stepSize, w.stepSize);
  QCOMPARE(restored->initialValue, w.initialValue);
  QCOMPARE(restored->icon, w.icon);
  QCOMPARE(restored->title, w.title);
  QCOMPARE(restored->transmitFunction, w.transmitFunction);
}

void TstFrameSerialization::outputWidgetWithoutATitleIsRejected()
{
  const OutputWidget w;
  QVERIFY(!fromJson<OutputWidget>(toJson(w)).has_value());
}

/**
 * @brief widgetId and groupId are positions inside the owning group, so the group reader stamps
 *        them and the file never carries them.
 */
void TstFrameSerialization::outputWidgetIdentityIsAssignedByTheGroupReader()
{
  OutputWidget w;
  w.title    = QStringLiteral("Throttle");
  w.widgetId = 7;
  w.groupId  = 3;

  const auto restored = fromJson<OutputWidget>(toJson(w));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->widgetId, -1);
  QCOMPARE(restored->groupId, -1);
}

//--------------------------------------------------------------------------------------------------
// Source
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::sourceRoundTripsEveryField()
{
  QJsonObject connection;
  connection.insert(QStringLiteral("port"), QStringLiteral("COM3"));

  QJsonObject params;
  params.insert(QStringLiteral("separator"), QStringLiteral(","));

  Source s;
  s.sourceId              = 3;
  s.busType               = 1;
  s.title                 = QStringLiteral("Ground Station");
  s.frameStart            = QStringLiteral("/*");
  s.frameEnd              = QStringLiteral("*/");
  s.checksumAlgorithm     = QStringLiteral("CRC-16");
  s.frameDetection        = 1;
  s.decoderMethod         = 2;
  s.hexadecimalDelimiters = true;
  s.frameParserLanguage   = 1;
  s.connectionSettings    = connection;
  s.frameParserCode       = QStringLiteral("function parse(f) { return [f] }");
  s.frameParserTemplate   = QStringLiteral("csv");
  s.frameParserParams     = params;

  const auto restored = fromJson<Source>(toJson(s));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->sourceId, s.sourceId);
  QCOMPARE(restored->busType, s.busType);
  QCOMPARE(restored->title, s.title);
  QCOMPARE(restored->frameStart, s.frameStart);
  QCOMPARE(restored->frameEnd, s.frameEnd);
  QCOMPARE(restored->checksumAlgorithm, s.checksumAlgorithm);
  QCOMPARE(restored->frameDetection, s.frameDetection);
  QCOMPARE(restored->decoderMethod, s.decoderMethod);
  QCOMPARE(restored->hexadecimalDelimiters, s.hexadecimalDelimiters);
  QCOMPARE(restored->frameParserLanguage, s.frameParserLanguage);
  QCOMPARE(restored->connectionSettings, s.connectionSettings);
  QCOMPARE(restored->frameParserCode, s.frameParserCode);
  QCOMPARE(restored->frameParserTemplate, s.frameParserTemplate);
  QCOMPARE(restored->frameParserParams, s.frameParserParams);
}

/**
 * @brief Projects written before the key rename carry "checksum" and "decoder"; the canonical
 *        keys win whenever both are present.
 */
void TstFrameSerialization::sourceFallsBackToLegacyChecksumAndDecoderKeys()
{
  QJsonObject legacy;
  legacy.insert(Keys::SourceId, 1);
  legacy.insert(Keys::Checksum, QStringLiteral("CRC-8"));
  legacy.insert(Keys::Decoder, 2);

  const auto fallback = fromJson<Source>(legacy);
  QVERIFY(fallback.has_value());
  QCOMPARE(fallback->checksumAlgorithm, QStringLiteral("CRC-8"));
  QCOMPARE(fallback->decoderMethod, 2);

  QJsonObject both = legacy;
  both.insert(Keys::ChecksumAlgorithm, QStringLiteral("CRC-32"));
  both.insert(Keys::DecoderMethod, 1);

  const auto canonical = fromJson<Source>(both);
  QVERIFY(canonical.has_value());
  QCOMPARE(canonical->checksumAlgorithm, QStringLiteral("CRC-32"));
  QCOMPARE(canonical->decoderMethod, 1);
}

/**
 * @brief A source with no end delimiter recorded falls back to the escaped newline sequence, the
 *        historical default for line-based devices.
 */
void TstFrameSerialization::sourceDefaultsAMissingFrameEnd()
{
  QJsonObject json;
  json.insert(Keys::SourceId, 0);

  const auto restored = fromJson<Source>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->frameEnd, QStringLiteral("\\n"));
  QCOMPARE(restored->frameStart, QString());
}

void TstFrameSerialization::sourceRejectsAnEmptyObject()
{
  Source s;
  s.title = QStringLiteral("kept");

  QVERIFY(!read(s, QJsonObject()));
  QCOMPARE(s.title, QStringLiteral("kept"));
}

//--------------------------------------------------------------------------------------------------
// RegisterDef / TableDef
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::registerDefRoundTripsNumericAndStringDefaults()
{
  RegisterDef numeric;
  numeric.name         = QStringLiteral("gain");
  numeric.type         = RegisterType::Computed;
  numeric.defaultValue = QVariant(2.5);

  const auto restoredNumeric = fromJson<RegisterDef>(toJson(numeric));
  QVERIFY(restoredNumeric.has_value());
  QCOMPARE(restoredNumeric->name, numeric.name);
  QCOMPARE(restoredNumeric->type, RegisterType::Computed);
  QCOMPARE(restoredNumeric->defaultValue.toDouble(), 2.5);

  RegisterDef text;
  text.name         = QStringLiteral("mode");
  text.defaultValue = QVariant(QStringLiteral("auto"));

  const auto restoredText = fromJson<RegisterDef>(toJson(text));
  QVERIFY(restoredText.has_value());
  QCOMPARE(restoredText->type, RegisterType::Constant);
  QCOMPARE(restoredText->defaultValue.toString(), QStringLiteral("auto"));

  RegisterDef empty;
  empty.name = QStringLiteral("spare");

  const auto restoredEmpty = fromJson<RegisterDef>(toJson(empty));
  QVERIFY(restoredEmpty.has_value());
  QCOMPARE(restoredEmpty->defaultValue.toDouble(), 0.0);
}

/**
 * @brief Only "computed" has a written spelling, so a System register comes back as Constant.
 */
void TstFrameSerialization::registerDefSystemTypeDegradesToConstant()
{
  RegisterDef reg;
  reg.name = QStringLiteral("uptime");
  reg.type = RegisterType::System;

  const auto restored = fromJson<RegisterDef>(toJson(reg));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->type, RegisterType::Constant);
}

void TstFrameSerialization::registerDefRequiresAName()
{
  QVERIFY(!fromJson<RegisterDef>(QJsonObject()).has_value());

  QJsonObject nameless;
  nameless.insert(Keys::RegisterTypeName, QStringLiteral("constant"));
  QVERIFY(!fromJson<RegisterDef>(nameless).has_value());
}

void TstFrameSerialization::tableDefRoundTripsItsRegisters()
{
  RegisterDef one;
  one.name         = QStringLiteral("offset");
  one.defaultValue = QVariant(1.5);

  RegisterDef two;
  two.name = QStringLiteral("scale");
  two.type = RegisterType::Computed;

  TableDef table;
  table.name           = QStringLiteral("Calibration");
  table.parentFolderId = 4;
  table.registers.push_back(one);
  table.registers.push_back(two);

  const auto restored = fromJson<TableDef>(toJson(table));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->name, table.name);
  QCOMPARE(restored->parentFolderId, 4);
  QCOMPARE(restored->registers.size(), size_t(2));
  QCOMPARE(restored->registers[0].name, one.name);
  QCOMPARE(restored->registers[1].type, RegisterType::Computed);
}

void TstFrameSerialization::tableDefOmitsATopLevelParentFolder()
{
  TableDef table;
  table.name = QStringLiteral("Calibration");

  const auto json = toJson(table);
  QVERIFY(!json.contains(Keys::ParentFolderId));

  const auto restored = fromJson<TableDef>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->parentFolderId, -1);
  QVERIFY(restored->registers.empty());
}

//--------------------------------------------------------------------------------------------------
// Workspaces and folders
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::workspaceRoundTripsWidgetReferences()
{
  Workspace w;
  w.workspaceId    = WorkspaceIds::UserStart;
  w.parentFolderId = 2;
  w.title          = QStringLiteral("Flight");
  w.icon           = QStringLiteral("Rocket Property");
  w.description    = QStringLiteral("Ascent instruments");
  w.widgetRefs.push_back({1, 10, 0});
  w.widgetRefs.push_back({2, 11, 3});

  const auto restored = fromJson<Workspace>(toJson(w));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->workspaceId, w.workspaceId);
  QCOMPARE(restored->parentFolderId, w.parentFolderId);
  QCOMPARE(restored->title, w.title);
  QCOMPARE(restored->icon, w.icon);
  QCOMPARE(restored->description, w.description);
  QCOMPARE(restored->widgetRefs.size(), size_t(2));
  QCOMPARE(restored->widgetRefs[1].widgetType, 2);
  QCOMPARE(restored->widgetRefs[1].groupUniqueId, 11);
  QCOMPARE(restored->widgetRefs[1].relativeIndex, 3);
}

void TstFrameSerialization::workspaceRequiresAnIdAndATitle()
{
  Workspace untitled;
  untitled.workspaceId = WorkspaceIds::UserStart;
  QVERIFY(!fromJson<Workspace>(toJson(untitled)).has_value());

  Workspace unidentified;
  unidentified.title = QStringLiteral("Flight");
  QVERIFY(!fromJson<Workspace>(toJson(unidentified)).has_value());
}

void TstFrameSerialization::folderStructsRoundTrip()
{
  WorkspaceFolder workspaceFolder;
  workspaceFolder.folderId       = 1;
  workspaceFolder.parentFolderId = 5;
  workspaceFolder.title          = QStringLiteral("Mission");

  const auto restoredWorkspace = fromJson<WorkspaceFolder>(toJson(workspaceFolder));
  QVERIFY(restoredWorkspace.has_value());
  QCOMPARE(restoredWorkspace->folderId, 1);
  QCOMPARE(restoredWorkspace->parentFolderId, 5);
  QCOMPARE(restoredWorkspace->title, workspaceFolder.title);

  GroupFolder groupFolder;
  groupFolder.folderId = 2;
  groupFolder.title    = QStringLiteral("Sensors");

  const auto groupJson = toJson(groupFolder);
  QVERIFY(!groupJson.contains(Keys::ParentFolderId));

  const auto restoredGroup = fromJson<GroupFolder>(groupJson);
  QVERIFY(restoredGroup.has_value());
  QCOMPARE(restoredGroup->parentFolderId, -1);

  TableFolder tableFolder;
  tableFolder.folderId = 3;
  tableFolder.title    = QStringLiteral("Lookups");

  const auto restoredTable = fromJson<TableFolder>(toJson(tableFolder));
  QVERIFY(restoredTable.has_value());
  QCOMPARE(restoredTable->folderId, 3);
  QCOMPARE(restoredTable->title, tableFolder.title);

  const GroupFolder unidentified;
  QVERIFY(!fromJson<GroupFolder>(toJson(unidentified)).has_value());
}

//--------------------------------------------------------------------------------------------------
// Dataset
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::datasetRoundTripsConfiguration()
{
  const Dataset d     = populatedDataset();
  const auto restored = fromJson<Dataset>(toJson(d));
  QVERIFY(restored.has_value());

  QCOMPARE(restored->index, d.index);
  QCOMPARE(restored->xAxisId, d.xAxisId);
  QCOMPARE(restored->waterfall, d.waterfall);
  QCOMPARE(restored->waterfallYAxis, d.waterfallYAxis);
  QCOMPARE(restored->fft, d.fft);
  QCOMPARE(restored->fftLogX, d.fftLogX);
  QCOMPARE(restored->fftBallistics, d.fftBallistics);
  QCOMPARE(restored->fftBallisticsRelease, d.fftBallisticsRelease);
  QCOMPARE(restored->pltLogX, d.pltLogX);
  QCOMPARE(restored->pltLogY, d.pltLogY);
  QCOMPARE(restored->led, d.led);
  QCOMPARE(restored->log, d.log);
  QCOMPARE(restored->plt, d.plt);
  QCOMPARE(restored->ledHigh, d.ledHigh);
  QCOMPARE(restored->fftSamples, d.fftSamples);
  QCOMPARE(restored->fftSamplingRate, d.fftSamplingRate);
  QCOMPARE(restored->fftWindow, d.fftWindow);
  QCOMPARE(restored->fftMin, d.fftMin);
  QCOMPARE(restored->fftMax, d.fftMax);
  QCOMPARE(restored->pltMin, d.pltMin);
  QCOMPARE(restored->pltMax, d.pltMax);
  QCOMPARE(restored->wgtMin, d.wgtMin);
  QCOMPARE(restored->wgtMax, d.wgtMax);
  QCOMPARE(restored->displayTickCount, d.displayTickCount);
  QCOMPARE(restored->displayFormat, d.displayFormat);
  QCOMPARE(restored->decimalPoints, d.decimalPoints);
  QCOMPARE(restored->title, d.title);
  QCOMPARE(restored->value, d.value);
  QCOMPARE(restored->units, d.units);
  QCOMPARE(restored->widget, d.widget);
  QCOMPARE(restored->color, d.color);
  QCOMPARE(restored->alias, d.alias);
  QCOMPARE(restored->uniqueId, d.uniqueId);
  QCOMPARE(restored->transformCode, d.transformCode);
  QCOMPARE(restored->transformLanguage, d.transformLanguage);
  QCOMPARE(restored->virtual_, d.virtual_);
  QCOMPARE(restored->hideOnDashboard, d.hideOnDashboard);
  QCOMPARE(restored->extremeHold, d.extremeHold);
  QCOMPARE(restored->overviewDisplay, d.overviewDisplay);
  QCOMPARE(restored->enabled, d.enabled);

  QCOMPARE(restored->numericValue, 42.5);
  QVERIFY(restored->isNumeric);
}

void TstFrameSerialization::defaultDatasetRoundTripsToItsDefaults()
{
  const Dataset original;
  const auto restored = fromJson<Dataset>(toJson(original));
  QVERIFY(restored.has_value());

  QCOMPARE(restored->index, original.index);
  QCOMPARE(restored->xAxisId, kXAxisTime);
  QCOMPARE(restored->fftSamples, original.fftSamples);
  QCOMPARE(restored->fftSamplingRate, original.fftSamplingRate);
  QCOMPARE(restored->fftWindow, original.fftWindow);
  QCOMPARE(restored->ledHigh, original.ledHigh);
  QCOMPARE(restored->displayTickCount, original.displayTickCount);
  QCOMPARE(restored->displayFormat, original.displayFormat);
  QCOMPARE(restored->decimalPoints, original.decimalPoints);
  QCOMPARE(restored->enabled, true);
  QCOMPARE(restored->virtual_, false);
  QCOMPARE(restored->hideOnDashboard, false);
  QCOMPARE(restored->extremeHold, false);
  QCOMPARE(restored->waterfall, false);
  QVERIFY(!restored->isNumeric);
  QVERIFY(restored->alarmBands.empty());
  QVERIFY(restored->fftMarkers.empty());
}

/**
 * @brief Spec 0036 defect fix: the registry-derived serializer writes "overviewDisplay" when set,
 *        so the flag now survives a save/reload cycle. Written only when true.
 */
void TstFrameSerialization::datasetOverviewDisplayRoundTrips()
{
  Dataset d;
  d.title           = QStringLiteral("Altitude");
  d.overviewDisplay = true;

  const auto json = toJson(d);
  QVERIFY(json.contains(Keys::Overview));

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->overviewDisplay, true);

  Dataset off;
  off.title = QStringLiteral("Altitude");
  QVERIFY(!toJson(off).contains(Keys::Overview));
}

/**
 * @brief Spec 0036 defect fix: a project file that omits a key now loads the same value a freshly
 *        created dataset carries, instead of the reader's own disagreeing fallback.
 */
void TstFrameSerialization::datasetOmittedKeysLoadTheStructDefaults()
{
  QJsonObject sparse;
  sparse.insert(Keys::Title, QStringLiteral("Altitude"));

  const Dataset fresh;
  const auto restored = fromJson<Dataset>(sparse);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->index, fresh.index);
  QCOMPARE(restored->fftSamples, fresh.fftSamples);
  QCOMPARE(restored->fftSamplingRate, fresh.fftSamplingRate);
  QCOMPARE(restored->ledHigh, fresh.ledHigh);
}

/**
 * @brief read() takes a dataset's source from "datasetSourceId", which serialize() never writes:
 *        the owning group is what actually carries the source on save.
 */
void TstFrameSerialization::datasetSourceIdIsNotSerialized()
{
  Dataset d;
  d.title    = QStringLiteral("Altitude");
  d.sourceId = 3;

  const auto json = toJson(d);
  QVERIFY(!json.contains(Keys::DatasetSourceId));

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->sourceId, 0);
}

/**
 * @brief groupId and datasetId are written for readability but ignored on load; the group reader
 *        restamps them from the dataset's position in the array.
 */
void TstFrameSerialization::datasetIdentityIsAssignedByTheGroupReader()
{
  Dataset d;
  d.title     = QStringLiteral("Altitude");
  d.groupId   = 2;
  d.datasetId = 4;

  const auto json = toJson(d);
  QVERIFY(json.contains(Keys::GroupId));
  QVERIFY(json.contains(Keys::DatasetId));

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->groupId, 0);
  QCOMPARE(restored->datasetId, 0);
}

void TstFrameSerialization::datasetNestedAlarmBandsAndMarkersRoundTrip()
{
  AlarmBand low;
  low.min      = 0;
  low.max      = 10;
  low.severity = AlarmSeverity::Warning;

  AlarmBand high;
  high.min      = 10;
  high.max      = 20;
  high.severity = AlarmSeverity::Critical;
  high.label    = QStringLiteral("Redline");

  FrequencyMarker marker;
  marker.frequency = 120;
  marker.label     = QStringLiteral("Rotor");

  Dataset d;
  d.title = QStringLiteral("Vibration");
  d.alarmBands.push_back(low);
  d.alarmBands.push_back(high);
  d.fftMarkers.push_back(marker);

  const auto restored = fromJson<Dataset>(toJson(d));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->alarmBands.size(), size_t(2));
  QCOMPARE(restored->alarmBands[1].max, 20.0);
  QCOMPARE(restored->alarmBands[1].severity, AlarmSeverity::Critical);
  QCOMPARE(restored->alarmBands[1].label, high.label);
  QCOMPARE(restored->fftMarkers.size(), size_t(1));
  QCOMPARE(restored->fftMarkers[0].frequency, 120.0);
  QCOMPARE(restored->fftMarkers[0].label, marker.label);
}

/**
 * @brief Pre-3.0 projects carried one "min"/"max" pair shared by the FFT, plot and widget ranges.
 */
void TstFrameSerialization::datasetFallsBackToLegacyMinMaxKeys()
{
  QJsonObject legacy;
  legacy.insert(Keys::Title, QStringLiteral("Altitude"));
  legacy.insert(Keys::Min, 1);
  legacy.insert(Keys::Max, 9);

  const auto restored = fromJson<Dataset>(legacy);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->fftMin, 1.0);
  QCOMPARE(restored->fftMax, 9.0);
  QCOMPARE(restored->pltMin, 1.0);
  QCOMPARE(restored->pltMax, 9.0);
  QCOMPARE(restored->wgtMin, 1.0);
  QCOMPARE(restored->wgtMax, 9.0);
}

void TstFrameSerialization::datasetNormalisesInvertedRanges()
{
  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Altitude"));
  json.insert(Keys::FFTMin, 9);
  json.insert(Keys::FFTMax, 1);
  json.insert(Keys::PltMin, 8);
  json.insert(Keys::PltMax, 2);
  json.insert(Keys::WgtMin, 7);
  json.insert(Keys::WgtMax, 3);

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->fftMin, 1.0);
  QCOMPARE(restored->fftMax, 9.0);
  QCOMPARE(restored->pltMin, 2.0);
  QCOMPARE(restored->pltMax, 8.0);
  QCOMPARE(restored->wgtMin, 3.0);
  QCOMPARE(restored->wgtMax, 7.0);
}

void TstFrameSerialization::datasetRepairsAnOutOfRangeFftWindow()
{
  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Altitude"));
  json.insert(Keys::FFTWindow, 99);

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->fftWindow, 5);
}

void TstFrameSerialization::datasetRejectsAnEmptyObjectWithoutMutating()
{
  Dataset d;
  d.title = QStringLiteral("kept");
  d.index = 11;

  QVERIFY(!read(d, QJsonObject()));
  QCOMPARE(d.title, QStringLiteral("kept"));
  QCOMPARE(d.index, 11);
}

void TstFrameSerialization::datasetToleratesUnknownKeys()
{
  auto json = toJson(populatedDataset());
  json.insert(QStringLiteral("aFieldFromTheFuture"), QStringLiteral("ignored"));

  const auto restored = fromJson<Dataset>(json);
  QVERIFY(restored.has_value());
  QCOMPARE(restored->title, QStringLiteral("Altitude"));
  QCOMPARE(restored->fftSamples, 512);
}

//--------------------------------------------------------------------------------------------------
// Group
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::groupRoundTripsItsDatasets()
{
  Group g          = minimalGroup(QStringLiteral("Sensors"));
  g.uniqueId       = 3;
  g.parentFolderId = 1;
  g.columns        = 4;
  g.sourceId       = 2;
  g.enabled        = false;

  const auto restored = fromJson<Group>(toJson(g));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->title, g.title);
  QCOMPARE(restored->widget, g.widget);
  QCOMPARE(restored->uniqueId, g.uniqueId);
  QCOMPARE(restored->parentFolderId, g.parentFolderId);
  QCOMPARE(restored->columns, g.columns);
  QCOMPARE(restored->sourceId, g.sourceId);
  QCOMPARE(restored->enabled, false);
  QCOMPARE(restored->groupType, GroupType::Input);
  QCOMPARE(restored->datasets.size(), size_t(1));
  QCOMPARE(restored->datasets[0].title, QStringLiteral("Voltage"));
  QCOMPARE(restored->datasets[0].datasetId, 0);
  QCOMPARE(restored->datasets[0].sourceId, g.sourceId);
  QCOMPARE(restored->groupId, -1);
}

void TstFrameSerialization::groupRequiresATitle()
{
  Group g = minimalGroup(QString());

  Group destination;
  destination.title = QStringLiteral("kept");
  QVERIFY(!read(destination, toJson(g)));
  QCOMPARE(destination.title, QStringLiteral("kept"));
  QVERIFY(destination.datasets.empty());
}

/**
 * @brief A dataset-less group is legal document state (the editor and API create groups before
 *        their first dataset), so it must round-trip for undo snapshots and saved projects alike.
 */
void TstFrameSerialization::groupWithoutDatasetsRoundTrips()
{
  Group plain;
  plain.title              = QStringLiteral("Sensors");
  plain.widget             = QStringLiteral("datagrid");
  const auto restoredPlain = fromJson<Group>(toJson(plain));
  QVERIFY(restoredPlain.has_value());
  QCOMPARE(restoredPlain->title, plain.title);
  QVERIFY(restoredPlain->datasets.empty());

  Group image;
  image.title  = QStringLiteral("Camera");
  image.widget = QStringLiteral("image");
  QVERIFY(fromJson<Group>(toJson(image)).has_value());

  Group painter;
  painter.title       = QStringLiteral("Sketch");
  painter.widget      = QStringLiteral("painter");
  painter.painterCode = QStringLiteral("function paint() {}");
  QVERIFY(fromJson<Group>(toJson(painter)).has_value());

  Group webview;
  webview.title      = QStringLiteral("Docs");
  webview.widget     = QStringLiteral("webview");
  webview.webViewUrl = QStringLiteral("https://serial-studio.com/");
  QVERIFY(fromJson<Group>(toJson(webview)).has_value());

  Group output;
  output.title     = QStringLiteral("Controls");
  output.groupType = GroupType::Output;
  QVERIFY(fromJson<Group>(toJson(output)).has_value());
}

void TstFrameSerialization::groupRoundTripsOutputWidgets()
{
  OutputWidget button;
  button.title = QStringLiteral("Arm");

  OutputWidget slider;
  slider.title = QStringLiteral("Throttle");
  slider.type  = OutputWidgetType::Slider;

  Group g;
  g.title     = QStringLiteral("Controls");
  g.groupType = GroupType::Output;
  g.outputWidgets.push_back(button);
  g.outputWidgets.push_back(slider);

  const auto restored = fromJson<Group>(toJson(g));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->groupType, GroupType::Output);
  QCOMPARE(restored->outputWidgets.size(), size_t(2));
  QCOMPARE(restored->outputWidgets[1].title, slider.title);
  QCOMPARE(restored->outputWidgets[1].type, OutputWidgetType::Slider);
  QCOMPARE(restored->outputWidgets[1].widgetId, 1);
}

void TstFrameSerialization::groupRoundTripsImagePainterAndWebViewFields()
{
  Group image;
  image.title            = QStringLiteral("Camera");
  image.widget           = QStringLiteral("image");
  image.imgDetectionMode = QStringLiteral("manual");
  image.imgStartSequence = QStringLiteral("AA55");
  image.imgEndSequence   = QStringLiteral("55AA");

  const auto restoredImage = fromJson<Group>(toJson(image));
  QVERIFY(restoredImage.has_value());
  QCOMPARE(restoredImage->imgDetectionMode, image.imgDetectionMode);
  QCOMPARE(restoredImage->imgStartSequence, image.imgStartSequence);
  QCOMPARE(restoredImage->imgEndSequence, image.imgEndSequence);

  Group painter;
  painter.title       = QStringLiteral("Sketch");
  painter.widget      = QStringLiteral("painter");
  painter.painterCode = QStringLiteral("function paint(ctx) {}");

  const auto restoredPainter = fromJson<Group>(toJson(painter));
  QVERIFY(restoredPainter.has_value());
  QCOMPARE(restoredPainter->painterCode, painter.painterCode);

  Group webview;
  webview.title      = QStringLiteral("Docs");
  webview.widget     = QStringLiteral("webview");
  webview.webViewUrl = QStringLiteral("https://serial-studio.com/");

  const auto restoredWeb = fromJson<Group>(toJson(webview));
  QVERIFY(restoredWeb.has_value());
  QCOMPARE(restoredWeb->webViewUrl, webview.webViewUrl);

  Group barPanel;
  barPanel.title         = QStringLiteral("Pressures");
  barPanel.widget        = QStringLiteral("barpanel");
  barPanel.barPanelStyle = QStringLiteral("vertical");

  const auto restoredPanel = fromJson<Group>(toJson(barPanel));
  QVERIFY(restoredPanel.has_value());
  QCOMPARE(restoredPanel->barPanelStyle, barPanel.barPanelStyle);

  Group autoPanel;
  autoPanel.title  = QStringLiteral("Temperatures");
  autoPanel.widget = QStringLiteral("barpanel");

  const auto json = toJson(autoPanel);
  QVERIFY(!json.contains(Keys::BarPanelStyle));
  const auto restoredAuto = fromJson<Group>(json);
  QVERIFY(restoredAuto.has_value());
  QVERIFY(restoredAuto->barPanelStyle.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Frame
//--------------------------------------------------------------------------------------------------

void TstFrameSerialization::frameRoundTripsGroupsAndActions()
{
  Action action;
  action.title  = QStringLiteral("Reboot");
  action.txData = QStringLiteral("R");

  Frame f;
  f.title = QStringLiteral("Telemetry");
  f.groups.push_back(minimalGroup(QStringLiteral("Sensors")));
  f.groups.push_back(minimalGroup(QStringLiteral("Power")));
  f.actions.push_back(action);

  const auto restored = fromJson<Frame>(toJson(f));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->title, f.title);
  QCOMPARE(restored->groups.size(), size_t(2));
  QCOMPARE(restored->actions.size(), size_t(1));
  QCOMPARE(restored->groups[1].title, QStringLiteral("Power"));
  QCOMPARE(restored->groups[1].groupId, 1);
  QCOMPARE(restored->groups[1].datasets[0].groupId, 1);
  QCOMPARE(restored->actions[0].title, action.title);
}

void TstFrameSerialization::frameRequiresATitleAndAtLeastOneGroup()
{
  Frame empty;
  QVERIFY(!read(empty, QJsonObject()));

  Frame untitled;
  untitled.groups.push_back(minimalGroup(QStringLiteral("Sensors")));
  QJsonObject groupsOnly;
  groupsOnly.insert(Keys::Groups, toJson(untitled).value(Keys::Groups));
  QVERIFY(!fromJson<Frame>(groupsOnly).has_value());

  QJsonObject titleOnly;
  titleOnly.insert(Keys::Title, QStringLiteral("Telemetry"));
  QVERIFY(!fromJson<Frame>(titleOnly).has_value());
}

/**
 * @brief serialize(Frame) omits the source list even though read(Frame) consumes it; sources are
 *        written by the project model, not by the frame serializer.
 */
void TstFrameSerialization::frameSourcesAreNotSerialized()
{
  Source source;
  source.sourceId = 1;
  source.title    = QStringLiteral("Ground Station");

  Frame f;
  f.title = QStringLiteral("Telemetry");
  f.groups.push_back(minimalGroup(QStringLiteral("Sensors")));
  f.sources.push_back(source);

  auto json = toJson(f);
  QVERIFY(!json.contains(Keys::Sources));

  const auto withoutSources = fromJson<Frame>(json);
  QVERIFY(withoutSources.has_value());
  QVERIFY(withoutSources->sources.empty());

  QJsonArray sources;
  sources.append(toJson(source));
  json.insert(Keys::Sources, sources);

  const auto withSources = fromJson<Frame>(json);
  QVERIFY(withSources.has_value());
  QCOMPARE(withSources->sources.size(), size_t(1));
  QCOMPARE(withSources->sources[0].title, source.title);
}

void TstFrameSerialization::frameWriterStampForcesASchemaVersion()
{
  Frame unstamped;
  unstamped.title = QStringLiteral("Telemetry");
  unstamped.groups.push_back(minimalGroup(QStringLiteral("Sensors")));

  const auto plainJson = toJson(unstamped);
  QVERIFY(!plainJson.contains(Keys::SchemaVersion));
  QVERIFY(!plainJson.contains(Keys::WriterVersion));

  const auto restoredPlain = fromJson<Frame>(plainJson);
  QVERIFY(restoredPlain.has_value());
  QCOMPARE(restoredPlain->schemaVersion, 0);

  Frame stamped         = unstamped;
  stamped.writerVersion = QStringLiteral("3.3.0");

  const auto stampedJson = toJson(stamped);
  QCOMPARE(stampedJson.value(Keys::SchemaVersion).toInt(), kSchemaVersion);

  const auto restoredStamped = fromJson<Frame>(stampedJson);
  QVERIFY(restoredStamped.has_value());
  QCOMPARE(restoredStamped->schemaVersion, kSchemaVersion);
  QCOMPARE(restoredStamped->writerVersion, stamped.writerVersion);

  Frame pinned         = stamped;
  pinned.schemaVersion = 2;
  QCOMPARE(toJson(pinned).value(Keys::SchemaVersion).toInt(), 2);
}

void TstFrameSerialization::frameControlScriptRoundTrips()
{
  Frame f;
  f.title = QStringLiteral("Telemetry");
  f.groups.push_back(minimalGroup(QStringLiteral("Sensors")));

  QVERIFY(!toJson(f).contains(Keys::ControlScriptCode));

  f.controlScriptCode = QStringLiteral("function setup() {}");

  const auto restored = fromJson<Frame>(toJson(f));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->controlScriptCode, f.controlScriptCode);
}

/**
 * @brief Current behaviour: a frame whose group list contains a malformed entry returns false
 *        *after* assigning the title and clearing the group vector, so the destination is left
 *        partially written. Pinned so a future tightening is a deliberate change.
 */
void TstFrameSerialization::frameWithAMalformedGroupReturnsFalseAfterMutating()
{
  QJsonObject malformedGroup;
  malformedGroup.insert(Keys::Widget, QStringLiteral("datagrid"));

  QJsonArray groups;
  groups.append(malformedGroup);

  QJsonObject json;
  json.insert(Keys::Title, QStringLiteral("Telemetry"));
  json.insert(Keys::Groups, groups);

  Frame f;
  f.title = QStringLiteral("kept");
  f.groups.push_back(minimalGroup(QStringLiteral("Sensors")));

  QVERIFY(!read(f, json));
  QCOMPARE(f.title, QStringLiteral("Telemetry"));
  QVERIFY(f.groups.empty());
}

/**
 * @brief finalize_frame() stamps every dataset with the owning group's source and a stable
 *        uniqueId derived from (source, group, dataset).
 */
void TstFrameSerialization::frameFinalizationAssignsDatasetUniqueIds()
{
  Group g    = minimalGroup(QStringLiteral("Sensors"));
  g.sourceId = 2;

  Dataset second;
  second.title = QStringLiteral("Current");
  g.datasets.push_back(second);

  Frame f;
  f.title = QStringLiteral("Telemetry");
  f.groups.push_back(g);

  const auto restored = fromJson<Frame>(toJson(f));
  QVERIFY(restored.has_value());
  QCOMPARE(restored->groups[0].datasets.size(), size_t(2));
  QCOMPARE(restored->groups[0].datasets[0].sourceId, 2);
  QCOMPARE(restored->groups[0].datasets[0].uniqueId, dataset_unique_id(2, 0, 0));
  QCOMPARE(restored->groups[0].datasets[1].uniqueId, dataset_unique_id(2, 0, 1));
}

QTEST_APPLESS_MAIN(TstFrameSerialization)

#include "tst_frame_serialization.moc"
