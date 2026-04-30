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

#include "DataModel/CodeFormatter.h"

#include <QSet>
#include <QStringList>
#include <QStringView>

namespace {

using DataModel::CodeFormatter::Language;

enum class ScanIn : uint8_t {
  Code,
  JsTemplate,
  JsBlockComment,
  LuaLongString,
  LuaBlockComment
};

struct ScanState {
  ScanIn in    = ScanIn::Code;
  int luaLevel = 0;
};

struct LineInfo {
  bool startsInsideMultiline = false;
  int outdentLeading         = 0;
  int netDelta               = 0;
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/** @brief Returns true when ch can begin or continue a Lua identifier. */
bool isLuaIdentChar(QChar ch, bool first)
{
  if (ch == QLatin1Char('_'))
    return true;

  if (ch.isLetter())
    return true;

  if (!first && ch.isDigit())
    return true;

  return false;
}

/** @brief If line[i:] opens a Lua long-bracket form, returns its level and length. */
bool tryLuaLongOpen(QStringView line, int i, int& outLevel, int& outConsume)
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

/** @brief If line[i:] closes a Lua long-bracket form at the given level, returns the length. */
bool tryLuaLongClose(QStringView line, int i, int level, int& outConsume)
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

/** @brief Skips a quoted string literal starting at line[i] (assumes line[i] is the open quote). */
int skipQuotedString(QStringView line, int i)
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

/** @brief Computes outdent/indent deltas for a single line and updates the carry-over state. */
LineInfo analyzeLine(QStringView line, Language lang, ScanState& state)
{
  LineInfo info;
  info.startsInsideMultiline = (state.in != ScanIn::Code);

  bool sawOpenerOrCode = false;

  auto registerCloser = [&](bool& sawCode) {
    if (!sawCode)
      ++info.outdentLeading;

    --info.netDelta;
    sawCode = true;
  };

  auto registerOpener = [&](bool& sawCode) {
    ++info.netDelta;
    sawCode = true;
  };

  static const QSet<QString> luaOpens = {
    QStringLiteral("then"),
    QStringLiteral("do"),
    QStringLiteral("function"),
    QStringLiteral("repeat"),
    QStringLiteral("else"),
  };
  static const QSet<QString> luaCloses = {
    QStringLiteral("end"),
    QStringLiteral("until"),
    QStringLiteral("else"),
    QStringLiteral("elseif"),
  };

  int i       = 0;
  const int n = line.size();
  while (i < n) {
    if (state.in == ScanIn::JsBlockComment) {
      if (i + 1 < n && line[i] == QLatin1Char('*') && line[i + 1] == QLatin1Char('/')) {
        state.in = ScanIn::Code;
        i += 2;
      } else
        ++i;
      continue;
    }

    if (state.in == ScanIn::JsTemplate) {
      const QChar c = line[i];
      if (c == QLatin1Char('\\')) {
        i += 2;
        continue;
      }
      if (c == QLatin1Char('`')) {
        state.in = ScanIn::Code;
        ++i;
        continue;
      }
      ++i;
      continue;
    }

    if (state.in == ScanIn::LuaLongString || state.in == ScanIn::LuaBlockComment) {
      int consume = 0;
      if (tryLuaLongClose(line, i, state.luaLevel, consume)) {
        state.in       = ScanIn::Code;
        state.luaLevel = 0;
        i += consume;
      } else
        ++i;
      continue;
    }

    // state.in == ScanIn::Code
    const QChar c = line[i];

    // JS: line comment // ... ends the line
    if (lang == Language::JavaScript && c == QLatin1Char('/') && i + 1 < n
        && line[i + 1] == QLatin1Char('/'))
      break;

    // Lua: line comment -- ... unless it's --[[ or --[==[
    if (lang == Language::Lua && c == QLatin1Char('-') && i + 1 < n
        && line[i + 1] == QLatin1Char('-')) {
      int level   = 0;
      int consume = 0;
      if (i + 2 < n && tryLuaLongOpen(line, i + 2, level, consume)) {
        state.in       = ScanIn::LuaBlockComment;
        state.luaLevel = level;
        i += 2 + consume;
        continue;
      }
      break;
    }

    // JS: block comment /* ... */
    if (lang == Language::JavaScript && c == QLatin1Char('/') && i + 1 < n
        && line[i + 1] == QLatin1Char('*')) {
      state.in = ScanIn::JsBlockComment;
      i += 2;
      continue;
    }

    // Quoted string literal
    if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
      i               = skipQuotedString(line, i);
      sawOpenerOrCode = true;
      continue;
    }

    // JS template literal
    if (lang == Language::JavaScript && c == QLatin1Char('`')) {
      state.in = ScanIn::JsTemplate;
      ++i;
      continue;
    }

    // Lua long string [[ ... ]] or [==[ ... ]==]
    if (lang == Language::Lua && c == QLatin1Char('[')) {
      int level   = 0;
      int consume = 0;
      if (tryLuaLongOpen(line, i, level, consume)) {
        state.in       = ScanIn::LuaLongString;
        state.luaLevel = level;
        i += consume;
        continue;
      }
    }

    // Brace tokens
    if (c == QLatin1Char('{')) {
      registerOpener(sawOpenerOrCode);
      ++i;
      continue;
    }
    if (c == QLatin1Char('}')) {
      registerCloser(sawOpenerOrCode);
      ++i;
      continue;
    }

    // Lua keyword tokens
    if (lang == Language::Lua && isLuaIdentChar(c, /*first=*/true)) {
      int j = i;
      while (j < n && isLuaIdentChar(line[j], /*first=*/false))
        ++j;

      const QString word = line.mid(i, j - i).toString();
      const bool isClose = luaCloses.contains(word);
      const bool isOpen  = luaOpens.contains(word);
      if (isClose)
        registerCloser(sawOpenerOrCode);

      if (isOpen)
        registerOpener(sawOpenerOrCode);

      if (!isClose && !isOpen)
        sawOpenerOrCode = true;

      i = j;
      continue;
    }

    if (!c.isSpace())
      sawOpenerOrCode = true;

    ++i;
  }

  return info;
}

/** @brief Splits text into lines while preserving the trailing-newline shape. */
QStringList splitLines(const QString& text)
{
  // Use Qt::KeepEmptyParts to retain trailing empty line semantics.
  return text.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
}

/** @brief Returns line stripped of trailing whitespace and any '\r' from CRLF normalization. */
QString rtrim(const QString& line)
{
  int end = line.size();
  while (end > 0
         && (line[end - 1] == QLatin1Char(' ') || line[end - 1] == QLatin1Char('\t')
             || line[end - 1] == QLatin1Char('\r')))
    --end;
  return line.left(end);
}

/** @brief Returns the count of leading space/tab characters in the line. */
int leadingWhitespaceCount(QStringView line)
{
  int i = 0;
  while (i < line.size() && (line[i] == QLatin1Char(' ') || line[i] == QLatin1Char('\t')))
    ++i;

  return i;
}

/** @brief Builds (lineInfos, depthAtStart) for every line, updating scanner state across lines. */
struct ScanResult {
  QList<LineInfo> infos;
  QList<int> depthAtStart;
};

ScanResult scanAllLines(const QStringList& lines, Language lang)
{
  ScanResult result;
  result.infos.reserve(lines.size());
  result.depthAtStart.reserve(lines.size());

  ScanState state;
  int depth = 0;
  for (const auto& raw : lines) {
    const QString trimmed = rtrim(raw);
    LineInfo info         = analyzeLine(QStringView(trimmed), lang, state);
    result.infos.append(info);
    result.depthAtStart.append(depth);
    depth += info.netDelta;
    if (depth < 0)
      depth = 0;
  }

  return result;
}

/** @brief Returns the rendered indentation string for a line. */
QString computeIndent(int depth, int outdentLeading, int indentSpaces)
{
  int rendered = depth - outdentLeading;
  if (rendered < 0)
    rendered = 0;

  return QString(rendered * indentSpaces, QLatin1Char(' '));
}

/** @brief Reformats one line based on its scanner state and depth. */
QString reformatOne(const QString& raw, const LineInfo& info, int depthAtStart, int indentSpaces)
{
  const QString trimmed = rtrim(raw);

  // Preserve leading whitespace for lines inside a multiline string/comment.
  if (info.startsInsideMultiline)
    return trimmed;

  // Pure-whitespace lines collapse to empty.
  const int lead = leadingWhitespaceCount(QStringView(trimmed));
  if (lead == trimmed.size())
    return QString();

  const QString body = trimmed.mid(lead);
  return computeIndent(depthAtStart, info.outdentLeading, indentSpaces) + body;
}

}  // namespace

namespace DataModel::CodeFormatter {

/** @brief Reformats the entire source string with normalized indentation. */
QString formatDocument(const QString& source, Language language, int indentSpaces)
{
  if (source.isEmpty())
    return source;

  const QStringList lines = splitLines(source);
  const ScanResult scan   = scanAllLines(lines, language);

  QStringList out;
  out.reserve(lines.size());
  for (int i = 0; i < lines.size(); ++i)
    out.append(reformatOne(lines[i], scan.infos[i], scan.depthAtStart[i], indentSpaces));

  return out.join(QLatin1Char('\n'));
}

/** @brief Reformats only the given inclusive line range, preserving everything else. */
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
    lines[i] = reformatOne(lines[i], scan.infos[i], scan.depthAtStart[i], indentSpaces);

  return lines.join(QLatin1Char('\n'));
}

}  // namespace DataModel::CodeFormatter
