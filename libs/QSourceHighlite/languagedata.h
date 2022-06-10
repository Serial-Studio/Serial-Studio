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

#ifndef QOWNLANGUAGEDATA_H
#define QOWNLANGUAGEDATA_H

template<typename key, typename val>
class QMultiHash;

class QLatin1String;

namespace QSourceHighlite {

using LanguageData = QMultiHash<char, QLatin1String>;

/**********************************************************/
/* LuaData ************************************************/
/**********************************************************/
void loadLuaData(LanguageData &typess,
                 LanguageData &keywordss,
                 LanguageData &builtins,
                 LanguageData &literalss,
                 LanguageData &others);

/**********************************************************/
/* C/C++ Data *********************************************/
/**********************************************************/
void loadCppData(LanguageData &typess,
             LanguageData &keywordss,
             LanguageData &builtins,
             LanguageData &literalss,
             LanguageData &others);

/**********************************************************/
/* Shell Data *********************************************/
/**********************************************************/
void loadShellData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/**********************************************************/
/* JS Data *********************************************/
/**********************************************************/
void loadJSData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/**********************************************************/
/* PHP Data *********************************************/
/**********************************************************/
void loadPHPData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/**********************************************************/
/* QML Data *********************************************/
/**********************************************************/
void loadQMLData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/**********************************************************/
/* Python Data *********************************************/
/**********************************************************/
void loadPythonData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   Rust DATA      ***********************************/
/********************************************************/
void loadRustData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   Java DATA      ***********************************/
/********************************************************/
void loadJavaData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   C# DATA      *************************************/
/********************************************************/
void loadCSharpData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   Go DATA      *************************************/
/********************************************************/
void loadGoData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   V DATA      **************************************/
/********************************************************/
void loadVData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   SQL DATA      ************************************/
/********************************************************/
void loadSQLData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   JSON DATA      ***********************************/
/********************************************************/
void loadJSONData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   CSS DATA      ***********************************/
/********************************************************/
void loadCSSData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   Typescript DATA  *********************************/
/********************************************************/
void loadTypescriptData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   YAML DATA  ***************************************/
/********************************************************/
void loadYAMLData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   VEX DATA   ***************************************/
/********************************************************/
void loadVEXData(LanguageData &types,
             LanguageData &keywords,
             LanguageData &builtin,
             LanguageData &literals,
             LanguageData &other);

/********************************************************/
/***   CMake DATA  **************************************/
/********************************************************/
void loadCMakeData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other);

/********************************************************/
/***   Make DATA  ***************************************/
/********************************************************/
void loadMakeData(QMultiHash<char, QLatin1String>& types,
    QMultiHash<char, QLatin1String>& keywords,
    QMultiHash<char, QLatin1String>& builtin,
    QMultiHash<char, QLatin1String>& literals,
    QMultiHash<char, QLatin1String>& other);

void loadAsmData(QMultiHash<char, QLatin1String>& types,
    QMultiHash<char, QLatin1String>& keywords,
    QMultiHash<char, QLatin1String>& builtin,
    QMultiHash<char, QLatin1String>& literals,
    QMultiHash<char, QLatin1String>& other);
}
#endif
