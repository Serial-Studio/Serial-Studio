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

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
// clang-format on

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <new>
#include <QByteArray>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QTest>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/JsCellCollector.h"
#include "DataModel/Scripting/LuaCellCollector.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "DataModel/Scripting/ScriptCells.h"
#include "DataModel/Scripting/TableApiScan.h"
#include "sanitizer_features.h"

// Spec 0086: the typed cell lane against the list path it replaces. The collectors are driven on a
// bare lua_State / QJSEngine on purpose: the engines' link sets pull the whole pipeline, and the
// collectors carry no engine state, so the unit tier pins them with the same inputs the engines
// hand them (the value a parser script returned).

//--------------------------------------------------------------------------------------------------
// Allocation counter
//--------------------------------------------------------------------------------------------------

namespace {

std::atomic<long> g_allocations{0};

}  // namespace

#if !SS_TSAN_ACTIVE

/**
 * @brief Counting replacement for the TU's global operator new, armed by reading the counter
 *        before and after a steady-state window (same pattern as tst_frame_builder_staging).
 */
void* operator new(std::size_t size)
{
  g_allocations.fetch_add(1, std::memory_order_relaxed);
  if (void* p = std::malloc(size ? size : 1))
    return p;

  throw std::bad_alloc();
}

/**
 * @brief Matching sized delete for the counting operator new.
 */
void operator delete(void* p) noexcept
{
  std::free(p);
}

/**
 * @brief Matching sized delete for the counting operator new.
 */
void operator delete(void* p, std::size_t) noexcept
{
  std::free(p);
}

#endif

//--------------------------------------------------------------------------------------------------
// Harness
//--------------------------------------------------------------------------------------------------

namespace {

constexpr qsizetype kMaxElements = 10000;
constexpr int kSteadyFrames      = 1000;

/**
 * @brief RAII lua_State with the restricted library subset the parser engine installs.
 */
class LuaState {
public:
  LuaState() : m_state(luaL_newstate())
  {
    SS_ASSERT(m_state != nullptr, return);

    static const luaL_Reg kSafeLibs[] = {
      {    "_G",   luaopen_base},
      { "table",  luaopen_table},
      {"string", luaopen_string},
      {  "math",   luaopen_math},
      { nullptr,        nullptr}
    };

    for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
      luaL_requiref(m_state, lib->name, lib->func, 1);
      lua_pop(m_state, 1);
    }
  }

  ~LuaState()
  {
    if (m_state != nullptr)
      lua_close(m_state);
  }

  LuaState(const LuaState&)            = delete;
  LuaState& operator=(const LuaState&) = delete;

  [[nodiscard]] lua_State* get() const noexcept { return m_state; }

  /**
   * @brief Evaluates "return <expr>" and leaves the value on the stack, exactly where the engine's
   *        parse call leaves the script's return value.
   */
  [[nodiscard]] bool push(const char* expr)
  {
    const QByteArray chunk = QByteArray("return ") + expr;
    if (luaL_loadstring(m_state, chunk.constData()) != 0)
      return false;

    return lua_pcall(m_state, 0, 1, 0) == 0;
  }

private:
  lua_State* m_state;
};

/**
 * @brief The list path's luaValueToString() rule, restated here as the oracle the cell text is
 *        held to.
 */
QString luaListText(lua_State* L)
{
  switch (lua_type(L, -1)) {
    case LUA_TSTRING:
      return QString::fromUtf8(lua_tostring(L, -1));
    case LUA_TNUMBER:
      if (lua_isinteger(L, -1))
        return QString::number(lua_tointeger(L, -1));

      return QString::number(lua_tonumber(L, -1), 'g', 15);
    default:
      break;
  }

  const char* coerced = lua_tostring(L, -1);
  return coerced ? QString::fromUtf8(coerced) : QString();
}

/**
 * @brief The list path's tableToStringList() over the table on top of the stack.
 */
QStringList luaListRow(lua_State* L)
{
  QStringList row;
  const auto len = static_cast<qsizetype>(lua_rawlen(L, -1));
  for (qsizetype i = 1; i <= len; ++i) {
    lua_rawgeti(L, -1, static_cast<int>(i));
    row.append(luaListText(L));
    lua_pop(L, 1);
  }

  return row;
}

/**
 * @brief Row @p row of @p rows as text, the way the builder consumes it.
 */
QStringList cellRowText(const DataModel::ScriptCellRows& rows, qsizetype row)
{
  QStringList out;
  qsizetype count   = 0;
  const auto* cells = rows.rowCells(row, count);
  for (qsizetype i = 0; i < count; ++i)
    out.append(QString::fromUtf8(rows.view(cells[i])));

  return out;
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief The cell lane's three contracts: every cell equals the list path's text, the numeric
 *        cells carry the value the script produced, and a steady frame shape allocates nothing.
 */
class TstScriptCells : public QObject {
  Q_OBJECT

private slots:
  void rowsKeepStorageAcrossClear();
  void luaMixedCellsMatchTheListPath();
  void luaTwoDimensionalResultsKeepRowOrder();
  void luaScalarAndEmptyResults();
  void luaMixedShapesFallBack();
  void luaSteadyShapeDoesNotAllocate();
  void luaNumberTextMatchesQString();
  void jsMixedCellsMatchTheListPath();
  void jsTwoDimensionalResultsKeepRowOrder();
  void jsNonArrayAndMixedShapesFallBack();
  void jsSteadyShapeDoesNotAllocate();
  void tableApiScanNamesEveryHelper();
  void tableApiScanRespectsWordBoundaries();
};

/**
 * @brief clear() forgets the rows but keeps the scratch, so the next frame's cells land at the
 *        same address without a heap operation.
 */
void TstScriptCells::rowsKeepStorageAcrossClear()
{
  DataModel::ScriptCellRows rows;
  rows.beginRow();
  rows.appendText("abc", 3);
  rows.appendNumber(1.5, "1.5", 3);

  qsizetype count   = 0;
  const auto* cells = rows.rowCells(0, count);
  QCOMPARE(count, qsizetype(2));
  QCOMPARE(rows.view(cells[0]), QByteArrayView("abc"));
  QCOMPARE(rows.view(cells[1]), QByteArrayView("1.5"));
  QCOMPARE(cells[1].kind, DataModel::CellKind::Number);
  QCOMPARE(cells[1].number, 1.5);
  const char* first = rows.view(cells[0]).data();

  rows.clear();
  QCOMPARE(rows.rowCount(), qsizetype(0));
  QCOMPARE(rows.cellCount(), qsizetype(0));
  QVERIFY(rows.rowCells(0, count) == nullptr);
  QCOMPARE(count, qsizetype(0));

  rows.beginRow();
  rows.appendText("xyz", 3);
  cells = rows.rowCells(0, count);
  QCOMPARE(rows.view(cells[0]).data(), first);
}

/**
 * @brief A flat Lua table of numbers, integers, numeric strings and text: one row whose text is
 *        the list path's, with the numeric kind only where the script produced a number.
 */
void TstScriptCells::luaMixedCellsMatchTheListPath()
{
  LuaState lua;
  QVERIFY(lua.push("{1, 2.5, 'x', '7', -0.1, 1e21, 3 / 2, true, nil, 'last', 'a\\0b', 0 / 0}"));
  const QStringList expected = luaListRow(lua.get());

  DataModel::ScriptCellRows rows;
  QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
  QCOMPARE(lua_gettop(lua.get()), 0);
  QCOMPARE(rows.rowCount(), qsizetype(1));
  QCOMPARE(cellRowText(rows, 0), expected);

  qsizetype count   = 0;
  const auto* cells = rows.rowCells(0, count);
  QCOMPARE(count, expected.size());
  QCOMPARE(cells[0].kind, DataModel::CellKind::Number);
  QCOMPARE(cells[0].number, 1.0);
  QCOMPARE(cells[1].kind, DataModel::CellKind::Number);
  QCOMPARE(cells[1].number, 2.5);
  QCOMPARE(cells[2].kind, DataModel::CellKind::Text);
  QCOMPARE(cells[3].kind, DataModel::CellKind::Text);
  QCOMPARE(cells[4].number, -0.1);
  QCOMPARE(cells[5].number, 1e21);
  QCOMPARE(cells[6].number, 1.5);
  QCOMPARE(cells[7].kind, DataModel::CellKind::Text);
  QCOMPARE(rows.view(cells[7]).size(), qsizetype(0));
  QCOMPARE(cells[9].kind, DataModel::CellKind::Text);
  QCOMPARE(rows.view(cells[10]), QByteArrayView("a"));
  QCOMPARE(rows.view(cells[11]), QByteArrayView("nan"));
  QVERIFY(std::isnan(cells[11].number));
}

/**
 * @brief A table of tables becomes one row per inner table, in order, with the list path's text.
 */
void TstScriptCells::luaTwoDimensionalResultsKeepRowOrder()
{
  LuaState lua;
  QVERIFY(lua.push("{{1, 'a'}, {2, 'b', 3.25}, {}, {'c'}}"));

  DataModel::ScriptCellRows rows;
  QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
  QCOMPARE(lua_gettop(lua.get()), 0);
  QCOMPARE(rows.rowCount(), qsizetype(4));
  QCOMPARE(cellRowText(rows, 0), QStringList({"1", "a"}));
  QCOMPARE(cellRowText(rows, 1), QStringList({"2", "b", "3.25"}));
  QCOMPARE(cellRowText(rows, 2), QStringList());
  QCOMPARE(cellRowText(rows, 3), QStringList({"c"}));
}

/**
 * @brief A scalar return is one cell; an empty table, nil or a boolean is no row at all, exactly
 *        what scalarToStringList() and the empty-table path produced.
 */
void TstScriptCells::luaScalarAndEmptyResults()
{
  DataModel::ScriptCellRows rows;
  {
    LuaState lua;
    QVERIFY(lua.push("42"));
    QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
    QCOMPARE(rows.rowCount(), qsizetype(1));
    QCOMPARE(cellRowText(rows, 0), QStringList({"42"}));
  }
  {
    LuaState lua;
    QVERIFY(lua.push("'text'"));
    QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
    QCOMPARE(cellRowText(rows, 0), QStringList({"text"}));
  }
  {
    LuaState lua;
    QVERIFY(lua.push("{}"));
    QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
    QCOMPARE(rows.rowCount(), qsizetype(0));
  }
  {
    LuaState lua;
    QVERIFY(lua.push("nil"));
    QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
    QCOMPARE(rows.rowCount(), qsizetype(0));
    QCOMPARE(lua_gettop(lua.get()), 0);
  }
}

/**
 * @brief Scalars and tables side by side is the unzip shape the list path owns: the collector
 *        declines and leaves the value on the stack for it.
 */
void TstScriptCells::luaMixedShapesFallBack()
{
  LuaState lua;
  QVERIFY(lua.push("{1, {2, 3}, 4}"));

  DataModel::ScriptCellRows rows;
  QVERIFY(!DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
  QCOMPARE(lua_gettop(lua.get()), 1);
  QVERIFY(lua_istable(lua.get(), -1));
  QCOMPARE(rows.rowCount(), qsizetype(0));
}

/**
 * @brief R2: the same numeric shape collected 1000 times after a warm-up frame performs zero heap
 *        allocations on the collector's side. Two probes, because Qt containers allocate through
 *        malloc and LuaJIT through its own allocator, both invisible to operator new: the counter
 *        covers the std::vector side, and the scratch and cell array addresses must not move.
 */
void TstScriptCells::luaSteadyShapeDoesNotAllocate()
{
  if (SS_TSAN_ACTIVE)
    QSKIP("the counting operator new is not linked under ThreadSanitizer");

  LuaState lua;
  DataModel::ScriptCellRows rows;
  const char* expr = "{1.25, 2, 3.5, 40000, 5e-3, 6, 7.75, 8}";

  QVERIFY(lua.push(expr));
  QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
  qsizetype count               = 0;
  const auto* const first_cells = rows.rowCells(0, count);
  const char* const first_text  = rows.view(first_cells[0]).data();

  const long before = g_allocations.load(std::memory_order_relaxed);
  for (int i = 0; i < kSteadyFrames; ++i) {
    QVERIFY(lua.push(expr));
    QVERIFY(DataModel::LuaCellCollector::collect(lua.get(), rows, kMaxElements));
    QCOMPARE(rows.rowCells(0, count), first_cells);
    QCOMPARE(rows.view(first_cells[0]).data(), first_text);
  }

  QCOMPARE(g_allocations.load(std::memory_order_relaxed) - before, 0L);
  QCOMPARE(rows.rowCount(), qsizetype(1));
  QCOMPARE(cellRowText(rows, 0),
           QStringList({"1.25", "2", "3.5", "40000", "0.005", "6", "7.75", "8"}));
}

/**
 * @brief R5 (Lua half): formatLuaNumber is byte-equal to the QString::number calls the list path
 *        made, over integers at the lua_isinteger boundary and a sweep of doubles.
 */
void TstScriptCells::luaNumberTextMatchesQString()
{
  const double doubles[] = {0.1,
                            -0.1,
                            2.5,
                            1.0 / 3.0,
                            1e-7,
                            1e21,
                            123.456,
                            1e15,
                            1e16,
                            1e17,
                            -1e300,
                            5e-324,
                            0.5,
                            1e-5,
                            3.14159265358979,
                            2.0 / 3,
                            1e100,
                            -2.5e-10,
                            12345.678901234567,
                            0.30000000000000004};
  for (const double v : doubles) {
    char text[32];
    const qsizetype len = DataModel::formatLuaNumber(v, false, text, sizeof(text));
    QVERIFY(len > 0);
    QCOMPARE(QString::fromLatin1(text, len), QString::number(v, 'g', 15));
  }

  const double specials[] = {std::numeric_limits<double>::quiet_NaN(),
                             -std::numeric_limits<double>::quiet_NaN(),
                             std::numeric_limits<double>::infinity(),
                             -std::numeric_limits<double>::infinity()};
  for (const double v : specials) {
    char text[32];
    const qsizetype len = DataModel::formatLuaNumber(v, false, text, sizeof(text));
    QVERIFY(len > 0);
    QCOMPARE(QString::fromLatin1(text, len), QString::number(v, 'g', 15));
  }

  const long long integers[] = {
    0, 1, -1, 42, -42, 1000000, 9007199254740992LL, -9007199254740992LL, 123456789012345LL};
  for (const long long v : integers) {
    char text[32];
    const qsizetype len =
      DataModel::formatLuaNumber(static_cast<double>(v), true, text, sizeof(text));
    QVERIFY(len > 0);
    QCOMPARE(QString::fromLatin1(text, len), QString::number(static_cast<qlonglong>(v)));
  }

  char tiny[2];
  QCOMPARE(DataModel::formatLuaNumber(123456.0, true, tiny, sizeof(tiny)), qsizetype(0));
}

/**
 * @brief A flat JS array of numbers, numeric strings, text, booleans and null: one row whose text
 *        is QJSValue::toString() per element, numeric kind only for numbers.
 */
void TstScriptCells::jsMixedCellsMatchTheListPath()
{
  QJSEngine engine;
  const QJSValue result = engine.evaluate(
    QStringLiteral("[1, 2.5, 'x', '7', -0.1, 1e21, 0.1 + 0.2, true, null, undefined, 'last']"));
  QVERIFY(result.isArray());

  const auto length = result.property(QStringLiteral("length")).toInt();
  QStringList expected;
  for (int i = 0; i < length; ++i)
    expected.append(result.property(static_cast<quint32>(i)).toString());

  DataModel::ScriptCellRows rows;
  QVERIFY(DataModel::JsCellCollector::collect(result, rows, kMaxElements));
  QCOMPARE(rows.rowCount(), qsizetype(1));
  QCOMPARE(cellRowText(rows, 0), expected);

  qsizetype count   = 0;
  const auto* cells = rows.rowCells(0, count);
  QCOMPARE(count, qsizetype(length));
  QCOMPARE(cells[0].kind, DataModel::CellKind::Number);
  QCOMPARE(cells[0].number, 1.0);
  QCOMPARE(cells[1].number, 2.5);
  QCOMPARE(cells[2].kind, DataModel::CellKind::Text);
  QCOMPARE(cells[3].kind, DataModel::CellKind::Text);
  QCOMPARE(cells[5].number, 1e21);
  QCOMPARE(rows.view(cells[5]), QByteArrayView("1e+21"));
  QCOMPARE(rows.view(cells[6]), QByteArrayView("0.30000000000000004"));
  QCOMPARE(cells[7].kind, DataModel::CellKind::Text);
  QCOMPARE(rows.view(cells[7]), QByteArrayView("true"));
  QCOMPARE(rows.view(cells[8]), QByteArrayView("null"));
  QCOMPARE(rows.view(cells[9]), QByteArrayView("undefined"));
}

/**
 * @brief An array of arrays becomes one row per inner array, in order.
 */
void TstScriptCells::jsTwoDimensionalResultsKeepRowOrder()
{
  QJSEngine engine;
  const QJSValue result = engine.evaluate(QStringLiteral("[[1, 'a'], [2, 'b', 3.25], [], ['c']]"));

  DataModel::ScriptCellRows rows;
  QVERIFY(DataModel::JsCellCollector::collect(result, rows, kMaxElements));
  QCOMPARE(rows.rowCount(), qsizetype(4));
  QCOMPARE(cellRowText(rows, 0), QStringList({"1", "a"}));
  QCOMPARE(cellRowText(rows, 1), QStringList({"2", "b", "3.25"}));
  QCOMPARE(cellRowText(rows, 2), QStringList());
  QCOMPARE(cellRowText(rows, 3), QStringList({"c"}));
}

/**
 * @brief A string, a number, an object and the mixed scalar-plus-array shape all decline, so the
 *        list path keeps its legacy handling of them; an empty array is zero rows.
 */
void TstScriptCells::jsNonArrayAndMixedShapesFallBack()
{
  QJSEngine engine;
  DataModel::ScriptCellRows rows;

  QVERIFY(!DataModel::JsCellCollector::collect(
    engine.evaluate(QStringLiteral("'1,2,3'")), rows, kMaxElements));
  QVERIFY(!DataModel::JsCellCollector::collect(
    engine.evaluate(QStringLiteral("42")), rows, kMaxElements));
  QVERIFY(!DataModel::JsCellCollector::collect(
    engine.evaluate(QStringLiteral("({a: 1})")), rows, kMaxElements));
  QVERIFY(!DataModel::JsCellCollector::collect(
    engine.evaluate(QStringLiteral("[1, [2, 3], 4]")), rows, kMaxElements));
  QCOMPARE(rows.rowCount(), qsizetype(0));

  QVERIFY(
    DataModel::JsCellCollector::collect(engine.evaluate(QStringLiteral("[]")), rows, kMaxElements));
  QCOMPARE(rows.rowCount(), qsizetype(0));
}

/**
 * @brief R2 (JS half): the rows' own storage never moves across a steady shape with text cells in
 *        it (the UTF-16 encode lands in the scratch tail, no QByteArray temporary). QJSValue's own
 *        per-element cost is Qt's and platform-dependent, so it is not counted here.
 */
void TstScriptCells::jsSteadyShapeDoesNotAllocate()
{
  QJSEngine engine;
  DataModel::ScriptCellRows rows;
  const QString expr = QStringLiteral("[1.25, 2, 'label', 40000, 5e-3, 'x', 7.75, 8]");

  QVERIFY(DataModel::JsCellCollector::collect(engine.evaluate(expr), rows, kMaxElements));
  qsizetype count               = 0;
  const auto* const first_cells = rows.rowCells(0, count);
  const char* const first_text  = rows.view(first_cells[0]).data();
  QCOMPARE(count, qsizetype(8));

  for (int i = 0; i < kSteadyFrames; ++i) {
    const QJSValue result = engine.evaluate(expr);
    QVERIFY(DataModel::JsCellCollector::collect(result, rows, kMaxElements));
    QCOMPARE(rows.rowCells(0, count), first_cells);
    QCOMPARE(rows.view(first_cells[0]).data(), first_text);
  }

  QCOMPARE(cellRowText(rows, 0),
           QStringList({"1.25", "2", "label", "40000", "0.005", "x", "7.75", "8"}));
}

/**
 * @brief Every helper of the table API arms the scan, on its own and through an alias, in either
 *        language's syntax, and even inside a comment (arming by design: a false positive only
 *        costs a capture, a false negative costs a stale read).
 */
void TstScriptCells::tableApiScanNamesEveryHelper()
{
  using namespace DataModel::TableApiScan;
  const char* helpers[] = {"tableGet",
                           "tableSet",
                           "tableHandle",
                           "tableHandleMany",
                           "tableGetH",
                           "tableSetH",
                           "datasetGetRaw",
                           "datasetGetFinal"};
  for (const char* helper : helpers) {
    const QString name = QString::fromLatin1(helper);
    QVERIFY2(
      referencesTableApi(QStringLiteral("function parse(f) return %1('t', 'r') end").arg(name)),
      helper);
    QVERIFY2(referencesTableApi(QStringLiteral("local g = %1").arg(name)), helper);
    QVERIFY2(referencesTableApi(QStringLiteral("const g = %1;").arg(name)), helper);
    QVERIFY2(referencesTableApi(QStringLiteral("-- uses %1 later").arg(name)), helper);
  }

  QVERIFY(referencesTableApi(QStringLiteral("__ss.tableGet")));
  QVERIFY(referencesTableApi(QStringLiteral("x = __ss")));
}

/**
 * @brief A longer identifier that merely starts with a helper name does not arm, and the documented
 *        gap (a name assembled at runtime) does not either.
 */
void TstScriptCells::tableApiScanRespectsWordBoundaries()
{
  using namespace DataModel::TableApiScan;
  QVERIFY(!referencesTableApi(QStringLiteral("function parse(f) return f:split(',') end")));
  QVERIFY(!referencesTableApi(QStringLiteral("local tableGetter = 1")));
  QVERIFY(!referencesTableApi(QStringLiteral("local mytableGet = 1")));
  QVERIFY(!referencesTableApi(QStringLiteral("_G['table' .. 'Get']('t', 'r')")));
  QVERIFY(!referencesTableApi(QString()));
}

QTEST_GUILESS_MAIN(TstScriptCells)

#include "tst_script_cells.moc"
