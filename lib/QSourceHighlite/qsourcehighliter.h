/*
 * Copyright (c) 2019-2020 Waqar Ahmed -- <waqar.17a@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef QSOURCEHIGHLITER_H
#define QSOURCEHIGHLITER_H

#include <QSyntaxHighlighter>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QStringView>
#endif

namespace QSourceHighlite
{

class QSourceHighliter : public QSyntaxHighlighter
{
public:
  enum Themes
  {
    Monokai = 1
  };

  explicit QSourceHighliter(QTextDocument *doc);
  QSourceHighliter(QTextDocument *doc, Themes theme);

  // languages
  /*********
   * When adding a language make sure that its value is a multiple of 2
   * This is because we use the next number as comment for that language
   * In case the language doesn't support multiline comments in the traditional
   * C++ sense, leave the next value empty. Otherwise mark the next value as
   * comment for that language. e.g CodeCpp = 200 CodeCppComment = 201
   */
  enum Language
  {
    // languages
    CodeCpp = 200,
    CodeCppComment = 201,
    CodeJs = 202,
    CodeJsComment = 203,
    CodeC = 204,
    CodeCComment = 205,
    CodeBash = 206,
    CodePHP = 208,
    CodePHPComment = 209,
    CodeQML = 210,
    CodeQMLComment = 211,
    CodePython = 212,
    CodeRust = 214,
    CodeRustComment = 215,
    CodeJava = 216,
    CodeJavaComment = 217,
    CodeCSharp = 218,
    CodeCSharpComment = 219,
    CodeGo = 220,
    CodeGoComment = 221,
    CodeV = 222,
    CodeVComment = 223,
    CodeSQL = 224,
    CodeJSON = 226,
    CodeXML = 228,
    CodeCSS = 230,
    CodeCSSComment = 231,
    CodeTypeScript = 232,
    CodeTypeScriptComment = 233,
    CodeYAML = 234,
    CodeINI = 236,
    CodeVex = 238,
    CodeVexComment = 239,
    CodeCMake = 240,
    CodeMake = 242,
    CodeAsm = 244,
    CodeLua = 246,
    CodeLuaComment = 247
  };
  Q_ENUM(Language)

  enum Token
  {
    CodeBlock,
    CodeKeyWord,
    CodeString,
    CodeComment,
    CodeType,
    CodeOther,
    CodeNumLiteral,
    CodeBuiltIn,
  };
  Q_ENUM(Token)

  void setCurrentLanguage(Language language);
  Q_REQUIRED_RESULT Language currentLanguage();
  void setTheme(Themes theme);

protected:
  void highlightBlock(const QString &text) override;

private:
  void highlightSyntax(const QString &text);
  Q_REQUIRED_RESULT int highlightNumericLiterals(const QString &text, int i);
  Q_REQUIRED_RESULT int highlightStringLiterals(const QChar strType,
                                                const QString &text, int i);

  /**
   * @brief returns true if c is octal
   * @param c the char being checked
   * @returns true if the number is octal, false otherwise
   */
  Q_REQUIRED_RESULT static constexpr inline bool isOctal(const char c)
  {
    return (c >= '0' && c <= '7');
  }

  /**
   * @brief returns true if c is hex
   * @param c the char being checked
   * @returns true if the number is hex, false otherwise
   */
  Q_REQUIRED_RESULT static constexpr inline bool isHex(const char c)
  {
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F'));
  }

  void cssHighlighter(const QString &text);
  void ymlHighlighter(const QString &text);
  void xmlHighlighter(const QString &text);
  void makeHighlighter(const QString &text);
  void highlightInlineAsmLabels(const QString &text);
  void asmHighlighter(const QString &text);
  void initFormats();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  static inline QStringView strMidRef(const QString &str, qsizetype position,
                                      qsizetype n = -1)
  {
    return QStringView(str).mid(position, n);
  }
#else
  static inline QStringRef strMidRef(const QString &str, int position,
                                     int n = -1)
  {
    return str.midRef(position, n);
  }
#endif

  QHash<Token, QTextCharFormat> _formats;
  Language _language;
};
} // namespace QSourceHighlite
#endif // QSOURCEHIGHLITER_H
