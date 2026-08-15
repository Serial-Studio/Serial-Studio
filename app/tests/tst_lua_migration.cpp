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

#include <QString>
#include <QTest>

#include "DataModel/Scripting/LuaMigration.h"

// Every expected rewrite below was checked by evaluating the original under lua5.4 and the rewrite
// under luajit and comparing the printed values, plus a 1949-case random-expression sweep against a
// Lua 5.4 shim whose bit.* functions are the native 5.3 operators. The equivalence stops where
// bit.* itself does: 32-bit width and shift counts outside 0..31.

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief Source-to-source contract of the Lua 5.3 -> LuaJIT rewriter.
 */
class TstLuaMigration : public QObject {
  Q_OBJECT

private slots:
  void detectsOperatorsInCodePosition_data();
  void detectsOperatorsInCodePosition();

  void rewritesExpressions_data();
  void rewritesExpressions();

  void rewritesStatements_data();
  void rewritesStatements();

  void leavesStringsAndCommentsIntact();
  void reportsNoChangeWhenAlreadyCompatible();
  void rewriteIsAFixedPoint();
  void bailsWhenAnOperandCannotBeDelimited();
  void bailsOnUnterminatedString();
};

//--------------------------------------------------------------------------------------------------
// Detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief An operator only counts when the lexer places it in code, never inside a literal.
 */
void TstLuaMigration::detectsOperatorsInCodePosition_data()
{
  QTest::addColumn<QString>("script");
  QTest::addColumn<bool>("detected");

  QTest::newRow("band") << QStringLiteral("local v = a & b") << true;
  QTest::newRow("bor") << QStringLiteral("local v = a | b") << true;
  QTest::newRow("bxor") << QStringLiteral("local v = a ~ b") << true;
  QTest::newRow("bnot") << QStringLiteral("local v = ~a") << true;
  QTest::newRow("lshift") << QStringLiteral("local v = a << 1") << true;
  QTest::newRow("rshift") << QStringLiteral("local v = a >> 1") << true;
  QTest::newRow("idiv") << QStringLiteral("local v = a // 8") << true;

  QTest::newRow("not-equal") << QStringLiteral("if a ~= b then end") << false;
  QTest::newRow("compare") << QStringLiteral("if a <= b and c >= d then end") << false;
  QTest::newRow("short string") << QStringLiteral("local s = \"a << b & c // d\"") << false;
  QTest::newRow("line comment") << QStringLiteral("-- a << b & c\nlocal v = 1") << false;
  QTest::newRow("long comment") << QStringLiteral("--[[ a & b\n c << d ]]\nlocal v = 1") << false;
  QTest::newRow("long string") << QStringLiteral("local s = [==[ x | y ]==]") << false;
  QTest::newRow("already migrated") << QStringLiteral("local v = bit.band(a, b)") << false;
}

/**
 * @brief usesLua53Operators() agrees with the lexer's view of every sample.
 */
void TstLuaMigration::detectsOperatorsInCodePosition()
{
  QFETCH(QString, script);
  QFETCH(bool, detected);

  QCOMPARE(DataModel::LuaMigration::usesLua53Operators(script), detected);
}

//--------------------------------------------------------------------------------------------------
// Expression rewriting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Precedence, associativity and operand delimitation, one expression at a time.
 */
void TstLuaMigration::rewritesExpressions_data()
{
  QTest::addColumn<QString>("script");
  QTest::addColumn<QString>("expected");

  QTest::newRow("band") << QStringLiteral("a & b") << QStringLiteral("bit.band(a, b)");
  QTest::newRow("bor") << QStringLiteral("a | b") << QStringLiteral("bit.bor(a, b)");
  QTest::newRow("bxor") << QStringLiteral("a ~ b") << QStringLiteral("bit.bxor(a, b)");
  QTest::newRow("bnot") << QStringLiteral("~a") << QStringLiteral("bit.bnot(a)");
  QTest::newRow("lshift") << QStringLiteral("a << 2") << QStringLiteral("bit.lshift(a, 2)");
  QTest::newRow("rshift") << QStringLiteral("a >> 2") << QStringLiteral("bit.rshift(a, 2)");
  QTest::newRow("idiv") << QStringLiteral("a // b") << QStringLiteral("math.floor(a / b)");

  QTest::newRow("band chain") << QStringLiteral("a & b & c")
                              << QStringLiteral("bit.band(bit.band(a, b), c)");
  QTest::newRow("band binds tighter than bor")
    << QStringLiteral("a | b & c") << QStringLiteral("bit.bor(a, bit.band(b, c))");
  QTest::newRow("bor takes the band")
    << QStringLiteral("a & b | c") << QStringLiteral("bit.bor(bit.band(a, b), c)");
  QTest::newRow("addition binds tighter, left")
    << QStringLiteral("a + b & c") << QStringLiteral("bit.band(a + b, c)");
  QTest::newRow("addition binds tighter, right")
    << QStringLiteral("a & b + c") << QStringLiteral("bit.band(a, b + c)");
  QTest::newRow("floor division takes the product")
    << QStringLiteral("a * b // c") << QStringLiteral("math.floor(a * b / c)");
  QTest::newRow("product takes the floor division")
    << QStringLiteral("a // b * c") << QStringLiteral("math.floor(a / b) * c");
  QTest::newRow("exponent binds tighter than floor division")
    << QStringLiteral("a ^ 2 // b") << QStringLiteral("math.floor(a ^ 2 / b)");
  QTest::newRow("unary binds tighter, left")
    << QStringLiteral("~a & b") << QStringLiteral("bit.band(bit.bnot(a), b)");
  QTest::newRow("unary binds tighter, right")
    << QStringLiteral("a & ~b") << QStringLiteral("bit.band(a, bit.bnot(b))");
  QTest::newRow("negation stays with its operand")
    << QStringLiteral("-a & b") << QStringLiteral("bit.band(-a, b)");
  QTest::newRow("comparison binds looser")
    << QStringLiteral("a < b & c") << QStringLiteral("a < bit.band(b, c)");
  QTest::newRow("shift binds tighter than band, left")
    << QStringLiteral("a >> 8 & 0xFF") << QStringLiteral("bit.band(bit.rshift(a, 8), 0xFF)");
  QTest::newRow("shift binds tighter than band, right")
    << QStringLiteral("a & 0xFF << 8") << QStringLiteral("bit.band(a, bit.lshift(0xFF, 8))");
  QTest::newRow("shift chain") << QStringLiteral("1 << 30 >> 2")
                               << QStringLiteral("bit.rshift(bit.lshift(1, 30), 2)");
  QTest::newRow("and binds looser, left")
    << QStringLiteral("a and b & c") << QStringLiteral("a and bit.band(b, c)");
  QTest::newRow("and binds looser, right")
    << QStringLiteral("a & b and c") << QStringLiteral("bit.band(a, b) and c");

  QTest::newRow("index suffix") << QStringLiteral("t[i + 1] & (1 << (j % 8))")
                                << QStringLiteral("bit.band(t[i + 1], (bit.lshift(1, (j % 8))))");
  QTest::newRow("method call suffix")
    << QStringLiteral("s:byte(1) & 0x7F") << QStringLiteral("bit.band(s:byte(1), 0x7F)");
  QTest::newRow("dotted call suffix") << QStringLiteral("math.floor(a / 2) & 0xFF")
                                      << QStringLiteral("bit.band(math.floor(a / 2), 0xFF)");
  QTest::newRow("parenthesised operands")
    << QStringLiteral("(a | b) & c") << QStringLiteral("bit.band((bit.bor(a, b)), c)");
}

/**
 * @brief migrateToLuaJit() reproduces the expected rewrite exactly.
 */
void TstLuaMigration::rewritesExpressions()
{
  QFETCH(QString, script);
  QFETCH(QString, expected);

  QCOMPARE(DataModel::LuaMigration::migrateToLuaJit(script), expected);
}

//--------------------------------------------------------------------------------------------------
// Statement rewriting
//--------------------------------------------------------------------------------------------------

/**
 * @brief The surrounding statement, and any trivia it carries, has to survive the rewrite.
 */
void TstLuaMigration::rewritesStatements_data()
{
  QTest::addColumn<QString>("script");
  QTest::addColumn<QString>("expected");

  QTest::newRow("assignment") << QStringLiteral("local v = a & b\nreturn v")
                              << QStringLiteral("local v = bit.band(a, b)\nreturn v");
  QTest::newRow("trailing comment") << QStringLiteral("local v = a & b -- mask")
                                    << QStringLiteral("local v = bit.band(a, b) -- mask");
  QTest::newRow("if condition") << QStringLiteral("if a & 1 ~= 0 then return 1 end")
                                << QStringLiteral("if bit.band(a, 1) ~= 0 then return 1 end");
  QTest::newRow("while body") << QStringLiteral("while a & 1 == 0 do a = a >> 1 end")
                              << QStringLiteral(
                                   "while bit.band(a, 1) == 0 do a = bit.rshift(a, 1) end");
  QTest::newRow("multiple returns") << QStringLiteral("return a & b, c | d")
                                    << QStringLiteral("return bit.band(a, b), bit.bor(c, d)");
  QTest::newRow("table constructor")
    << QStringLiteral("local t = { [a & 1] = b | 2, c // 3 }")
    << QStringLiteral("local t = { [bit.band(a, 1)] = bit.bor(b, 2), math.floor(c / 3) }");
  QTest::newRow("call arguments") << QStringLiteral("f(a & 1, b // 2, c << 3)")
                                  << QStringLiteral(
                                       "f(bit.band(a, 1), math.floor(b / 2), bit.lshift(c, 3))");
  QTest::newRow("continued expression")
    << QStringLiteral("local id = (a << 8)\n         | (b << 4)\n         | c")
    << QStringLiteral("local id = bit.bor(bit.bor((bit.lshift(a, 8)), (bit.lshift(b, 4))), c)");
}

/**
 * @brief migrateToLuaJit() rewrites inside statements without disturbing the rest of the source.
 */
void TstLuaMigration::rewritesStatements()
{
  QFETCH(QString, script);
  QFETCH(QString, expected);

  QCOMPARE(DataModel::LuaMigration::migrateToLuaJit(script), expected);
}

//--------------------------------------------------------------------------------------------------
// Boundaries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Operators inside literals are text, and text is copied through untouched.
 */
void TstLuaMigration::leavesStringsAndCommentsIntact()
{
  const auto script = QStringLiteral("local m = \"x&y\" --[[ z | w ]]\nlocal v = a & b");
  const auto expect = QStringLiteral("local m = \"x&y\" --[[ z | w ]]\nlocal v = bit.band(a, b)");

  QCOMPARE(DataModel::LuaMigration::migrateToLuaJit(script), expect);
}

/**
 * @brief A script with nothing to rewrite reports no rewrite rather than a copy of itself.
 */
void TstLuaMigration::reportsNoChangeWhenAlreadyCompatible()
{
  QVERIFY(DataModel::LuaMigration::migrateToLuaJit(QStringLiteral("local v = a + b")).isEmpty());
  QVERIFY(
    DataModel::LuaMigration::migrateToLuaJit(QStringLiteral("local v = bit.band(a, b)")).isEmpty());
  QVERIFY(DataModel::LuaMigration::migrateToLuaJit(QString()).isEmpty());
}

/**
 * @brief Rewriting the rewrite is a no-op, which is what makes the offer safe to repeat.
 */
void TstLuaMigration::rewriteIsAFixedPoint()
{
  const auto script = QStringLiteral("local id = ((b1 & 0x1F) << 24) | (f[2] << 16) | f[3]");
  const auto once   = DataModel::LuaMigration::migrateToLuaJit(script);

  QVERIFY(!once.isEmpty());
  QVERIFY(!DataModel::LuaMigration::usesLua53Operators(once));
  QVERIFY(DataModel::LuaMigration::migrateToLuaJit(once).isEmpty());
}

/**
 * @brief An operand the scanner cannot delimit fails the whole rewrite: a partial one would change
 *        the arithmetic while still compiling.
 */
void TstLuaMigration::bailsWhenAnOperandCannotBeDelimited()
{
  const auto script = QStringLiteral("local v = function() return 1 end & 1");

  QVERIFY(DataModel::LuaMigration::usesLua53Operators(script));
  QVERIFY(DataModel::LuaMigration::migrateToLuaJit(script).isEmpty());
}

/**
 * @brief A source the lexer cannot finish is never rewritten and never reported as migratable.
 */
void TstLuaMigration::bailsOnUnterminatedString()
{
  const auto script = QStringLiteral("local s = \"abc\nlocal v = a & b");

  QVERIFY(!DataModel::LuaMigration::usesLua53Operators(script));
  QVERIFY(DataModel::LuaMigration::migrateToLuaJit(script).isEmpty());
}

QTEST_APPLESS_MAIN(TstLuaMigration)

#include "tst_lua_migration.moc"
