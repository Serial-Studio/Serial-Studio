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

#include "DataModel/Scripting/ExpressionTransform.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QChar>
#include <QObject>
#include <QStringView>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

namespace DataModel::Expression {

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief Nesting bound for parentheses and conditionals (recursion depth of the parser).
 */
static constexpr int kMaxNesting = 32;

/**
 * @brief Binary operator table: symbol, precedence (higher binds tighter), right-assoc, opcode.
 */
struct BinaryOp {
  const char* symbol;
  int precedence;
  bool rightAssoc;
  Op op;
};

// clang-format off
static constexpr BinaryOp kBinaryOps[] = {
  {"||", 1, false, Op::Or},
  {"&&", 2, false, Op::And},
  {"==", 3, false, Op::Eq},
  {"!=", 3, false, Op::Ne},
  {"<",  4, false, Op::Lt},
  {"<=", 4, false, Op::Le},
  {">",  4, false, Op::Gt},
  {">=", 4, false, Op::Ge},
  {"+",  5, false, Op::Add},
  {"-",  5, false, Op::Sub},
  {"*",  6, false, Op::Mul},
  {"/",  6, false, Op::Div},
  {"%",  6, false, Op::Mod},
  {"^",  7, true,  Op::Pow},
};
// clang-format on

//--------------------------------------------------------------------------------------------------
// Function table
//--------------------------------------------------------------------------------------------------

/**
 * @brief One callable in the fixed function set: name, arity and the IEEE-double implementation.
 */
struct FunctionSpec {
  const char* name;
  int arity;
  double (*fn1)(double);
  double (*fn2)(double, double);
  double (*fn3)(double, double, double);
};

static constexpr double kDegPerRad = 180.0 / M_PI;
static constexpr double kRadPerDeg = M_PI / 180.0;

// code-verify off
// The user-facing function set is a table of pointer-to-function; pow() is deliberate here.
static constexpr FunctionSpec kFunctions[] = {
  {"abs", 1, +[](double a) { return std::fabs(a); }, nullptr, nullptr},
  {"floor", 1, +[](double a) { return std::floor(a); }, nullptr, nullptr},
  {"ceil", 1, +[](double a) { return std::ceil(a); }, nullptr, nullptr},
  {"round", 1, +[](double a) { return std::round(a); }, nullptr, nullptr},
  {"sqrt", 1, +[](double a) { return std::sqrt(a); }, nullptr, nullptr},
  {"cbrt", 1, +[](double a) { return std::cbrt(a); }, nullptr, nullptr},
  {"exp", 1, +[](double a) { return std::exp(a); }, nullptr, nullptr},
  {"log", 1, +[](double a) { return std::log(a); }, nullptr, nullptr},
  {"log10", 1, +[](double a) { return std::log10(a); }, nullptr, nullptr},
  {"log2", 1, +[](double a) { return std::log2(a); }, nullptr, nullptr},
  {"sin", 1, +[](double a) { return std::sin(a); }, nullptr, nullptr},
  {"cos", 1, +[](double a) { return std::cos(a); }, nullptr, nullptr},
  {"tan", 1, +[](double a) { return std::tan(a); }, nullptr, nullptr},
  {"asin", 1, +[](double a) { return std::asin(a); }, nullptr, nullptr},
  {"acos", 1, +[](double a) { return std::acos(a); }, nullptr, nullptr},
  {"atan", 1, +[](double a) { return std::atan(a); }, nullptr, nullptr},
  {"sinh", 1, +[](double a) { return std::sinh(a); }, nullptr, nullptr},
  {"cosh", 1, +[](double a) { return std::cosh(a); }, nullptr, nullptr},
  {"tanh", 1, +[](double a) { return std::tanh(a); }, nullptr, nullptr},
  {"deg", 1, +[](double a) { return a * kDegPerRad; }, nullptr, nullptr},
  {"rad", 1, +[](double a) { return a * kRadPerDeg; }, nullptr, nullptr},
  {"min", 2, nullptr, +[](double a, double b) { return std::fmin(a, b); }, nullptr},
  {"max", 2, nullptr, +[](double a, double b) { return std::fmax(a, b); }, nullptr},
  {"pow", 2, nullptr, +[](double a, double b) { return std::pow(a, b); }, nullptr},
  {"atan2", 2, nullptr, +[](double a, double b) { return std::atan2(a, b); }, nullptr},
  {"hypot", 2, nullptr, +[](double a, double b) { return std::hypot(a, b); }, nullptr},
  {"clamp",
   3, nullptr,
   nullptr, +[](double a, double lo, double hi) {
     return std::fmin(std::fmax(a, lo), hi);
   }},
  {"lerp",
   3, nullptr,
   nullptr, +[](double a, double b, double f) {
     return a + (b - a) * f;
   }},
};
// code-verify on

static constexpr int kFunctionCount = static_cast<int>(sizeof(kFunctions) / sizeof(kFunctions[0]));

/**
 * @brief Index of the named function in kFunctions, or -1.
 */
[[nodiscard]] static int functionIndex(QStringView name)
{
  for (int i = 0; i < kFunctionCount; ++i)
    if (name == QLatin1StringView(kFunctions[i].name))
      return i;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Tokenizer
//--------------------------------------------------------------------------------------------------

enum class TokenKind : std::uint8_t {
  End,
  Number,
  Identifier,
  Symbol,
  Error
};

/**
 * @brief One lexical token: kind, source column, numeric payload, and the identifier / symbol
 *        text as a view into the expression string.
 */
struct Token {
  TokenKind kind;
  int column;
  double number;
  QStringView text;
};

/**
 * @brief Cursor over the expression text with one token of lookahead.
 */
struct Lexer {
  QStringView src;
  int pos;
  Token current;
};

/**
 * @brief Letter or underscore opens a bare identifier.
 */
[[nodiscard]] static bool isIdentStart(QChar c)
{
  return c.isLetter() || c == QLatin1Char('_');
}

/**
 * @brief Letters, digits and underscores continue a bare identifier.
 */
[[nodiscard]] static bool isIdentBody(QChar c)
{
  return c.isLetterOrNumber() || c == QLatin1Char('_');
}

/**
 * @brief Reads a decimal literal (digits, optional fraction and exponent) starting at @p pos.
 */
[[nodiscard]] static Token lexNumber(QStringView src, int pos)
{
  int end = pos;
  for (; end < src.size() && (src[end].isDigit() || src[end] == QLatin1Char('.')); ++end) {
  }

  if (end < src.size() && (src[end] == QLatin1Char('e') || src[end] == QLatin1Char('E'))) {
    int expEnd = end + 1;
    if (expEnd < src.size() && (src[expEnd] == QLatin1Char('+') || src[expEnd] == QLatin1Char('-')))
      ++expEnd;

    if (expEnd < src.size() && src[expEnd].isDigit()) {
      for (; expEnd < src.size() && src[expEnd].isDigit(); ++expEnd) {
      }

      end = expEnd;
    }
  }

  const QStringView literal = src.mid(pos, end - pos);
  const bool oneDot         = literal.count(QLatin1Char('.')) <= 1;
  bool ok                   = false;
  const double value        = SerialStudio::toDouble(literal, &ok);
  return Token{(ok && oneDot) ? TokenKind::Number : TokenKind::Error, pos, value, literal};
}

/**
 * @brief Reads a `{braced name}` (any characters up to the closing brace) or a bare identifier.
 */
[[nodiscard]] static Token lexIdentifier(QStringView src, int pos)
{
  if (src[pos] == QLatin1Char('{')) {
    const qsizetype close = src.indexOf(QLatin1Char('}'), pos + 1);
    if (close < 0)
      return Token{TokenKind::Error, pos, 0.0, src.mid(pos)};

    return Token{TokenKind::Identifier, pos, 0.0, src.mid(pos + 1, close - pos - 1).trimmed()};
  }

  int end = pos + 1;
  for (; end < src.size() && isIdentBody(src[end]); ++end) {
  }

  return Token{TokenKind::Identifier, pos, 0.0, src.mid(pos, end - pos)};
}

/**
 * @brief Reads one operator token, preferring the two-character forms.
 */
[[nodiscard]] static Token lexSymbol(QStringView src, int pos)
{
  static constexpr const char* kTwoChar[] = {"<=", ">=", "==", "!=", "&&", "||"};
  if (pos + 1 < src.size()) {
    const QStringView pair = src.mid(pos, 2);
    for (const char* two : kTwoChar)
      if (pair == QLatin1StringView(two))
        return Token{TokenKind::Symbol, pos, 0.0, pair};
  }

  static constexpr const char* kOneChar = "+-*/%^()?:,<>!";
  const QChar c                         = src[pos];
  for (const char* p = kOneChar; *p != '\0'; ++p)
    if (c == QLatin1Char(*p))
      return Token{TokenKind::Symbol, pos, 0.0, src.mid(pos, 1)};

  return Token{TokenKind::Error, pos, 0.0, src.mid(pos, 1)};
}

/**
 * @brief Advances the lexer to the next token, skipping whitespace and `#` comments; `#` cannot
 *        start any token, so a comment never collides with the operator set.
 */
static void nextToken(Lexer& lx)
{
  const QStringView src = lx.src;
  while (lx.pos < src.size()) {
    if (src[lx.pos].isSpace()) {
      ++lx.pos;
      continue;
    }

    if (src[lx.pos] != QLatin1Char('#'))
      break;

    while (lx.pos < src.size() && src[lx.pos] != QLatin1Char('\n'))
      ++lx.pos;
  }

  if (lx.pos >= src.size()) {
    lx.current = Token{TokenKind::End, lx.pos, 0.0, QStringView()};
    return;
  }

  const QChar c = src[lx.pos];
  if (c.isDigit()
      || (c == QLatin1Char('.') && lx.pos + 1 < src.size() && src[lx.pos + 1].isDigit()))
    lx.current = lexNumber(src, lx.pos);
  else if (isIdentStart(c) || c == QLatin1Char('{'))
    lx.current = lexIdentifier(src, lx.pos);
  else
    lx.current = lexSymbol(src, lx.pos);

  int consumed = static_cast<int>(lx.current.text.size());
  if (c == QLatin1Char('{') && lx.current.kind == TokenKind::Identifier)
    consumed = static_cast<int>(src.indexOf(QLatin1Char('}'), lx.pos + 1)) - lx.pos + 1;

  lx.pos += std::max(1, consumed);
}

//--------------------------------------------------------------------------------------------------
// Parser (precedence climbing to postfix)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parser state: lexer, output program, the resolver, and the first error seen.
 */
struct Parser {
  Lexer lx;
  Program* out;
  const NameResolver* resolver;
  const TableResolver* tables;
  QString error;
  int depth;
  int maxDepth;
  int nesting;
};

/**
 * @brief Records the first error with its column and returns false for the caller to propagate.
 */
[[nodiscard]] static bool parseFail(Parser& p, const QString& what)
{
  if (p.error.isEmpty())
    p.error = QObject::tr("%1 (column %2)").arg(what, QString::number(p.lx.current.column + 1));

  return false;
}

/**
 * @brief Appends a node, tracking the stack depth it produces; false past kMaxNodes / depth.
 */
[[nodiscard]] static bool emitNode(Parser& p, Op op, std::int32_t arg, double value, int delta)
{
  if (p.out->code.size() >= static_cast<std::size_t>(kMaxNodes))
    return parseFail(p, QObject::tr("expression too long"));

  p.out->code.push_back(Node{op, arg, value});
  p.depth    += delta;
  p.maxDepth  = std::max(p.maxDepth, p.depth);
  if (p.maxDepth > kMaxStackDepth)
    return parseFail(p, QObject::tr("expression too deeply nested"));

  return true;
}

/**
 * @brief True when the current token is the operator @p sym.
 */
[[nodiscard]] static bool isSymbol(const Parser& p, const char* sym)
{
  return p.lx.current.kind == TokenKind::Symbol && p.lx.current.text == QLatin1StringView(sym);
}

/**
 * @brief Consumes @p sym or records an "expected" error.
 */
[[nodiscard]] static bool expectSymbol(Parser& p, const char* sym)
{
  if (!isSymbol(p, sym))
    return parseFail(p, QObject::tr("expected '%1'").arg(QLatin1StringView(sym)));

  nextToken(p.lx);
  return true;
}

[[nodiscard]] static bool parseExpression(Parser& p);

/**
 * @brief Resolves a bare or braced name: built-in variables and constants first, then a
 *        sibling dataset through the resolver.
 */
[[nodiscard]] static bool parseName(Parser& p, QStringView name)
{
  if (name == u"v")
    return emitNode(p, Op::PushV, 0, 0.0, +1);

  if (name == u"t")
    return emitNode(p, Op::PushT, 0, 0.0, +1);

  if (name == u"n")
    return emitNode(p, Op::PushN, 0, 0.0, +1);

  if (name == u"dt")
    return emitNode(p, Op::PushDt, 0, 0.0, +1);

  if (name == u"pi")
    return emitNode(p, Op::PushConst, 0, M_PI, +1);

  if (name == u"e")
    return emitNode(p, Op::PushConst, 0, M_E, +1);

  if (name == u"nan")
    return emitNode(p, Op::PushConst, 0, std::numeric_limits<double>::quiet_NaN(), +1);

  if (name == u"inf")
    return emitNode(p, Op::PushConst, 0, std::numeric_limits<double>::infinity(), +1);

  const int slot = (*p.resolver)(name);
  if (slot < 0)
    return parseFail(p, QObject::tr("unknown name '%1'").arg(name));

  p.out->maxSlot = std::max(p.out->maxSlot, slot);
  return emitNode(p, Op::PushSlot, slot, 0.0, +1);
}

/**
 * @brief `table(name, register)`: both names resolve at compile time to one store handle, so the
 *        evaluator only carries an integer. Read-only by design; writing a register from an
 *        expression would give it an ordering dependency, which is where Lua takes over.
 */
[[nodiscard]] static bool parseTableCall(Parser& p)
{
  if (!expectSymbol(p, "("))
    return false;

  if (!p.tables || !*p.tables)
    return parseFail(p, QObject::tr("table() is not available for this source"));

  if (p.lx.current.kind != TokenKind::Identifier)
    return parseFail(p, QObject::tr("table() needs a table name first"));

  const QStringView table_name = p.lx.current.text;
  nextToken(p.lx);
  if (!expectSymbol(p, ","))
    return false;

  if (p.lx.current.kind != TokenKind::Identifier)
    return parseFail(p, QObject::tr("table() needs a variable name second"));

  const QStringView register_name = p.lx.current.text;
  const qint64 handle             = (*p.tables)(table_name, register_name);
  if (handle < 0)
    return parseFail(p, QObject::tr("unknown variable '%1.%2'").arg(table_name, register_name));

  nextToken(p.lx);
  if (!expectSymbol(p, ")"))
    return false;

  return emitNode(p, Op::PushTable, 0, static_cast<double>(handle), +1);
}

/**
 * @brief `sample(name, k)`: the name is resolved to a slot, k is an expression.
 */
[[nodiscard]] static bool parseSampleCall(Parser& p)
{
  if (!expectSymbol(p, "("))
    return false;

  if (p.lx.current.kind != TokenKind::Identifier)
    return parseFail(p, QObject::tr("sample() needs a dataset name first"));

  const int slot = (*p.resolver)(p.lx.current.text);
  if (slot < 0)
    return parseFail(p, QObject::tr("unknown name '%1'").arg(p.lx.current.text));

  nextToken(p.lx);
  if (!expectSymbol(p, ","))
    return false;

  if (!parseExpression(p))
    return false;

  if (!expectSymbol(p, ")"))
    return false;

  p.out->maxSlot     = std::max(p.out->maxSlot, slot);
  p.out->usesHistory = true;
  return emitNode(p, Op::Sample, slot, 0.0, 0);
}

/**
 * @brief `fn(a[, b[, c]])` from the fixed function table; arity is checked at compile time.
 */
[[nodiscard]] static bool parseFunctionCall(Parser& p, int fnIndex)
{
  const int arity = kFunctions[fnIndex].arity;
  if (!expectSymbol(p, "("))
    return false;

  for (int i = 0; i < arity; ++i) {
    if (i > 0 && !expectSymbol(p, ","))
      return false;

    if (!parseExpression(p))
      return false;
  }

  if (!expectSymbol(p, ")"))
    return parseFail(p,
                     QObject::tr("%1() takes %2 argument(s)")
                       .arg(QLatin1StringView(kFunctions[fnIndex].name))
                       .arg(arity));

  const Op op = (arity == 1) ? Op::Call1 : (arity == 2 ? Op::Call2 : Op::Call3);
  return emitNode(p, op, fnIndex, 0.0, 1 - arity);
}

/**
 * @brief Primary: number, name, function call, sample(), parenthesised expression, or unary.
 */
[[nodiscard]] static bool parsePrimary(Parser& p)
{
  const Token tok = p.lx.current;
  if (tok.kind == TokenKind::Number) {
    nextToken(p.lx);
    return emitNode(p, Op::PushConst, 0, tok.number, +1);
  }

  if (tok.kind == TokenKind::Identifier) {
    nextToken(p.lx);
    if (isSymbol(p, "(")) {
      if (tok.text == u"sample")
        return parseSampleCall(p);

      if (tok.text == u"table")
        return parseTableCall(p);

      const int fn = functionIndex(tok.text);
      if (fn < 0)
        return parseFail(p, QObject::tr("unknown function '%1'").arg(tok.text));

      return parseFunctionCall(p, fn);
    }

    return parseName(p, tok.text);
  }

  if (isSymbol(p, "(")) {
    nextToken(p.lx);
    if (++p.nesting > kMaxNesting)
      return parseFail(p, QObject::tr("too many nested parentheses"));

    const bool ok = parseExpression(p) && expectSymbol(p, ")");
    --p.nesting;
    return ok;
  }

  if (isSymbol(p, "-")) {
    nextToken(p.lx);
    return parsePrimary(p) && emitNode(p, Op::Neg, 0, 0.0, 0);
  }

  if (isSymbol(p, "!")) {
    nextToken(p.lx);
    return parsePrimary(p) && emitNode(p, Op::Not, 0, 0.0, 0);
  }

  if (isSymbol(p, "+")) {
    nextToken(p.lx);
    return parsePrimary(p);
  }

  if (tok.kind == TokenKind::End)
    return parseFail(p, QObject::tr("unexpected end of expression"));

  return parseFail(p, QObject::tr("unexpected '%1'").arg(tok.text));
}

[[nodiscard]] static const BinaryOp* currentBinaryOp(const Parser& p)
{
  if (p.lx.current.kind != TokenKind::Symbol)
    return nullptr;

  for (const auto& op : kBinaryOps)
    if (p.lx.current.text == QLatin1StringView(op.symbol))
      return &op;

  return nullptr;
}

/**
 * @brief Precedence climbing over kBinaryOps; the loop is bounded by the token count.
 */
[[nodiscard]] static bool parseBinary(Parser& p, int minPrecedence)
{
  if (!parsePrimary(p))
    return false;

  for (int guard = 0; guard < kMaxNodes; ++guard) {
    const BinaryOp* op = currentBinaryOp(p);
    if (!op || op->precedence < minPrecedence)
      return true;

    nextToken(p.lx);
    const int next = op->rightAssoc ? op->precedence : op->precedence + 1;
    if (!parseBinary(p, next))
      return false;

    if (!emitNode(p, op->op, 0, 0.0, -1))
      return false;
  }

  return parseFail(p, QObject::tr("expression too long"));
}

/**
 * @brief Full expression: binary chain plus an optional right-associative `? :`.
 */
[[nodiscard]] static bool parseExpression(Parser& p)
{
  if (!parseBinary(p, 1))
    return false;

  if (!isSymbol(p, "?"))
    return true;

  nextToken(p.lx);
  if (++p.nesting > kMaxNesting)
    return parseFail(p, QObject::tr("too many nested conditionals"));

  const bool ok = parseExpression(p) && expectSymbol(p, ":") && parseExpression(p);
  --p.nesting;
  return ok && emitNode(p, Op::Select, 0, 0.0, -2);
}

/**
 * @brief Parses @p text to postfix; the postfix stack must end at exactly one value.
 */
bool compile(const QString& text, const NameResolver& resolver, Program& out, QString& error)
{
  return compile(text, resolver, TableResolver(), out, error);
}

/**
 * @brief compile() with read-only data-table access.
 */
bool compile(const QString& text,
             const NameResolver& resolver,
             const TableResolver& tables,
             Program& out,
             QString& error)
{
  out = Program();
  error.clear();

  Parser p{
    Lexer{QStringView(text), 0, Token{}},
    &out, &resolver, &tables, QString(), 0, 0, 0
  };
  nextToken(p.lx);

  if (p.lx.current.kind == TokenKind::End) {
    error = QObject::tr("empty expression");
    return false;
  }

  const bool ok = parseExpression(p);
  if (ok && p.lx.current.kind != TokenKind::End)
    (void)parseFail(p, QObject::tr("unexpected '%1'").arg(p.lx.current.text));

  if (!p.error.isEmpty() || !ok || p.depth != 1) {
    error = p.error.isEmpty() ? QObject::tr("malformed expression") : p.error;
    out   = Program();
    return false;
  }

  out.stackDepth = p.maxDepth;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Evaluator
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies one binary opcode; comparisons and logic yield 0.0 / 1.0.
 */
[[nodiscard]] static double applyBinary(Op op, double a, double b) noexcept
{
  switch (op) {
    case Op::Add:
      return a + b;
    case Op::Sub:
      return a - b;
    case Op::Mul:
      return a * b;
    case Op::Div:
      return a / b;
    case Op::Mod:
      return std::fmod(a, b);
    case Op::Pow:
      // code-verify off
      return std::pow(a, b);
      // code-verify on
    case Op::Lt:
      return a < b ? 1.0 : 0.0;
    case Op::Le:
      return a <= b ? 1.0 : 0.0;
    case Op::Gt:
      return a > b ? 1.0 : 0.0;
    case Op::Ge:
      return a >= b ? 1.0 : 0.0;
    case Op::Eq:
      return a == b ? 1.0 : 0.0;
    case Op::Ne:
      return a != b ? 1.0 : 0.0;
    case Op::And:
      return (a != 0.0 && b != 0.0) ? 1.0 : 0.0;
    case Op::Or:
      return (a != 0.0 || b != 0.0) ? 1.0 : 0.0;
    default:
      return std::numeric_limits<double>::quiet_NaN();
  }
}

/**
 * @brief Pushes the value a leaf opcode denotes; false for an opcode that is not a leaf.
 */
[[nodiscard]] static bool pushLeaf(const Node& node, const Context& ctx, double& out) noexcept
{
  switch (node.op) {
    case Op::PushConst:
      out = node.value;
      return true;
    case Op::PushV:
      out = ctx.v;
      return true;
    case Op::PushT:
      out = ctx.t;
      return true;
    case Op::PushN:
      out = ctx.n;
      return true;
    case Op::PushDt:
      out = ctx.dt;
      return true;
    case Op::PushSlot:
      out = (node.arg >= 0 && static_cast<std::size_t>(node.arg) < ctx.slotCount)
            ? ctx.slotValues[node.arg]
            : std::numeric_limits<double>::quiet_NaN();
      return true;
    case Op::PushTable:
      out = ctx.tableValue ? ctx.tableValue(ctx.tableOwner, static_cast<qint64>(node.value))
                           : std::numeric_limits<double>::quiet_NaN();
      return true;
    default:
      return false;
  }
}

/**
 * @brief Stack machine over the postfix program: leaves push, operators fold in place; any
 *        stack inconsistency yields NaN instead of reading past the fixed array.
 */
double evaluate(const Program& program, const Context& ctx) noexcept
{
  constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
  double stack[kMaxStackDepth];
  int sp = 0;

  const std::size_t count = program.code.size();
  SS_ASSERT(count > 0, return kNaN);
  SS_ASSERT(program.stackDepth <= kMaxStackDepth, return kNaN);

  for (std::size_t i = 0; i < count; ++i) {
    const Node& node = program.code[i];
    double leaf      = 0.0;
    if (pushLeaf(node, ctx, leaf)) {
      if (sp >= kMaxStackDepth) [[unlikely]]
        return kNaN;

      stack[sp++] = leaf;
      continue;
    }

    if (node.op == Op::Neg || node.op == Op::Not) {
      if (sp < 1) [[unlikely]]
        return kNaN;

      stack[sp - 1] = (node.op == Op::Neg) ? -stack[sp - 1] : (stack[sp - 1] == 0.0 ? 1.0 : 0.0);
      continue;
    }

    if (node.op == Op::Sample) {
      if (sp < 1) [[unlikely]]
        return kNaN;

      const double kRaw = stack[sp - 1];
      const int k       = std::isfinite(kRaw) ? static_cast<int>(std::clamp(kRaw, 0.0, 1e6)) : 0;
      stack[sp - 1]     = ctx.history ? ctx.history(ctx.historyOwner, node.arg, k) : kNaN;
      continue;
    }

    if (node.op == Op::Select) {
      if (sp < 3) [[unlikely]]
        return kNaN;

      const double c  = stack[sp - 3];
      stack[sp - 3]   = (c != 0.0) ? stack[sp - 2] : stack[sp - 1];
      sp             -= 2;
      continue;
    }

    if (node.op == Op::Call1 || node.op == Op::Call2 || node.op == Op::Call3) {
      const int arity = (node.op == Op::Call1) ? 1 : (node.op == Op::Call2 ? 2 : 3);
      if (sp < arity || node.arg < 0 || node.arg >= kFunctionCount) [[unlikely]]
        return kNaN;

      const FunctionSpec& fn  = kFunctions[node.arg];
      double* args            = &stack[sp - arity];
      const double result     = (arity == 1) ? fn.fn1(args[0])
                              : (arity == 2) ? fn.fn2(args[0], args[1])
                                             : fn.fn3(args[0], args[1], args[2]);
      sp                     -= arity - 1;
      stack[sp - 1]           = result;
      continue;
    }

    if (sp < 2) [[unlikely]]
      return kNaN;

    stack[sp - 2] = applyBinary(node.op, stack[sp - 2], stack[sp - 1]);
    --sp;
  }

  return (sp == 1) ? stack[0] : kNaN;
}

//--------------------------------------------------------------------------------------------------
// SlotTable
//--------------------------------------------------------------------------------------------------

/**
 * @brief Empty table; slots are allocated by slotFor() at compile time.
 */
SlotTable::SlotTable() : m_slotByUniqueId(), m_uniqueIdOfSlot(), m_latest(), m_rings() {}

/**
 * @brief Returns the slot of @p uniqueId, allocating one (with its history ring) on first use.
 *        Compile-time only; the hotpath never reaches this.
 */
int SlotTable::slotFor(int uniqueId)
{
  if (uniqueId < 0)
    return -1;

  const int existing = slotOf(uniqueId);
  if (existing >= 0)
    return existing;

  const auto index = static_cast<std::size_t>(uniqueId);
  if (m_slotByUniqueId.size() <= index)
    m_slotByUniqueId.resize(index + 1, -1);

  const int slot          = static_cast<int>(m_uniqueIdOfSlot.size());
  m_slotByUniqueId[index] = slot;
  m_uniqueIdOfSlot.push_back(uniqueId);
  m_latest.push_back(std::numeric_limits<double>::quiet_NaN());
  m_rings.push_back(Ring{std::vector<double>(static_cast<std::size_t>(kMaxHistory),
                                             std::numeric_limits<double>::quiet_NaN()),
                         0,
                         0});
  return slot;
}

/**
 * @brief Slot of @p uniqueId, or -1 when no expression refers to it (O(1) vector read).
 */
int SlotTable::slotOf(int uniqueId) const noexcept
{
  if (uniqueId < 0 || static_cast<std::size_t>(uniqueId) >= m_slotByUniqueId.size())
    return -1;

  return m_slotByUniqueId[static_cast<std::size_t>(uniqueId)];
}

/**
 * @brief Number of slots handed out so far.
 */
int SlotTable::slotCount() const noexcept
{
  return static_cast<int>(m_uniqueIdOfSlot.size());
}

/**
 * @brief Dataset uniqueId behind @p slot, or -1.
 */
int SlotTable::uniqueIdAt(int slot) const noexcept
{
  if (slot < 0 || static_cast<std::size_t>(slot) >= m_uniqueIdOfSlot.size())
    return -1;

  return m_uniqueIdOfSlot[static_cast<std::size_t>(slot)];
}

/**
 * @brief Latest published value per slot, index-aligned with the slots.
 */
const double* SlotTable::latestValues() const noexcept
{
  return m_latest.data();
}

/**
 * @brief True when no expression of this source refers to any dataset.
 */
bool SlotTable::empty() const noexcept
{
  return m_uniqueIdOfSlot.empty();
}

/**
 * @brief k-th most recent published value of @p slot (0 = latest); NaN before enough history.
 */
double SlotTable::sample(int slot, int k) const noexcept
{
  if (slot < 0 || static_cast<std::size_t>(slot) >= m_rings.size() || k < 0)
    return std::numeric_limits<double>::quiet_NaN();

  const Ring& ring = m_rings[static_cast<std::size_t>(slot)];
  if (static_cast<std::size_t>(k) >= ring.fill)
    return std::numeric_limits<double>::quiet_NaN();

  const std::size_t cap   = ring.values.size();
  const std::size_t index = (ring.head + cap - 1 - static_cast<std::size_t>(k)) % cap;
  return ring.values[index];
}

/**
 * @brief Publishes a dataset's value: no-op for datasets no expression refers to.
 */
void SlotTable::publish(int uniqueId, double value) noexcept
{
  const int slot = slotOf(uniqueId);
  if (slot >= 0)
    publishSlot(slot, value);
}

/**
 * @brief Publishes into a known slot: latest value plus one ring push.
 */
void SlotTable::publishSlot(int slot, double value) noexcept
{
  SS_ASSERT(slot >= 0 && static_cast<std::size_t>(slot) < m_rings.size(), return);

  m_latest[static_cast<std::size_t>(slot)] = value;

  Ring& ring             = m_rings[static_cast<std::size_t>(slot)];
  const std::size_t cap  = ring.values.size();
  ring.values[ring.head] = value;
  ring.head              = (ring.head + 1) % cap;
  ring.fill              = std::min(cap, ring.fill + 1);
}

/**
 * @brief Forgets every published value but keeps the slot layout (a program stays valid).
 */
void SlotTable::reset() noexcept
{
  std::fill(m_latest.begin(), m_latest.end(), std::numeric_limits<double>::quiet_NaN());
  for (auto& ring : m_rings) {
    ring.head = 0;
    ring.fill = 0;
  }
}

/**
 * @brief Context::history adapter: @p owner is the SlotTable the program reads.
 */
double SlotTable::historyThunk(const void* owner, int slot, int k) noexcept
{
  SS_ASSERT(owner != nullptr, return std::numeric_limits<double>::quiet_NaN());
  return static_cast<const SlotTable*>(owner)->sample(slot, k);
}

//--------------------------------------------------------------------------------------------------
// Runtime
//--------------------------------------------------------------------------------------------------

/**
 * @brief Evaluates one sample: `n` is the running count, `dt` the gap to the previous `t`
 *        (0 for the first sample), siblings and history come from @p table.
 */
double Runtime::run(double v, double t, const SlotTable& table) noexcept
{
  SS_ASSERT(program.valid(), return std::numeric_limits<double>::quiet_NaN());

  Context ctx;
  ctx.v            = v;
  ctx.t            = t;
  ctx.n            = static_cast<double>(count);
  ctx.dt           = (count > 0) ? (t - lastT) : 0.0;
  ctx.slotValues   = table.latestValues();
  ctx.slotCount    = static_cast<std::size_t>(table.slotCount());
  ctx.historyOwner = &table;
  ctx.history      = &SlotTable::historyThunk;
  ctx.tableOwner   = tableOwner;
  ctx.tableValue   = tableValue;

  const double result = evaluate(program, ctx);
  lastT               = t;
  ++count;
  return result;
}

/**
 * @brief Restarts `n` and `dt` (a reconnect or a rebuilt program).
 */
void Runtime::reset() noexcept
{
  lastT = 0.0;
  count = 0;
}

}  // namespace DataModel::Expression
