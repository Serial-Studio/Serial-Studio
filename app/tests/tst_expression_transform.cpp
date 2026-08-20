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

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <QString>
#include <QTest>

#include "DataModel/Scripting/ExpressionTransform.h"

/**
 * @file tst_expression_transform.cpp
 * @brief Grammar, evaluation and slot-slot_table contract of the compiled expression transforms
 *        (spec 0060): precedence, the function set, comparisons and conditionals, sibling names
 *        (bare and braced), `sample()` history, `n`/`dt`, compile-time errors, and the
 *        allocation-free evaluator's NaN-on-inconsistency behaviour.
 */

namespace {

using namespace DataModel::Expression;

/**
 * @brief Compiles @p text against a resolver that knows `a`, `b` and `{Long Name}` and returns
 *        the value for @p v (t = 0). Compile failures surface as NaN plus @p error.
 */
[[nodiscard]] double evalWith(const QString& text,
                              SlotTable& slot_table,
                              double v,
                              QString* error = nullptr)
{
  const NameResolver resolver = [&slot_table](QStringView name) -> int {
    if (name == u"a")
      return slot_table.slotFor(10);

    if (name == u"b")
      return slot_table.slotFor(11);

    if (name == u"Long Name")
      return slot_table.slotFor(12);

    return -1;
  };

  Runtime runtime;
  QString err;
  if (!compile(text, resolver, runtime.program, err)) {
    if (error)
      *error = err;

    return std::numeric_limits<double>::quiet_NaN();
  }

  return runtime.run(v, 0.0, slot_table);
}

[[nodiscard]] double evalPlain(const QString& text, double v)
{
  SlotTable slot_table;
  return evalWith(text, slot_table, v);
}

}  // namespace

class TstExpressionTransform : public QObject {
  Q_OBJECT

private slots:
  void arithmeticAndPrecedence();
  void functionsAndConstants();
  void comparisonsAndConditional();
  void siblingNames();
  void sampleHistory();
  void countersAndDt();
  void compileErrors();
  void tableRegisters();
  void nanPropagatesAndDegenerateProgram();
};

namespace {

// Stand-in for the DataTableStore: the handle is the index, so the thunk is a bounds check
constexpr double kFakeRegisters[] = {2.5, -1.0};

double fakeTableValue(const void*, qint64 handle)
{
  const qint64 count = static_cast<qint64>(sizeof(kFakeRegisters) / sizeof(kFakeRegisters[0]));
  return (handle >= 0 && handle < count) ? kFakeRegisters[handle]
                                         : std::numeric_limits<double>::quiet_NaN();
}

}  // namespace

/**
 * @brief `table(name, register)` resolves to a handle at compile time and reads through the
 *        thunk; without a resolver installed the call is rejected instead of silently NaN.
 */
void TstExpressionTransform::tableRegisters()
{
  const NameResolver names = [](QStringView) {
    return -1;
  };
  const TableResolver tables = [](QStringView slot_table, QStringView reg) -> qint64 {
    if (slot_table == u"cal" && reg == u"scale")
      return 0;

    if (slot_table == u"cal" && reg == u"offset")
      return 1;

    return -1;
  };

  SlotTable slot_table;
  Runtime runtime;
  QString error;
  QVERIFY2(
    compile("v * table(cal, scale) + table(cal, offset)", names, tables, runtime.program, error),
    qPrintable(error));

  runtime.tableValue = &fakeTableValue;
  QCOMPARE(runtime.run(4.0, 0.0, slot_table), 9.0);

  Program program;
  QVERIFY(!compile("table(cal, missing)", names, tables, program, error));
  QVERIFY(error.contains(QStringLiteral("unknown variable")));

  QVERIFY(!compile("table(cal, scale)", names, program, error));
  QVERIFY(error.contains(QStringLiteral("not available")));
}

/**
 * @brief Operators bind as expected: unary minus, ^ right-associative and tighter than *, % .
 */
void TstExpressionTransform::arithmeticAndPrecedence()
{
  QCOMPARE(evalPlain("v * 0.0625 - 40", 1600.0), 60.0);
  QCOMPARE(evalPlain("1 + 2 * 3", 0.0), 7.0);
  QCOMPARE(evalPlain("(1 + 2) * 3", 0.0), 9.0);
  QCOMPARE(evalPlain("2 ^ 3 ^ 2", 0.0), 512.0);
  QCOMPARE(evalPlain("-2 ^ 2", 0.0), 4.0);
  QCOMPARE(evalPlain("7 % 4", 0.0), 3.0);
  QCOMPARE(evalPlain("-v", 3.0), -3.0);
  QCOMPARE(evalPlain("+v", 3.0), 3.0);
  QCOMPARE(evalPlain("1e3 + 2.5e-1", 0.0), 1000.25);
  QCOMPARE(evalPlain(".5 * 4", 0.0), 2.0);
}

/**
 * @brief The fixed function set and the named constants evaluate through std::math.
 */
void TstExpressionTransform::functionsAndConstants()
{
  QCOMPARE(evalPlain("sqrt(v)", 16.0), 4.0);
  QCOMPARE(evalPlain("abs(v)", -2.5), 2.5);
  QCOMPARE(evalPlain("min(v, 3)", 5.0), 3.0);
  QCOMPARE(evalPlain("max(v, 3)", 5.0), 5.0);
  QCOMPARE(evalPlain("clamp(v, 0, 1)", 5.0), 1.0);
  QCOMPARE(evalPlain("lerp(0, 10, 0.25)", 0.0), 2.5);
  QCOMPARE(evalPlain("hypot(3, 4)", 0.0), 5.0);
  QCOMPARE(evalPlain("pow(2, 10)", 0.0), 1024.0);
  QCOMPARE(evalPlain("floor(2.7) + ceil(2.2) + round(2.5)", 0.0), 8.0);
  QCOMPARE(evalPlain("log10(1000)", 0.0), 3.0);
  QCOMPARE(evalPlain("log2(8)", 0.0), 3.0);
  QCOMPARE(evalPlain("deg(pi)", 0.0), 180.0);
  QCOMPARE(evalPlain("rad(180)", 0.0), M_PI);
  QCOMPARE(evalPlain("cos(0) + sin(0)", 0.0), 1.0);
  QCOMPARE(evalPlain("atan2(0, 1)", 0.0), 0.0);
  QCOMPARE(evalPlain("e", 0.0), M_E);
  QVERIFY(std::isnan(evalPlain("nan", 0.0)));
  QVERIFY(std::isinf(evalPlain("inf", 0.0)));
  QVERIFY(std::isinf(evalPlain("1 / 0", 0.0)));
}

/**
 * @brief Comparisons yield 0/1, logic short-circuits by value, and `? :` selects.
 */
void TstExpressionTransform::comparisonsAndConditional()
{
  QCOMPARE(evalPlain("v > 2", 3.0), 1.0);
  QCOMPARE(evalPlain("v > 2", 1.0), 0.0);
  QCOMPARE(evalPlain("v >= 2 && v <= 4", 3.0), 1.0);
  QCOMPARE(evalPlain("v < 2 || v == 3", 3.0), 1.0);
  QCOMPARE(evalPlain("v != 3", 3.0), 0.0);
  QCOMPARE(evalPlain("!(v > 0)", 3.0), 0.0);
  QCOMPARE(evalPlain("v > 0 ? v : -v", -4.0), 4.0);
  QCOMPARE(evalPlain("v > 10 ? 1 : v > 5 ? 2 : 3", 7.0), 2.0);
  QCOMPARE(evalPlain("(v > 1) + (v > 2)", 3.0), 2.0);
}

/**
 * @brief Bare and braced names resolve to slots; the latest published value is what is read
 *        and an unpublished slot reads NaN.
 */
void TstExpressionTransform::siblingNames()
{
  SlotTable slot_table;
  QVERIFY(std::isnan(evalWith("a + b", slot_table, 0.0)));

  slot_table.publish(10, 2.0);
  slot_table.publish(11, 5.0);
  QCOMPARE(evalWith("a + b", slot_table, 0.0), 7.0);
  QCOMPARE(evalWith("v - a", slot_table, 10.0), 8.0);

  QCOMPARE(slot_table.slotFor(12), 2);
  slot_table.publish(12, 100.0);
  QCOMPARE(evalWith("{Long Name} / 4", slot_table, 0.0), 25.0);
  QCOMPARE(evalWith("{ Long Name } / 4", slot_table, 0.0), 25.0);

  slot_table.publish(99, 1.0);
  QCOMPARE(slot_table.slotOf(99), -1);
  QCOMPARE(slot_table.slotCount(), 3);
  QCOMPARE(slot_table.uniqueIdAt(0), 10);
}

/**
 * @brief sample(name, k): 0 is the latest published value, k counts back, NaN past the fill;
 *        the ring is bounded at kMaxHistory.
 */
void TstExpressionTransform::sampleHistory()
{
  SlotTable slot_table;
  const NameResolver resolver = [&slot_table](QStringView name) -> int {
    return (name == u"x") ? slot_table.slotFor(7) : -1;
  };

  Runtime runtime;
  QString error;
  QVERIFY(compile(
    "sample(x, 0) + 10 * sample(x, 1) + 100 * sample(x, 2)", resolver, runtime.program, error));
  QVERIFY(runtime.program.usesHistory);
  QVERIFY(std::isnan(runtime.run(0.0, 0.0, slot_table)));

  slot_table.publish(7, 1.0);
  slot_table.publish(7, 2.0);
  slot_table.publish(7, 3.0);
  QCOMPARE(runtime.run(0.0, 0.0, slot_table), 3.0 + 20.0 + 100.0);

  for (int i = 0; i < kMaxHistory + 5; ++i)
    slot_table.publish(7, static_cast<double>(i));

  QCOMPARE(slot_table.sample(0, 0), static_cast<double>(kMaxHistory + 4));
  QCOMPARE(slot_table.sample(0, kMaxHistory - 1), 5.0);
  QVERIFY(std::isnan(slot_table.sample(0, kMaxHistory)));
  QVERIFY(std::isnan(slot_table.sample(0, -1)));
  QVERIFY(std::isnan(slot_table.sample(3, 0)));

  Runtime negative;
  QVERIFY(compile("sample(x, -3)", resolver, negative.program, error));
  QCOMPARE(negative.run(0.0, 0.0, slot_table), slot_table.sample(0, 0));

  slot_table.reset();
  QVERIFY(std::isnan(slot_table.sample(0, 0)));
  QCOMPARE(slot_table.slotCount(), 1);
}

/**
 * @brief `n` counts evaluations from zero and `dt` is the gap to the previous `t` (0 first).
 */
void TstExpressionTransform::countersAndDt()
{
  SlotTable slot_table;
  Runtime runtime;
  QString error;
  QVERIFY(compile("n * 1000 + dt", [](QStringView) { return -1; }, runtime.program, error));

  QCOMPARE(runtime.run(0.0, 10.0, slot_table), 0.0);
  QCOMPARE(runtime.run(0.0, 10.5, slot_table), 1000.5);
  QCOMPARE(runtime.run(0.0, 12.0, slot_table), 2001.5);

  runtime.reset();
  QCOMPARE(runtime.run(0.0, 20.0, slot_table), 0.0);
}

/**
 * @brief Every rejection is a compile-time error with a message; the program stays invalid.
 */
void TstExpressionTransform::compileErrors()
{
  SlotTable slot_table;
  QString error;

  QVERIFY(std::isnan(evalWith("sqtr(v)", slot_table, 1.0, &error)));
  QVERIFY(error.contains(QStringLiteral("sqtr")));

  QVERIFY(std::isnan(evalWith("zz + 1", slot_table, 1.0, &error)));
  QVERIFY(error.contains(QStringLiteral("zz")));

  QVERIFY(std::isnan(evalWith("(1 + 2", slot_table, 1.0, &error)));
  QVERIFY(!error.isEmpty());

  QVERIFY(std::isnan(evalWith("1 +", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("min(1)", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("sample(1, 2)", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("1 2", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("1.2.3", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("{unterminated", slot_table, 1.0, &error)));
  QVERIFY(std::isnan(evalWith("v $ 2", slot_table, 1.0, &error)));

  Program program;
  QVERIFY(compile("v", [](QStringView) { return -1; }, program, error));
  QVERIFY(program.valid());
  QCOMPARE(program.maxSlot, -1);
  QVERIFY(!program.usesHistory);
}

/**
 * @brief NaN inputs propagate through arithmetic, and a hand-built inconsistent program yields
 *        NaN instead of touching memory outside the fixed stack.
 */
void TstExpressionTransform::nanPropagatesAndDegenerateProgram()
{
  QVERIFY(std::isnan(evalPlain("v * 2", std::numeric_limits<double>::quiet_NaN())));
  QVERIFY(std::isnan(evalPlain("sqrt(v)", -1.0)));

  Program broken;
  broken.code.push_back(Node{Op::Add, 0, 0.0});
  broken.stackDepth = 1;

  Context ctx{};
  ctx.v = 1.0;
  QVERIFY(std::isnan(evaluate(broken, ctx)));
}

QTEST_APPLESS_MAIN(TstExpressionTransform)

#include "tst_expression_transform.moc"
