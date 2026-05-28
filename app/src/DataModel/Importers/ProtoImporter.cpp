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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Importers/ProtoImporter.h"

#include <functional>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QStandardPaths>
#include <QTextStream>

#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Internal: proto3 lexer + parser implementation
//--------------------------------------------------------------------------------------------------

namespace detail {

/**
 * @brief Token kinds produced by the proto3 lexer.
 */
enum class Tok {
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
struct Token {
  Tok type;
  QString text;
  int line;
};

/**
 * @brief Diagnostic output from the proto3 parser: line number and message string.
 */
struct ParseError {
  int line;
  QString message;
};

/**
 * @brief Streaming lexer for a proto3 source buffer.
 */
class Lexer {
public:
  explicit Lexer(const QString& src);

  Token next();

private:
  void skipWsAndComments();
  void skipLineComment();
  void skipBlockComment();
  Token lexStringLiteral(QChar quote, int line);

  QString m_src;
  int m_pos;
  int m_line;
};

/**
 * @brief Wraps a proto3 source buffer for streaming tokenization.
 */
Lexer::Lexer(const QString& src) : m_src(src), m_pos(0), m_line(1) {}

/**
 * @brief Skips whitespace, line comments, and block comments.
 */
void Lexer::skipWsAndComments()
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
void Lexer::skipLineComment()
{
  while (m_pos < m_src.size() && m_src[m_pos] != QLatin1Char('\n'))
    ++m_pos;
}

/**
 * @brief Skips a block comment, tracking line numbers across newlines.
 */
void Lexer::skipBlockComment()
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

/**
 * @brief Returns the next token from the source buffer, or Eof on end of input.
 */
Token Lexer::next()
{
  skipWsAndComments();

  if (m_pos >= m_src.size())
    return {Tok::Eof, QString(), m_line};

  const QChar c   = m_src[m_pos];
  const int line  = m_line;
  const int start = m_pos;

  // Identifier or keyword
  if (c.isLetter() || c == QLatin1Char('_')) {
    while (m_pos < m_src.size()
           && (m_src[m_pos].isLetterOrNumber() || m_src[m_pos] == QLatin1Char('_')))
      ++m_pos;
    return {Tok::Ident, m_src.mid(start, m_pos - start), line};
  }

  // Numeric literal (decimal, hex, or octal)
  if (c.isDigit()) {
    while (m_pos < m_src.size()
           && (m_src[m_pos].isLetterOrNumber() || m_src[m_pos] == QLatin1Char('.')
               || m_src[m_pos] == QLatin1Char('+') || m_src[m_pos] == QLatin1Char('-')))
      ++m_pos;
    return {Tok::IntLit, m_src.mid(start, m_pos - start), line};
  }

  if (c == QLatin1Char('"') || c == QLatin1Char('\''))
    return lexStringLiteral(c, line);

  ++m_pos;
  switch (c.unicode()) {
    case '{':
      return {Tok::LBrace, QStringLiteral("{"), line};
    case '}':
      return {Tok::RBrace, QStringLiteral("}"), line};
    case '[':
      return {Tok::LBracket, QStringLiteral("["), line};
    case ']':
      return {Tok::RBracket, QStringLiteral("]"), line};
    case '(':
      return {Tok::LParen, QStringLiteral("("), line};
    case ')':
      return {Tok::RParen, QStringLiteral(")"), line};
    case '<':
      return {Tok::LAngle, QStringLiteral("<"), line};
    case '>':
      return {Tok::RAngle, QStringLiteral(">"), line};
    case '=':
      return {Tok::Eq, QStringLiteral("="), line};
    case ',':
      return {Tok::Comma, QStringLiteral(","), line};
    case ';':
      return {Tok::Semi, QStringLiteral(";"), line};
    case '.':
      return {Tok::Dot, QStringLiteral("."), line};
    case '/':
      return {Tok::Slash, QStringLiteral("/"), line};
    case '-':
      return {Tok::Minus, QStringLiteral("-"), line};
    case '+':
      return {Tok::Plus, QStringLiteral("+"), line};
  }
  return {Tok::Error, QString(c), line};
}

/**
 * @brief Returns a StrLit token for a `"..."` or `'...'` body, honoring simple backslash escapes.
 */
Token Lexer::lexStringLiteral(QChar quote, int line)
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

  return {Tok::StrLit, s, line};
}

/**
 * @brief Maps a scalar type keyword to its ProtoScalar enum, or MessageRef if unrecognized.
 */
DataModel::ProtoScalar classifyScalar(const QString& type)
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

/**
 * @brief Recursive-descent parser for a small proto3 subset (messages, scalars, nested types).
 */
class Parser {
public:
  Parser(const QString& src, QString& packageOut, QVector<DataModel::ProtoMessage>& outMessages);

  bool parseFile(ParseError& err);

private:
  bool accept(Tok t);
  bool expect(Tok t, const QString& what, ParseError& err);
  void advance();
  void skipToSemicolon();
  void skipBlock();
  void skipOptionList();

  bool parseMessage(const QString& parentQualified, DataModel::ProtoMessage& out, ParseError& err);
  bool parseField(DataModel::ProtoMessage& msg, ParseError& err);
  bool parseOneof(DataModel::ProtoMessage& msg, ParseError& err);
  bool parseMap(DataModel::ProtoMessage& msg, ParseError& err);
  bool parseEnumBody(ParseError& err);
  int tryParseMessageBodyKeyword(DataModel::ProtoMessage& out, ParseError& err);

  Lexer m_lexer;
  Token m_cur;
  QString& m_packageOut;
  QVector<DataModel::ProtoMessage>& m_messages;
};

/**
 * @brief Constructs the parser over a proto3 source buffer; primes the lookahead token.
 */
Parser::Parser(const QString& src,
               QString& packageOut,
               QVector<DataModel::ProtoMessage>& outMessages)
  : m_lexer(src), m_packageOut(packageOut), m_messages(outMessages)
{
  m_cur = m_lexer.next();
}

/**
 * @brief Returns true and advances if the current token matches kind t.
 */
bool Parser::accept(Tok t)
{
  if (m_cur.type == t) {
    advance();
    return true;
  }
  return false;
}

/**
 * @brief Consumes a required token kind or records a parse error and returns false.
 */
bool Parser::expect(Tok t, const QString& what, ParseError& err)
{
  if (m_cur.type == t) {
    advance();
    return true;
  }
  err = {m_cur.line, QObject::tr("Expected %1, got '%2'").arg(what, m_cur.text)};
  return false;
}

/**
 * @brief Advances the lookahead by one token.
 */
void Parser::advance()
{
  m_cur = m_lexer.next();
}

/**
 * @brief Skips tokens up to and including the next semicolon.
 */
void Parser::skipToSemicolon()
{
  while (m_cur.type != Tok::Eof && m_cur.type != Tok::Semi)
    advance();

  if (m_cur.type == Tok::Semi)
    advance();
}

/**
 * @brief Skips a balanced `{ ... }` block starting at the current `{`.
 */
void Parser::skipBlock()
{
  if (!accept(Tok::LBrace))
    return;

  int depth = 1;
  while (m_cur.type != Tok::Eof && depth > 0) {
    if (m_cur.type == Tok::LBrace)
      ++depth;
    else if (m_cur.type == Tok::RBrace)
      --depth;

    advance();
  }
}

/**
 * @brief Parses an `enum Name { ... }` block. Body is skipped (enum names accepted as field types).
 */
bool Parser::parseEnumBody(ParseError& err)
{
  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected enum name after 'enum'")};
    return false;
  }
  advance();
  skipBlock();
  return true;
}

/**
 * @brief Parses a `oneof Name { fields }` block; fields are appended as plain fields.
 */
bool Parser::parseOneof(DataModel::ProtoMessage& msg, ParseError& err)
{
  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected oneof name")};
    return false;
  }
  advance();
  if (!expect(Tok::LBrace, QStringLiteral("'{'"), err))
    return false;

  while (m_cur.type != Tok::Eof && m_cur.type != Tok::RBrace) {
    if (m_cur.type == Tok::Ident && m_cur.text == QLatin1String("option")) {
      skipToSemicolon();
      continue;
    }
    if (!parseField(msg, err))
      return false;
  }
  return expect(Tok::RBrace, QStringLiteral("'}'"), err);
}

/**
 * @brief Parses a `map<K,V> name = tag;` declaration as a single MessageRef-like field.
 */
bool Parser::parseMap(DataModel::ProtoMessage& msg, ParseError& err)
{
  if (!expect(Tok::LAngle, QStringLiteral("'<'"), err))
    return false;

  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected key type in map<>")};
    return false;
  }
  advance();
  if (!expect(Tok::Comma, QStringLiteral("','"), err))
    return false;

  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected value type in map<>")};
    return false;
  }
  advance();
  if (!expect(Tok::RAngle, QStringLiteral("'>'"), err))
    return false;

  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected map field name")};
    return false;
  }
  DataModel::ProtoField f;
  f.name     = m_cur.text;
  f.repeated = true;
  f.scalar   = DataModel::ProtoScalar::Bytes;
  advance();
  if (!expect(Tok::Eq, QStringLiteral("'='"), err))
    return false;

  if (m_cur.type != Tok::IntLit) {
    err = {m_cur.line, QObject::tr("Expected map field tag")};
    return false;
  }
  f.tag = m_cur.text.toInt();
  advance();
  if (m_cur.type == Tok::LBracket)
    skipBlock();

  if (!expect(Tok::Semi, QStringLiteral("';'"), err))
    return false;

  msg.fields.append(f);
  return true;
}

/**
 * @brief Parses a single field declaration inside a message body.
 */
bool Parser::parseField(DataModel::ProtoMessage& msg, ParseError& err)
{
  DataModel::ProtoField f;

  if (m_cur.type == Tok::Ident
      && (m_cur.text == QLatin1String("repeated") || m_cur.text == QLatin1String("optional")
          || m_cur.text == QLatin1String("required"))) {
    f.repeated = (m_cur.text == QLatin1String("repeated"));
    advance();
  }

  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected field type, got '%1'").arg(m_cur.text)};
    return false;
  }

  // Support dotted names like `pkg.Type` and a leading dot for absolute references.
  QString typeName = m_cur.text;
  advance();
  while (m_cur.type == Tok::Dot) {
    typeName += QLatin1Char('.');
    advance();
    if (m_cur.type == Tok::Ident) {
      typeName += m_cur.text;
      advance();
    }
  }

  f.scalar  = classifyScalar(typeName);
  f.typeRef = typeName;

  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected field name after type")};
    return false;
  }
  f.name = m_cur.text;
  advance();

  if (!expect(Tok::Eq, QStringLiteral("'='"), err))
    return false;

  if (m_cur.type != Tok::IntLit) {
    err = {m_cur.line, QObject::tr("Expected field tag number")};
    return false;
  }
  f.tag = m_cur.text.toInt();
  advance();

  skipOptionList();

  if (!expect(Tok::Semi, QStringLiteral("';'"), err))
    return false;

  msg.fields.append(f);
  return true;
}

/**
 * @brief Skips an optional `[ option = value, ... ]` clause, including nested brackets.
 */
void Parser::skipOptionList()
{
  if (m_cur.type != Tok::LBracket)
    return;

  int depth = 1;
  advance();
  while (m_cur.type != Tok::Eof && depth > 0) {
    if (m_cur.type == Tok::LBracket)
      ++depth;
    else if (m_cur.type == Tok::RBracket)
      --depth;

    advance();
  }
}

/**
 * @brief Parses a `message Name { ... }` block, recursively collecting nested messages.
 */
bool Parser::parseMessage(const QString& parentQualified,
                          DataModel::ProtoMessage& out,
                          ParseError& err)
{
  if (m_cur.type != Tok::Ident) {
    err = {m_cur.line, QObject::tr("Expected message name")};
    return false;
  }
  out.name = m_cur.text;
  out.qualifiedName =
    parentQualified.isEmpty() ? out.name : (parentQualified + QLatin1Char('.') + out.name);
  advance();

  if (!expect(Tok::LBrace, QStringLiteral("'{'"), err))
    return false;

  while (m_cur.type != Tok::Eof && m_cur.type != Tok::RBrace) {
    if (m_cur.type == Tok::Semi) {
      advance();
      continue;
    }

    const int kw = tryParseMessageBodyKeyword(out, err);
    if (kw < 0)
      return false;

    if (kw > 0)
      continue;

    if (!parseField(out, err))
      return false;
  }

  return expect(Tok::RBrace, QStringLiteral("'}'"), err);
}

/**
 * @brief Returns 1 if the current token started a known body keyword, -1 on parse error, 0
 * otherwise.
 */
int Parser::tryParseMessageBodyKeyword(DataModel::ProtoMessage& out, ParseError& err)
{
  if (m_cur.type != Tok::Ident)
    return 0;

  const QString kw = m_cur.text;
  if (kw == QLatin1String("option") || kw == QLatin1String("reserved")
      || kw == QLatin1String("extensions")) {
    skipToSemicolon();
    return 1;
  }
  if (kw == QLatin1String("enum")) {
    advance();
    return parseEnumBody(err) ? 1 : -1;
  }
  if (kw == QLatin1String("oneof")) {
    advance();
    return parseOneof(out, err) ? 1 : -1;
  }
  if (kw == QLatin1String("map")) {
    advance();
    return parseMap(out, err) ? 1 : -1;
  }
  if (kw == QLatin1String("message")) {
    advance();
    DataModel::ProtoMessage nested;
    if (!parseMessage(out.qualifiedName, nested, err))
      return -1;

    out.nested.append(nested);
    return 1;
  }
  return 0;
}

/**
 * @brief Parses the whole file: syntax, package, top-level message/enum/option entries.
 */
bool Parser::parseFile(ParseError& err)
{
  while (m_cur.type != Tok::Eof) {
    if (m_cur.type == Tok::Semi) {
      advance();
      continue;
    }

    if (m_cur.type != Tok::Ident) {
      err = {m_cur.line, QObject::tr("Unexpected token '%1' at file scope").arg(m_cur.text)};
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
      while (m_cur.type == Tok::Ident || m_cur.type == Tok::Dot) {
        pkg += m_cur.text;
        advance();
      }
      m_packageOut = pkg;
      skipToSemicolon();
      continue;
    }
    if (kw == QLatin1String("enum")) {
      advance();
      if (!parseEnumBody(err))
        return false;

      continue;
    }
    if (kw == QLatin1String("service")) {
      advance();
      if (m_cur.type == Tok::Ident)
        advance();

      skipBlock();
      continue;
    }
    if (kw == QLatin1String("message")) {
      advance();
      DataModel::ProtoMessage msg;
      if (!parseMessage(m_packageOut, msg, err))
        return false;

      m_messages.append(msg);
      continue;
    }

    err = {m_cur.line, QObject::tr("Unsupported top-level keyword '%1'").arg(kw)};
    return false;
  }
  return true;
}

/**
 * @brief Returns true if the scalar should be modeled as a numeric dashboard dataset.
 */
bool isNumericScalar(DataModel::ProtoScalar s)
{
  return s != DataModel::ProtoScalar::String && s != DataModel::ProtoScalar::Bytes
      && s != DataModel::ProtoScalar::MessageRef;
}

/**
 * @brief Chooses a reasonable (min,max) widget range for a scalar field type.
 */
QPair<double, double> defaultRangeFor(DataModel::ProtoScalar s)
{
  using S = DataModel::ProtoScalar;
  switch (s) {
    case S::Bool:
      return {0.0, 1.0};
    case S::UInt32:
    case S::UInt64:
    case S::Fixed32:
    case S::Fixed64:
      return {0.0, 100.0};
    case S::Int32:
    case S::Int64:
    case S::SInt32:
    case S::SInt64:
    case S::SFixed32:
    case S::SFixed64:
      return {-100.0, 100.0};
    case S::Float:
    case S::Double:
      return {-100.0, 100.0};
    default:
      break;
  }
  return {0.0, 100.0};
}

}  // namespace detail

using namespace detail;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ProtoImporter singleton.
 */
DataModel::ProtoImporter::ProtoImporter() {}

/**
 * @brief Returns the singleton ProtoImporter instance.
 */
DataModel::ProtoImporter& DataModel::ProtoImporter::instance()
{
  static ProtoImporter instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of fields summed across every parsed top-level message.
 */
int DataModel::ProtoImporter::fieldCount() const noexcept
{
  int n = 0;
  for (const auto& m : m_messages)
    n += countFieldsRecursive(m);

  return n;
}

/**
 * @brief Returns the number of top-level messages parsed from the proto file.
 */
int DataModel::ProtoImporter::messageCount() const noexcept
{
  return m_messages.size();
}

/**
 * @brief Returns the file name (without directory) of the loaded proto file.
 */
QString DataModel::ProtoImporter::protoFileName() const
{
  return QFileInfo(m_protoFilePath).fileName();
}

/**
 * @brief Returns a "N: QualifiedName (X fields)" string for the preview message list.
 */
QString DataModel::ProtoImporter::messageInfo(int index) const
{
  const ProtoMessage* m = messageAt(index);
  if (!m)
    return QString();

  return QStringLiteral("%1: %2 (%3 fields)")
    .arg(index + 1)
    .arg(m->qualifiedName)
    .arg(countFieldsRecursive(*m));
}

/**
 * @brief Returns a "tag <name> : <type>" string for one field of a message.
 */
QString DataModel::ProtoImporter::fieldInfo(int messageIndex, int fieldIndex) const
{
  const ProtoMessage* m = messageAt(messageIndex);
  if (!m || fieldIndex < 0 || fieldIndex >= m->fields.size())
    return QString();

  const auto& f        = m->fields.at(fieldIndex);
  const QString prefix = f.repeated ? QStringLiteral("repeated ") : QString();
  return QStringLiteral("[%1] %2%3 %4").arg(QString::number(f.tag), prefix, f.typeRef, f.name);
}

//--------------------------------------------------------------------------------------------------
// User interface operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file dialog and triggers preview generation on selection.
 */
void DataModel::ProtoImporter::importProto()
{
  const auto p = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Import Protocol Buffers File"),
                                 p,
                                 tr("Proto Files (*.proto);;All Files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  // Defer to next tick; macOS NSSavePanel KVO callback must unwind first.
  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { showPreview(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Clears the parsed message state and notifies QML.
 */
void DataModel::ProtoImporter::cancelImport()
{
  m_messages.clear();
  m_protoFilePath.clear();
  Q_EMIT messagesChanged();
  Q_EMIT fileNameChanged();
}

/**
 * @brief Parses the proto file at filePath and emits previewReady on success.
 */
void DataModel::ProtoImporter::showPreview(const QString& filePath)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(
      tr("Failed to open proto file: %1").arg(file.errorString()),
      tr("Verify the file path and read permissions, then try again."),
      QMessageBox::Critical,
      tr("Protobuf Import Error"));
    return;
  }

  QTextStream stream(&file);
  const QString src = stream.readAll();
  file.close();

  QVector<ProtoMessage> messages;
  QString package;
  ParseError err{0, QString()};

  Parser parser(src, package, messages);
  if (!parser.parseFile(err)) {
    Misc::Utilities::showMessageBox(
      tr("Failed to parse proto file at line %1: %2").arg(QString::number(err.line), err.message),
      tr("Only proto3 syntax is supported. Verify the file format and try again."),
      QMessageBox::Critical,
      tr("Protobuf Import Error"));
    return;
  }

  if (messages.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("Proto file contains no message definitions"),
                                    tr("The selected file has no `message` blocks to import."),
                                    QMessageBox::Warning,
                                    tr("Protobuf Import Warning"));
    return;
  }

  m_messages      = messages;
  m_protoFilePath = filePath;

  Q_EMIT messagesChanged();
  Q_EMIT fileNameChanged();
  Q_EMIT previewReady();
}

/**
 * @brief Generates a project covering every top-level message and hands it to ProjectModel.
 */
void DataModel::ProtoImporter::confirmImport()
{
  if (m_messages.isEmpty())
    return;

  const auto project       = generateProject();
  const QString suggestion = QFileInfo(m_protoFilePath).baseName();

  const int messageCount = m_messages.size();
  int totalFields        = 0;
  for (const auto& m : m_messages)
    totalFields += countFieldsRecursive(m);

  auto& pm = ProjectModel::instance();
  QObject::connect(
    &pm,
    &ProjectModel::importCompleted,
    this,
    [messageCount, totalFields](bool accepted, const QString&) {
      if (!accepted)
        return;

      Misc::Utilities::showMessageBox(
        tr("Successfully imported %1 message(s) and %2 field(s) from the proto file.")
          .arg(messageCount)
          .arg(totalFields),
        tr("The project editor is now open for customization."),
        QMessageBox::Information,
        tr("Protobuf Import Complete"));
    },
    Qt::SingleShotConnection);

  pm.importProjectFromJson(project, suggestion);
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a complete .ssproj QJsonObject with one group per top-level message.
 */
QJsonObject DataModel::ProtoImporter::generateProject() const
{
  QJsonObject project;
  const auto info         = QFileInfo(m_protoFilePath);
  const auto projectTitle = info.baseName();

  project[Keys::Title]   = projectTitle;
  project[Keys::Actions] = QJsonArray();

  // Build groups + per-occurrence dispatch records for every top-level message
  QJsonArray groups;
  QVector<DispatchRecord> dispatchRecords;
  int groupIdCounter = 0;
  int datasetIndex   = 1;
  for (const auto& m : m_messages)
    buildGroups(m, QString(), 0, groupIdCounter, datasetIndex, groups, dispatchRecords);

  const int totalDatasets = datasetIndex - 1;

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("Protobuf");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::UART);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::ChecksumAlgorithm]     = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::DecoderMethod]         = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = generateFrameParser(totalDatasets, dispatchRecords);
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Lua);

  project[Keys::Sources] = QJsonArray{source};
  project[Keys::Groups]  = groups;

  return project;
}

/**
 * @brief Recursively emits one group per message + a dispatch record per occurrence.
 */
void DataModel::ProtoImporter::buildGroups(const ProtoMessage& message,
                                           const QString& parentFieldName,
                                           int sourceId,
                                           int& groupIdCounter,
                                           int& datasetIndexCounter,
                                           QJsonArray& groupsOut,
                                           QVector<DispatchRecord>& dispatchOut) const
{
  Group group;
  group.groupId  = groupIdCounter++;
  group.sourceId = sourceId;
  group.title    = parentFieldName.isEmpty() ? message.name : parentFieldName;
  group.widget   = selectGroupWidget(message);

  const bool isMultiPlot = (group.widget == SerialStudio::groupWidgetId(SerialStudio::MultiPlot));

  const int myDispatchIdx = dispatchOut.size();
  DispatchRecord rec;
  rec.msg              = &message;
  rec.isTopLevel       = parentFieldName.isEmpty();
  rec.baseDatasetIndex = datasetIndexCounter;
  rec.varName          = rec.isTopLevel ? QStringLiteral("dispatch_%1").arg(message.name)
                                        : QStringLiteral("dispatch_sub_%1").arg(myDispatchIdx);
  dispatchOut.append(rec);

  for (const auto& field : message.fields) {
    if (field.scalar == ProtoScalar::MessageRef)
      continue;

    if (!isNumericScalar(field.scalar) && field.scalar != ProtoScalar::String
        && field.scalar != ProtoScalar::Bytes && field.scalar != ProtoScalar::EnumRef)
      continue;

    Dataset d;
    d.index  = datasetIndexCounter++;
    d.title  = field.name;
    d.units  = QString();
    d.fft    = false;
    d.log    = true;
    d.led    = false;
    d.widget = QString();

    const bool isNumeric = isNumericScalar(field.scalar) && field.scalar != ProtoScalar::Bool;
    const bool isText = (field.scalar == ProtoScalar::String || field.scalar == ProtoScalar::Bytes);

    // Bool -> LED, text -> log only, numeric -> plot history
    if (field.scalar == ProtoScalar::Bool) {
      d.led     = true;
      d.ledHigh = 1;
      d.plt     = false;
    } else if (isText) {
      d.plt = false;
    } else {
      d.plt = isNumeric || isMultiPlot;
    }

    const auto range = defaultRangeFor(field.scalar);
    d.wgtMin         = range.first;
    d.wgtMax         = range.second;
    d.pltMin         = range.first;
    d.pltMax         = range.second;
    d.fftMin         = range.first;
    d.fftMax         = range.second;

    group.datasets.push_back(d);
  }

  groupsOut.append(serialize(group));

  // Recurse for each MessageRef field, recording child indices for parent dispatch
  for (const auto& field : message.fields) {
    if (field.scalar != ProtoScalar::MessageRef)
      continue;

    const auto* sub = findMessage(field.typeRef);
    if (sub) {
      const int childIdx = dispatchOut.size();
      dispatchOut[myDispatchIdx].childRecordIndex.append(childIdx);
      buildGroups(
        *sub, field.name, sourceId, groupIdCounter, datasetIndexCounter, groupsOut, dispatchOut);
    } else {
      dispatchOut[myDispatchIdx].childRecordIndex.append(-1);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Widget heuristics (restricted to DataGrid / MultiPlot / Plot per dataset)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns MultiPlot when contents look like a uniform channel block, else DataGrid.
 */
QString DataModel::ProtoImporter::selectGroupWidget(const ProtoMessage& message) const
{
  QVector<const ProtoField*> scalars;
  for (const auto& f : message.fields) {
    if (f.scalar == ProtoScalar::MessageRef)
      continue;

    if (!isNumericScalar(f.scalar) && f.scalar != ProtoScalar::String
        && f.scalar != ProtoScalar::Bytes && f.scalar != ProtoScalar::EnumRef)
      continue;
    scalars.append(&f);
  }

  const int n = scalars.size();
  if (n <= 1)
    return SerialStudio::groupWidgetId(SerialStudio::NoGroupWidget);

  // MultiPlot when 2-8 numeric non-bool scalars share the same wire type
  bool uniformNumeric     = (n >= 2 && n <= 8);
  ProtoScalar firstScalar = ProtoScalar::Int32;
  bool firstSet           = false;
  for (const auto* f : scalars) {
    if (!isNumericScalar(f->scalar) || f->scalar == ProtoScalar::Bool) {
      uniformNumeric = false;
      break;
    }
    if (!firstSet) {
      firstScalar = f->scalar;
      firstSet    = true;
    } else if (f->scalar != firstScalar) {
      uniformNumeric = false;
      break;
    }
  }

  if (uniformNumeric)
    return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);

  return SerialStudio::groupWidgetId(SerialStudio::DataGrid);
}

//--------------------------------------------------------------------------------------------------
// Frame parser generation (Lua)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Lua type tag (string used in dispatch entries) for a proto scalar.
 */
static QString luaTypeTagForScalar(DataModel::ProtoScalar s)
{
  switch (s) {
    case DataModel::ProtoScalar::Bool:
      return QStringLiteral("bool");
    case DataModel::ProtoScalar::Int32:
      return QStringLiteral("varint_s32");
    case DataModel::ProtoScalar::Int64:
      return QStringLiteral("varint_s64");
    case DataModel::ProtoScalar::UInt32:
      return QStringLiteral("varint_u32");
    case DataModel::ProtoScalar::UInt64:
      return QStringLiteral("varint_u64");
    case DataModel::ProtoScalar::SInt32:
      return QStringLiteral("sint32");
    case DataModel::ProtoScalar::SInt64:
      return QStringLiteral("sint64");
    case DataModel::ProtoScalar::Fixed32:
      return QStringLiteral("fixed32");
    case DataModel::ProtoScalar::Fixed64:
      return QStringLiteral("fixed64");
    case DataModel::ProtoScalar::SFixed32:
      return QStringLiteral("sfixed32");
    case DataModel::ProtoScalar::SFixed64:
      return QStringLiteral("sfixed64");
    case DataModel::ProtoScalar::Float:
      return QStringLiteral("float");
    case DataModel::ProtoScalar::Double:
      return QStringLiteral("double");
    case DataModel::ProtoScalar::String:
      return QStringLiteral("string");
    case DataModel::ProtoScalar::Bytes:
      return QStringLiteral("bytes");
    case DataModel::ProtoScalar::EnumRef:
      return QStringLiteral("enum");
    case DataModel::ProtoScalar::MessageRef:
      break;
  }
  return QStringLiteral("bytes");
}

/**
 * @brief Returns the protobuf wire type (0/1/2/5) implied by a scalar definition.
 */
static int wireTypeFor(DataModel::ProtoScalar s)
{
  using S = DataModel::ProtoScalar;
  switch (s) {
    case S::Bool:
    case S::Int32:
    case S::Int64:
    case S::UInt32:
    case S::UInt64:
    case S::SInt32:
    case S::SInt64:
    case S::EnumRef:
      return 0;  // varint
    case S::Fixed64:
    case S::SFixed64:
    case S::Double:
      return 1;  // 64-bit
    case S::String:
    case S::Bytes:
    case S::MessageRef:
      return 2;  // length-delimited
    case S::Fixed32:
    case S::SFixed32:
    case S::Float:
      return 5;  // 32-bit
  }
  return 2;
}

/**
 * @brief Generates the complete Lua frame parser (banner + helpers + tables + parse() entry).
 */
QString DataModel::ProtoImporter::generateFrameParser(int totalDatasets,
                                                      const QVector<DispatchRecord>& records) const
{
  QString code;
  emitParserBanner(code, QFileInfo(m_protoFilePath).fileName(), totalDatasets);
  code += QStringLiteral("local N_DATASETS = %1\n").arg(totalDatasets);
  code += QStringLiteral("local values = {}\n");
  code += QStringLiteral("for i = 1, N_DATASETS do values[i] = 0 end\n\n");
  code += frameParserDecoder();
  emitDispatchTables(code, records);
  emitTopLevelDispatchers(code, records);
  emitScoreDispatcher(code);
  emitParseEntry(code);
  return code;
}

/**
 * @brief Appends the auto-generated banner identifying the .proto source and dataset count.
 */
void DataModel::ProtoImporter::emitParserBanner(QString& code,
                                                const QString& sourceFile,
                                                int totalDatasets)
{
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Protobuf Frame Parser\n");
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Auto-generated from: %1\n").arg(sourceFile);
  code += QStringLiteral("-- Total fields:        %1\n").arg(totalDatasets);
  code += QStringLiteral("--\n");
  code +=
    QStringLiteral("-- The parser auto-detects which top-level message each incoming frame\n");
  code +=
    QStringLiteral("-- represents by scoring it against every dispatch table using field tag\n");
  code +=
    QStringLiteral("-- + wire-type signatures. Matching slots are updated; slots belonging to\n");
  code += QStringLiteral("-- other messages keep their previous values (protobuf's default).\n");
  code += QStringLiteral("-- Re-import the .proto file to regenerate this script.\n");
  code += QStringLiteral("--\n\n");
}

/**
 * @brief Emits dispatch tables in reverse so each var is defined before any parent references it.
 */
void DataModel::ProtoImporter::emitDispatchTables(QString& code,
                                                  const QVector<DispatchRecord>& records)
{
  for (int i = records.size() - 1; i >= 0; --i) {
    const auto& rec = records.at(i);

    int idx         = rec.baseDatasetIndex;
    int childCursor = 0;
    QString body    = QStringLiteral("{ ");
    bool first      = true;

    for (const auto& f : rec.msg->fields) {
      const QString entry = formatDispatchEntry(f, rec, records, idx, childCursor);
      if (entry.isEmpty())
        continue;

      if (!first)
        body += QStringLiteral(", ");

      body  += entry;
      first  = false;
    }

    body += QStringLiteral(" }");
    if (first)
      body = QStringLiteral("{}");

    code += QStringLiteral("-- Dispatch table for %1\n").arg(rec.msg->qualifiedName);
    code += QStringLiteral("local %1 = %2\n\n").arg(rec.varName, body);
  }
}

/**
 * @brief Formats one `[tag] = { ... }` Lua dispatch entry; returns empty when the field is skipped.
 */
QString DataModel::ProtoImporter::formatDispatchEntry(const ProtoField& field,
                                                      const DispatchRecord& rec,
                                                      const QVector<DispatchRecord>& records,
                                                      int& datasetIdx,
                                                      int& childCursor)
{
  if (field.scalar == ProtoScalar::MessageRef) {
    const int childRecIdx =
      (childCursor < rec.childRecordIndex.size()) ? rec.childRecordIndex.at(childCursor) : -1;
    ++childCursor;
    if (childRecIdx < 0) {
      const QString entry = QStringLiteral("[%1] = { type = 'bytes', wire = 2, out = %2 }")
                              .arg(field.tag)
                              .arg(datasetIdx);
      ++datasetIdx;
      return entry;
    }
    return QStringLiteral("[%1] = { type = 'message', wire = 2, table = %2 }")
      .arg(field.tag)
      .arg(records.at(childRecIdx).varName);
  }

  const bool emittable = isNumericScalar(field.scalar) || field.scalar == ProtoScalar::String
                      || field.scalar == ProtoScalar::Bytes || field.scalar == ProtoScalar::EnumRef;
  if (!emittable)
    return QString();

  const QString entry = QStringLiteral("[%1] = { type = '%2', wire = %3, out = %4 }")
                          .arg(field.tag)
                          .arg(luaTypeTagForScalar(field.scalar))
                          .arg(wireTypeFor(field.scalar))
                          .arg(datasetIdx);
  ++datasetIdx;
  return entry;
}

/**
 * @brief Emits the topDispatchers array (only records with isTopLevel == true).
 */
void DataModel::ProtoImporter::emitTopLevelDispatchers(QString& code,
                                                       const QVector<DispatchRecord>& records)
{
  code += QStringLiteral("local topDispatchers = {\n");
  for (const auto& rec : records) {
    if (!rec.isTopLevel)
      continue;

    code += QStringLiteral("  { name = \"%1\", tbl = %2 },\n").arg(rec.msg->name, rec.varName);
  }
  code += QStringLiteral("}\n\n");
}

/**
 * @brief Emits scoreDispatcher: tallies matched/mismatched wire types per dispatch table.
 */
void DataModel::ProtoImporter::emitScoreDispatcher(QString& code)
{
  code += QStringLiteral("local function scoreDispatcher(buf, tbl, bufLen)\n");
  code += QStringLiteral("  local p = 1\n");
  code += QStringLiteral("  local endP = bufLen + 1\n");
  code += QStringLiteral("  local good = 0\n");
  code += QStringLiteral("  local bad = 0\n");
  code += QStringLiteral("  local iter = 0\n");
  code += QStringLiteral("  while p < endP do\n");
  code += QStringLiteral("    iter = iter + 1\n");
  code += QStringLiteral("    if iter > 4096 then break end\n");
  code += QStringLiteral("    local before = p\n");
  code += QStringLiteral("    local tag, np = readVarint(buf, p, bufLen)\n");
  code += QStringLiteral("    if np == p then return -1 end\n");
  code += QStringLiteral("    p = np\n");
  code += QStringLiteral("    local fieldNum = tag >> 3\n");
  code += QStringLiteral("    local wt = tag & 7\n");
  code += QStringLiteral("    local e = tbl[fieldNum]\n");
  code += QStringLiteral("    if e ~= nil then\n");
  code += QStringLiteral("      if e.wire == wt then good = good + 1\n");
  code += QStringLiteral("      else bad = bad + 1 end\n");
  code += QStringLiteral("    end\n");
  code += QStringLiteral("    p = skipWire(buf, p, wt, bufLen)\n");
  code += QStringLiteral("    if p <= before or p > endP then break end\n");
  code += QStringLiteral("  end\n");
  code += QStringLiteral("  return good - bad\n");
  code += QStringLiteral("end\n\n");
}

/**
 * @brief Emits parse(): scores all top-level dispatchers and decodes with the winner.
 */
void DataModel::ProtoImporter::emitParseEntry(QString& code)
{
  code += QStringLiteral("function parse(frame)\n");
  code += QStringLiteral("  if type(frame) ~= 'table' then return values end\n");
  code += QStringLiteral("  local bufLen = #frame\n");
  code += QStringLiteral("  if bufLen <= 0 then return values end\n");
  code += QStringLiteral("  pcall(function()\n");
  code += QStringLiteral("    local best = nil\n");
  code += QStringLiteral("    local bestScore = 0\n");
  code += QStringLiteral("    for _, d in ipairs(topDispatchers) do\n");
  code += QStringLiteral("      local s = scoreDispatcher(frame, d.tbl, bufLen)\n");
  code += QStringLiteral("      if s > bestScore then\n");
  code += QStringLiteral("        bestScore = s\n");
  code += QStringLiteral("        best = d.tbl\n");
  code += QStringLiteral("      end\n");
  code += QStringLiteral("    end\n");
  code += QStringLiteral("    if best ~= nil then\n");
  code += QStringLiteral("      parseMsg(frame, 1, bufLen + 1, best, bufLen, 0)\n");
  code += QStringLiteral("    end\n");
  code += QStringLiteral("  end)\n");
  code += QStringLiteral("  return values\n");
  code += QStringLiteral("end\n");
}

/**
 * @brief Returns the inline Lua wire-format decoder helpers + parseMsg.
 */
QString DataModel::ProtoImporter::frameParserDecoder() const
{
  QString code;
  code += QStringLiteral("-- Wire-format decoder helpers.\n");
  code += QStringLiteral("-- `frame` arrives as a 1-indexed table of bytes (each 0..255).\n");
  code += QStringLiteral("-- Positions below are also 1-indexed; sub-ranges use [start, end)\n");
  code += QStringLiteral("-- where end is one past the last byte.\n\n");
  emitDecoderReaders(code);
  emitDecoderParseMsg(code);
  return code;
}

/**
 * @brief Emits the full decoder reader prelude: varints, fixed-width, strings, skipWire.
 */
void DataModel::ProtoImporter::emitDecoderReaders(QString& code)
{
  emitVarintAndSignedReaders(code);
  emitFixedAndFloatReaders(code);
  emitStringAndSkipWireReaders(code);
}

/**
 * @brief Emits readVarint, zigzag, signed32, signed64, and bytesToString helpers.
 */
void DataModel::ProtoImporter::emitVarintAndSignedReaders(QString& code)
{
  code += QStringLiteral("local function readVarint(buf, p, bufLen)\n");
  code += QStringLiteral("  local v = 0\n");
  code += QStringLiteral("  local mul = 1\n");
  code += QStringLiteral("  for _ = 1, 10 do\n");
  code += QStringLiteral("    if p > bufLen then return v, p end\n");
  code += QStringLiteral("    local b = buf[p]\n");
  code += QStringLiteral("    if b == nil then return v, p end\n");
  code += QStringLiteral("    p = p + 1\n");
  code += QStringLiteral("    v = v + (b & 0x7f) * mul\n");
  code += QStringLiteral("    if (b & 0x80) == 0 then return v, p end\n");
  code += QStringLiteral("    mul = mul * 128\n");
  code += QStringLiteral("  end\n");
  code += QStringLiteral("  return v, p\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function zigzag(n)\n");
  code += QStringLiteral("  if (n & 1) == 0 then return n // 2 end\n");
  code += QStringLiteral("  return -((n + 1) // 2)\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function signed32(n)\n");
  code += QStringLiteral("  if n >= 0x80000000 then return n - 0x100000000 end\n");
  code += QStringLiteral("  return n\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function signed64(n)\n");
  code += QStringLiteral("  if n >= 0x8000000000000000 then return n - 0x10000000000000000 end\n");
  code += QStringLiteral("  return n\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function bytesToString(buf, p, n, bufLen)\n");
  code += QStringLiteral("  if n <= 0 or p < 1 then return \"\" end\n");
  code += QStringLiteral("  local available = bufLen - p + 1\n");
  code += QStringLiteral("  if available <= 0 then return \"\" end\n");
  code += QStringLiteral("  if n > available then n = available end\n");
  code += QStringLiteral("  local t = {}\n");
  code += QStringLiteral("  for i = 1, n do\n");
  code += QStringLiteral("    local b = buf[p + i - 1]\n");
  code += QStringLiteral("    if b == nil then break end\n");
  code += QStringLiteral("    t[i] = string.char(b & 0xff)\n");
  code += QStringLiteral("  end\n");
  code += QStringLiteral("  return table.concat(t)\n");
  code += QStringLiteral("end\n\n");
}

/**
 * @brief Emits readFixed32/64, readSFixed32/64, readFloat32/64 little-endian helpers.
 */
void DataModel::ProtoImporter::emitFixedAndFloatReaders(QString& code)
{
  code += QStringLiteral("local function readFixed32(buf, p, bufLen)\n");
  code += QStringLiteral("  if p < 1 or p + 3 > bufLen then return 0, p + 4 end\n");
  code +=
    QStringLiteral("  return string.unpack(\"<I4\", bytesToString(buf, p, 4, bufLen)), p + 4\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function readSFixed32(buf, p, bufLen)\n");
  code += QStringLiteral("  if p < 1 or p + 3 > bufLen then return 0, p + 4 end\n");
  code +=
    QStringLiteral("  return string.unpack(\"<i4\", bytesToString(buf, p, 4, bufLen)), p + 4\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function readFloat32(buf, p, bufLen)\n");
  code += QStringLiteral("  if p < 1 or p + 3 > bufLen then return 0, p + 4 end\n");
  code +=
    QStringLiteral("  return string.unpack(\"<f\", bytesToString(buf, p, 4, bufLen)), p + 4\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function readFixed64(buf, p, bufLen)\n");
  code += QStringLiteral("  if p < 1 or p + 7 > bufLen then return 0, p + 8 end\n");
  code +=
    QStringLiteral("  return string.unpack(\"<I8\", bytesToString(buf, p, 8, bufLen)), p + 8\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function readSFixed64(buf, p, bufLen)\n");
  code += QStringLiteral("  if p < 1 or p + 7 > bufLen then return 0, p + 8 end\n");
  code +=
    QStringLiteral("  return string.unpack(\"<i8\", bytesToString(buf, p, 8, bufLen)), p + 8\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function readFloat64(buf, p, bufLen)\n");
  code += QStringLiteral("  if p < 1 or p + 7 > bufLen then return 0, p + 8 end\n");
  code +=
    QStringLiteral("  return string.unpack(\"<d\", bytesToString(buf, p, 8, bufLen)), p + 8\n");
  code += QStringLiteral("end\n\n");
}

/**
 * @brief Emits readString and skipWire (length and offset clamped against bufLen).
 */
void DataModel::ProtoImporter::emitStringAndSkipWireReaders(QString& code)
{
  code += QStringLiteral("local function readString(buf, p, bufLen)\n");
  code += QStringLiteral("  local len, np = readVarint(buf, p, bufLen)\n");
  code += QStringLiteral("  if len < 0 then return \"\", np end\n");
  code += QStringLiteral("  local available = bufLen - np + 1\n");
  code += QStringLiteral("  if available < 0 then available = 0 end\n");
  code += QStringLiteral("  if len > available then len = available end\n");
  code += QStringLiteral("  return bytesToString(buf, np, len, bufLen), np + len\n");
  code += QStringLiteral("end\n\n");

  code += QStringLiteral("local function skipWire(buf, p, wt, bufLen)\n");
  code += QStringLiteral("  if wt == 0 then\n");
  code += QStringLiteral("    local _, np = readVarint(buf, p, bufLen) return np\n");
  code += QStringLiteral("  elseif wt == 1 then return p + 8\n");
  code += QStringLiteral("  elseif wt == 2 then\n");
  code += QStringLiteral("    local len, np = readVarint(buf, p, bufLen)\n");
  code += QStringLiteral("    if len < 0 then return bufLen + 1 end\n");
  code += QStringLiteral("    local target = np + len\n");
  code += QStringLiteral("    if target < np or target > bufLen + 1 then return bufLen + 1 end\n");
  code += QStringLiteral("    return target\n");
  code += QStringLiteral("  elseif wt == 5 then return p + 4\n");
  code += QStringLiteral("  end\n");
  code += QStringLiteral("  return bufLen + 1\n");
  code += QStringLiteral("end\n\n");
}

/**
 * @brief Emits parseMsg(): tag-dispatch loop with bogus-length and infinite-loop guards.
 */
void DataModel::ProtoImporter::emitDecoderParseMsg(QString& code)
{
  code += QStringLiteral("local parseMsg\n");
  code += QStringLiteral("parseMsg = function(buf, startPos, endPos, tbl, bufLen, depth)\n");
  code += QStringLiteral("  if depth > 8 then return end\n");
  code += QStringLiteral("  if startPos < 1 then return end\n");
  code += QStringLiteral("  local maxEnd = bufLen + 1\n");
  code += QStringLiteral("  if endPos > maxEnd then endPos = maxEnd end\n");
  code += QStringLiteral("  if endPos <= startPos then return end\n");
  code += QStringLiteral("  local p = startPos\n");
  code += QStringLiteral("  local iter = 0\n");
  code += QStringLiteral("  while p < endPos do\n");
  code += QStringLiteral("    iter = iter + 1\n");
  code += QStringLiteral("    if iter > 8192 then break end\n");
  code += QStringLiteral("    local before = p\n");
  code += QStringLiteral("    local tag, np = readVarint(buf, p, bufLen)\n");
  code += QStringLiteral("    if np == p then break end\n");
  code += QStringLiteral("    p = np\n");
  code += QStringLiteral("    local fieldNum = tag >> 3\n");
  code += QStringLiteral("    local wt = tag & 7\n");
  code += QStringLiteral("    local e = tbl[fieldNum]\n");
  code += QStringLiteral("    if e == nil or e.wire ~= wt then\n");
  code += QStringLiteral("      p = skipWire(buf, p, wt, bufLen)\n");
  code += QStringLiteral("    else\n");
  code += QStringLiteral("      local t = e.type\n");
  code += QStringLiteral("      if t == 'message' then\n");
  code += QStringLiteral("        local len, lp = readVarint(buf, p, bufLen)\n");
  code += QStringLiteral("        if len < 0 then break end\n");
  code += QStringLiteral("        local target = lp + len\n");
  code += QStringLiteral("        if target < lp or target > maxEnd then target = maxEnd end\n");
  code +=
    QStringLiteral("        parseMsg(buf, lp, target, e.table, bufLen, depth + 1) p = target\n");
  code += QStringLiteral("      elseif t == 'string' or t == 'bytes' then\n");
  code +=
    QStringLiteral("        local v, npp = readString(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral(
    "      elseif t == 'bool' or t == 'enum' or t == 'varint_u32' or t == 'varint_u64' then\n");
  code +=
    QStringLiteral("        local v, npp = readVarint(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      elseif t == 'varint_s32' then\n");
  code += QStringLiteral(
    "        local v, npp = readVarint(buf, p, bufLen) p = npp values[e.out] = signed32(v)\n");
  code += QStringLiteral("      elseif t == 'varint_s64' then\n");
  code += QStringLiteral(
    "        local v, npp = readVarint(buf, p, bufLen) p = npp values[e.out] = signed64(v)\n");
  code += QStringLiteral("      elseif t == 'sint32' or t == 'sint64' then\n");
  code += QStringLiteral(
    "        local v, npp = readVarint(buf, p, bufLen) p = npp values[e.out] = zigzag(v)\n");
  code += QStringLiteral("      elseif t == 'fixed32' then\n");
  code += QStringLiteral(
    "        local v, npp = readFixed32(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      elseif t == 'sfixed32' then\n");
  code += QStringLiteral(
    "        local v, npp = readSFixed32(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      elseif t == 'float' then\n");
  code += QStringLiteral(
    "        local v, npp = readFloat32(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      elseif t == 'fixed64' then\n");
  code += QStringLiteral(
    "        local v, npp = readFixed64(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      elseif t == 'sfixed64' then\n");
  code += QStringLiteral(
    "        local v, npp = readSFixed64(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      elseif t == 'double' then\n");
  code += QStringLiteral(
    "        local v, npp = readFloat64(buf, p, bufLen) p = npp values[e.out] = v\n");
  code += QStringLiteral("      else\n");
  code += QStringLiteral("        p = skipWire(buf, p, wt, bufLen)\n");
  code += QStringLiteral("      end\n");
  code += QStringLiteral("    end\n");
  code += QStringLiteral("    if p <= before or p > endPos then break end\n");
  code += QStringLiteral("  end\n");
  code += QStringLiteral("end\n\n");
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Looks up a message by qualified name, dotted partial name, or unqualified name.
 */
const DataModel::ProtoMessage* DataModel::ProtoImporter::findMessage(const QString& typeRef) const
{
  if (typeRef.isEmpty())
    return nullptr;

  QString needle = typeRef;
  while (needle.startsWith(QLatin1Char('.')))
    needle.remove(0, 1);

  // Exact qualified match across top-level + every nested level.
  std::function<const ProtoMessage*(const ProtoMessage&)> walk =
    [&](const ProtoMessage& m) -> const ProtoMessage* {
    if (m.qualifiedName == needle || m.name == needle)
      return &m;

    for (const auto& sub : m.nested)
      if (const auto* r = walk(sub))
        return r;

    return nullptr;
  };

  for (const auto& m : m_messages)
    if (const auto* r = walk(m))
      return r;

  return nullptr;
}

/**
 * @brief Returns a pointer to the n-th top-level message, or nullptr if out of range.
 */
const DataModel::ProtoMessage* DataModel::ProtoImporter::messageAt(int index) const
{
  if (index < 0 || index >= m_messages.size())
    return nullptr;

  return &m_messages.at(index);
}

/**
 * @brief Counts immediate scalar fields in a message (no descent into submessages).
 */
int DataModel::ProtoImporter::countScalarFields(const ProtoMessage& message) const
{
  int n = 0;
  for (const auto& f : message.fields) {
    if (f.scalar == ProtoScalar::MessageRef)
      continue;

    ++n;
  }
  return n;
}

/**
 * @brief Counts scalar fields recursively, descending into referenced submessages.
 */
int DataModel::ProtoImporter::countFieldsRecursive(const ProtoMessage& message) const
{
  int n = countScalarFields(message);
  for (const auto& f : message.fields) {
    if (f.scalar != ProtoScalar::MessageRef)
      continue;

    if (const auto* sub = findMessage(f.typeRef))
      n += countFieldsRecursive(*sub);
  }
  return n;
}
