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

#include <QByteArray>
#include <QByteArrayView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QTest>

#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "SerialStudio.h"

// Every test function here is self-contained: each builds its own CFrameParser, so Qt Test's
// declaration-order execution is never load-bearing. Every load passes showMessageBoxes = false,
// which routes a load failure to qWarning() instead of Misc::Utilities::showMessageBox().

//--------------------------------------------------------------------------------------------------
// Shared inputs
//--------------------------------------------------------------------------------------------------

// The three frames the catalog-wide smoke test feeds to every template's text, UTF-8 and binary
// entry points. They are deliberately generic: the point is that no template aborts, over-reads or
// mis-sizes its output on a payload it was not designed for.
static const QString kSmokeText("temperature=25,humidity=40");
static const QByteArray kSmokeUtf8("temperature=25,humidity=40");
static const QByteArray kSmokeBinary = QByteArray::fromHex("0102030405060708090a0b0c0d0e0f10");

// Registry sizes as shipped: ten text templates, thirteen binary protocols, two multi-frame
// expanders. Adding a template is expected to update these numbers together with the catalog.
static constexpr qsizetype kTextTemplateCount   = 10;
static constexpr qsizetype kBinaryTemplateCount = 13;
static constexpr qsizetype kMultiTemplateCount  = 2;
static constexpr qsizetype kTotalTemplateCount =
  kTextTemplateCount + kBinaryTemplateCount + kMultiTemplateCount;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads a template descriptor into the parser with message boxes disabled.
 */
[[nodiscard]] static bool load(DataModel::CFrameParser& parser,
                               const QString& id,
                               const QJsonObject& params)
{
  return parser.loadScript(DataModel::CFrameParser::buildDescriptor(id, params), 0, false);
}

/**
 * @brief Returns the first frame of a parse result, or an empty row when nothing was produced.
 */
[[nodiscard]] static QStringList firstRow(const QList<QStringList>& frames)
{
  return frames.isEmpty() ? QStringList() : frames.constFirst();
}

/**
 * @brief Returns the schema entry for one parameter key, or an empty object when absent.
 */
[[nodiscard]] static QJsonObject paramEntry(const QJsonObject& schema, const QString& key)
{
  const auto params = schema.value(QStringLiteral("params")).toArray();
  for (const auto& value : params) {
    const auto entry = value.toObject();
    if (entry.value(QStringLiteral("key")).toString() == key)
      return entry;
  }

  return QJsonObject();
}

/**
 * @brief Appends a 32-bit little-endian integer to a byte buffer.
 */
static void appendI32Le(QByteArray& bytes, qint32 value)
{
  const auto raw = static_cast<quint32>(value);
  bytes.append(static_cast<char>(raw & 0xFF));
  bytes.append(static_cast<char>((raw >> 8) & 0xFF));
  bytes.append(static_cast<char>((raw >> 16) & 0xFF));
  bytes.append(static_cast<char>((raw >> 24) & 0xFF));
}

/**
 * @brief Returns true when the channel string parses to @p expected within 1e-3.
 */
[[nodiscard]] static bool nearly(const QString& text, double expected)
{
  return qAbs(text.toDouble() - expected) < 1e-3;
}

/**
 * @brief Contract of the Built-In (Native) frame parser: catalog, descriptor loading, and the
 *        per-template decode behavior of all twenty-five shipped templates.
 */
class TstCFrameParser : public QObject {
  Q_OBJECT

private slots:
  void catalogIsCompleteAndUnique();
  void catalogMatchesFamilyRegistries();
  void catalogEntriesCarryTranslatedMetadata();

  void schemaRoundTripsDelimitedDefaults();
  void schemaOfUnknownTemplateIsEmpty();
  void schemaCarriesEnumOptionsAndNumericBounds();
  void schemaOfParameterlessTemplateIsStillDescribed();

  void descriptorRoundTripsThroughLoadScript();
  void loadScriptRejectsNonJson();
  void loadScriptRejectsUnknownTemplateId();
  void loadScriptRejectsInvalidParameters();
  void successfulLoadClearsPreviousError();

  void delimitedSkipEmptyControlsFieldCount();
  void delimitedQuotingKeepsSeparatorInsideField();
  void delimitedResolvesEscapeSequencesInSeparator();
  void fixedWidthSlicesConfiguredWidths();
  void fixedWidthWithoutWidthsSplitsOnWhitespace();
  void keyValueParsesPairsAndLatchesMissingKeys();
  void iniConfigSkipsCommentsAndLatchesValues();
  void atCommandsRoutesResponseParameters();
  void nmea0183DecodesGgaSentence();
  void nmea0183RejectsCorruptChecksum();
  void urlEncodedDecodesPercentEscapes();
  void jsonDataExtractsFlatScalarFields();
  void xmlDataExtractsElementText();
  void yamlDataExtractsFlatKeys();

  void rawBytesHonorsEndianness();
  void rawBytesHonorsSignedFlag();
  void hexadecimalBytesMatchesRawBytesOnEquivalentInput();
  void base64DecodesKnownPayload();
  void binaryTlvRoutesTagsToChannels();
  void cobsRestoresStuffedZero();
  void slipResolvesEscapeSequences();
  void ubxDecodesNavPosLlh();
  void sirfRejectsTruncatedMessage();
  void mavlinkDecodesAttitude();
  void nmea2000DecodesPositionRapidUpdate();
  void rtcmRejectsFrameWithoutPreamble();
  void modbusDecodesHoldingRegisters();
  void messagePackDecodesFixArrayAndMap();

  void batchedSensorDataExpandsSamples();
  void batchedSensorRejectsEmptyVectorField();
  void timeSeries2dExpandsRecords();

  void spanFastLaneServesDelimitedOnly();
  void resetClearsLoadedState();
  void parsingBeforeLoadYieldsNothing();
  void languageIsNative();

  void everyTemplateLoadsWithDefaults_data();
  void everyTemplateLoadsWithDefaults();
};

//--------------------------------------------------------------------------------------------------
// Catalog
//--------------------------------------------------------------------------------------------------

/**
 * @brief The catalog is what the project editor lists; a duplicate id would make one template
 *        unreachable through nativeTemplateById(), which resolves the first match.
 */
void TstCFrameParser::catalogIsCompleteAndUnique()
{
  const auto catalog = DataModel::CFrameParser::templateCatalog();

  QVERIFY(!catalog.isEmpty());
  QCOMPARE(catalog.size(), kTotalTemplateCount);
  QCOMPARE(catalog.size(), DataModel::nativeTemplates().size());

  QStringList ids;
  for (const auto& value : catalog) {
    const QString id = value.toObject().value(QStringLiteral("id")).toString();
    QVERIFY(!id.isEmpty());
    QVERIFY(!ids.contains(id));
    QVERIFY(DataModel::nativeTemplateById(id) != nullptr);
    ids.append(id);
  }

  QCOMPARE(ids.size(), kTotalTemplateCount);
}

/**
 * @brief The registry is the concatenation of the three family lists, default template first.
 */
void TstCFrameParser::catalogMatchesFamilyRegistries()
{
  QCOMPARE(DataModel::textNativeTemplates().size(), kTextTemplateCount);
  QCOMPARE(DataModel::binaryNativeTemplates().size(), kBinaryTemplateCount);
  QCOMPARE(DataModel::multiFrameNativeTemplates().size(), kMultiTemplateCount);

  QCOMPARE(DataModel::defaultNativeTemplateId(), QStringLiteral("delimited"));

  const auto catalog = DataModel::CFrameParser::templateCatalog();
  QCOMPARE(catalog.constBegin()->toObject().value(QStringLiteral("id")).toString(),
           QStringLiteral("delimited"));
}

/**
 * @brief Every catalog row carries the three fields the template picker binds to.
 */
void TstCFrameParser::catalogEntriesCarryTranslatedMetadata()
{
  const auto catalog = DataModel::CFrameParser::templateCatalog();

  for (const auto& value : catalog) {
    const auto entry = value.toObject();
    QCOMPARE(entry.size(), 3);
    QVERIFY(!entry.value(QStringLiteral("id")).toString().isEmpty());
    QVERIFY(!entry.value(QStringLiteral("name")).toString().isEmpty());
    QVERIFY(!entry.value(QStringLiteral("description")).toString().isEmpty());
  }
}

//--------------------------------------------------------------------------------------------------
// Parameter schema
//--------------------------------------------------------------------------------------------------

/**
 * @brief The schema is the only source the parameter form reads, so every declared key must carry
 *        its type and default: a missing default leaves the form field empty on first open.
 */
void TstCFrameParser::schemaRoundTripsDelimitedDefaults()
{
  const auto schema = DataModel::CFrameParser::templateSchema(QStringLiteral("delimited"));

  QCOMPARE(schema.value(QStringLiteral("id")).toString(), QStringLiteral("delimited"));
  QVERIFY(!schema.value(QStringLiteral("name")).toString().isEmpty());
  QVERIFY(!schema.value(QStringLiteral("description")).toString().isEmpty());

  const auto params = schema.value(QStringLiteral("params")).toArray();
  QCOMPARE(params.size(), 4);

  const auto separator = paramEntry(schema, QStringLiteral("separator"));
  QCOMPARE(separator.value(QStringLiteral("type")).toString(), QStringLiteral("string"));
  QCOMPARE(separator.value(QStringLiteral("default")).toString(), QStringLiteral(","));

  const auto quote = paramEntry(schema, QStringLiteral("quoteChar"));
  QCOMPARE(quote.value(QStringLiteral("type")).toString(), QStringLiteral("char"));
  QCOMPARE(quote.value(QStringLiteral("default")).toString(), QString());

  const auto trim = paramEntry(schema, QStringLiteral("trimFields"));
  QCOMPARE(trim.value(QStringLiteral("type")).toString(), QStringLiteral("bool"));
  QCOMPARE(trim.value(QStringLiteral("default")).toBool(), false);

  const auto skip = paramEntry(schema, QStringLiteral("skipEmpty"));
  QCOMPARE(skip.value(QStringLiteral("type")).toString(), QStringLiteral("bool"));
  QCOMPARE(skip.value(QStringLiteral("default")).toBool(), false);

  const auto defaults =
    DataModel::nativeTemplateDefaults(*DataModel::nativeTemplateById(QStringLiteral("delimited")));
  QCOMPARE(defaults.keys().size(), params.size());
}

/**
 * @brief An unknown id yields an empty object rather than a partially-filled schema.
 */
void TstCFrameParser::schemaOfUnknownTemplateIsEmpty()
{
  QVERIFY(DataModel::CFrameParser::templateSchema(QStringLiteral("not_a_template")).isEmpty());
  QVERIFY(DataModel::CFrameParser::templateSchema(QString()).isEmpty());
  QVERIFY(DataModel::CFrameParser::templateSchema(QStringLiteral("DELIMITED")).isEmpty());
}

/**
 * @brief Enum parameters publish parallel value/label options and numeric ones publish bounds;
 *        the form spins and combo boxes have no other source for either.
 */
void TstCFrameParser::schemaCarriesEnumOptionsAndNumericBounds()
{
  const auto schema = DataModel::CFrameParser::templateSchema(QStringLiteral("raw_bytes"));

  const auto group = paramEntry(schema, QStringLiteral("bytesPerValue"));
  QCOMPARE(group.value(QStringLiteral("type")).toString(), QStringLiteral("int"));
  QCOMPARE(group.value(QStringLiteral("min")).toDouble(), 1.0);
  QCOMPARE(group.value(QStringLiteral("max")).toDouble(), 8.0);
  QCOMPARE(group.value(QStringLiteral("default")).toInt(), 1);

  const auto endian = paramEntry(schema, QStringLiteral("endianness"));
  QCOMPARE(endian.value(QStringLiteral("type")).toString(), QStringLiteral("enum"));

  const auto options = endian.value(QStringLiteral("options")).toArray();
  QCOMPARE(options.size(), 2);
  QCOMPARE(options.at(0).toObject().value(QStringLiteral("value")).toString(),
           QStringLiteral("big"));
  QCOMPARE(options.at(1).toObject().value(QStringLiteral("value")).toString(),
           QStringLiteral("little"));
  QVERIFY(!options.at(0).toObject().value(QStringLiteral("label")).toString().isEmpty());

  const auto sign = paramEntry(schema, QStringLiteral("signedValues"));
  QCOMPARE(sign.value(QStringLiteral("type")).toString(), QStringLiteral("bool"));
  QVERIFY(!sign.contains(QStringLiteral("min")));
}

/**
 * @brief A template with no parameters still returns its metadata plus an empty params array.
 */
void TstCFrameParser::schemaOfParameterlessTemplateIsStillDescribed()
{
  const auto schema = DataModel::CFrameParser::templateSchema(QStringLiteral("base64_encoded"));

  QVERIFY(!schema.isEmpty());
  QCOMPARE(schema.value(QStringLiteral("id")).toString(), QStringLiteral("base64_encoded"));
  QVERIFY(schema.value(QStringLiteral("params")).toArray().isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Descriptor loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief buildDescriptor() writes what loadScript() reads: the descriptor is persisted verbatim in
 *        project files, so the two must stay each other's inverse.
 */
void TstCFrameParser::descriptorRoundTripsThroughLoadScript()
{
  QJsonObject params;
  params.insert(QStringLiteral("separator"), QStringLiteral(";"));

  const QString text =
    DataModel::CFrameParser::buildDescriptor(QStringLiteral("delimited"), params);
  const auto doc = QJsonDocument::fromJson(text.toUtf8());

  QVERIFY(doc.isObject());
  QCOMPARE(doc.object().value(QStringLiteral("template")).toString(), QStringLiteral("delimited"));
  QCOMPARE(doc.object().value(QStringLiteral("params")).toObject(), params);

  DataModel::CFrameParser parser;
  QVERIFY(parser.loadScript(text, 0, false));
  QVERIFY(parser.isLoaded());
  QCOMPARE(parser.templateId(), QStringLiteral("delimited"));
  QVERIFY(parser.lastError().isEmpty());
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("1;2;3"))).size(), 3);
}

/**
 * @brief A descriptor that is not a JSON object fails with a stored error and no parser.
 */
void TstCFrameParser::loadScriptRejectsNonJson()
{
  DataModel::CFrameParser parser;

  QVERIFY(!parser.loadScript(QStringLiteral("return [1, 2, 3];"), 0, false));
  QVERIFY(!parser.isLoaded());
  QVERIFY(!parser.lastError().isEmpty());
  QVERIFY(parser.templateId().isEmpty());

  QVERIFY(!parser.loadScript(QStringLiteral("[1, 2, 3]"), 0, false));
  QVERIFY(!parser.isLoaded());
  QVERIFY(!parser.lastError().isEmpty());
}

/**
 * @brief An unknown template id names itself in the error, so a project written by a newer build
 *        reports which template is missing instead of failing anonymously.
 */
void TstCFrameParser::loadScriptRejectsUnknownTemplateId()
{
  DataModel::CFrameParser parser;

  QVERIFY(!load(parser, QStringLiteral("ancient_template"), QJsonObject()));
  QVERIFY(!parser.isLoaded());
  QVERIFY(parser.lastError().contains(QStringLiteral("ancient_template")));
  QVERIFY(parser.lastError().contains(QStringLiteral("Unknown Built-In parser template")));

  QVERIFY(!parser.loadScript(QStringLiteral("{\"params\":{}}"), 0, false));
  QVERIFY(!parser.isLoaded());
}

/**
 * @brief Parameter validation lives in makeParser(); a rejected combination must leave the engine
 *        unloaded with the template's own message, not a generic one.
 */
void TstCFrameParser::loadScriptRejectsInvalidParameters()
{
  DataModel::CFrameParser parser;

  QJsonObject clash;
  clash.insert(QStringLiteral("separator"), QStringLiteral(","));
  clash.insert(QStringLiteral("quoteChar"), QStringLiteral(","));
  QVERIFY(!load(parser, QStringLiteral("delimited"), clash));
  QVERIFY(!parser.isLoaded());
  QCOMPARE(parser.lastError(),
           QStringLiteral("The quote character must differ from the field separator."));

  QJsonObject empty;
  empty.insert(QStringLiteral("separator"), QString());
  QVERIFY(!load(parser, QStringLiteral("delimited"), empty));
  QCOMPARE(parser.lastError(), QStringLiteral("The separator must not be empty."));

  QJsonObject group;
  group.insert(QStringLiteral("bytesPerValue"), 9);
  QVERIFY(!load(parser, QStringLiteral("raw_bytes"), group));
  QCOMPARE(parser.lastError(), QStringLiteral("Bytes per value must be between 1 and 8."));

  QJsonObject widths;
  widths.insert(QStringLiteral("widths"), QStringLiteral("4,zero"));
  QVERIFY(!load(parser, QStringLiteral("fixed_width"), widths));
  QCOMPARE(parser.lastError(), QStringLiteral("Column widths must be positive integers."));
}

/**
 * @brief A later successful load clears the stored error so the problem center stops reporting it.
 */
void TstCFrameParser::successfulLoadClearsPreviousError()
{
  DataModel::CFrameParser parser;

  QVERIFY(!load(parser, QStringLiteral("no_such_template"), QJsonObject()));
  QVERIFY(!parser.lastError().isEmpty());

  QVERIFY(load(parser, QStringLiteral("delimited"), QJsonObject()));
  QVERIFY(parser.lastError().isEmpty());
  QVERIFY(parser.isLoaded());
  QCOMPARE(parser.templateId(), QStringLiteral("delimited"));
}

//--------------------------------------------------------------------------------------------------
// Text templates
//--------------------------------------------------------------------------------------------------

/**
 * @brief skipEmpty decides whether an empty field is a channel: "a,b,,c" is four channels with a
 *        hole, or three channels shifted left. Both readings ship; the flag picks one.
 */
void TstCFrameParser::delimitedSkipEmptyControlsFieldCount()
{
  DataModel::CFrameParser keep;
  QVERIFY(load(keep, QStringLiteral("delimited"), QJsonObject()));

  const auto kept = firstRow(keep.parseString(QStringLiteral("a,b,,c")));
  QCOMPARE(kept.size(), 4);
  QCOMPARE(kept,
           QStringList({QStringLiteral("a"), QStringLiteral("b"), QString(), QStringLiteral("c")}));

  QJsonObject params;
  params.insert(QStringLiteral("skipEmpty"), true);

  DataModel::CFrameParser drop;
  QVERIFY(load(drop, QStringLiteral("delimited"), params));

  const auto dropped = firstRow(drop.parseString(QStringLiteral("a,b,,c")));
  QCOMPARE(dropped.size(), 3);
  QCOMPARE(dropped, QStringList({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}));
}

/**
 * @brief A separator inside quotes is data, and a doubled quote is a literal quote.
 */
void TstCFrameParser::delimitedQuotingKeepsSeparatorInsideField()
{
  QJsonObject params;
  params.insert(QStringLiteral("quoteChar"), QStringLiteral("\""));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("delimited"), params));

  const auto row = firstRow(parser.parseString(QStringLiteral("a,\"b,c\",d")));
  QCOMPARE(row.size(), 3);
  QCOMPARE(row.at(1), QStringLiteral("b,c"));

  const auto escaped = firstRow(parser.parseString(QStringLiteral("\"a\"\"b\",c")));
  QCOMPARE(escaped.size(), 2);
  QCOMPARE(escaped.at(0), QStringLiteral("a\"b"));
}

/**
 * @brief Separators are stored as text, so a tab is written "\t" in the project file and has to be
 *        resolved before the split.
 */
void TstCFrameParser::delimitedResolvesEscapeSequencesInSeparator()
{
  QJsonObject params;
  params.insert(QStringLiteral("separator"), QStringLiteral("\\t"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("delimited"), params));

  const auto row = firstRow(parser.parseString(QStringLiteral("1\t2\t3")));
  QCOMPARE(row, QStringList({QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}));
}

/**
 * @brief A width list slices by character count and trims the padding away.
 */
void TstCFrameParser::fixedWidthSlicesConfiguredWidths()
{
  QJsonObject params;
  params.insert(QStringLiteral("widths"), QStringLiteral("3,3,4"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("fixed_width"), params));

  const auto row = firstRow(parser.parseString(QStringLiteral("ab cd 1234")));
  QCOMPARE(row.size(), 3);
  QCOMPARE(row.at(0), QStringLiteral("ab"));
  QCOMPARE(row.at(1), QStringLiteral("cd"));
  QCOMPARE(row.at(2), QStringLiteral("1234"));
}

/**
 * @brief An empty width list degrades to a whitespace split, collapsing runs.
 */
void TstCFrameParser::fixedWidthWithoutWidthsSplitsOnWhitespace()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("fixed_width"), QJsonObject()));

  const auto row = firstRow(parser.parseString(QStringLiteral("1   2\t3")));
  QCOMPARE(row, QStringList({QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}));
}

/**
 * @brief Key-value is a latching template: a key absent from this frame keeps the value the last
 *        frame gave it, which is what keeps a sparse device from blanking its dashboard.
 */
void TstCFrameParser::keyValueParsesPairsAndLatchesMissingKeys()
{
  QJsonObject params;
  params.insert(QStringLiteral("keys"), QStringLiteral("a,b"));
  params.insert(QStringLiteral("pairSeparator"), QStringLiteral(";"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("key_value"), params));

  QCOMPARE(firstRow(parser.parseString(QStringLiteral("a=1;b=2"))),
           QStringList({QStringLiteral("1"), QStringLiteral("2")}));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("a=5"))),
           QStringList({QStringLiteral("5"), QStringLiteral("2")}));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("a=hello"))),
           QStringList({QStringLiteral("5"), QStringLiteral("2")}));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("unmapped=9"))),
           QStringList({QStringLiteral("5"), QStringLiteral("2")}));
}

/**
 * @brief INI comment markers are skipped and surrounding whitespace is dropped.
 */
void TstCFrameParser::iniConfigSkipsCommentsAndLatchesValues()
{
  QJsonObject params;
  params.insert(QStringLiteral("keys"), QStringLiteral("alpha,beta"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("ini_config"), params));

  const auto row =
    firstRow(parser.parseString(QStringLiteral("; note\nalpha = 1\n# skip\nbeta=2\n")));
  QCOMPARE(row, QStringList({QStringLiteral("1"), QStringLiteral("2")}));

  QCOMPARE(firstRow(parser.parseString(QStringLiteral("beta=7"))),
           QStringList({QStringLiteral("1"), QStringLiteral("7")}));
}

/**
 * @brief The routing table maps a modem response name onto its channel indices.
 */
void TstCFrameParser::atCommandsRoutesResponseParameters()
{
  QJsonObject params;
  params.insert(QStringLiteral("commands"), QStringLiteral("CSQ:0,1;CREG:2"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("at_commands"), params));

  QCOMPARE(firstRow(parser.parseString(QStringLiteral("+CSQ: 25,99"))),
           QStringList({QStringLiteral("25"), QStringLiteral("99"), QStringLiteral("0")}));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("+CREG: 1"))),
           QStringList({QStringLiteral("25"), QStringLiteral("99"), QStringLiteral("1")}));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("OK"))),
           QStringList({QStringLiteral("25"), QStringLiteral("99"), QStringLiteral("1")}));
}

/**
 * @brief GGA carries position as DDMM.MMMM plus a hemisphere; the template converts to signed
 *        decimal degrees, which is the only form the GPS widget accepts.
 */
void TstCFrameParser::nmea0183DecodesGgaSentence()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("nmea_0183"), QJsonObject()));

  const auto row = firstRow(parser.parseString(
    QStringLiteral("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47")));

  QCOMPARE(row.size(), 16);
  QVERIFY(nearly(row.at(0), 48.1173));
  QVERIFY(nearly(row.at(1), 11.51667));
  QVERIFY(nearly(row.at(2), 545.4));
  QVERIFY(nearly(row.at(3), 1.0));
  QVERIFY(nearly(row.at(4), 8.0));
  QVERIFY(nearly(row.at(5), 0.9));
}

/**
 * @brief A sentence failing its XOR checksum must not update any channel.
 */
void TstCFrameParser::nmea0183RejectsCorruptChecksum()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("nmea_0183"), QJsonObject()));

  const auto row = firstRow(parser.parseString(
    QStringLiteral("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*00")));

  QCOMPARE(row.size(), 16);
  QCOMPARE(row.at(0), QStringLiteral("0"));
  QCOMPARE(row.at(1), QStringLiteral("0"));
}

/**
 * @brief Percent escapes and '+' both decode before the key lookup.
 */
void TstCFrameParser::urlEncodedDecodesPercentEscapes()
{
  QJsonObject params;
  params.insert(QStringLiteral("keys"), QStringLiteral("temp,name"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("url_encoded"), params));

  QCOMPARE(firstRow(parser.parseString(QStringLiteral("temp=25&name=a%20b"))),
           QStringList({QStringLiteral("25"), QStringLiteral("a b")}));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("name=c+d"))),
           QStringList({QStringLiteral("25"), QStringLiteral("c d")}));
}

/**
 * @brief Numbers, strings and booleans map to channel text; a container keeps the latched value.
 */
void TstCFrameParser::jsonDataExtractsFlatScalarFields()
{
  QJsonObject params;
  params.insert(QStringLiteral("fields"), QStringLiteral("a,b,c,d"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("json_data"), params));

  const auto row =
    firstRow(parser.parseString(QStringLiteral("{\"a\":1.5,\"b\":\"x\",\"c\":true,\"d\":[1,2]}")));

  QCOMPARE(row.size(), 4);
  QCOMPARE(row.at(0), QStringLiteral("1.5"));
  QCOMPARE(row.at(1), QStringLiteral("x"));
  QCOMPARE(row.at(2), QStringLiteral("true"));
  QCOMPARE(row.at(3), QStringLiteral("0"));

  QCOMPARE(firstRow(parser.parseUtf8(QByteArray("{\"a\":2}"))).at(0), QStringLiteral("2"));
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("not json"))).at(0), QStringLiteral("2"));
}

/**
 * @brief The XML template is a text scanner, not a parser: it takes the first <tag>...</tag> span.
 */
void TstCFrameParser::xmlDataExtractsElementText()
{
  QJsonObject params;
  params.insert(QStringLiteral("tags"), QStringLiteral("temp,hum"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("xml_data"), params));

  const auto row =
    firstRow(parser.parseString(QStringLiteral("<root><temp> 25 </temp><hum>40</hum></root>")));

  QCOMPARE(row, QStringList({QStringLiteral("25"), QStringLiteral("40")}));
}

/**
 * @brief Flat YAML lines: inline comments are cut, quotes stripped, and yes/no normalized.
 */
void TstCFrameParser::yamlDataExtractsFlatKeys()
{
  QJsonObject params;
  params.insert(QStringLiteral("keys"), QStringLiteral("temp,flag,name"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("yaml_data"), params));

  const auto row = firstRow(
    parser.parseString(QStringLiteral("---\ntemp: 25 # degrees\nflag: yes\nname: \"bob\"\n")));

  QCOMPARE(row, QStringList({QStringLiteral("25"), QStringLiteral("true"), QStringLiteral("bob")}));

  QCOMPARE(firstRow(parser.parseString(QStringLiteral("flag: off"))).at(1),
           QStringLiteral("false"));
}

//--------------------------------------------------------------------------------------------------
// Binary templates
//--------------------------------------------------------------------------------------------------

/**
 * @brief The same four bytes are two different pairs of values depending on byte order; getting
 *        this backwards is silent, so both directions are pinned to known values.
 */
void TstCFrameParser::rawBytesHonorsEndianness()
{
  const auto frame = QByteArray::fromHex("01020304");

  QJsonObject big;
  big.insert(QStringLiteral("bytesPerValue"), 2);
  big.insert(QStringLiteral("endianness"), QStringLiteral("big"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("raw_bytes"), big));
  QCOMPARE(firstRow(parser.parseBinary(frame)),
           QStringList({QStringLiteral("258"), QStringLiteral("772")}));

  QJsonObject little;
  little.insert(QStringLiteral("bytesPerValue"), 2);
  little.insert(QStringLiteral("endianness"), QStringLiteral("little"));

  DataModel::CFrameParser swapped;
  QVERIFY(load(swapped, QStringLiteral("raw_bytes"), little));
  QCOMPARE(firstRow(swapped.parseBinary(frame)),
           QStringList({QStringLiteral("513"), QStringLiteral("1027")}));
}

/**
 * @brief The sign flag reinterprets the same bits as two's complement; trailing partial groups are
 *        dropped rather than zero-padded.
 */
void TstCFrameParser::rawBytesHonorsSignedFlag()
{
  QJsonObject unsigned_bytes;
  unsigned_bytes.insert(QStringLiteral("bytesPerValue"), 1);

  DataModel::CFrameParser plain;
  QVERIFY(load(plain, QStringLiteral("raw_bytes"), unsigned_bytes));
  QCOMPARE(firstRow(plain.parseBinary(QByteArray::fromHex("ff7f"))),
           QStringList({QStringLiteral("255"), QStringLiteral("127")}));

  QJsonObject signed_words;
  signed_words.insert(QStringLiteral("bytesPerValue"), 2);
  signed_words.insert(QStringLiteral("signedValues"), true);

  DataModel::CFrameParser signed_parser;
  QVERIFY(load(signed_parser, QStringLiteral("raw_bytes"), signed_words));
  QCOMPARE(firstRow(signed_parser.parseBinary(QByteArray::fromHex("fffe007b"))),
           QStringList({QStringLiteral("-2"), QStringLiteral("123")}));
  QCOMPARE(firstRow(signed_parser.parseBinary(QByteArray::fromHex("fffe00"))),
           QStringList({QStringLiteral("-2")}));
}

/**
 * @brief The hex template must agree byte for byte with the raw template: they differ only in the
 *        decoder that feeds them.
 */
void TstCFrameParser::hexadecimalBytesMatchesRawBytesOnEquivalentInput()
{
  QJsonObject params;
  params.insert(QStringLiteral("bytesPerValue"), 2);
  params.insert(QStringLiteral("endianness"), QStringLiteral("little"));

  DataModel::CFrameParser hex;
  QVERIFY(load(hex, QStringLiteral("hexadecimal_bytes"), params));

  DataModel::CFrameParser raw;
  QVERIFY(load(raw, QStringLiteral("raw_bytes"), params));

  const auto expected = raw.parseBinary(QByteArray::fromHex("0102fffe"));
  QCOMPARE(hex.parseString(QStringLiteral("01 02 FF FE")), expected);
  QCOMPARE(hex.parseUtf8(QByteArray("0102fffe")), expected);
  QCOMPARE(firstRow(expected), QStringList({QStringLiteral("513"), QStringLiteral("65279")}));
}

/**
 * @brief Base64 decodes to one decimal channel per byte.
 */
void TstCFrameParser::base64DecodesKnownPayload()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("base64_encoded"), QJsonObject()));

  const QByteArray payload = QByteArray::fromHex("0102030f");
  QCOMPARE(payload.toBase64(), QByteArray("AQIDDw=="));

  const QStringList expected(
    {QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"), QStringLiteral("15")});
  QCOMPARE(firstRow(parser.parseString(QStringLiteral("AQIDDw=="))), expected);
  QCOMPARE(firstRow(parser.parseUtf8(QByteArray("AQIDDw=="))), expected);
  QCOMPARE(firstRow(parser.parseBinary(QByteArray("AQIDDw=="))), expected);
}

/**
 * @brief TLV walks tag/length/value triplets and routes only the mapped tags.
 */
void TstCFrameParser::binaryTlvRoutesTagsToChannels()
{
  QJsonObject params;
  params.insert(QStringLiteral("tagMap"), QStringLiteral("1:0,2:1"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("binary_tlv"), params));

  const auto row = firstRow(parser.parseBinary(QByteArray::fromHex("01012a02020100")));
  QCOMPARE(row, QStringList({QStringLiteral("42"), QStringLiteral("256")}));

  QCOMPARE(firstRow(parser.parseBinary(QByteArray::fromHex("09010f"))), row);
}

/**
 * @brief COBS removes the stuffing byte and puts the zero back where it belonged.
 */
void TstCFrameParser::cobsRestoresStuffedZero()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("cobs_encoded"), QJsonObject()));

  const auto row = firstRow(parser.parseBinary(QByteArray::fromHex("0311220233")));
  QCOMPARE(
    row,
    QStringList(
      {QStringLiteral("17"), QStringLiteral("34"), QStringLiteral("0"), QStringLiteral("51")}));
}

/**
 * @brief SLIP restores the END and ESC bytes their escape pairs stand for.
 */
void TstCFrameParser::slipResolvesEscapeSequences()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("slip_encoded"), QJsonObject()));

  const auto row = firstRow(parser.parseBinary(QByteArray::fromHex("c001dbdcdbdd02c0")));
  QCOMPARE(
    row,
    QStringList(
      {QStringLiteral("1"), QStringLiteral("192"), QStringLiteral("219"), QStringLiteral("2")}));
}

/**
 * @brief NAV-POSLLH scales the raw integers into degrees and metres; the sync bytes are stripped
 *        by the frame delimiter, so the template sees the message from its class byte onward.
 */
void TstCFrameParser::ubxDecodesNavPosLlh()
{
  QJsonObject params;
  params.insert(QStringLiteral("validateChecksum"), false);

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("ubx_ublox"), params));

  QByteArray frame = QByteArray::fromHex("01021c00");
  appendI32Le(frame, 0);
  appendI32Le(frame, 113100000);
  appendI32Le(frame, 480703800);
  appendI32Le(frame, 100000);
  appendI32Le(frame, 95000);
  appendI32Le(frame, 0);
  appendI32Le(frame, 0);
  frame.append(QByteArray::fromHex("0000"));

  const auto row = firstRow(parser.parseBinary(frame));
  QCOMPARE(row.size(), 20);
  QVERIFY(nearly(row.at(0), 48.07038));
  QVERIFY(nearly(row.at(1), 11.31));
  QVERIFY(nearly(row.at(2), 95.0));
  QVERIFY(nearly(row.at(3), 100.0));
}

/**
 * @brief A SiRF message whose declared length runs past the frame updates nothing.
 */
void TstCFrameParser::sirfRejectsTruncatedMessage()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("sirf_binary"), QJsonObject()));

  const auto row = firstRow(parser.parseBinary(QByteArray::fromHex("00ff2900000000")));
  QCOMPARE(row.size(), 20);
  for (const auto& value : row)
    QCOMPARE(value, QStringLiteral("0"));
}

/**
 * @brief ATTITUDE carries three little-endian floats; MAVLink v1 is keyed on the 0xFE marker, so a
 *        v2 frame handed to a v1 parser must be ignored rather than misread.
 */
void TstCFrameParser::mavlinkDecodesAttitude()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("mavlink"), QJsonObject()));

  QByteArray frame = QByteArray::fromHex("fe100000001e");
  frame.append(QByteArray::fromHex("00000000"));
  frame.append(QByteArray::fromHex("0000803f"));
  frame.append(QByteArray::fromHex("00000040"));
  frame.append(QByteArray::fromHex("000080bf"));

  const auto row = firstRow(parser.parseBinary(frame));
  QCOMPARE(row.size(), 16);
  QVERIFY(nearly(row.at(0), 1.0));
  QVERIFY(nearly(row.at(1), 2.0));
  QVERIFY(nearly(row.at(2), -1.0));

  QByteArray v2      = frame;
  v2[0]              = static_cast<char>(0xFD);
  const auto ignored = firstRow(parser.parseBinary(v2));
  QCOMPARE(ignored, row);
}

/**
 * @brief PGN 129025 (position rapid update) is extracted from the 29-bit CAN identifier that the
 *        preprocessed frame carries in its first four bytes.
 */
void TstCFrameParser::nmea2000DecodesPositionRapidUpdate()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("nmea_2000"), QJsonObject()));

  QByteArray frame;
  appendI32Le(frame, 0x09F80100);
  frame.append(static_cast<char>(8));
  appendI32Le(frame, 480703800);
  appendI32Le(frame, 113100000);

  const auto row = firstRow(parser.parseBinary(frame));
  QCOMPARE(row.size(), 20);
  QVERIFY(nearly(row.at(3), 48.07038));
  QVERIFY(nearly(row.at(4), 11.31));
}

/**
 * @brief RTCM3 frames are identified by the 0xD3 preamble; anything else leaves the channels as
 *        they were.
 */
void TstCFrameParser::rtcmRejectsFrameWithoutPreamble()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("rtcm_corrections"), QJsonObject()));

  const auto row = firstRow(parser.parseBinary(QByteArray::fromHex("d200060102030405")));
  QCOMPARE(row.size(), 16);
  for (const auto& value : row)
    QCOMPARE(value, QStringLiteral("0"));
}

/**
 * @brief A read-holding-registers response yields one channel per 16-bit big-endian register, and
 *        the unfilled channels keep their latched value.
 */
void TstCFrameParser::modbusDecodesHoldingRegisters()
{
  QJsonObject params;
  params.insert(QStringLiteral("numItems"), 4);

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("modbus"), params));

  const auto row = firstRow(parser.parseBinary(QByteArray::fromHex("010304000a0014")));
  QCOMPARE(
    row,
    QStringList(
      {QStringLiteral("10"), QStringLiteral("20"), QStringLiteral("0"), QStringLiteral("0")}));

  const auto error = firstRow(parser.parseBinary(QByteArray::fromHex("018302")));
  QCOMPARE(error, row);
}

/**
 * @brief Array mode emits every element in order; map mode routes keys through the key list and
 *        latches the keys the packet omits.
 */
void TstCFrameParser::messagePackDecodesFixArrayAndMap()
{
  DataModel::CFrameParser array;
  QVERIFY(load(array, QStringLiteral("messagepack"), QJsonObject()));
  QCOMPARE(firstRow(array.parseBinary(QByteArray::fromHex("93010203"))),
           QStringList({QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}));

  QJsonObject params;
  params.insert(QStringLiteral("mode"), QStringLiteral("map"));
  params.insert(QStringLiteral("keys"), QStringLiteral("a,b"));

  DataModel::CFrameParser map;
  QVERIFY(load(map, QStringLiteral("messagepack"), params));

  QByteArray frame = QByteArray::fromHex("82");
  frame.append(QByteArray::fromHex("a1")).append('a').append(QByteArray::fromHex("07"));
  frame.append(QByteArray::fromHex("a1")).append('b').append(QByteArray::fromHex("08"));

  QCOMPARE(firstRow(map.parseBinary(frame)),
           QStringList({QStringLiteral("7"), QStringLiteral("8")}));
}

//--------------------------------------------------------------------------------------------------
// Multi-frame templates
//--------------------------------------------------------------------------------------------------

/**
 * @brief One packet becomes N frames: the scalar metadata repeats and the sample lands last. This
 *        is the only template family that returns more than one row per call.
 */
void TstCFrameParser::batchedSensorDataExpandsSamples()
{
  QJsonObject params;
  params.insert(QStringLiteral("scalarFields"), QStringLiteral("device_id,battery"));
  params.insert(QStringLiteral("vectorField"), QStringLiteral("samples"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("batched_sensor_data"), params));

  const auto frames = parser.parseString(
    QStringLiteral("{\"device_id\":\"n1\",\"battery\":97,\"samples\":[10,11,12]}"));

  QCOMPARE(frames.size(), 3);
  for (qsizetype i = 0; i < frames.size(); ++i) {
    QCOMPARE(frames.at(i).size(), 3);
    QCOMPARE(frames.at(i).at(0), QStringLiteral("n1"));
    QCOMPARE(frames.at(i).at(1), QStringLiteral("97"));
  }

  QCOMPARE(frames.at(0).at(2), QStringLiteral("10"));
  QCOMPARE(frames.at(2).at(2), QStringLiteral("12"));

  const auto empty = parser.parseString(QStringLiteral("{\"device_id\":\"n1\",\"battery\":97}"));
  QCOMPARE(empty.size(), 1);
  QCOMPARE(empty.constFirst().size(), 2);

  QVERIFY(parser.parseString(QStringLiteral("not json")).isEmpty());
}

/**
 * @brief Without a sample-array field the template has nothing to expand, so the load is refused.
 */
void TstCFrameParser::batchedSensorRejectsEmptyVectorField()
{
  QJsonObject params;
  params.insert(QStringLiteral("vectorField"), QStringLiteral("   "));

  DataModel::CFrameParser parser;
  QVERIFY(!load(parser, QStringLiteral("batched_sensor_data"), params));
  QVERIFY(!parser.isLoaded());
  QCOMPARE(parser.lastError(), QStringLiteral("The sample array field must not be empty."));
}

/**
 * @brief Each record becomes one frame with its values ordered by the field list; a field the
 *        record omits becomes "0" rather than shifting the remaining channels left.
 */
void TstCFrameParser::timeSeries2dExpandsRecords()
{
  QJsonObject params;
  params.insert(QStringLiteral("recordsField"), QStringLiteral("records"));
  params.insert(QStringLiteral("fields"), QStringLiteral("t,v"));

  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("time_series_2d"), params));

  const auto frames = parser.parseString(
    QStringLiteral("{\"records\":[{\"t\":1,\"v\":10},{\"t\":2,\"v\":20},{\"t\":3}]}"));

  QCOMPARE(frames.size(), 3);
  QCOMPARE(frames.at(0), QStringList({QStringLiteral("1"), QStringLiteral("10")}));
  QCOMPARE(frames.at(1), QStringList({QStringLiteral("2"), QStringLiteral("20")}));
  QCOMPARE(frames.at(2), QStringList({QStringLiteral("3"), QStringLiteral("0")}));

  QVERIFY(parser.parseString(QStringLiteral("{\"records\":[]}")).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Engine contract
//--------------------------------------------------------------------------------------------------

/**
 * @brief The span fast lane is the allocation-free hotpath: only the unquoted delimited splitter
 *        can satisfy it, and every other template must return -1 so the caller falls back instead
 *        of reading an uninitialized span array.
 */
void TstCFrameParser::spanFastLaneServesDelimitedOnly()
{
  QByteArrayView spans[8];

  DataModel::CFrameParser delimited;
  QVERIFY(load(delimited, QStringLiteral("delimited"), QJsonObject()));

  const auto count = delimited.parseUtf8Spans(QByteArrayView("1,22,333"), spans, 8);
  QCOMPARE(count, qsizetype(3));
  QCOMPARE(spans[0].toByteArray(), QByteArray("1"));
  QCOMPARE(spans[1].toByteArray(), QByteArray("22"));
  QCOMPARE(spans[2].toByteArray(), QByteArray("333"));

  QJsonObject quoted;
  quoted.insert(QStringLiteral("quoteChar"), QStringLiteral("\""));

  DataModel::CFrameParser quoting;
  QVERIFY(load(quoting, QStringLiteral("delimited"), quoted));
  QCOMPARE(quoting.parseUtf8Spans(QByteArrayView("1,2"), spans, 8), qsizetype(-1));

  DataModel::CFrameParser raw;
  QVERIFY(load(raw, QStringLiteral("raw_bytes"), QJsonObject()));
  QCOMPARE(raw.parseUtf8Spans(QByteArrayView("12"), spans, 8), qsizetype(-1));

  DataModel::CFrameParser json;
  QVERIFY(load(json, QStringLiteral("json_data"), QJsonObject()));
  QCOMPARE(json.parseUtf8Spans(QByteArrayView("{}"), spans, 8), qsizetype(-1));
}

/**
 * @brief reset() drops the configured template, which is what a source reconfiguration relies on:
 *        a stale parser would keep decoding the previous project's layout.
 */
void TstCFrameParser::resetClearsLoadedState()
{
  DataModel::CFrameParser parser;
  QVERIFY(load(parser, QStringLiteral("delimited"), QJsonObject()));
  QVERIFY(parser.isLoaded());

  parser.collectGarbage();
  QVERIFY(parser.isLoaded());

  parser.reset();

  QVERIFY(!parser.isLoaded());
  QVERIFY(parser.templateId().isEmpty());
  QVERIFY(parser.lastError().isEmpty());
  QVERIFY(parser.parseString(QStringLiteral("1,2,3")).isEmpty());

  QVERIFY(load(parser, QStringLiteral("delimited"), QJsonObject()));
  QVERIFY(parser.isLoaded());
}

/**
 * @brief A parse before any load returns nothing instead of dereferencing the null parser.
 */
void TstCFrameParser::parsingBeforeLoadYieldsNothing()
{
  DataModel::CFrameParser parser;

  QVERIFY(!parser.isLoaded());
  QVERIFY(parser.templateId().isEmpty());
  QVERIFY(parser.lastError().isEmpty());
  QVERIFY(!parser.disabled());
  QCOMPARE(parser.errorCount(), quint64(0));

  QVERIFY(parser.parseString(QStringLiteral("1,2,3")).isEmpty());
  QVERIFY(parser.parseUtf8(QByteArray("1,2,3")).isEmpty());
  QVERIFY(parser.parseBinary(QByteArray::fromHex("0102")).isEmpty());

  QByteArrayView spans[4];
  QCOMPARE(parser.parseUtf8Spans(QByteArrayView("1,2"), spans, 4), qsizetype(-1));
}

/**
 * @brief The engine reports the Native language id, which is how the frame parser picks it.
 */
void TstCFrameParser::languageIsNative()
{
  DataModel::CFrameParser parser;

  QCOMPARE(parser.language(), static_cast<int>(SerialStudio::Native));
}

//--------------------------------------------------------------------------------------------------
// Catalog-wide smoke
//--------------------------------------------------------------------------------------------------

void TstCFrameParser::everyTemplateLoadsWithDefaults_data()
{
  QTest::addColumn<QString>("id");

  const auto catalog = DataModel::CFrameParser::templateCatalog();
  for (const auto& value : catalog) {
    const QString id = value.toObject().value(QStringLiteral("id")).toString();
    QTest::newRow(id.toUtf8().constData()) << id;
  }
}

/**
 * @brief Every shipped template must accept its own published defaults and survive all three
 *        entry points on a payload it was not designed for: the project editor loads a template
 *        with exactly these defaults the moment the user selects it, before any device is open.
 */
void TstCFrameParser::everyTemplateLoadsWithDefaults()
{
  QFETCH(QString, id);

  const auto* tmpl = DataModel::nativeTemplateById(id);
  QVERIFY(tmpl != nullptr);

  const auto defaults  = DataModel::nativeTemplateDefaults(*tmpl);
  const QString script = DataModel::CFrameParser::buildDescriptor(id, defaults);

  DataModel::CFrameParser parser;
  QVERIFY(parser.loadScript(script, 0, false));
  QVERIFY(parser.isLoaded());
  QCOMPARE(parser.templateId(), id);
  QVERIFY(parser.lastError().isEmpty());

  const QList<QList<QStringList>> results({parser.parseString(kSmokeText),
                                           parser.parseUtf8(kSmokeUtf8),
                                           parser.parseBinary(kSmokeBinary)});
  for (const auto& frames : results) {
    QVERIFY(frames.size() <= 10000);
    for (const auto& row : frames)
      QVERIFY(row.size() <= 65536);
  }

  QVERIFY(parser.isLoaded());
  QVERIFY(parser.lastError().isEmpty());

  parser.reset();
  QVERIFY(!parser.isLoaded());
}

QTEST_GUILESS_MAIN(TstCFrameParser)

#include "tst_cframe_parser.moc"
