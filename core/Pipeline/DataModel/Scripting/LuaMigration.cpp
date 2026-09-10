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

#include "DataModel/Scripting/LuaMigration.h"

#include <QStringList>
#include <vector>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Token model
//--------------------------------------------------------------------------------------------------

// Nested under `lua` so these types can't collide with another TU's `detail::Token` (ODR).
namespace detail::lua {

/**
 * @brief Lexical class of one Lua token; trivia covers whitespace and comments alike.
 */
enum class TokenKind {
  Trivia,
  String,
  Number,
  Name,
  Operator
};

/**
 * @brief One Lua token, carrying its exact source text so a rewrite can re-emit the script
 *        verbatim except for the spans it replaces.
 */
struct Token {
  TokenKind kind;
  QString text;
};

/**
 * @brief Result of probing for a long-bracket literal ([[...]], [==[...]==]) at a position.
 */
struct LongBracket {
  bool present;
  bool terminated;
  qsizetype end;
};

/**
 * @brief The operator selected for the next rewrite pass, with the precedence it binds at.
 */
struct Target {
  qsizetype index;
  int precedence;
  bool unary;
};

}  // namespace detail::lua

using detail::lua::LongBracket;
using detail::lua::Target;
using detail::lua::Token;
using detail::lua::TokenKind;

static constexpr int kMaxScanSteps = 100000;
static constexpr int kLuaMaxDepth  = 64;
static constexpr int kMaxRewrites  = 8192;

//--------------------------------------------------------------------------------------------------
// Lexer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the first non-whitespace character at or after @p i.
 */
[[nodiscard]] static qsizetype skipSpaces(const QString& s, qsizetype i)
{
  qsizetype j = i;
  while (j < s.size() && s[j].isSpace())
    ++j;

  return j;
}

/**
 * @brief Probes for a long-bracket opener at @p i and locates its matching close.
 */
[[nodiscard]] static LongBracket readLongBracket(const QString& s, qsizetype i)
{
  const qsizetype n = s.size();
  if (i >= n || s[i] != QLatin1Char('['))
    return {false, false, i};

  qsizetype level = 0;
  qsizetype j     = i + 1;
  while (j < n && s[j] == QLatin1Char('=')) {
    ++level;
    ++j;
  }

  if (j >= n || s[j] != QLatin1Char('['))
    return {false, false, i};

  const QString close =
    QStringLiteral("]") + QString(level, QLatin1Char('=')) + QStringLiteral("]");
  const qsizetype end = s.indexOf(close, j + 1);
  if (end < 0)
    return {true, false, n};

  return {true, true, end + close.size()};
}

/**
 * @brief Returns the end of the comment starting at @p i, or -1 when a long comment is unclosed.
 */
[[nodiscard]] static qsizetype readComment(const QString& s, qsizetype i)
{
  const LongBracket bracket = readLongBracket(s, i + 2);
  if (bracket.present)
    return bracket.terminated ? bracket.end : -1;

  const qsizetype end = s.indexOf(QLatin1Char('\n'), i + 2);
  return end < 0 ? s.size() : end;
}

/**
 * @brief Returns the end of the quoted string starting at @p i, or -1 when it is unterminated.
 */
[[nodiscard]] static qsizetype readShortString(const QString& s, qsizetype i)
{
  const QChar quote = s[i];
  const qsizetype n = s.size();
  for (qsizetype j = i + 1; j < n; ++j) {
    if (s[j] == QLatin1Char('\\')) {
      ++j;
      continue;
    }

    if (s[j] == quote)
      return j + 1;

    if (s[j] == QLatin1Char('\n'))
      return -1;
  }

  return -1;
}

/**
 * @brief Returns the end of the numeric literal at @p i; the exponent marker depends on the base
 *        so a hexadecimal digit is never mistaken for one (0xE+1 is an addition, not an exponent).
 */
[[nodiscard]] static qsizetype readNumber(const QString& s, qsizetype i)
{
  const qsizetype n = s.size();
  const bool hex    = (i + 1 < n) && s[i] == QLatin1Char('0')
                && (s[i + 1] == QLatin1Char('x') || s[i + 1] == QLatin1Char('X'));

  qsizetype j = i;
  while (j < n && (s[j].isLetterOrNumber() || s[j] == QLatin1Char('.'))) {
    const bool marker = hex ? (s[j] == QLatin1Char('p') || s[j] == QLatin1Char('P'))
                            : (s[j] == QLatin1Char('e') || s[j] == QLatin1Char('E'));
    const bool signed_exponent =
      marker && j + 1 < n && (s[j + 1] == QLatin1Char('+') || s[j + 1] == QLatin1Char('-'));
    j += signed_exponent ? 2 : 1;
  }

  return j;
}

/**
 * @brief Returns the end of the identifier or keyword starting at @p i.
 */
[[nodiscard]] static qsizetype readName(const QString& s, qsizetype i)
{
  qsizetype j = i;
  while (j < s.size() && (s[j].isLetterOrNumber() || s[j] == QLatin1Char('_')))
    ++j;

  return j;
}

/**
 * @brief Returns the end of the punctuation token at @p i, preferring the longest spelling.
 */
[[nodiscard]] static qsizetype readOperator(const QString& s, qsizetype i)
{
  static const QString kThreeChar[] = {QStringLiteral("...")};
  static const QString kTwoChar[]   = {QStringLiteral(".."),
                                       QStringLiteral("=="),
                                       QStringLiteral("~="),
                                       QStringLiteral("<="),
                                       QStringLiteral(">="),
                                       QStringLiteral("<<"),
                                       QStringLiteral(">>"),
                                       QStringLiteral("//"),
                                       QStringLiteral("::")};

  for (const auto& candidate : kThreeChar)
    if (QStringView(s).sliced(i).startsWith(candidate))
      return i + candidate.size();

  for (const auto& candidate : kTwoChar)
    if (QStringView(s).sliced(i).startsWith(candidate))
      return i + candidate.size();

  return i + 1;
}

/**
 * @brief Appends the source span [i, end) as one token and advances @p i past it.
 */
[[nodiscard]] static bool appendToken(
  const QString& s, qsizetype& i, qsizetype end, TokenKind kind, std::vector<Token>& out)
{
  if (end <= i)
    return false;

  out.push_back({kind, s.mid(i, end - i)});
  i = end;
  return true;
}

/**
 * @brief Reads the single token starting at @p i, dispatching on its first character.
 */
[[nodiscard]] static bool readOneToken(const QString& s, qsizetype& i, std::vector<Token>& out)
{
  const qsizetype n = s.size();
  const QChar c     = s[i];
  if (c.isSpace())
    return appendToken(s, i, skipSpaces(s, i), TokenKind::Trivia, out);

  if (c == QLatin1Char('-') && i + 1 < n && s[i + 1] == QLatin1Char('-'))
    return appendToken(s, i, readComment(s, i), TokenKind::Trivia, out);

  if (c == QLatin1Char('"') || c == QLatin1Char('\''))
    return appendToken(s, i, readShortString(s, i), TokenKind::String, out);

  if (c == QLatin1Char('[')) {
    const LongBracket bracket = readLongBracket(s, i);
    if (bracket.present)
      return bracket.terminated && appendToken(s, i, bracket.end, TokenKind::String, out);
  }

  if (c.isDigit() || (c == QLatin1Char('.') && i + 1 < n && s[i + 1].isDigit()))
    return appendToken(s, i, readNumber(s, i), TokenKind::Number, out);

  if (c.isLetter() || c == QLatin1Char('_'))
    return appendToken(s, i, readName(s, i), TokenKind::Name, out);

  return appendToken(s, i, readOperator(s, i), TokenKind::Operator, out);
}

/**
 * @brief Splits the script into tokens, failing on an unterminated string or long comment.
 */
[[nodiscard]] static bool tokenizeLuaSource(const QString& source, std::vector<Token>& out)
{
  out.clear();
  const qsizetype n = source.size();

  qsizetype i = 0;
  while (i < n) {
    const qsizetype start = i;
    if (!readOneToken(source, i, out))
      return false;

    SS_ASSERT(i > start, return false);
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Token classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the identifier is a reserved Lua keyword.
 */
[[nodiscard]] static bool isKeyword(const QString& text)
{
  static const QStringList kKeywords = {
    QStringLiteral("and"),   QStringLiteral("break"),  QStringLiteral("do"),
    QStringLiteral("else"),  QStringLiteral("elseif"), QStringLiteral("end"),
    QStringLiteral("false"), QStringLiteral("for"),    QStringLiteral("function"),
    QStringLiteral("goto"),  QStringLiteral("if"),     QStringLiteral("in"),
    QStringLiteral("local"), QStringLiteral("nil"),    QStringLiteral("not"),
    QStringLiteral("or"),    QStringLiteral("repeat"), QStringLiteral("return"),
    QStringLiteral("then"),  QStringLiteral("true"),   QStringLiteral("until"),
    QStringLiteral("while")};

  return kKeywords.contains(text);
}

/**
 * @brief Returns whether the keyword is itself a value literal rather than a control word.
 */
[[nodiscard]] static bool isValueKeyword(const QString& text)
{
  return text == QLatin1String("nil") || text == QLatin1String("true")
      || text == QLatin1String("false");
}

/**
 * @brief Returns whether the token can be the last one of a complete value expression, which is
 *        what separates a binary operator from a unary prefix at the same spelling.
 */
[[nodiscard]] static bool endsValue(const Token& token)
{
  if (token.kind == TokenKind::Number || token.kind == TokenKind::String)
    return true;

  if (token.kind == TokenKind::Name)
    return !isKeyword(token.text) || isValueKeyword(token.text);

  return token.text == QLatin1String(")") || token.text == QLatin1String("]")
      || token.text == QLatin1String("}") || token.text == QLatin1String("...");
}

/**
 * @brief Returns the Lua 5.3 binding power of the token as a binary operator, or 0 if it is none.
 */
[[nodiscard]] static int binaryPrecedence(const Token& token)
{
  const QString& t = token.text;
  if (token.kind == TokenKind::Name)
    return t == QLatin1String("or") ? 1 : (t == QLatin1String("and") ? 2 : 0);

  if (token.kind != TokenKind::Operator)
    return 0;

  if (t == QLatin1String("<") || t == QLatin1String(">") || t == QLatin1String("<=")
      || t == QLatin1String(">=") || t == QLatin1String("~=") || t == QLatin1String("=="))
    return 3;

  if (t == QLatin1String("|"))
    return 4;

  if (t == QLatin1String("~"))
    return 5;

  if (t == QLatin1String("&"))
    return 6;

  if (t == QLatin1String("<<") || t == QLatin1String(">>"))
    return 7;

  if (t == QLatin1String(".."))
    return 8;

  if (t == QLatin1String("+") || t == QLatin1String("-"))
    return 9;

  if (t == QLatin1String("*") || t == QLatin1String("/") || t == QLatin1String("//")
      || t == QLatin1String("%"))
    return 10;

  return t == QLatin1String("^") ? 12 : 0;
}

/**
 * @brief Returns whether the token can open a unary expression.
 */
[[nodiscard]] static bool isUnaryPrefix(const Token& token)
{
  if (token.kind == TokenKind::Name)
    return token.text == QLatin1String("not");

  return token.text == QLatin1String("-") || token.text == QLatin1String("#")
      || token.text == QLatin1String("~");
}

//--------------------------------------------------------------------------------------------------
// Token cursor helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the first non-trivia token index at or after @p i, or -1 past the end.
 */
[[nodiscard]] static qsizetype nextCode(const std::vector<Token>& v, qsizetype i)
{
  const auto n = static_cast<qsizetype>(v.size());
  qsizetype j  = i < 0 ? 0 : i;
  while (j < n && v[j].kind == TokenKind::Trivia)
    ++j;

  return j < n ? j : -1;
}

/**
 * @brief Returns the last non-trivia token index at or before @p i, or -1 before the start.
 */
[[nodiscard]] static qsizetype prevCode(const std::vector<Token>& v, qsizetype i)
{
  const auto n = static_cast<qsizetype>(v.size());
  qsizetype j  = i >= n ? n - 1 : i;
  while (j >= 0 && v[j].kind == TokenKind::Trivia)
    --j;

  return j;
}

/**
 * @brief Returns whether the token at @p i sits where an expression may only start, which makes
 *        an ambiguous spelling (-, ~) a unary prefix rather than a binary operator.
 */
[[nodiscard]] static bool inPrefixPosition(const std::vector<Token>& v, qsizetype i)
{
  const qsizetype p = prevCode(v, i - 1);
  return p < 0 || !endsValue(v[p]);
}

/**
 * @brief Returns whether the token opens a bracketed group.
 */
[[nodiscard]] static bool isOpenBracket(const Token& token)
{
  return token.text == QLatin1String("(") || token.text == QLatin1String("[")
      || token.text == QLatin1String("{");
}

/**
 * @brief Returns whether the token closes a bracketed group.
 */
[[nodiscard]] static bool isCloseBracket(const Token& token)
{
  return token.text == QLatin1String(")") || token.text == QLatin1String("]")
      || token.text == QLatin1String("}");
}

/**
 * @brief Returns the index closing the group opened at @p open, or -1 when it never closes.
 */
[[nodiscard]] static qsizetype matchForward(const std::vector<Token>& v, qsizetype open)
{
  const auto n = static_cast<qsizetype>(v.size());
  int depth    = 0;
  for (qsizetype i = open; i < n; ++i) {
    if (isOpenBracket(v[i]))
      ++depth;
    else if (isCloseBracket(v[i]))
      --depth;
    else
      continue;

    if (depth == 0)
      return i;

    if (depth < 0)
      return -1;
  }

  return -1;
}

/**
 * @brief Returns the index opening the group closed at @p close, or -1 when it never opens.
 */
[[nodiscard]] static qsizetype matchBackward(const std::vector<Token>& v, qsizetype close)
{
  int depth = 0;
  for (qsizetype i = close; i >= 0; --i) {
    if (isCloseBracket(v[i]))
      ++depth;
    else if (isOpenBracket(v[i]))
      --depth;
    else
      continue;

    if (depth == 0)
      return i;

    if (depth < 0)
      return -1;
  }

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Operand delimitation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Extends a primary expression rightwards across one suffix (field, call, index), or
 *        returns -1 when the token at @p j is not a suffix.
 */
[[nodiscard]] static qsizetype growSuffix(const std::vector<Token>& v, qsizetype j)
{
  const Token& token = v[j];
  if (token.text == QLatin1String(".") || token.text == QLatin1String(":")) {
    const qsizetype k = nextCode(v, j + 1);
    return (k >= 0 && v[k].kind == TokenKind::Name) ? k : -1;
  }

  if (isOpenBracket(token))
    return matchForward(v, j);

  return token.kind == TokenKind::String ? j : -1;
}

/**
 * @brief Locates the last token of the primary expression starting at @p i.
 */
[[nodiscard]] static bool scanPrimaryForward(const std::vector<Token>& v,
                                             qsizetype i,
                                             qsizetype& last)
{
  const auto n = static_cast<qsizetype>(v.size());
  if (i < 0 || i >= n)
    return false;

  const Token& token = v[i];
  if (token.kind == TokenKind::Name && isKeyword(token.text) && !isValueKeyword(token.text))
    return false;

  qsizetype base = -1;
  if (token.kind == TokenKind::Name || token.kind == TokenKind::Number
      || token.kind == TokenKind::String || token.text == QLatin1String("..."))
    base = i;
  else if (token.text == QLatin1String("(") || token.text == QLatin1String("{"))
    base = matchForward(v, i);

  if (base < 0)
    return false;

  for (int step = 0; step < kMaxScanSteps; ++step) {
    const qsizetype j = nextCode(v, base + 1);
    if (j < 0)
      break;

    const qsizetype grown = growSuffix(v, j);
    if (grown < 0)
      break;

    base = grown;
  }

  last = base;
  return true;
}

/**
 * @brief Returns the index of the first token past the unary prefixes starting at @p i.
 */
[[nodiscard]] static qsizetype skipUnaryPrefixes(const std::vector<Token>& v, qsizetype i)
{
  qsizetype k = i;
  for (int step = 0; step < kMaxScanSteps; ++step) {
    if (k < 0 || !isUnaryPrefix(v[k]))
      return k;

    k = nextCode(v, k + 1);
  }

  return -1;
}

/**
 * @brief Locates the last token of the right operand of an operator binding at @p precedence:
 *        the operand absorbs every operator that binds strictly tighter, and stops at the first
 *        one that does not.
 */
[[nodiscard]] static bool scanOperandForward(const std::vector<Token>& v,
                                             qsizetype start,
                                             int precedence,
                                             qsizetype& end)
{
  qsizetype i = nextCode(v, start);
  for (int step = 0; step < kMaxScanSteps; ++step) {
    i = skipUnaryPrefixes(v, i);
    if (i < 0)
      return false;

    qsizetype last = -1;
    if (!scanPrimaryForward(v, i, last))
      return false;

    end               = last;
    const qsizetype j = nextCode(v, last + 1);
    if (j < 0 || binaryPrecedence(v[j]) <= precedence)
      return true;

    i = nextCode(v, j + 1);
  }

  return false;
}

/**
 * @brief Locates the first token of the primary expression ending at @p i, walking back through
 *        bracket groups and dotted prefixes.
 */
[[nodiscard]] static bool scanPrimaryBackward(const std::vector<Token>& v,
                                              qsizetype i,
                                              int depth,
                                              qsizetype& first)
{
  if (i < 0 || depth > kLuaMaxDepth)
    return false;

  const Token& token = v[i];
  if (isCloseBracket(token)) {
    const qsizetype open = matchBackward(v, i);
    if (open < 0)
      return false;

    const qsizetype q = prevCode(v, open - 1);
    if (q >= 0 && endsValue(v[q]))
      return scanPrimaryBackward(v, q, depth + 1, first);

    first = open;
    return true;
  }

  if (!endsValue(token))
    return false;

  const qsizetype p = prevCode(v, i - 1);
  if (p >= 0 && (v[p].text == QLatin1String(".") || v[p].text == QLatin1String(":")))
    return scanPrimaryBackward(v, prevCode(v, p - 1), depth + 1, first);

  if (p >= 0 && token.kind == TokenKind::String && endsValue(v[p]))
    return scanPrimaryBackward(v, p, depth + 1, first);

  first = i;
  return true;
}

/**
 * @brief Extends a located primary leftwards across the unary prefixes that apply to it.
 */
[[nodiscard]] static qsizetype absorbPrefixesBackward(const std::vector<Token>& v, qsizetype begin)
{
  qsizetype k = begin;
  for (int step = 0; step < kMaxScanSteps; ++step) {
    const qsizetype p = prevCode(v, k - 1);
    if (p < 0 || !isUnaryPrefix(v[p]) || !inPrefixPosition(v, p))
      return k;

    k = p;
  }

  return begin;
}

/**
 * @brief Locates the first token of the left operand of an operator binding at @p precedence.
 *        Every target operator is left-associative, so the left operand absorbs its equals as
 *        well as its betters: a * b // c divides the product, not just b.
 */
[[nodiscard]] static bool scanOperandBackward(const std::vector<Token>& v,
                                              qsizetype start,
                                              int precedence,
                                              qsizetype& first)
{
  qsizetype i = prevCode(v, start);
  for (int step = 0; step < kMaxScanSteps; ++step) {
    qsizetype begin = -1;
    if (!scanPrimaryBackward(v, i, 0, begin))
      return false;

    first             = absorbPrefixesBackward(v, begin);
    const qsizetype p = prevCode(v, first - 1);
    if (p < 0 || inPrefixPosition(v, p) || binaryPrecedence(v[p]) < precedence)
      return true;

    i = prevCode(v, p - 1);
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Rewriting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the binding power at which the token must be rewritten, or 0 when it is not a
 *        Lua 5.3-only operator.
 */
[[nodiscard]] static int targetPrecedence(const std::vector<Token>& v, qsizetype i)
{
  const Token& token = v[i];
  if (token.kind != TokenKind::Operator)
    return 0;

  const QString& t = token.text;
  if (t == QLatin1String("~"))
    return inPrefixPosition(v, i) ? 11 : 5;

  if (t == QLatin1String("//"))
    return 10;

  if (t == QLatin1String("<<") || t == QLatin1String(">>"))
    return 7;

  if (t == QLatin1String("&"))
    return 6;

  return t == QLatin1String("|") ? 4 : 0;
}

/**
 * @brief Selects the tightest-binding operator left to rewrite, leftmost among equals: rewriting
 *        it first leaves both of its operands free of any other pending rewrite.
 */
[[nodiscard]] static bool findTarget(const std::vector<Token>& v, Target& out)
{
  bool found   = false;
  const auto n = static_cast<qsizetype>(v.size());
  for (qsizetype i = 0; i < n; ++i) {
    const int precedence = targetPrecedence(v, i);
    if (precedence <= 0 || (found && precedence <= out.precedence))
      continue;

    out.index      = i;
    out.precedence = precedence;
    out.unary      = v[i].text == QLatin1String("~") && precedence == 11;
    found          = true;
  }

  return found;
}

/**
 * @brief Returns the LuaJIT bit library function implementing the binary operator.
 */
[[nodiscard]] static QString bitFunctionFor(const QString& op)
{
  if (op == QLatin1String("<<"))
    return QStringLiteral("bit.lshift");

  if (op == QLatin1String(">>"))
    return QStringLiteral("bit.rshift");

  if (op == QLatin1String("&"))
    return QStringLiteral("bit.band");

  if (op == QLatin1String("|"))
    return QStringLiteral("bit.bor");

  return QStringLiteral("bit.bxor");
}

/**
 * @brief Concatenates the source text of tokens [from, to].
 */
[[nodiscard]] static QString joinRange(const std::vector<Token>& v, qsizetype from, qsizetype to)
{
  QString out;
  const auto n      = static_cast<qsizetype>(v.size());
  const qsizetype a = from < 0 ? 0 : from;
  const qsizetype b = to >= n ? n - 1 : to;
  for (qsizetype i = a; i <= b; ++i)
    out += v[i].text;

  return out;
}

/**
 * @brief Returns the trivia spanning [from, to] unless it is pure whitespace, so a comment sitting
 *        next to a rewritten operator survives the move while stray spacing does not.
 */
[[nodiscard]] static QString significantGap(const std::vector<Token>& v,
                                            qsizetype from,
                                            qsizetype to)
{
  const QString gap = joinRange(v, from, to);
  return gap.trimmed().isEmpty() ? QString() : gap;
}

/**
 * @brief Rewrites the single tightest-binding Lua 5.3 operator in the script; reports no change
 *        once none is left, and fails when an operand cannot be delimited.
 */
[[nodiscard]] static bool rewriteOnce(const QString& source, QString& out, bool& changed)
{
  changed = false;

  std::vector<Token> v;
  if (!tokenizeLuaSource(source, v))
    return false;

  Target target{-1, 0, false};
  if (!findTarget(v, target))
    return true;

  qsizetype rightEnd = -1;
  if (!scanOperandForward(v, target.index + 1, target.precedence, rightEnd))
    return false;

  const qsizetype rightStart = nextCode(v, target.index + 1);
  const QString rightGap     = significantGap(v, target.index + 1, rightStart - 1);
  const QString right        = joinRange(v, rightStart, rightEnd);
  const auto count           = static_cast<qsizetype>(v.size());
  const QString tail         = joinRange(v, rightEnd + 1, count - 1);

  changed = true;
  if (target.unary) {
    out = joinRange(v, 0, target.index - 1) + QStringLiteral("bit.bnot(") + rightGap + right
        + QStringLiteral(")") + tail;
    return true;
  }

  qsizetype leftStart = -1;
  if (!scanOperandBackward(v, target.index - 1, target.precedence, leftStart))
    return false;

  const qsizetype leftEnd = prevCode(v, target.index - 1);
  const QString leftGap   = significantGap(v, leftEnd + 1, target.index - 1);
  const QString left      = joinRange(v, leftStart, leftEnd);
  const QString head      = joinRange(v, 0, leftStart - 1);
  const QString gaps      = leftGap + rightGap;

  if (v[target.index].text == QLatin1String("//"))
    out = head + QStringLiteral("math.floor(") + left + QStringLiteral(" / ") + gaps + right
        + QStringLiteral(")") + tail;
  else
    out = head + bitFunctionFor(v[target.index].text) + QStringLiteral("(") + left
        + QStringLiteral(", ") + gaps + right + QStringLiteral(")") + tail;

  return true;
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports whether the script uses a Lua 5.3-only operator in code position.
 */
bool DataModel::LuaMigration::usesLua53Operators(const QString& script)
{
  std::vector<Token> v;
  if (!tokenizeLuaSource(script, v))
    return false;

  Target target{-1, 0, false};
  return findTarget(v, target);
}

/**
 * @brief Rewrites every Lua 5.3-only operator into its LuaJIT equivalent, returning an empty
 *        string when nothing needs rewriting or an operand could not be delimited.
 */
QString DataModel::LuaMigration::migrateToLuaJit(const QString& script)
{
  if (script.isEmpty())
    return {};

  QString current = script;
  for (int pass = 0; pass < kMaxRewrites; ++pass) {
    QString next;
    bool changed = false;
    if (!rewriteOnce(current, next, changed))
      return {};

    if (!changed)
      return current == script ? QString() : current;

    current = next;
  }

  return {};
}
