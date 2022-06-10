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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

#include "qsourcehighliter.h"
#include "languagedata.h"
#include "qsourcehighliterthemes.h"

#include <QDebug>
#include <algorithm>
#include <QTextDocument>

namespace QSourceHighlite {

QSourceHighliter::QSourceHighliter(QTextDocument *doc)
    : QSyntaxHighlighter(doc),
      _language(CodeC)
{
    initFormats();
}

QSourceHighliter::QSourceHighliter(QTextDocument *doc, QSourceHighliter::Themes theme)
    : QSyntaxHighlighter(doc),
      _language(CodeC)
{
    setTheme(theme);
}

void QSourceHighliter::initFormats() {
    /****************************************
     * Formats for syntax highlighting
     ***************************************/

    QTextCharFormat format = QTextCharFormat();

    _formats[Token::CodeBlock] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#F92672"));
    _formats[Token::CodeKeyWord] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#a39b4e"));
    _formats[Token::CodeString] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#75715E"));
    _formats[Token::CodeComment] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#54aebf"));
    _formats[Token::CodeType] = format;

    format = QTextCharFormat();
    format.setForeground(QColor("#db8744"));
    _formats[Token::CodeOther] = format;

    format = QTextCharFormat();
    format.setForeground(QColor("#AE81FF"));
    _formats[Token::CodeNumLiteral] = format;

    format = QTextCharFormat();
    format.setForeground(QColor("#018a0f"));
    _formats[Token::CodeBuiltIn] = format;
}

void QSourceHighliter::setCurrentLanguage(Language language) {
    if (language != _language)
        _language = language;
}

QSourceHighliter::Language QSourceHighliter::currentLanguage() {
    return _language;
}

void QSourceHighliter::setTheme(QSourceHighliter::Themes theme)
{
    _formats = QSourceHighliterTheme::theme(theme);
    rehighlight();
}

void QSourceHighliter::highlightBlock(const QString &text)
{
    if (currentBlock() == document()->firstBlock()) {
        setCurrentBlockState(_language);
    } else {
        previousBlockState() == _language ?
                    setCurrentBlockState(_language) :
                    setCurrentBlockState(_language + 1);
    }

    highlightSyntax(text);
}

/**
 * @brief Does the code syntax highlighting
 * @param text
 */
void QSourceHighliter::highlightSyntax(const QString &text)
{
    if (text.isEmpty()) return;

    const auto textLen = text.length();

    QChar comment;
    bool isCSS = false;
    bool isYAML = false;
    bool isMake = false;
    bool isAsm = false;
    bool isSQL = false;

    LanguageData keywords{},
                others{},
                types{},
                builtin{},
                literals{};

    switch (currentBlockState()) {
        case CodeLua :
        case CodeLuaComment :
            loadLuaData(types, keywords, builtin, literals, others);
            break;
        case CodeCpp :
        case CodeCppComment :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case CodeJs :
        case CodeJsComment :
            loadJSData(types, keywords, builtin, literals, others);
            break;
        case CodeC :
        case CodeCComment :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case CodeBash :
            loadShellData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodePHP :
        case CodePHPComment :
            loadPHPData(types, keywords, builtin, literals, others);
            break;
        case CodeQML :
        case CodeQMLComment :
            loadQMLData(types, keywords, builtin, literals, others);
            break;
        case CodePython :
            loadPythonData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodeRust :
        case CodeRustComment :
            loadRustData(types, keywords, builtin, literals, others);
            break;
        case CodeJava :
        case CodeJavaComment :
            loadJavaData(types, keywords, builtin, literals, others);
            break;
        case CodeCSharp :
        case CodeCSharpComment :
            loadCSharpData(types, keywords, builtin, literals, others);
            break;
        case CodeGo :
        case CodeGoComment :
            loadGoData(types, keywords, builtin, literals, others);
            break;
        case CodeV :
        case CodeVComment :
            loadVData(types, keywords, builtin, literals, others);
            break;
        case CodeSQL :
            isSQL = true;
            loadSQLData(types, keywords, builtin, literals, others);
            break;
        case CodeJSON :
            loadJSONData(types, keywords, builtin, literals, others);
            break;
        case CodeXML :
            xmlHighlighter(text);
            return;
        case CodeCSS :
        case CodeCSSComment :
            isCSS = true;
            loadCSSData(types, keywords, builtin, literals, others);
            break;
        case CodeTypeScript:
        case CodeTypeScriptComment:
            loadTypescriptData(types, keywords, builtin, literals, others);
            break;
        case CodeYAML:
            isYAML = true;
            loadYAMLData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodeINI:
            comment = QLatin1Char('#');
            break;
        case CodeVex:
        case CodeVexComment:
            loadVEXData(types, keywords, builtin, literals, others);
            break;
        case CodeCMake:
            loadCMakeData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodeMake:
            isMake = true;
            loadMakeData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodeAsm:
            isAsm = true;
            loadAsmData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        default:
            break;
    }

    // keep the default code block format
    // this statement is very slow
    // TODO: do this formatting when necessary instead of
    // applying it to the whole block in the beginning
    setFormat(0, textLen, _formats[CodeBlock]);

    auto applyCodeFormat =
        [this](int i, const LanguageData &data,
               const QString &text, const QTextCharFormat &fmt) -> int {
        // check if we are at the beginning OR if this is the start of a word
        if (i == 0 || (!text.at(i - 1).isLetterOrNumber() &&
                       text.at(i-1) != QLatin1Char('_'))) {
            const auto wordList = data.values(text.at(i).toLatin1());
            for (const QLatin1String &word : wordList) {
                // we have a word match check
                // 1. if we are at the end
                // 2. if we have a complete word
                if (word == strMidRef(text, i, word.size()) &&
                    (i + word.size() == text.length() ||
                     (!text.at(i + word.size()).isLetterOrNumber() &&
                      text.at(i + word.size()) != QLatin1Char('_')))) {
                    setFormat(i, word.size(), fmt);
                    i += word.size();
                }
            }
        }
        return i;
    };

    const QTextCharFormat &formatType = _formats[CodeType];
    const QTextCharFormat &formatKeyword = _formats[CodeKeyWord];
    const QTextCharFormat &formatComment = _formats[CodeComment];
    const QTextCharFormat &formatNumLit = _formats[CodeNumLiteral];
    const QTextCharFormat &formatBuiltIn = _formats[CodeBuiltIn];
    const QTextCharFormat &formatOther = _formats[CodeOther];

    for (int i = 0; i < textLen; ++i) {

        if (currentBlockState() % 2 != 0) goto Comment;

        while (i < textLen && !text[i].isLetter()) {
            if (text[i].isSpace()) {
                ++i;
                //make sure we don't cross the bound
                if (i == textLen) return;
                if (text[i].isLetter()) break;
                else continue;
            }
            //inline comment
            if (comment.isNull() && text[i] == QLatin1Char('/')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('/')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    } else if(text[i+1] == QLatin1Char('*')) {
                        Comment:
                        int next = text.indexOf(QLatin1String("*/"));
                        if (next == -1) {
                            //we didn't find a comment end.
                            //Check if we are already in a comment block
                            if (currentBlockState() % 2 == 0)
                                setCurrentBlockState(currentBlockState() + 1);
                            setFormat(i, textLen,  formatComment);
                            return;
                        } else {
                            //we found a comment end
                            //mark this block as code if it was previously comment
                            //first check if the comment ended on the same line
                            //if modulo 2 is not equal to zero, it means we are in a comment
                            //-1 will set this block's state as language
                            if (currentBlockState() % 2 != 0) {
                                setCurrentBlockState(currentBlockState() - 1);
                            }
                            next += 2;
                            setFormat(i, next - i,  formatComment);
                            i = next;
                            if (i >= textLen) return;
                        }
                    }
                }
            } else if (isSQL && comment.isNull() && text[i] == QLatin1Char('-')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('-')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    }
                }
            } else if (text[i] == comment) {
                setFormat(i, textLen, formatComment);
                i = textLen;
            //integer literal
            } else if (text[i].isNumber()) {
               i = highlightNumericLiterals(text, i);
            //string literals
            } else if (text[i] == QLatin1Char('\"')) {
               i = highlightStringLiterals('\"', text, i);
            }  else if (text[i] == QLatin1Char('\'')) {
               i = highlightStringLiterals('\'', text, i);
            }
            if (i >= textLen) {
                break;
            }
            ++i;
        }

        const int pos = i;

        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Types */
        i = applyCodeFormat(i, types, text, formatType);
        /************************************************
         next letter is usually a space, in that case
         going forward is useless, so continue;
         We can ++i here and go to the beginning of the next word
         so that the next formatter can check for formatting but this will
         cause problems in case the next word is also of 'Type' or the current
         type(keyword/builtin). We can work around it and reset the value of i
         in the beginning of the loop to the word's first letter but I am not
         sure about its efficiency yet.
         ************************************************/
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Keywords */
        i = applyCodeFormat(i, keywords, text, formatKeyword);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Literals (true/false/NULL,nullptr) */
        i = applyCodeFormat(i, literals, text, formatNumLit);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Builtin library stuff */
        i = applyCodeFormat(i, builtin, text, formatBuiltIn);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight other stuff (preprocessor etc.) */
        if (( i == 0 || !text.at(i-1).isLetter()) && others.contains(text[i].toLatin1())) {
            const QList<QLatin1String> wordList = others.values(text[i].toLatin1());
            for(const QLatin1String &word : wordList) {
                if (word == strMidRef(text, i, word.size()) // we have a word match
                        &&
                        (i + word.size() == text.length() // check if we are at the end
                         ||
                         !text.at(i + word.size()).isLetter()) //OR if we have a complete word
                        ) {
                    currentBlockState() == CodeCpp ?
                                setFormat(i - 1, word.size() + 1, formatOther) :
                                setFormat(i, word.size(), formatOther);
                    i += word.size();
                }
            }
        }

        //we were unable to find any match, lets skip this word
        if (pos == i) {
            int count = i;
            while (count < textLen) {
                if (!text[count].isLetter()) break;
                ++count;
            }
            i = count;
        }
    }

    if (isCSS) cssHighlighter(text);
    if (isYAML) ymlHighlighter(text);
    if (isMake) makeHighlighter(text);
    if (isAsm)  asmHighlighter(text);
}

/**
 * @brief Highlight string literals in code
 * @param strType str type i.e., ' or "
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the string
 */
int QSourceHighliter::highlightStringLiterals(const QChar strType, const QString &text, int i) {
    setFormat(i, 1,  _formats[CodeString]);
    ++i;

    while (i < text.length()) {
        //look for string end
        //make sure it's not an escape seq
        if (text.at(i) == strType && text.at(i-1) != QLatin1Char('\\')) {
            setFormat(i, 1,  _formats[CodeString]);
            ++i;
            break;
        }
        //look for escape sequence
        if (text.at(i) == QLatin1Char('\\') && (i+1) < text.length()) {
            int len = 0;
            switch(text.at(i+1).toLatin1()) {
            case 'a':
            case 'b':
            case 'e':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case '\'':
            case '"':
            case '\\':
            case '\?':
                //2 because we have to highlight \ as well as the following char
                len = 2;
                break;
            //octal esc sequence \123
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
                if (i + 4 <= text.length()) {
                    bool isCurrentOctal = true;
                    if (!isOctal(text.at(i+2).toLatin1())) {
                        isCurrentOctal = false;
                        break;
                    }
                    if (!isOctal(text.at(i+3).toLatin1())) {
                        isCurrentOctal = false;
                        break;
                    }
                    len = isCurrentOctal ? 4 : 0;
                }
                break;
            }
            //hex numbers \xFA
            case 'x':
            {
                if (i + 3 <= text.length()) {
                    bool isCurrentHex = true;
                    if (!isHex(text.at(i+2).toLatin1())) {
                        isCurrentHex = false;
                        break;
                    }
                    if (!isHex(text.at(i+3).toLatin1())) {
                        isCurrentHex = false;
                        break;
                    }
                    len = isCurrentHex ? 4 : 0;
                }
                break;
            }
            //TODO: implement unicode code point escaping
            default:
                break;
            }

            //if len is zero, that means this wasn't an esc seq
            //increment i so that we skip this backslash
            if (len == 0) {
                setFormat(i, 1,  _formats[CodeString]);
                ++i;
                continue;
            }

            setFormat(i, len, _formats[CodeNumLiteral]);
            i += len;
            continue;
        }
        setFormat(i, 1,  _formats[CodeString]);
        ++i;
    }
    return i;
}

/**
 * @brief Highlight number literals in code
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the number
 */
int QSourceHighliter::highlightNumericLiterals(const QString &text, int i)
{
    bool isPreAllowed = false;
    if (i == 0) isPreAllowed = true;
    else {
        //these values are allowed before a number
        switch(text.at(i - 1).toLatin1()) {
        //css number
        case ':':
            if (currentBlockState() == CodeCSS)
                isPreAllowed = true;
            break;
        case '$':
            if (currentBlockState() == CodeAsm)
                isPreAllowed = true;
            break;
        case '[':
        case '(':
        case '{':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
            isPreAllowed = true;
            break;
        }
    }

    if (!isPreAllowed) return i;

    const int start = i;

    if ((i+1) >= text.length()) {
        setFormat(i, 1, _formats[CodeNumLiteral]);
        return ++i;
    }

    ++i;
    //hex numbers highlighting (only if there's a preceding zero)
    if (text.at(i) == QChar('x') && text.at(i - 1) == QChar('0'))
        ++i;

    while (i < text.length()) {
        if (!text.at(i).isNumber() && text.at(i) != QChar('.') &&
             text.at(i) != QChar('e')) //exponent
            break;
        ++i;
    }

    bool isPostAllowed = false;
    if (i == text.length()) {
        //cant have e at the end
        if (text.at(i - 1) != QChar('e'))
            isPostAllowed = true;
    } else {
        //these values are allowed after a number
        switch(text.at(i).toLatin1()) {
        case ']':
        case ')':
        case '}':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '>':
        case '<':
        case ';':
            isPostAllowed = true;
            break;
        // for 100u, 1.0F
        case 'p':
            if (currentBlockState() == CodeCSS)
                if (i + 1 < text.length() && text.at(i+1) == QChar('x')) {
                    if (i + 2 == text.length() || !text.at(i+2).isLetterOrNumber())
                    isPostAllowed = true;
                }
            break;
        case 'e':
            if (currentBlockState() == CodeCSS)
                if (i + 1 < text.length() && text.at(i+1) == QChar('m')) {
                    if (i + 2 == text.length() || !text.at(i+2).isLetterOrNumber())
                    isPostAllowed = true;
                }
            break;
        case 'u':
        case 'l':
        case 'f':
        case 'U':
        case 'L':
        case 'F':
            if (i + 1 == text.length() || !text.at(i+1).isLetterOrNumber()) {
                isPostAllowed = true;
                ++i;
            }
            break;
        }
    }
    if (isPostAllowed) {
        int end = i;
        setFormat(start, end - start, _formats[CodeNumLiteral]);
    }
    //decrement so that the index is at the last number, not after it
    return --i;
}

/**
 * @brief The YAML highlighter
 * @param text
 * @details This function post processes a line after the main syntax
 * highlighter has run for additional highlighting. It does these things
 *
 * If the current line is a comment, skip it
 *
 * Highlight all the words that have a colon after them as 'keyword' except:
 * If the word is a string, skip it.
 * If the colon is in between a path, skip it (C:\)
 *
 * Once the colon is found, the function will skip every character except 'h'
 *
 * If an h letter is found, check the next 4/5 letters for http/https and
 * highlight them as a link (underlined)
 */
void QSourceHighliter::ymlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    bool colonNotFound = false;

    //if this is a comment don't do anything and just return
    if (text.trimmed().at(0) == QLatin1Char('#'))
        return;

    for (int i = 0; i < textLen; ++i) {
        if (!text.at(i).isLetter()) continue;

        if (colonNotFound && text.at(i) != QLatin1Char('h')) continue;

        //we found a string literal, skip it
        if (i != 0 && (text.at(i-1) == QLatin1Char('"') || text.at(i-1) == QLatin1Char('\''))) {
            const int next = text.indexOf(text.at(i-1), i);
            if (next == -1) break;
            i = next;
            continue;
        }

        const int colon = text.indexOf(QLatin1Char(':'), i);

        //if colon isn't found, we set this true
        if (colon == -1) colonNotFound = true;

        if (!colonNotFound) {
            //if the line ends here, format and return
            if (colon+1 == textLen) {
                setFormat(i, colon - i, _formats[CodeKeyWord]);
                return;
            } else {
                //colon is found, check if it isn't some path or something else
                if (!(text.at(colon+1) == QLatin1Char('\\') && text.at(colon+1) == QLatin1Char('/'))) {
                    setFormat(i, colon - i, _formats[CodeKeyWord]);
                }
            }
        }

        //underlined links
        if (text.at(i) == QLatin1Char('h')) {
            if (strMidRef(text, i, 5) == QLatin1String("https") ||
                    strMidRef(text, i, 4) == QLatin1String("http")) {
                int space = text.indexOf(QChar(' '), i);
                if (space == -1) space = textLen;
                QTextCharFormat f = _formats[CodeString];
                f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(i, space - i, f);
                i = space;
            }
        }
    }
}

void QSourceHighliter::cssHighlighter(const QString &text)
{
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    for (int i = 0; i<textLen; ++i) {
        if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char('#')) {
            if (i+1 >= textLen) return;
            if (text[i + 1].isSpace() || text[i+1].isNumber()) continue;
            int space = text.indexOf(QLatin1Char(' '), i);
            if (space < 0) {
                space = text.indexOf('{');
                if (space < 0) {
                    space = textLen;
                }
            }
            setFormat(i, space - i, _formats[CodeKeyWord]);
            i = space;
        } else if (text[i] == QLatin1Char('c')) {
            if (strMidRef(text, i, 5) == QLatin1String("color")) {
                i += 5;
                int colon = text.indexOf(QLatin1Char(':'), i);
                if (colon < 0) continue;
                i = colon;
                i++;
                while(i < textLen) {
                    if (!text[i].isSpace()) break;
                    i++;
                }
                int semicolon = text.indexOf(QLatin1Char(';'));
                if (semicolon < 0) semicolon = textLen;
                const QString color = text.mid(i, semicolon-i);
                QTextCharFormat f = _formats[CodeBlock];
                QColor c(color);
                if (color.startsWith(QLatin1String("rgb"))) {
                    int t = text.indexOf(QLatin1Char('('), i);
                    int rPos = text.indexOf(QLatin1Char(','), t);
                    int gPos = text.indexOf(QLatin1Char(','), rPos+1);
                    int bPos = text.indexOf(QLatin1Char(')'), gPos);
                    if (rPos > -1 && gPos > -1 && bPos > -1) {
                        const auto r = strMidRef(text, t+1, rPos - (t+1));
                        const auto g = strMidRef(text, rPos+1, gPos - (rPos + 1));
                        const auto b = strMidRef(text, gPos+1, bPos - (gPos+1));
                        c.setRgb(r.toInt(), g.toInt(), b.toInt());
                    } else {
                        c = _formats[CodeBlock].background().color();
                    }
                }

                if (!c.isValid()) {
                    continue;
                }

                int lightness{};
                QColor foreground;
                //really dark
                if (c.lightness() <= 20) {
                    foreground = Qt::white;
                } else if (c.lightness() > 20 && c.lightness() <= 51){
                    foreground = QColor("#ccc");
                } else if (c.lightness() > 51 && c.lightness() <= 78){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 78 && c.lightness() <= 110){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 127) {
                    lightness = c.lightness() + 100;
                    foreground = c.darker(lightness);
                }
                else {
                    lightness = c.lightness() + 100;
                    foreground = c.lighter(lightness);
                }

                f.setBackground(c);
                f.setForeground(foreground);
                setFormat(i, semicolon - i, QTextCharFormat()); //clear prev format
                setFormat(i, semicolon - i, f);
                i = semicolon;
            }
        }
    }
}


void QSourceHighliter::xmlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    setFormat(0, textLen, _formats[CodeBlock]);

    for (int i = 0; i < textLen; ++i) {
        if (text[i] == QLatin1Char('<') && text[i+1] != QLatin1Char('!')) {

            const int found = text.indexOf(QLatin1Char('>'), i);
            if (found > 0) {
                ++i;
                if (text[i] == QLatin1Char('/')) ++i;
                setFormat(i, found - i, _formats[CodeKeyWord]);
            }
        }

        if (text[i] == QLatin1Char('=')) {
            int lastSpace = text.lastIndexOf(QLatin1Char(' '), i);
            if (lastSpace == i-1) lastSpace = text.lastIndexOf(QLatin1Char(' '), i-2);
            if (lastSpace > 0) {
                setFormat(lastSpace, i - lastSpace, _formats[CodeBuiltIn]);
            }
        }

        if (text[i] == QLatin1Char('\"')) {
            const int pos = i;
            int cnt = 1;
            ++i;
            //bound check
            if ( (i+1) >= textLen) return;
            while (i < textLen) {
                if (text[i] == QLatin1Char('\"')) {
                    ++cnt;
                    ++i;
                    break;
                }
                ++i; ++cnt;
                //bound check
                if ( (i+1) >= textLen) {
                    ++cnt;
                    break;
                }
            }
            setFormat(pos, cnt, _formats[CodeString]);
        }
    }
}

void QSourceHighliter::makeHighlighter(const QString &text)
{
    int colonPos = text.indexOf(QLatin1Char(':'));
    if (colonPos == -1)
        return;
    setFormat(0, colonPos, _formats[Token::CodeBuiltIn]);
}

/**
 * @brief highlight inline labels such as 'func()' in "call func()"
 * @param text
 */
void QSourceHighliter::highlightInlineAsmLabels(const QString &text)
{
#define Q(s) QStringLiteral(s)
    static const QString jumps[27] = {
        //0 - 19
        Q("jmp"), Q("je"), Q("jne"), Q("jz"), Q("jnz"), Q("ja"), Q("jb"), Q("jg"), Q("jge"), Q("jae"), Q("jl"), Q("jle"),
        Q("jbe"), Q("jo"), Q("jno"), Q("js"), Q("jns"), Q("jcxz"), Q("jecxz"), Q("jrcxz"),
        //20 - 24
        Q("loop"), Q("loope"), Q("loopne"), Q("loopz"), Q("loopnz"),
        //25 - 26
        Q("call"), Q("callq")
    };
#undef Q

    auto format = _formats[Token::CodeBuiltIn];
    format.setFontUnderline(true);

    const QString trimmed = text.trimmed();
    int start = -1;
    int end = -1;
    char c{};
    if (!trimmed.isEmpty())
        c = trimmed.at(0).toLatin1();
    if (c == 'j') {
        start = 0; end = 20;
    } else if (c == 'c') {
        start = 25; end = 27;
    } else if (c == 'l') {
        start = 20; end = 25;
    } else {
        return;
    }

    auto skipSpaces = [&text](int& j){
        while (text.at(j).isSpace()) j++;
        return j;
    };

    for (int i = start; i < end; ++i) {
        if (trimmed.startsWith(jumps[i])) {
            int j = 0;
            skipSpaces(j);
            j = j + jumps[i].length() + 1;
            skipSpaces(j);
            int len = text.length() - j;
            setFormat(j, len, format);
        }
    }
}

void QSourceHighliter::asmHighlighter(const QString& text)
{
    highlightInlineAsmLabels(text);
    //label highlighting
    //examples:
    //L1:
    //LFB1:           # local func begin
    //
    //following e.gs are not a label
    //mov %eax, Count::count(%rip)
    //.string ": #%s"

    //look for the last occurence of a colon
    int colonPos = text.lastIndexOf(QLatin1Char(':'));
    if (colonPos == -1)
        return;
    //check if this colon is in a comment maybe?
    bool isComment = text.lastIndexOf('#', colonPos) != -1;
    if (isComment) {
        int commentPos = text.lastIndexOf('#', colonPos);
        colonPos = text.lastIndexOf(':', commentPos);
    }

    auto format = _formats[Token::CodeBuiltIn];
    format.setFontUnderline(true);

    if (colonPos >= text.length() - 1) {
        setFormat(0, colonPos, format);
    }

    int i = 0;
    bool isLabel = true;
    for (i = colonPos + 1; i < text.length(); ++i) {
        if (!text.at(i).isSpace()) {
            isLabel = false;
            break;
        }
    }

    if (!isLabel && i < text.length() && text.at(i) == QLatin1Char('#'))
        setFormat(0, colonPos, format);
}
}
