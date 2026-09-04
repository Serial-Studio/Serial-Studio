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

#include <QString>
#include <QVector>

#include "DataModel/Importers/ProtoLexer.h"

namespace DataModel {

/**
 * @brief Wire-format-aware classification of a Protocol Buffers scalar field type.
 */
enum class ProtoScalar {
  Double,
  Float,
  Int32,
  Int64,
  UInt32,
  UInt64,
  SInt32,
  SInt64,
  Fixed32,
  Fixed64,
  SFixed32,
  SFixed64,
  Bool,
  String,
  Bytes,
  MessageRef,
  EnumRef,
};

/**
 * @brief A single field declared inside a parsed `.proto` message.
 */
struct ProtoField {
  int tag            = 0;
  bool repeated      = false;
  ProtoScalar scalar = ProtoScalar::Int32;
  QString name;
  QString typeRef;
};

/**
 * @brief A message definition parsed from a `.proto` schema, possibly with nested messages.
 */
struct ProtoMessage {
  QString name;
  QString qualifiedName;
  QVector<ProtoField> fields;
  QVector<ProtoMessage> nested;
};

/**
 * @brief Diagnostic output from the proto3 parser: line number and message string.
 */
struct ProtoParseError {
  int line = 0;
  QString message;
};

/**
 * @brief Maps a scalar type keyword to its ProtoScalar enum, or MessageRef if unrecognized.
 */
[[nodiscard]] ProtoScalar classifyProtoScalar(const QString& type);

/**
 * @brief Recursive-descent parser for a small proto3 subset (messages, scalars, nested types).
 *        Owns the parsed model: source text in, package + top-level messages out, with no
 *        project, session or dialog dependency.
 */
class ProtoParser {
public:
  explicit ProtoParser(const QString& source);
  ProtoParser(ProtoParser&&)                 = delete;
  ProtoParser(const ProtoParser&)            = delete;
  ProtoParser& operator=(ProtoParser&&)      = delete;
  ProtoParser& operator=(const ProtoParser&) = delete;

public:
  [[nodiscard]] bool parse();

  [[nodiscard]] QString package() const;
  [[nodiscard]] const ProtoParseError& error() const noexcept;
  [[nodiscard]] const QVector<ProtoMessage>& messages() const noexcept;

private:
  [[nodiscard]] bool accept(ProtoTokenKind kind);
  [[nodiscard]] bool expect(ProtoTokenKind kind, const QString& what);
  void advance();
  void skipToSemicolon();
  void skipBlock();
  void skipOptionList();

  [[nodiscard]] bool parseMessage(const QString& parentQualified, ProtoMessage& out, int depth);
  [[nodiscard]] bool parseField(ProtoMessage& msg);
  [[nodiscard]] bool parseOneof(ProtoMessage& msg);
  [[nodiscard]] bool parseMap(ProtoMessage& msg);
  [[nodiscard]] bool parseEnumBody();
  [[nodiscard]] bool parseFieldTag(int& tag);
  [[nodiscard]] int tryParseMessageBodyKeyword(ProtoMessage& out, int depth);

  static constexpr int kMaxMessageNestingDepth = 64;
  static constexpr int kMaxProtoFieldTag       = 536870911;

private:
  ProtoLexer m_lexer;
  ProtoToken m_cur;
  ProtoParseError m_error;
  QString m_package;
  QVector<ProtoMessage> m_messages;
};

}  // namespace DataModel
