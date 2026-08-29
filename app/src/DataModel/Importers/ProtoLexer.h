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

#pragma once

#include <QChar>
#include <QString>

namespace DataModel {

/**
 * @brief Token kinds produced by the proto3 lexer.
 */
enum class ProtoTokenKind {
  Ident,
  IntLit,
  StrLit,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  LParen,
  RParen,
  LAngle,
  RAngle,
  Eq,
  Comma,
  Semi,
  Dot,
  Slash,
  Minus,
  Plus,
  Eof,
  Error,
};

/**
 * @brief A single lexer token: kind, text, and source line for diagnostics.
 */
struct ProtoToken {
  ProtoTokenKind kind;
  QString text;
  int line;
};

/**
 * @brief Streaming lexer for a proto3 source buffer.
 */
class ProtoLexer {
public:
  explicit ProtoLexer(const QString& source);
  ProtoLexer(ProtoLexer&&)                 = delete;
  ProtoLexer(const ProtoLexer&)            = delete;
  ProtoLexer& operator=(ProtoLexer&&)      = delete;
  ProtoLexer& operator=(const ProtoLexer&) = delete;

public:
  [[nodiscard]] ProtoToken next();

private:
  void skipWhitespaceAndComments();
  void skipLineComment();
  void skipBlockComment();

  [[nodiscard]] ProtoToken lexStringLiteral(QChar quote, int line);

private:
  QString m_src;
  int m_pos;
  int m_line;
};

}  // namespace DataModel
