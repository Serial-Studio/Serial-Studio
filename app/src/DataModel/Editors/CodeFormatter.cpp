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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Editors/CodeFormatter.h"

#include <QSet>
#include <QStringList>
#include <QStringView>

using DataModel::CodeFormatter::Language;

namespace detail {

/**
 * @brief Scanner mode for the formatter's per-character pass.
 */
enum class ScanIn : uint8_t {
  Code,
  JsTemplate,
  JsBlockComment,
  LuaLongString,
  LuaBlockComment
};

/**
 * @brief Carry-over state threaded across lines by the scanner.
 */
struct ScanState {
  ScanIn in    = ScanIn::Code;
  int luaLevel = 0;
};

/**
 * @brief Per-line indent/outdent deltas produced by the scanner.
 */
struct LineInfo {
  bool startsInsideMultiline = false;
  bool hasCode               = false;
  bool opensHanging          = false;
  bool startsWithOpenBrace   = false;
  bool startsWithElse        = false;
  int outdentLeading         = 0;
  int netDelta               = 0;
};

/**
 * @brief Result of a full-file scan: per-line info paired with the brace depth at line start.
 */
struct ScanResult {
  QList<LineInfo> infos;
  QList<int> depthAtStart;
  QList<int> hangAtStart;
};

}  // namespace detail

using detail::LineInfo;
using detail::ScanIn;
using detail::ScanResult;
using detail::ScanState;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when ch can begin or continue a JavaScript identifier.
 */
static bool isJsIdentChar(QChar ch, bool first)
{
  if (ch == QLatin1Char('_') || ch == QLatin1Char('$'))
    return true;

  if (ch.isLetter())
    return true;

  if (!first && ch.isDigit())
    return true;

  return false;
}

/**
 * @brief Returns the JS keywords whose brace-free "(...)"-terminated headers hang the next line.
 */
static const QSet<QString>& jsHangingKeywords()
{
  static const QSet<QString> kSet = {
    QStringLiteral("if"),
    QStringLiteral("for"),
    QStringLiteral("while"),
    QStringLiteral("else"),
  };
  return kSet;
}

/**
 * @brief Returns true when ch can begin or continue a Lua identifier.
 */
static bool isLuaIdentChar(QChar ch, bool first)
{
  if (ch == QLatin1Char('_'))
    return true;

  if (ch.isLetter())
    return true;

  if (!first && ch.isDigit())
    return true;

  return false;
}

/**
 * @brief If line[i:] opens a Lua long-bracket form, returns its level and length.
 */
static bool tryLuaLongOpen(QStringView line, int i, int& outLevel, int& outConsume)
{
  if (i >= line.size() || line[i] != QLatin1Char('['))
    return false;

  int level = 0;
  int j     = i + 1;
  while (j < line.size() && line[j] == QLatin1Char('=')) {
    ++level;
    ++j;
  }

  if (j >= line.size() || line[j] != QLatin1Char('['))
    return false;

  outLevel   = level;
  outConsume = (j - i) + 1;
  return true;
}

/**
 * @brief If line[i:] closes a Lua long-bracket form at the given level, returns the length.
 */
static bool tryLuaLongClose(QStringView line, int i, int level, int& outConsume)
{
  if (i >= line.size() || line[i] != QLatin1Char(']'))
    return false;

  int j    = i + 1;
  int seen = 0;
  while (seen < level && j < line.size() && line[j] == QLatin1Char('=')) {
    ++seen;
    ++j;
  }

  if (seen != level)
    return false;

  if (j >= line.size() || line[j] != QLatin1Char(']'))
    return false;

  outConsume = (j - i) + 1;
  return true;
}

/**
 * @brief Skips a quoted string literal starting at line[i] (assumes line[i] is the open quote).
 */
static int skipQuotedString(QStringView line, int i)
{
  const QChar quote = line[i];
  int j             = i + 1;
  while (j < line.size()) {
    const QChar c = line[j];
    if (c == QLatin1Char('\\')) {
      j += 2;
      continue;
    }
    if (c == quote)
      return j + 1;

    ++j;
  }
  return j;
}

/**
 * @brief Advances past a JS block comment, possibly closing it.
 */
static void advanceJsBlockComment(QStringView line, int& i, int n, ScanState& state)
{
  if (i + 1 < n && line[i] == QLatin1Char('*') && line[i + 1] == QLatin1Char('/')) {
    state.in  = ScanIn::Code;
    i        += 2;
  } else
    ++i;
}

/**
 * @brief Advances past characters inside a JS template literal.
 */
static void advanceJsTemplate(QStringView line, int& i, ScanState& state)
{
  const QChar c = line[i];
  if (c == QLatin1Char('\\')) {
    i += 2;
    return;
  }
  if (c == QLatin1Char('`')) {
    state.in = ScanIn::Code;
    ++i;
    return;
  }
  ++i;
}

/**
 * @brief Advances past characters inside a Lua long string or block comment.
 */
static void advanceLuaLong(QStringView line, int& i, ScanState& state)
{
  int consume = 0;
  if (tryLuaLongClose(line, i, state.luaLevel, consume)) {
    state.in        = ScanIn::Code;
    state.luaLevel  = 0;
    i              += consume;
  } else
    ++i;
}

/**
 * @brief Returns the open/close keyword sets for Lua block tokens.
 */
static const QSet<QString>& luaOpenKeywords()
{
  static const QSet<QString> kSet = {
    QStringLiteral("then"),
    QStringLiteral("do"),
    QStringLiteral("function"),
    QStringLiteral("repeat"),
    QStringLiteral("else"),
  };
  return kSet;
}

/**
 * @brief Returns the close keyword set for Lua block tokens.
 */
static const QSet<QString>& luaCloseKeywords()
{
  static const QSet<QString> kSet = {
    QStringLiteral("end"),
    QStringLiteral("until"),
    QStringLiteral("else"),
    QStringLiteral("elseif"),
  };
  return kSet;
}

/** @brief Tries to open a JS or Lua line/block comment at line[i]; returns true if scan continues.
 */
static bool tryEnterComment(
  QStringView line, int& i, int n, Language lang, ScanState& state, bool& stop)
{
  const QChar c = line[i];
  stop          = false;

  if (lang == Language::JavaScript && c == QLatin1Char('/') && i + 1 < n
      && line[i + 1] == QLatin1Char('/')) {
    stop = true;
    return true;
  }

  if (lang == Language::Lua && c == QLatin1Char('-') && i + 1 < n
      && line[i + 1] == QLatin1Char('-')) {
    int level   = 0;
    int consume = 0;
    if (i + 2 < n && tryLuaLongOpen(line, i + 2, level, consume)) {
      state.in        = ScanIn::LuaBlockComment;
      state.luaLevel  = level;
      i              += 2 + consume;
      return true;
    }
    stop = true;
    return true;
  }

  if (lang == Language::JavaScript && c == QLatin1Char('/') && i + 1 < n
      && line[i + 1] == QLatin1Char('*')) {
    state.in  = ScanIn::JsBlockComment;
    i        += 2;
    return true;
  }

  return false;
}

/** @brief Tries to enter a string/template/long-string at line[i]; returns true if it consumed it.
 */
static bool tryEnterStringLiteral(
  QStringView line, int& i, Language lang, ScanState& state, bool& sawOpenerOrCode)
{
  const QChar c = line[i];

  if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
    i               = skipQuotedString(line, i);
    sawOpenerOrCode = true;
    return true;
  }

  if (lang == Language::JavaScript && c == QLatin1Char('`')) {
    state.in = ScanIn::JsTemplate;
    ++i;
    return true;
  }

  if (lang == Language::Lua && c == QLatin1Char('[')) {
    int level   = 0;
    int consume = 0;
    if (tryLuaLongOpen(line, i, level, consume)) {
      state.in        = ScanIn::LuaLongString;
      state.luaLevel  = level;
      i              += consume;
      return true;
    }
  }

  return false;
}

/**
 * @brief Token-level facts accumulated while scanning one line of code.
 */
struct TokenTrack {
  bool sawCode         = false;
  bool lastTokenIsWord = false;
  int parenDelta       = 0;
  QChar firstCodeChar;
  QChar lastCodeChar;
  QString firstWord;
  QString lastWord;
};

/**
 * @brief Records a consumed code character (or final character of a word) in the track.
 */
static void markCode(TokenTrack& tk, QChar c, bool isWord)
{
  if (tk.firstCodeChar.isNull())
    tk.firstCodeChar = c;

  tk.lastCodeChar    = c;
  tk.lastTokenIsWord = isWord;
  tk.sawCode         = true;
}

/**
 * @brief Consumes a Lua identifier, applying block open/close keyword deltas; returns next index.
 */
static int consumeLuaWord(QStringView line, int i, TokenTrack& tk, LineInfo& info)
{
  int j = i;
  while (j < line.size() && isLuaIdentChar(line[j], false))
    ++j;

  const QString word = line.mid(i, j - i).toString();
  if (luaCloseKeywords().contains(word)) {
    if (!tk.sawCode)
      ++info.outdentLeading;

    --info.netDelta;
  }

  if (luaOpenKeywords().contains(word))
    ++info.netDelta;

  if (tk.firstWord.isEmpty())
    tk.firstWord = word;

  tk.lastWord = word;
  markCode(tk, line[j - 1], true);
  return j;
}

/**
 * @brief Consumes a JavaScript identifier and records it in the track; returns next index.
 */
static int consumeJsWord(QStringView line, int i, TokenTrack& tk)
{
  int j = i;
  while (j < line.size() && isJsIdentChar(line[j], false))
    ++j;

  if (tk.firstWord.isEmpty())
    tk.firstWord = line.mid(i, j - i).toString();

  tk.lastWord = line.mid(i, j - i).toString();
  markCode(tk, line[j - 1], true);
  return j;
}

/**
 * @brief Flags the line as a hanging header when its code is a brace-free JS control statement.
 */
static void finalizeJsHanging(const TokenTrack& tk, LineInfo& info)
{
  const bool headerParen = tk.lastCodeChar == QLatin1Char(')') && tk.parenDelta == 0
                        && jsHangingKeywords().contains(tk.firstWord);
  const bool trailingKeyword =
    tk.lastTokenIsWord
    && (tk.lastWord == QLatin1String("else") || tk.lastWord == QLatin1String("do"));
  info.opensHanging   = headerParen || trailingKeyword;
  info.startsWithElse = (tk.firstWord == QLatin1String("else"));
}

/**
 * @brief Computes outdent/indent deltas for a single line and updates the carry-over state.
 */
static LineInfo analyzeLine(QStringView line, Language lang, ScanState& state)
{
  LineInfo info;
  info.startsInsideMultiline = (state.in != ScanIn::Code);

  TokenTrack tk;

  int i       = 0;
  const int n = line.size();
  while (i < n) {
    if (state.in == ScanIn::JsBlockComment) {
      advanceJsBlockComment(line, i, n, state);
      continue;
    }

    if (state.in == ScanIn::JsTemplate) {
      advanceJsTemplate(line, i, state);
      continue;
    }

    if (state.in == ScanIn::LuaLongString || state.in == ScanIn::LuaBlockComment) {
      advanceLuaLong(line, i, state);
      continue;
    }

    bool stop = false;
    if (tryEnterComment(line, i, n, lang, state, stop)) {
      if (stop)
        break;

      continue;
    }

    const QChar c = line[i];
    if (tryEnterStringLiteral(line, i, lang, state, tk.sawCode)) {
      markCode(tk, c, false);
      continue;
    }

    if (c == QLatin1Char('{')) {
      ++info.netDelta;
      markCode(tk, c, false);
      ++i;
      continue;
    }
    if (c == QLatin1Char('}')) {
      if (!tk.sawCode)
        ++info.outdentLeading;

      --info.netDelta;
      markCode(tk, c, false);
      ++i;
      continue;
    }

    if (lang == Language::Lua && isLuaIdentChar(c, true)) {
      i = consumeLuaWord(line, i, tk, info);
      continue;
    }

    if (lang == Language::JavaScript && isJsIdentChar(c, true)) {
      i = consumeJsWord(line, i, tk);
      continue;
    }

    if (!c.isSpace()) {
      if (c == QLatin1Char('('))
        ++tk.parenDelta;

      if (c == QLatin1Char(')'))
        --tk.parenDelta;

      markCode(tk, c, false);
    }

    ++i;
  }

  info.hasCode             = tk.sawCode;
  info.startsWithOpenBrace = (tk.firstCodeChar == QLatin1Char('{'));
  if (lang == Language::JavaScript)
    finalizeJsHanging(tk, info);

  return info;
}

/**
 * @brief Splits text into lines while preserving the trailing-newline shape.
 */
static QStringList splitLines(const QString& text)
{
  return text.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
}

/**
 * @brief Returns line stripped of trailing whitespace and any '\r' from CRLF normalization.
 */
static QString rtrim(const QString& line)
{
  int end = line.size();
  while (end > 0
         && (line[end - 1] == QLatin1Char(' ') || line[end - 1] == QLatin1Char('\t')
             || line[end - 1] == QLatin1Char('\r')))
    --end;
  return line.left(end);
}

/**
 * @brief Returns the count of leading space/tab characters in the line.
 */
static int leadingWhitespaceCount(QStringView line)
{
  int i = 0;
  while (i < line.size() && (line[i] == QLatin1Char(' ') || line[i] == QLatin1Char('\t')))
    ++i;

  return i;
}

/**
 * @brief Carry-over hanging-indent state threaded across lines.
 */
struct HangState {
  int pending  = 0;
  int prevLine = 0;
};

/**
 * @brief Returns the hanging-indent level for a line and advances the carried hang state; a
 *        bare "else" re-attaches to the innermost brace-free "if" via the prev-line level.
 */
static int applyHanging(const LineInfo& info, HangState& hang)
{
  if (info.startsInsideMultiline) {
    if (info.hasCode)
      hang.pending = 0;

    return 0;
  }

  if (!info.hasCode)
    return hang.pending;

  if (info.startsWithOpenBrace)
    hang.pending = 0;

  int lineHang = hang.pending;
  if (info.startsWithElse && hang.pending == 0 && info.outdentLeading == 0 && hang.prevLine > 0)
    lineHang = hang.prevLine - 1;

  hang.pending  = info.opensHanging ? lineHang + 1 : 0;
  hang.prevLine = lineHang;
  return lineHang;
}

/**
 * @brief Builds per-line LineInfo and depth-at-start arrays, threading scanner state across lines.
 */
static ScanResult scanAllLines(const QStringList& lines, Language lang)
{
  ScanResult result;
  result.infos.reserve(lines.size());
  result.depthAtStart.reserve(lines.size());
  result.hangAtStart.reserve(lines.size());

  ScanState state;
  HangState hang;
  int depth = 0;
  for (const auto& raw : lines) {
    const QString trimmed = rtrim(raw);
    LineInfo info         = analyzeLine(QStringView(trimmed), lang, state);
    result.infos.append(info);
    result.depthAtStart.append(depth);
    result.hangAtStart.append(applyHanging(info, hang));
    depth += info.netDelta;
    if (depth < 0)
      depth = 0;
  }

  return result;
}

/**
 * @brief Returns the rendered indentation string for a line.
 */
static QString computeIndent(int depth, int outdentLeading, int indentSpaces)
{
  int rendered = depth - outdentLeading;
  if (rendered < 0)
    rendered = 0;

  return QString(rendered * indentSpaces, QLatin1Char(' '));
}

/**
 * @brief Reformats one line based on its scanner state, brace depth and hanging indent.
 */
static QString reformatOne(
  const QString& raw, const LineInfo& info, int depthAtStart, int hangAtStart, int indentSpaces)
{
  QString trimmed = rtrim(raw);

  if (info.startsInsideMultiline)
    return trimmed;

  const int lead = leadingWhitespaceCount(QStringView(trimmed));
  if (lead == trimmed.size())
    return QString();

  const QString body = trimmed.mid(lead);
  return computeIndent(depthAtStart + hangAtStart, info.outdentLeading, indentSpaces) + body;
}

namespace DataModel::CodeFormatter {

/**
 * @brief Reformats the entire source string with normalized indentation.
 */
QString formatDocument(const QString& source, Language language, int indentSpaces)
{
  if (source.isEmpty())
    return source;

  const QStringList lines = splitLines(source);
  const ScanResult scan   = scanAllLines(lines, language);

  QStringList out;
  out.reserve(lines.size());
  for (int i = 0; i < lines.size(); ++i)
    out.append(reformatOne(
      lines[i], scan.infos[i], scan.depthAtStart[i], scan.hangAtStart[i], indentSpaces));

  return out.join(QLatin1Char('\n'));
}

/**
 * @brief Reformats only the given inclusive line range, preserving everything else.
 */
QString formatLineRange(
  const QString& source, Language language, int firstLine, int lastLine, int indentSpaces)
{
  if (source.isEmpty())
    return source;

  QStringList lines = splitLines(source);
  if (firstLine < 0)
    firstLine = 0;

  if (lastLine >= lines.size())
    lastLine = lines.size() - 1;

  if (firstLine > lastLine)
    return source;

  const ScanResult scan = scanAllLines(lines, language);

  for (int i = firstLine; i <= lastLine; ++i)
    lines[i] =
      reformatOne(lines[i], scan.infos[i], scan.depthAtStart[i], scan.hangAtStart[i], indentSpaces);

  return lines.join(QLatin1Char('\n'));
}

}  // namespace DataModel::CodeFormatter
