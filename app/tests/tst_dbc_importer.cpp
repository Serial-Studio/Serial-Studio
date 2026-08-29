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

#include <algorithm>
#include <QCanDbcFileParser>
#include <QCanMessageDescription>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QTest>

#include "DataModel/Frame.h"
#include "DataModel/Importers/DBCImporter.h"
#include "SessionContext.h"

// The seam is projectFromMessages(): it reaches no session state, so the whole generation path
// runs on a stack SessionContext with no composition root and no project model. Everything below
// reads the returned JSON, which is also what the app writes to disk.
//
// Qt keeps a message's signals in a QHash, so signalDescriptions() hands them back in an order
// that changes between processes. Nothing here compares whole documents: the spec lines are
// checked as a sorted set, and the ordering rules as relative positions. That non-determinism is
// exactly why the importer orders selectors before their dependents topologically instead of
// trusting declaration order.

//--------------------------------------------------------------------------------------------------
// Fixture paths
//--------------------------------------------------------------------------------------------------

#ifndef SS_DBC_FIXTURE_DIR
#  define SS_DBC_FIXTURE_DIR "."
#endif

/**
 * @brief Extended and simple multiplexing in the DBC importer's generated project (spec 0073).
 */
class TstDbcImporter : public QObject {
  Q_OBJECT

private slots:
  void extendedFixtureSkipsNothing();
  void selectorsPrecedeTheirDependents();
  void extendedRangesReachTheLuaSpec();
  void datasetTitlesCarryTheSwitchValues();
  void generatedLuaMatchesRangesAtRuntime();
  void generatedLuaStaysWithinLua51();
  void simpleMuxKeepsTheScalarForm();
  void simpleMuxSpecMatchesTheStoredExpectation();

private:
  [[nodiscard]] QJsonObject project(const QString& fixture);
  [[nodiscard]] static QStringList specLines(const QString& lua);
  [[nodiscard]] static QString specLineFor(const QString& lua, const QString& signal);
  [[nodiscard]] static QStringList datasetTitles(const QJsonObject& project);
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses one fixture DBC and returns the project the importer generates from it, sorting
 *        the messages by id the way showPreview() does so group order is deterministic.
 */
QJsonObject TstDbcImporter::project(const QString& fixture)
{
  QCanDbcFileParser parser;
  const QString path = QStringLiteral(SS_DBC_FIXTURE_DIR "/") + fixture;
  if (!parser.parse(path))
    return QJsonObject();

  auto messages = parser.messageDescriptions();
  std::sort(messages.begin(),
            messages.end(),
            [](const QCanMessageDescription& a, const QCanMessageDescription& b) {
              return a.uniqueId() < b.uniqueId();
            });

  SessionContext ctx;
  DataModel::DBCImporter importer(ctx);
  return importer.projectFromMessages(messages);
}

/**
 * @brief Returns every signal line of the generated MESSAGES spec, trimmed and sorted so a
 *        comparison never depends on Qt's hash order.
 */
QStringList TstDbcImporter::specLines(const QString& lua)
{
  QStringList lines;
  for (const auto& line : lua.split(QLatin1Char('\n')))
    if (line.trimmed().startsWith(QStringLiteral("{ name = ")))
      lines.append(line.trimmed());

  lines.sort();
  return lines;
}

/**
 * @brief Returns the spec line of one named signal, or an empty string when it was not emitted.
 */
QString TstDbcImporter::specLineFor(const QString& lua, const QString& signal)
{
  const auto needle = QStringLiteral("{ name = \"%1\",").arg(signal);
  for (const auto& line : specLines(lua))
    if (line.startsWith(needle))
      return line;

  return QString();
}

/**
 * @brief Returns the titles of every dataset in the project, in emission order.
 */
QStringList TstDbcImporter::datasetTitles(const QJsonObject& project)
{
  QStringList titles;
  for (const auto& group : project[Keys::Groups].toArray())
    for (const auto& dataset : group.toObject()[Keys::Datasets].toArray())
      titles.append(dataset.toObject()[Keys::Title].toString());

  return titles;
}

//--------------------------------------------------------------------------------------------------
// Classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every signal of the extended fixture imports: two groups, eight datasets, dense indices
 *        and one data-table register per signal. A dropped SG_MUL_VAL_ signal shows up here as a
 *        missing dataset, which is what the importer used to do to all five of them.
 */
void TstDbcImporter::extendedFixtureSkipsNothing()
{
  const auto doc    = project(QStringLiteral("extended_mux.dbc"));
  const auto groups = doc[Keys::Groups].toArray();
  QCOMPARE(groups.size(), 2);
  QCOMPARE(groups.at(0).toObject()[Keys::Title].toString(), QStringLiteral("DIAG_RESPONSE"));
  QCOMPARE(groups.at(0).toObject()[Keys::Datasets].toArray().size(), 6);
  QCOMPARE(groups.at(1).toObject()[Keys::Datasets].toArray().size(), 2);

  int expected = 1;
  for (const auto& group : groups)
    for (const auto& dataset : group.toObject()[Keys::Datasets].toArray())
      QCOMPARE(dataset.toObject()[Keys::Index].toInt(), expected++);

  QCOMPARE(expected, 9);

  const auto tables = doc[Keys::Tables].toArray();
  QCOMPARE(tables.size(), 2);
  QCOMPARE(tables.at(0).toObject()[Keys::Registers].toArray().size(), 6);
  QCOMPARE(tables.at(1).toObject()[Keys::Registers].toArray().size(), 2);
}

/**
 * @brief parse() reads the spec top to bottom and latches selector values as it goes, so every
 *        switch must precede the signals it gates: Mode before SubMode, SubMode before the two
 *        signals nested under it.
 */
void TstDbcImporter::selectorsPrecedeTheirDependents()
{
  const auto doc = project(QStringLiteral("extended_mux.dbc"));
  const auto lua = doc[Keys::Sources].toArray().at(0).toObject()[Keys::FrameParserCode].toString();

  const auto mode        = lua.indexOf(QStringLiteral("{ name = \"Mode\","));
  const auto subMode     = lua.indexOf(QStringLiteral("{ name = \"SubMode\","));
  const auto packVoltage = lua.indexOf(QStringLiteral("{ name = \"PackVoltage\","));
  const auto faultCount  = lua.indexOf(QStringLiteral("{ name = \"FaultCount\","));
  const auto temperature = lua.indexOf(QStringLiteral("{ name = \"Temperature\","));

  QVERIFY(mode >= 0);
  QVERIFY(subMode > mode);
  QVERIFY(packVoltage > subMode);
  QVERIFY(faultCount > subMode);
  QVERIFY(temperature > mode);
}

//--------------------------------------------------------------------------------------------------
// Generated spec
//--------------------------------------------------------------------------------------------------

/**
 * @brief The SG_MUL_VAL_ ranges land verbatim in the spec: a nested switch carries both
 *        selector = true and its own gate, disjoint ranges keep both entries, and a single point
 *        on the top-level multiplexor still emits the bare numeric form.
 */
void TstDbcImporter::extendedRangesReachTheLuaSpec()
{
  const auto doc = project(QStringLiteral("extended_mux.dbc"));
  const auto lua = doc[Keys::Sources].toArray().at(0).toObject()[Keys::FrameParserCode].toString();

  const auto mode = specLineFor(lua, QStringLiteral("Mode"));
  QVERIFY(mode.contains(QStringLiteral(", selector = true")));
  QVERIFY(!mode.contains(QStringLiteral(", mux = ")));

  const auto subMode = specLineFor(lua, QStringLiteral("SubMode"));
  QVERIFY(subMode.contains(QStringLiteral(", selector = true")));
  QVERIFY(subMode.contains(QStringLiteral(", mux = {p = \"Mode\", r = {{1, 1}}}")));

  QVERIFY(specLineFor(lua, QStringLiteral("PackVoltage"))
            .contains(QStringLiteral(", mux = {p = \"SubMode\", r = {{0, 3}}}")));
  QVERIFY(specLineFor(lua, QStringLiteral("FaultCount"))
            .contains(QStringLiteral(", mux = {p = \"SubMode\", r = {{4, 4}, {9, 11}}}")));
  QVERIFY(specLineFor(lua, QStringLiteral("Temperature"))
            .contains(QStringLiteral(", mux = {p = \"Mode\", r = {{2, 2}, {5, 7}}}")));
  QVERIFY(specLineFor(lua, QStringLiteral("SerialNumber")).contains(QStringLiteral(", mux = 3")));
}

/**
 * @brief Two mux variants of one message must never share a dataset title, so the switch values
 *        ride in the title: a point, a range, a comma-joined set, and the nested selector's
 *        double role.
 */
void TstDbcImporter::datasetTitlesCarryTheSwitchValues()
{
  const auto titles = datasetTitles(project(QStringLiteral("extended_mux.dbc")));

  QVERIFY(titles.contains(QStringLiteral("Mode (selector)")));
  QVERIFY(titles.contains(QStringLiteral("SubMode (selector, mux 1)")));
  QVERIFY(titles.contains(QStringLiteral("PackVoltage (mux 0-3)")));
  QVERIFY(titles.contains(QStringLiteral("FaultCount (mux 4,9-11)")));
  QVERIFY(titles.contains(QStringLiteral("Temperature (mux 2,5-7)")));
  QVERIFY(titles.contains(QStringLiteral("SerialNumber (mux 3)")));
  QVERIFY(titles.contains(QStringLiteral("Uptime")));
}

//--------------------------------------------------------------------------------------------------
// Generated machinery
//--------------------------------------------------------------------------------------------------

/**
 * @brief The static parse() machinery keeps selector values by name and gates each signal on its
 *        own parent, which is what makes a nested chain decode; the bare numeric form still
 *        compares against the message's top-level multiplexor.
 */
void TstDbcImporter::generatedLuaMatchesRangesAtRuntime()
{
  const auto doc = project(QStringLiteral("extended_mux.dbc"));
  const auto lua = doc[Keys::Sources].toArray().at(0).toObject()[Keys::FrameParserCode].toString();

  QVERIFY(lua.contains(QStringLiteral("local function in_ranges(value, ranges)")));
  QVERIFY(lua.contains(QStringLiteral("if value >= range[1] and value <= range[2] then")));
  QVERIFY(lua.contains(QStringLiteral("local function mux_ok(mux, selectors, root)")));
  QVERIFY(lua.contains(QStringLiteral("if type(mux) == \"number\" then")));
  QVERIFY(lua.contains(QStringLiteral("return mux == root")));
  QVERIFY(lua.contains(QStringLiteral("return value ~= nil and in_ranges(value, mux.r)")));
  QVERIFY(
    lua.contains(QStringLiteral("if sig.mux == nil or mux_ok(sig.mux, selectors, root) then")));
  QVERIFY(lua.contains(QStringLiteral("selectors[sig.name] = raw")));
  QVERIFY(lua.contains(QStringLiteral("root = root or raw")));
}

/**
 * @brief The runtime is LuaJIT 2.1 on a Lua 5.1 surface: 5.3 bitwise and floor-division syntax
 *        does not parse and string.pack/unpack do not exist, so a generated parser using either
 *        dies on the first frame instead of at import time.
 */
void TstDbcImporter::generatedLuaStaysWithinLua51()
{
  const auto doc = project(QStringLiteral("extended_mux.dbc"));
  const auto lua = doc[Keys::Sources].toArray().at(0).toObject()[Keys::FrameParserCode].toString();

  QVERIFY(!lua.contains(QStringLiteral("<<")));
  QVERIFY(!lua.contains(QStringLiteral(">>")));
  QVERIFY(!lua.contains(QStringLiteral(" // ")));
  QVERIFY(!lua.contains(QStringLiteral("string.pack")));
  QVERIFY(!lua.contains(QStringLiteral("string.unpack")));
  QVERIFY(lua.contains(QStringLiteral("bit.band")));
  QVERIFY(lua.contains(QStringLiteral("math.floor")));
}

//--------------------------------------------------------------------------------------------------
// Simple multiplexing is unchanged
//--------------------------------------------------------------------------------------------------

/**
 * @brief A DBC with only simple multiplexing keeps the compact numeric gate the importer has
 *        always written: no {p = ..., r = ...} table anywhere, and the same titles.
 */
void TstDbcImporter::simpleMuxKeepsTheScalarForm()
{
  const auto doc = project(QStringLiteral("simple_mux.dbc"));
  const auto lua = doc[Keys::Sources].toArray().at(0).toObject()[Keys::FrameParserCode].toString();

  QVERIFY(!lua.contains(QStringLiteral("mux = {p = ")));
  QVERIFY(
    specLineFor(lua, QStringLiteral("Channel")).contains(QStringLiteral(", selector = true")));
  QVERIFY(specLineFor(lua, QStringLiteral("ChannelVoltage")).contains(QStringLiteral(", mux = 0")));
  QVERIFY(specLineFor(lua, QStringLiteral("ChannelCurrent")).contains(QStringLiteral(", mux = 1")));
  QVERIFY(specLineFor(lua, QStringLiteral("ChannelTemp")).contains(QStringLiteral(", mux = 2")));

  const auto titles = datasetTitles(doc);
  QVERIFY(titles.contains(QStringLiteral("Channel (selector)")));
  QVERIFY(titles.contains(QStringLiteral("ChannelVoltage (mux 0)")));
  QVERIFY(titles.contains(QStringLiteral("ChannelTemp (mux 2)")));
  QVERIFY(titles.contains(QStringLiteral("Timestamp")));
}

/**
 * @brief The full spec of the simple-mux fixture, byte for byte. This is the guard on the
 *        extended-mux work: every field, its order inside the line, and the numeric gate are
 *        what the importer emitted before SG_MUL_VAL_ support existed.
 */
void TstDbcImporter::simpleMuxSpecMatchesTheStoredExpectation()
{
  const auto doc = project(QStringLiteral("simple_mux.dbc"));
  const auto lua = doc[Keys::Sources].toArray().at(0).toObject()[Keys::FrameParserCode].toString();

  const QStringList expected = {
    QStringLiteral("{ name = \"ChannelCurrent\", start = 8, len = 16, factor = 0.1, mux = 1 },"),
    QStringLiteral("{ name = \"ChannelTemp\", start = 8, len = 16, signed = true, factor = 0.5, "
                   "mux = 2 },"),
    QStringLiteral("{ name = \"ChannelVoltage\", start = 8, len = 16, factor = 0.1, mux = 0 },"),
    QStringLiteral("{ name = \"Channel\", start = 0, len = 8, selector = true }, -- Selects which "
                   "measurement rides in bytes 1-2"),
    QStringLiteral("{ name = \"LinkUp\", start = 24, len = 1 }, -- Gateway link state"),
    QStringLiteral("{ name = \"Timestamp\", start = 48, len = 16 },"),
    QStringLiteral("{ name = \"Uptime\", start = 0, len = 24 },"),
  };

  auto sorted = expected;
  sorted.sort();
  QCOMPARE(specLines(lua), sorted);
}

QTEST_APPLESS_MAIN(TstDbcImporter)

#include "tst_dbc_importer.moc"
