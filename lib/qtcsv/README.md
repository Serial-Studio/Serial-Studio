# qtcsv

[![Build status](https://ci.appveyor.com/api/projects/status/7uv7ghs9uexf08bv/branch/master?svg=true)](https://ci.appveyor.com/project/iamantony/qtcsv/branch/master)

Small easy-to-use library for reading and writing [csv-files][csvwiki] in Qt.

Qt suppport:
- Qt6: branch `master` (you're here)
- Qt4 and Qt5: branch `qt4_qt5`

Tested on:
- Ubuntu with gcc, Qt6
- Windows with MinGW, Qt6
- OS X with clang, Qt6

## Table of contents
* [1. Quick Example](#1-quick-example)
* [2. Usage](#2-usage)
  * [2.1 Containers](#21-containers)
    * [2.1.1 AbstractData](#211-abstractdata)
    * [2.1.2 StringData](#212-stringdata)
    * [2.1.3 VariantData](#213-variantdata)
  * [2.2 Reader](#22-reader)
    * [2.2.1 Reader functions](#221-reader-functions)
    * [2.2.2 AbstractProcessor](#222-abstractprocessor)
  * [2.3 Writer](#23-writer)
* [3. Requirements](#3-requirements)
* [4. Build](#4-build)
  * [4.1 Building on Linux, OS X](#41-building-on-linux-os-x)
    * [4.1.1 Using qmake](#411-using-qmake)
    * [4.1.2 Using cmake](#412-using-cmake)
  * [4.2 Building on Windows](#42-building-on-windows)
    * [4.2.1 Prebuild step on Windows](#421-prebuild-step-on-windows)
    * [4.2.2 Using qmake](#422-using-qmake)
    * [4.2.3 Using cmake](#423-using-cmake)
* [5. Run tests](#5-run-tests)
  * [5.1 Linux, OS X](#51-linux-os-x)
  * [5.2 Windows](#52-windows)
* [6. Installation](#6-installation)
* [7. Examples](#7-examples)
* [8. Other](#8-other)
* [9. Creators](#9-creators)

## 1. Quick Example

```cpp
#include <QList>
#include <QString>
#include <QDir>
#include <QDebug>

#include "qtcsv/stringdata.h"
#include "qtcsv/reader.h"
#include "qtcsv/writer.h"

int main()
{
    // prepare data that you want to save to csv-file
    QList<QString> strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);
    strData.addEmptyRow();
    strData << strList << "this is the last row";

    // write to file
    const auto filePath = QDir::currentPath() + "/test.csv";
    QtCSV::Writer::write(filePath, strData);

    // read data from file
    const auto readData = QtCSV::Reader::readToList(filePath);
    for (auto i = 0; i < readData.size(); ++i)
    {
        qDebug() << readData.at(i).join(",");
    }

    return 0;
}
```

## 2. Usage

Library could be separated into three parts: **_Reader_**,
**_Writer_** and **_Containers_**.

### 2.1 Containers

*qtcsv* library can work with standard Qt containers (like QList) and special data containers.

#### 2.1.1 AbstractData

**[_AbstractData_][absdata]** is a pure abstract class that provides
interface for a class of special data containers.

```cpp
class AbstractData {
public:
    virtual ~AbstractData() = default;

    virtual void addEmptyRow() = 0;
    virtual void addRow(const QList<QString>& values) = 0;
    virtual void clear() = 0;
    virtual bool isEmpty() const = 0;
    virtual qsizetype rowCount() const = 0;
    virtual QList<QString> rowValues(qsizetype row) const = 0;
};
```

As you can see, **_AbstractData_** declare virtual functions for adding new rows,
getting rows values, clearing all information and so on. Basic stuff for a
container class.

#### 2.1.2 StringData

**[_StringData_][strdata]** inhertis interface of **_AbstractData_**
class and provides some useful functions for inserting/removing rows and
so on. Class uses strings to store data.

#### 2.1.3 VariantData

If you store information in different types - integers, floating point
values, strings or (almost) anything else (example: [1, 3.14, "check"]) -
and you don't want to manually transform each element to string, then you
can use **_QVariant_** magic. Wrap your data into **_QVariants_** and pass it to
**[_VariantData_][vardata]** class. It also inherits interface of **_AbstractData_**
plus has several useful methods.

### 2.2 Reader

Use **[_Reader_][reader]** class to read csv-files / csv-data. Let's see it's functions.

#### 2.2.1 Reader functions

1. Read data to **_QList\<QList\<QString\>\>_**
  ```cpp
  QList<QList<QString>> readToList(
      const QString& filePath,
      const QString& separator = QString(","),
      const QString& textDelimiter = QString("\""),
      QStringConverter::Encoding codec = QStringConverter::Utf8);
                                
  QList<QList<QString>> readToList(
      QIODevice& ioDevice,
      const QString& separator = QString(","),
      const QString& textDelimiter = QString("\""),
      QStringConverter::Encoding codec = QStringConverter::Utf8);
  ```

  - *filePath* - string with absolute path to existent csv-file
    (example: "/home/user/my-file.csv");
  - *ioDevice* - IO Device that contains csv-formatted data;
  - *separator* (optional) - delimiter symbol, that separates elements
  in a row (by default it is comma - ",");
  - *textDelimiter* (optional) - text delimiter symbol that encloses
  each element in a row (by default it is double quoute - ");
  - *codec* (optional) - codec type that will be used
  to read data from the file (by default it is UTF-8 codec).

  As a result function will return **_QList\<QList\<QString\>\>_** that holds content
  of the file / IO Device. Size of it will be equal to the number of rows
  in csv-data source. Each **_QList\<QString\>_** will contain elements of the
  corresponding row. On error these functions will return empty list.

2. Read data to **_AbstractData_**-based container
  ```cpp
  bool readToData(
      const QString& filePath,
      AbstractData& data,
      const QString& separator = QString(","),
      const QString& textDelimiter = QString("\""),
      QStringConverter::Encoding codec = QStringConverter::Utf8);
                  
  bool readToData(
      QIODevice& ioDevice,
      AbstractData& data,
      const QString& separator = QString(","),
      const QString& textDelimiter = QString("\""),
      QStringConverter::Encoding codec = QStringConverter::Utf8);
  ```

  These functions are little more advanced and, I hope, a little more useful.

  - *filePath* - string with absolute path to existent csv-file;
  - *ioDevice* - IO Device that contains csv-formatted data;
  - *data* - reference to **_AbstractData_**-based class object;
  - *separator* (optional) - delimiter symbol;
  - *textDelimiter* (optional) - text delimiter symbol;
  - *codec* (optional) - codec type.

  Functions will save content of the file / IO Device in *data* object using virtual
  function **_AbstractData::addRow(QList\<QString\>)_**. Elements of csv-data will be
  saved as strings in objects of **_StringData_** / **_VariantData_**.

  If you would like to convert row elements to the target types on-the-fly during
  file reading, please implement your own **_AbstractData_**-based container class.

3. Read data and process it line-by-line by **_AbstractProcessor_**-based processor
  ```cpp
  bool readToProcessor(
      const QString& filePath,
      AbstractProcessor& processor,
      const QString& separator = QString(","),
      const QString& textDelimiter = QString("\""),
      QStringConverter::Encoding codec = QStringConverter::Utf8);
                       
  bool readToProcessor(
      QIODevice& ioDevice,
      AbstractProcessor& processor,
      const QString& separator = QString(","),
      const QString& textDelimiter = QString("\""),
      QStringConverter::Encoding codec = QStringConverter::Utf8);
  ```

  - *filePath* - string with absolute path to existent csv-file;
  - *ioDevice* - IO Device that contains csv-formatted data;
  - *processor* - reference to **_AbstractProcessor_**-based class object;
  - *separator* (optional) - delimiter symbol;
  - *textDelimiter* (optional) - text delimiter symbol;
  - *codec* (optional) - codec type.

  This function will read csv-data from file / IO Device line-by-line and
  pass data to *processor* object.

#### 2.2.2 AbstractProcessor

**[_AbstractProcessor_][reader]** is an abstract class with two methods:

``` cpp
class AbstractProcessor
{
public:
    virtual ~AbstractProcessor() = default;
    virtual void preProcessRawLine(QString& /*editable_line*/) {}
    virtual bool processRowElements(const QList<QString>& elements) = 0;
};
```

When **_Reader_** opens a csv-data source (file or IO Device), it starts
reading it line by line in a cycle. First of all, **_Reader_** passes each
new line to processor's method **_preProcessRawLine(QString&)_**. In this method
you can edit the line - replace values, remove sensitive information and so on.

After that **_Reader_** parses elements of the row and passes them to processor's
method **_processRowElements(QList\<QString\>)_**. At that step you can do whatever
you want with row elements - convert/edit/save/filter the elements. Please check out
**_ReadToListProcessor_** class (defined in [reader.cpp][reader-cpp]) as an example of
such processor.

### 2.3 Writer

Use **[_Writer_][writer]** class to write csv-data to files / IO Devices.

```cpp
bool write(
    const QString& filePath,
    const AbstractData& data,
    const QString& separator = QString(","),
    const QString& textDelimiter = QString("\""),
    WriteMode mode = WriteMode::REWRITE,
    const QList<QString>& header = {},
    const QList<QString>& footer = {},
    QStringConverter::Encoding codec = QStringConverter::Utf8);
                   
bool write(
    QIODevice& ioDevice,
    const AbstractData& data,
    const QString& separator = QString(","),
    const QString& textDelimiter = QString("\""),
    const QList<QString>& header = {},
    const QList<QString>& footer = {},
    QStringConverter::Encoding codec = QStringConverter::Utf8);
```

- *filePath* - string with absolute path to csv-file (new or existent);
- *ioDevice* - IO Device;
- *data* - object, that contains information that you want to write to
csv-file / IO Device;
- *separator* (optional) - delimiter symbol (by default it is comma - ",");
- *textDelimiter* (optional) - text delimiter symbol that encloses
each element in a row (by default it is double quoute - ");
- *mode* (optional) - write mode flag.
If it set to **_WriteMode::REWRITE_** and csv-file exist, then csv-file will be
rewritten. If *mode* set to **_WriteMode::APPEND_** and csv-file exist, then new
information will be appended to the end of the file. By default mode is set
to **_WriteMode::REWRITE_**.
- *header* (optional) - strings that will be written as the first row;
- *footer* (optional) - strings that will be written at the last row;
- *codec* (optional) - codec type that will be used
in write operations (by default it is UTF-8 codec).

**_Writer_** uses *CRLF* as line ending symbols in accordance with [standard][rfc].
If element of the row contains separator symbol or line ending symbols, such
element will be enclosed by text delimiter symbols (or double quoute if you have set
empty string as text delimiter symbol).

## 3. Requirements

Qt6, only core/base modules.

## 4. Build

### 4.1 Building on Linux, OS X

#### 4.1.1 Using qmake

```bash
cd /path/to/folder/with/qtcsv

# Create build directory
mkdir ./build
cd ./build

# Build library. You can choose build type: release or debug
qmake ../qtcsv.pro CONFIG+=[release|debug]
make

# Create build directory for tests
mkdir ./tests
cd ./tests

# Build tests. Besides of setting build type, we set path where linker could find compiled library file.
qmake ../../tests/tests.pro CONFIG+=[release|debug] LIBS+=-L../
make
```

#### 4.1.2 Using cmake

```bash
cd /path/to/folder/with/qtcsv

# Create build directory
mkdir ./build
cd ./build

# Build library and tests. See CMakeLists.txt for list of additional options that you can set.
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
make
```

### 4.2 Building on Windows

#### 4.2.1 Prebuild step on Windows

If you going to build *qtcsv* library on Windows with MinGW, first of all [check that your PATH variable][path_var] contains paths to _Qt_ and _MinGW_ toolsets.

#### 4.2.2 Using qmake

```bash
cd C:\path\to\folder\with\qtcsv

# Create build directory
mkdir .\build
cd .\build

# Build library. You can choose build type: release or debug. Set DESTDIR to current directory.
qmake ..\qtcsv.pro CONFIG+=[release|debug] DESTDIR=%cd%
mingw32-make

# Create build directory for tests
mkdir .\tests
cd .\tests

# Copy library file into 'tests' directory
copy ..\qtcsv.dll .\

# Build tests
qmake ..\..\tests\tests.pro CONFIG+=[release|debug] DESTDIR=%cd%
mingw32-make
```

#### 4.2.3 Using cmake

```bash
cd C:\path\to\folder\with\qtcsv

# Create build directory
mkdir .\build
cd .\build

# Build library and tests. See CMakeLists.txt for list of additional options that you can set.
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
mingw32-make
```

## 5. Run tests

To run tests use these commands after build of *qtcsv*:

### 5.1 Linux, OS X

```bash
cd /path/to/folder/with/qtcsv/build/tests

# Set LD_LIBRARY_PATH variable so test binary will know where to search library file.
# Suppose, that library file is located in "build" directory, up a level from current directory.
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/../

chmod 777 qtcsv_tests
./qtcsv_tests
```

### 5.2 Windows

```bash
cd /path/to/folder/with/qtcsv/build/tests

# Copy library file into "tests" directory
copy ..\*qtcsv.dll .\

qtcsv_tests.exe
```

## 6. Installation

On Unix-like OS you can install *qtcsv* library using these commands:

```bash
sudo make install
sudo ldconfig -n -v /usr/local/lib
```

These commands will copy all compiled files (libqtcsv.so\*) from build
folder to *"/usr/local/lib"*. Also all headers files will be copied
from *"./include"* folder to *"/usr/local/include/"*.

All installation settings are defined in [*qtcsv.pro*][qtcsv-pro] file.
See *copy_lib_headers* and *target* variables.

For additional information, see [Qt documentation][install-files] about
files installation.

## 7. Examples

If you would like to try *qtcsv*, you can download [qtcsv-example project][qtcsv-example].
Don't forget to read README.md file!

## 8. Other

If you want to know more about csv-file format, please read [RFC 4180][rfc] standard.

Also on [this page][csvlint] you can find useful tips about how should look
proper csv-file.

## 9. Creators

Author: [Antony Cherepanov][mypage] (antony.cherepanov@gmail.com)  
Contributors: [Patrizio "pbek" Bekerle][pbek], [Furkan "Furkanzmc" Üzümcü][Furkanzmc], [Martin "schulmar" Schulze][schulmar], [cguentherTUChemnitz][cguentherTUChemnitz], [David Jung][David_Jung], [Nicu Tofan][TNick], [Florian Apolloner][apollo13], [Michael Pollind][pollend], [Kuba Ober][KubaO], [Akram Abdeslem Chaima][gakramx], [Bogdan Cristea][cristeab], [Markus Krause][markusdd]

[csvwiki]: http://en.wikipedia.org/wiki/Comma-separated_values
[reader]: https://github.com/iamantony/qtcsv/blob/master/include/qtcsv/reader.h
[reader-cpp]: https://github.com/iamantony/qtcsv/blob/master/sources/reader.cpp
[writer]: https://github.com/iamantony/qtcsv/blob/master/include/qtcsv/writer.h
[absdata]: https://github.com/iamantony/qtcsv/blob/master/include/qtcsv/abstractdata.h
[strdata]: https://github.com/iamantony/qtcsv/blob/master/include/qtcsv/stringdata.h
[vardata]: https://github.com/iamantony/qtcsv/blob/master/include/qtcsv/variantdata.h
[qtcsv-pro]: https://github.com/iamantony/qtcsv/blob/master/qtcsv.pro
[install-files]: https://doc.qt.io/qt-6/qmake-advanced-usage.html#installing-files
[qtcsv-example]: https://github.com/iamantony/qtcsv-example
[rfc]: http://tools.ietf.org/pdf/rfc4180.pdf
[path_var]: http://superuser.com/questions/284342/what-are-path-and-other-environment-variables-and-how-can-i-set-or-use-them
[csvlint]: http://csvlint.io/about
[mypage]: https://github.com/iamantony
[pbek]: https://github.com/pbek
[Furkanzmc]: https://github.com/Furkanzmc
[schulmar]: https://github.com/schulmar
[cguentherTUChemnitz]: https://github.com/cguentherTUChemnitz
[David_Jung]: https://github.com/davidljung
[TNick]: https://github.com/TNick
[apollo13]: https://github.com/apollo13
[pollend]: https://github.com/pollend
[KubaO]: https://github.com/KubaO
[gakramx]: https://github.com/gakramx
[cristeab]: https://github.com/cristeab
[markusdd]: https://github.com/markusdd
