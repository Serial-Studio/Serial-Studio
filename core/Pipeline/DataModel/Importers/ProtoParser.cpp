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

#include "DataModel/Importers/ProtoParser.h"

#include <QHash>
#include <QObject>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Scalar classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a scalar type keyword to its ProtoScalar enum, or MessageRef if unrecognized.
 */
DataModel::ProtoScalar DataModel::classifyProtoScalar(const QString& type)
{
  using S                              = DataModel::ProtoScalar;
  static const QHash<QString, S> table = {
    {  QStringLiteral("double"),   S::Double},
    {   QStringLiteral("float"),    S::Float},
    {   QStringLiteral("int32"),    S::Int32},
    {   QStringLiteral("int64"),    S::Int64},
    {  QStringLiteral("uint32"),   S::UInt32},
    {  QStringLiteral("uint64"),   S::UInt64},
    {  QStringLiteral("sint32"),   S::SInt32},
    {  QStringLiteral("sint64"),   S::SInt64},
    { QStringLiteral("fixed32"),  S::Fixed32},
    { QStringLiteral("fixed64"),  S::Fixed64},
    {QStringLiteral("sfixed32"), S::SFixed32},
    {QStringLiteral("sfixed64"), S::SFixed64},
    {    QStringLiteral("bool"),     S::Bool},
    {  QStringLiteral("string"),   S::String},
    {   QStringLiteral("bytes"),    S::Bytes},
  };
  return table.value(type, S::MessageRef);
}

//--------------------------------------------------------------------------------------------------
// Construction & results
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the parser over a proto3 source buffer; primes the lookahead token.
 */
DataModel::ProtoParser::ProtoParser(const QString& source) : m_lexer(source)
{
  m_cur = m_lexer.next();
}

/**
 * @brief Returns the `package` declared by the file, or an empty string when it declares none.
 */
QString DataModel::ProtoParser::package() const
{
  return m_package;
}

/**
 * @brief Returns the diagnostic recorded by the failing parse(); line 0 when none was recorded.
 */
const DataModel::ProtoParseError& DataModel::ProtoParser::error() const noexcept
{
  return m_error;
}

/**
 * @brief Returns the top-level messages collected by parse(), in declaration order.
 */
const QVector<DataModel::ProtoMessage>& DataModel::ProtoParser::messages() const noexcept
{
  return m_messages;
}

//--------------------------------------------------------------------------------------------------
// Token plumbing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true and advances if the current token matches @p kind.
 */
bool DataModel::ProtoParser::accept(ProtoTokenKind kind)
{
  if (m_cur.kind == kind) {
    advance();
    return true;
  }
  return false;
}

/**
 * @brief Consumes a required token kind or records a parse error and returns false.
 */
bool DataModel::ProtoParser::expect(ProtoTokenKind kind, const QString& what)
{
  if (m_cur.kind == kind) {
    advance();
    return true;
  }
  m_error = {m_cur.line, QObject::tr("Expected %1, got '%2'").arg(what, m_cur.text)};
  return false;
}

/**
 * @brief Advances the lookahead by one token.
 */
void DataModel::ProtoParser::advance()
{
  m_cur = m_lexer.next();
}

/**
 * @brief Skips tokens up to and including the next semicolon.
 */
void DataModel::ProtoParser::skipToSemicolon()
{
  while (m_cur.kind != ProtoTokenKind::Eof && m_cur.kind != ProtoTokenKind::Semi)
    advance();

  if (m_cur.kind == ProtoTokenKind::Semi)
    advance();
}

/**
 * @brief Skips a balanced `{ ... }` block starting at the current `{`.
 */
void DataModel::ProtoParser::skipBlock()
{
  if (!accept(ProtoTokenKind::LBrace))
    return;

  int depth = 1;
  while (m_cur.kind != ProtoTokenKind::Eof && depth > 0) {
    if (m_cur.kind == ProtoTokenKind::LBrace)
      ++depth;
    else if (m_cur.kind == ProtoTokenKind::RBrace)
      --depth;

    advance();
  }
}

/**
 * @brief Skips an optional `[ option = value, ... ]` clause, including nested brackets.
 */
void DataModel::ProtoParser::skipOptionList()
{
  if (m_cur.kind != ProtoTokenKind::LBracket)
    return;

  int depth = 1;
  advance();
  while (m_cur.kind != ProtoTokenKind::Eof && depth > 0) {
    if (m_cur.kind == ProtoTokenKind::LBracket)
      ++depth;
    else if (m_cur.kind == ProtoTokenKind::RBracket)
      --depth;

    advance();
  }
}

//--------------------------------------------------------------------------------------------------
// Declarations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses an `enum Name { ... }` block. Body is skipped (enum names accepted as field types).
 */
bool DataModel::ProtoParser::parseEnumBody()
{
  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected enum name after 'enum'")};
    return false;
  }
  advance();
  skipBlock();
  return true;
}

/**
 * @brief Parses a `oneof Name { fields }` block; fields are appended as plain fields.
 */
bool DataModel::ProtoParser::parseOneof(ProtoMessage& msg)
{
  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected oneof name")};
    return false;
  }
  advance();
  if (!expect(ProtoTokenKind::LBrace, QStringLiteral("'{'")))
    return false;

  while (m_cur.kind != ProtoTokenKind::Eof && m_cur.kind != ProtoTokenKind::RBrace) {
    if (m_cur.kind == ProtoTokenKind::Ident && m_cur.text == QLatin1String("option")) {
      skipToSemicolon();
      continue;
    }
    if (!parseField(msg))
      return false;
  }
  return expect(ProtoTokenKind::RBrace, QStringLiteral("'}'"));
}

/**
 * @brief Reads the current IntLit token as a protobuf field tag and validates its range.
 */
bool DataModel::ProtoParser::parseFieldTag(int& tag)
{
  SS_ASSERT(m_cur.kind == ProtoTokenKind::IntLit, return false);

  bool ok        = false;
  const int next = m_cur.text.toInt(&ok);
  if (!ok || next < 1 || next > kMaxProtoFieldTag) {
    m_error = {m_cur.line,
               QObject::tr("Field tag '%1' out of range (1..%2)")
                 .arg(m_cur.text, QString::number(kMaxProtoFieldTag))};
    return false;
  }

  tag = next;
  return true;
}

/**
 * @brief Parses a `map<K,V> name = tag;` declaration as a single MessageRef-like field.
 */
bool DataModel::ProtoParser::parseMap(ProtoMessage& msg)
{
  if (!expect(ProtoTokenKind::LAngle, QStringLiteral("'<'")))
    return false;

  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected key type in map<>")};
    return false;
  }
  advance();
  if (!expect(ProtoTokenKind::Comma, QStringLiteral("','")))
    return false;

  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected value type in map<>")};
    return false;
  }
  advance();
  if (!expect(ProtoTokenKind::RAngle, QStringLiteral("'>'")))
    return false;

  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected map field name")};
    return false;
  }
  ProtoField f;
  f.name     = m_cur.text;
  f.repeated = true;
  f.scalar   = ProtoScalar::Bytes;
  advance();
  if (!expect(ProtoTokenKind::Eq, QStringLiteral("'='")))
    return false;

  if (m_cur.kind != ProtoTokenKind::IntLit) {
    m_error = {m_cur.line, QObject::tr("Expected map field tag")};
    return false;
  }
  if (!parseFieldTag(f.tag))
    return false;

  advance();
  if (m_cur.kind == ProtoTokenKind::LBracket)
    skipBlock();

  if (!expect(ProtoTokenKind::Semi, QStringLiteral("';'")))
    return false;

  msg.fields.append(f);
  return true;
}

/**
 * @brief Parses a single field declaration inside a message body.
 */
bool DataModel::ProtoParser::parseField(ProtoMessage& msg)
{
  ProtoField f;

  if (m_cur.kind == ProtoTokenKind::Ident
      && (m_cur.text == QLatin1String("repeated") || m_cur.text == QLatin1String("optional")
          || m_cur.text == QLatin1String("required"))) {
    f.repeated = (m_cur.text == QLatin1String("repeated"));
    advance();
  }

  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected field type, got '%1'").arg(m_cur.text)};
    return false;
  }

  QString typeName = m_cur.text;
  advance();
  while (m_cur.kind != ProtoTokenKind::Eof && m_cur.kind == ProtoTokenKind::Dot) {
    typeName += QLatin1Char('.');
    advance();
    if (m_cur.kind == ProtoTokenKind::Ident) {
      typeName += m_cur.text;
      advance();
    }
  }

  f.scalar  = classifyProtoScalar(typeName);
  f.typeRef = typeName;

  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected field name after type")};
    return false;
  }
  f.name = m_cur.text;
  advance();

  if (!expect(ProtoTokenKind::Eq, QStringLiteral("'='")))
    return false;

  if (m_cur.kind != ProtoTokenKind::IntLit) {
    m_error = {m_cur.line, QObject::tr("Expected field tag number")};
    return false;
  }
  if (!parseFieldTag(f.tag))
    return false;

  advance();

  skipOptionList();

  if (!expect(ProtoTokenKind::Semi, QStringLiteral("';'")))
    return false;

  msg.fields.append(f);
  return true;
}

/**
 * @brief Parses a `message Name { ... }` block, recursively collecting nested messages.
 */
bool DataModel::ProtoParser::parseMessage(const QString& parentQualified,
                                          ProtoMessage& out,
                                          int depth)
{
  if (depth > kMaxMessageNestingDepth) {
    m_error = {m_cur.line,
               QObject::tr("Message nesting too deep (limit %1)").arg(kMaxMessageNestingDepth)};
    return false;
  }

  if (m_cur.kind != ProtoTokenKind::Ident) {
    m_error = {m_cur.line, QObject::tr("Expected message name")};
    return false;
  }
  out.name = m_cur.text;
  out.qualifiedName =
    parentQualified.isEmpty() ? out.name : (parentQualified + QLatin1Char('.') + out.name);
  advance();

  if (!expect(ProtoTokenKind::LBrace, QStringLiteral("'{'")))
    return false;

  while (m_cur.kind != ProtoTokenKind::Eof && m_cur.kind != ProtoTokenKind::RBrace) {
    if (m_cur.kind == ProtoTokenKind::Semi) {
      advance();
      continue;
    }

    const int kw = tryParseMessageBodyKeyword(out, depth);
    if (kw < 0)
      return false;

    if (kw > 0)
      continue;

    if (!parseField(out))
      return false;
  }

  return expect(ProtoTokenKind::RBrace, QStringLiteral("'}'"));
}

/**
 * @brief Returns 1 if the current token started a known body keyword, -1 on parse error, 0
 * otherwise.
 */
int DataModel::ProtoParser::tryParseMessageBodyKeyword(ProtoMessage& out, int depth)
{
  if (m_cur.kind != ProtoTokenKind::Ident)
    return 0;

  const QString kw = m_cur.text;
  if (kw == QLatin1String("option") || kw == QLatin1String("reserved")
      || kw == QLatin1String("extensions")) {
    skipToSemicolon();
    return 1;
  }
  if (kw == QLatin1String("enum")) {
    advance();
    return parseEnumBody() ? 1 : -1;
  }
  if (kw == QLatin1String("oneof")) {
    advance();
    return parseOneof(out) ? 1 : -1;
  }
  if (kw == QLatin1String("map")) {
    advance();
    return parseMap(out) ? 1 : -1;
  }
  if (kw == QLatin1String("message")) {
    advance();
    ProtoMessage nested;
    if (!parseMessage(out.qualifiedName, nested, depth + 1))
      return -1;

    out.nested.append(nested);
    return 1;
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------
// File scope
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses the whole file: syntax, package, top-level message/enum/option entries.
 */
bool DataModel::ProtoParser::parse()
{
  SS_ASSERT(m_messages.isEmpty(), m_messages.clear());

  while (m_cur.kind != ProtoTokenKind::Eof) {
    if (m_cur.kind == ProtoTokenKind::Semi) {
      advance();
      continue;
    }

    if (m_cur.kind != ProtoTokenKind::Ident) {
      m_error = {m_cur.line, QObject::tr("Unexpected token '%1' at file scope").arg(m_cur.text)};
      return false;
    }

    const QString kw = m_cur.text;
    if (kw == QLatin1String("syntax") || kw == QLatin1String("option")
        || kw == QLatin1String("import")) {
      skipToSemicolon();
      continue;
    }
    if (kw == QLatin1String("package")) {
      advance();
      QString pkg;
      while (m_cur.kind != ProtoTokenKind::Eof
             && (m_cur.kind == ProtoTokenKind::Ident || m_cur.kind == ProtoTokenKind::Dot)) {
        pkg += m_cur.text;
        advance();
      }
      m_package = pkg;
      skipToSemicolon();
      continue;
    }
    if (kw == QLatin1String("enum")) {
      advance();
      if (!parseEnumBody())
        return false;

      continue;
    }
    if (kw == QLatin1String("service")) {
      advance();
      if (m_cur.kind == ProtoTokenKind::Ident)
        advance();

      skipBlock();
      continue;
    }
    if (kw == QLatin1String("message")) {
      advance();
      ProtoMessage msg;
      if (!parseMessage(m_package, msg, 1))
        return false;

      m_messages.append(msg);
      continue;
    }

    m_error = {m_cur.line, QObject::tr("Unsupported top-level keyword '%1'").arg(kw)};
    return false;
  }
  return true;
}
