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

#include "DataModel/Importers/ProtoLexer.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps a proto3 source buffer for streaming tokenization.
 */
DataModel::ProtoLexer::ProtoLexer(const QString& source) : m_src(source), m_pos(0), m_line(1) {}

//--------------------------------------------------------------------------------------------------
// Tokenization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the next token from the source buffer, or Eof on end of input.
 */
DataModel::ProtoToken DataModel::ProtoLexer::next()
{
  skipWhitespaceAndComments();

  if (m_pos >= m_src.size())
    return {ProtoTokenKind::Eof, QString(), m_line};

  const QChar c   = m_src[m_pos];
  const int line  = m_line;
  const int start = m_pos;

  if (c.isLetter() || c == QLatin1Char('_')) {
    while (m_pos < m_src.size()
           && (m_src[m_pos].isLetterOrNumber() || m_src[m_pos] == QLatin1Char('_')))
      ++m_pos;
    return {ProtoTokenKind::Ident, m_src.mid(start, m_pos - start), line};
  }

  if (c.isDigit()) {
    while (m_pos < m_src.size()
           && (m_src[m_pos].isLetterOrNumber() || m_src[m_pos] == QLatin1Char('.')
               || m_src[m_pos] == QLatin1Char('+') || m_src[m_pos] == QLatin1Char('-')))
      ++m_pos;
    return {ProtoTokenKind::IntLit, m_src.mid(start, m_pos - start), line};
  }

  if (c == QLatin1Char('"') || c == QLatin1Char('\''))
    return lexStringLiteral(c, line);

  ++m_pos;
  switch (c.unicode()) {
    case '{':
      return {ProtoTokenKind::LBrace, QStringLiteral("{"), line};
    case '}':
      return {ProtoTokenKind::RBrace, QStringLiteral("}"), line};
    case '[':
      return {ProtoTokenKind::LBracket, QStringLiteral("["), line};
    case ']':
      return {ProtoTokenKind::RBracket, QStringLiteral("]"), line};
    case '(':
      return {ProtoTokenKind::LParen, QStringLiteral("("), line};
    case ')':
      return {ProtoTokenKind::RParen, QStringLiteral(")"), line};
    case '<':
      return {ProtoTokenKind::LAngle, QStringLiteral("<"), line};
    case '>':
      return {ProtoTokenKind::RAngle, QStringLiteral(">"), line};
    case '=':
      return {ProtoTokenKind::Eq, QStringLiteral("="), line};
    case ',':
      return {ProtoTokenKind::Comma, QStringLiteral(","), line};
    case ';':
      return {ProtoTokenKind::Semi, QStringLiteral(";"), line};
    case '.':
      return {ProtoTokenKind::Dot, QStringLiteral("."), line};
    case '/':
      return {ProtoTokenKind::Slash, QStringLiteral("/"), line};
    case '-':
      return {ProtoTokenKind::Minus, QStringLiteral("-"), line};
    case '+':
      return {ProtoTokenKind::Plus, QStringLiteral("+"), line};
  }
  return {ProtoTokenKind::Error, QString(c), line};
}

/**
 * @brief Returns a StrLit token for a `"..."` or `'...'` body, honoring simple backslash escapes.
 */
DataModel::ProtoToken DataModel::ProtoLexer::lexStringLiteral(QChar quote, int line)
{
  ++m_pos;
  QString s;
  while (m_pos < m_src.size() && m_src[m_pos] != quote) {
    const bool hasEscape = (m_src[m_pos] == QLatin1Char('\\') && m_pos + 1 < m_src.size());
    if (hasEscape) {
      const QChar esc = m_src[m_pos + 1];
      switch (esc.unicode()) {
        case 'n':
          s.append(QLatin1Char('\n'));
          break;
        case 'r':
          s.append(QLatin1Char('\r'));
          break;
        case 't':
          s.append(QLatin1Char('\t'));
          break;
        case '\\':
          s.append(QLatin1Char('\\'));
          break;
        case '\'':
          s.append(QLatin1Char('\''));
          break;
        case '"':
          s.append(QLatin1Char('"'));
          break;
        default:
          s.append(esc);
      }
      m_pos += 2;
      continue;
    }
    if (m_src[m_pos] == QLatin1Char('\n'))
      ++m_line;

    s.append(m_src[m_pos]);
    ++m_pos;
  }
  if (m_pos < m_src.size())
    ++m_pos;

  return {ProtoTokenKind::StrLit, s, line};
}

//--------------------------------------------------------------------------------------------------
// Trivia
//--------------------------------------------------------------------------------------------------

/**
 * @brief Skips whitespace, line comments, and block comments.
 */
void DataModel::ProtoLexer::skipWhitespaceAndComments()
{
  while (m_pos < m_src.size()) {
    const QChar c = m_src[m_pos];

    if (c == QLatin1Char('\n')) {
      ++m_line;
      ++m_pos;
      continue;
    }

    if (c.isSpace()) {
      ++m_pos;
      continue;
    }

    if (c == QLatin1Char('/') && m_pos + 1 < m_src.size()) {
      const QChar c2 = m_src[m_pos + 1];
      if (c2 == QLatin1Char('/')) {
        skipLineComment();
        continue;
      }
      if (c2 == QLatin1Char('*')) {
        skipBlockComment();
        continue;
      }
    }

    break;
  }
}

/**
 * @brief Skips a `// ...\n` line comment, leaving position at the trailing newline.
 */
void DataModel::ProtoLexer::skipLineComment()
{
  while (m_pos < m_src.size() && m_src[m_pos] != QLatin1Char('\n'))
    ++m_pos;
}

/**
 * @brief Skips a block comment, tracking line numbers across newlines.
 */
void DataModel::ProtoLexer::skipBlockComment()
{
  m_pos += 2;
  while (m_pos + 1 < m_src.size()) {
    if (m_src[m_pos] == QLatin1Char('*') && m_src[m_pos + 1] == QLatin1Char('/')) {
      m_pos += 2;
      return;
    }
    if (m_src[m_pos] == QLatin1Char('\n'))
      ++m_line;

    ++m_pos;
  }
}
