/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include "DataModel/Editors/CodeFormatter.h"

/**
 * @file tst_code_formatter.cpp
 * @brief Indentation contract of the JS/Lua code formatter shared by every embedded editor:
 *        brace and block-keyword depth, the brace-free hanging indent (including the `else`
 *        re-attachment), the string/comment blind spots that must not move depth, whitespace
 *        normalization, and the line-range variant's "touch only these lines" guarantee.
 */

namespace {

using Language = DataModel::CodeFormatter::Language;

}  // namespace

/**
 * @brief Every test function here is self-contained: no state is carried between slots, so Qt
 *        Test's declaration-order execution is never load-bearing.
 */
class TstCodeFormatter : public QObject {
  Q_OBJECT

private slots:
  void formatDocument_data();
  void formatDocument();

  void emptySourceIsReturnedVerbatim();
  void indentWidthIsConfigurable();
  void formattingIsIdempotent();

  void formatLineRange_touchesOnlyTheRange();
  void formatLineRange_clampsOutOfRangeBounds();
  void formatLineRange_rejectsInvertedRange();
};

//--------------------------------------------------------------------------------------------------
// Whole-document formatting
//--------------------------------------------------------------------------------------------------

void TstCodeFormatter::formatDocument_data()
{
  QTest::addColumn<QString>("source");
  QTest::addColumn<int>("language");
  QTest::addColumn<QString>("expected");

  QTest::newRow("js braces indent the body")
    << QStringLiteral("function f() {\nx = 1;\n}") << int(Language::JavaScript)
    << QStringLiteral("function f() {\n  x = 1;\n}");

  QTest::newRow("js nested braces stack")
    << QStringLiteral("function f() {\nif (a) {\nx = 1;\n}\n}") << int(Language::JavaScript)
    << QStringLiteral("function f() {\n  if (a) {\n    x = 1;\n  }\n}");

  QTest::newRow("js over-indented input is normalized down")
    << QStringLiteral("function f() {\n            x = 1;\n}") << int(Language::JavaScript)
    << QStringLiteral("function f() {\n  x = 1;\n}");

  QTest::newRow("js brace-free if hangs exactly one line")
    << QStringLiteral("if (a)\nb = 1;\nc = 2;") << int(Language::JavaScript)
    << QStringLiteral("if (a)\n  b = 1;\nc = 2;");

  QTest::newRow("js bare else re-attaches to its if")
    << QStringLiteral("if (a)\nb = 1;\nelse\nc = 2;") << int(Language::JavaScript)
    << QStringLiteral("if (a)\n  b = 1;\nelse\n  c = 2;");

  QTest::newRow("js brace inside a string does not open a block")
    << QStringLiteral("var s = \"{\";\nx = 1;") << int(Language::JavaScript)
    << QStringLiteral("var s = \"{\";\nx = 1;");

  QTest::newRow("js brace inside a line comment does not open a block")
    << QStringLiteral("// {\nx = 1;") << int(Language::JavaScript)
    << QStringLiteral("// {\nx = 1;");

  QTest::newRow("js block comment body is left alone")
    << QStringLiteral("/*\n   keep me\n*/\nx = 1;") << int(Language::JavaScript)
    << QStringLiteral("/*\n   keep me\n*/\nx = 1;");

  QTest::newRow("lua function/end block")
    << QStringLiteral("function f()\nx = 1\nend") << int(Language::Lua)
    << QStringLiteral("function f()\n  x = 1\nend");

  QTest::newRow("lua then/else/end outdents the middle keyword")
    << QStringLiteral("if a then\nx = 1\nelse\ny = 2\nend") << int(Language::Lua)
    << QStringLiteral("if a then\n  x = 1\nelse\n  y = 2\nend");

  QTest::newRow("lua comment brace-free keywords stay put")
    << QStringLiteral("-- end\nx = 1") << int(Language::Lua) << QStringLiteral("-- end\nx = 1");

  QTest::newRow("trailing whitespace is stripped")
    << QStringLiteral("x = 1;   \ny = 2;\t") << int(Language::JavaScript)
    << QStringLiteral("x = 1;\ny = 2;");

  QTest::newRow("whitespace-only lines collapse to empty")
    << QStringLiteral("x = 1;\n    \ny = 2;") << int(Language::JavaScript)
    << QStringLiteral("x = 1;\n\ny = 2;");

  QTest::newRow("carriage returns are normalized away")
    << QStringLiteral("x = 1;\r\ny = 2;\r\n") << int(Language::JavaScript)
    << QStringLiteral("x = 1;\ny = 2;\n");

  QTest::newRow("trailing newline shape is preserved")
    << QStringLiteral("x = 1;\n") << int(Language::JavaScript) << QStringLiteral("x = 1;\n");

  QTest::newRow("unbalanced closing braces clamp at depth zero")
    << QStringLiteral("}\n}\nx = 1;") << int(Language::JavaScript)
    << QStringLiteral("}\n}\nx = 1;");
}

/**
 * @brief Drives the table above through formatDocument() at the default two-space indent.
 */
void TstCodeFormatter::formatDocument()
{
  QFETCH(QString, source);
  QFETCH(int, language);
  QFETCH(QString, expected);

  QCOMPARE(DataModel::CodeFormatter::formatDocument(source, static_cast<Language>(language)),
           expected);
}

//--------------------------------------------------------------------------------------------------
// Document-level invariants
//--------------------------------------------------------------------------------------------------

/**
 * @brief An empty document is the one input that short-circuits before the scanner runs.
 */
void TstCodeFormatter::emptySourceIsReturnedVerbatim()
{
  QCOMPARE(DataModel::CodeFormatter::formatDocument(QString(), Language::JavaScript), QString());
  QCOMPARE(DataModel::CodeFormatter::formatLineRange(QString(), Language::Lua, 0, 10), QString());
}

/**
 * @brief The indent width is a parameter, not a constant: depth is rendered, never hard-coded.
 */
void TstCodeFormatter::indentWidthIsConfigurable()
{
  const QString source = QStringLiteral("function f() {\nx = 1;\n}");

  QCOMPARE(DataModel::CodeFormatter::formatDocument(source, Language::JavaScript, 4),
           QStringLiteral("function f() {\n    x = 1;\n}"));

  QCOMPARE(DataModel::CodeFormatter::formatDocument(source, Language::JavaScript, 0),
           QStringLiteral("function f() {\nx = 1;\n}"));
}

/**
 * @brief Formatting an already-formatted document must be a no-op, or the editor's Ctrl+Shift+I
 *        would keep producing new undo steps for identical text.
 */
void TstCodeFormatter::formattingIsIdempotent()
{
  const QString source = QStringLiteral("function f() {\nif (a)\nb = 1;\nelse\nc = 2;\n\nvar s = "
                                        "\"}\";\n}\n");

  const QString once  = DataModel::CodeFormatter::formatDocument(source, Language::JavaScript);
  const QString twice = DataModel::CodeFormatter::formatDocument(once, Language::JavaScript);
  QCOMPARE(twice, once);
}

//--------------------------------------------------------------------------------------------------
// Line-range formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Only the requested lines are rewritten; the depth used for them still comes from a scan
 *        of the whole document, so a selection deep inside a block indents correctly.
 */
void TstCodeFormatter::formatLineRange_touchesOnlyTheRange()
{
  const QString source = QStringLiteral("function f() {\nx = 1;\ny = 2;\n}");

  QCOMPARE(DataModel::CodeFormatter::formatLineRange(source, Language::JavaScript, 1, 1),
           QStringLiteral("function f() {\n  x = 1;\ny = 2;\n}"));

  QCOMPARE(DataModel::CodeFormatter::formatLineRange(source, Language::JavaScript, 1, 2),
           QStringLiteral("function f() {\n  x = 1;\n  y = 2;\n}"));
}

/**
 * @brief Out-of-range bounds clamp to the document instead of reading past it.
 */
void TstCodeFormatter::formatLineRange_clampsOutOfRangeBounds()
{
  const QString source = QStringLiteral("function f() {\nx = 1;\n}");

  QCOMPARE(DataModel::CodeFormatter::formatLineRange(source, Language::JavaScript, -5, 999),
           DataModel::CodeFormatter::formatDocument(source, Language::JavaScript));
}

/**
 * @brief A range whose start is past its end leaves the source untouched.
 */
void TstCodeFormatter::formatLineRange_rejectsInvertedRange()
{
  const QString source = QStringLiteral("function f() {\nx = 1;\n}");
  QCOMPARE(DataModel::CodeFormatter::formatLineRange(source, Language::JavaScript, 2, 1), source);
}

QTEST_APPLESS_MAIN(TstCodeFormatter)

#include "tst_code_formatter.moc"
