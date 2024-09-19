#include "include/qtcsv/reader.h"
#include "include/qtcsv/abstractdata.h"
#include "sources/filechecker.h"
#include "sources/symbols.h"
#include <QDebug>
#include <QFile>
#include <QStringView>
#include <QTextStream>

using namespace QtCSV;

bool openFile(const QString &filePath, QFile &file)
{
  if (!CheckFile(filePath, true))
  {
    qDebug() << __FUNCTION__ << "Error - wrong file path:" << filePath;
    return false;
  }

  file.setFileName(filePath);
  const auto result = file.open(QIODevice::ReadOnly);
  if (!result)
  {
    qDebug() << __FUNCTION__ << "Error - can't open file:" << filePath;
  }

  return result;
}

// ElementInfo is a helper struct that is used as indicator of row end
struct ElementInfo
{
  bool isEnded = true;
};

class ReaderPrivate
{
  // Check if file path and separator are valid
  static bool checkParams(const QString &separator);

  // Split string to elements
  static QList<QString> splitElements(const QString &line,
                                      const QString &separator,
                                      const QString &textDelimiter,
                                      ElementInfo &elemInfo);

  // Try to find end position of first or middle element
  static qsizetype findMiddleElementPosition(const QString &str,
                                             const qsizetype &startPos,
                                             const QString &separator,
                                             const QString &txtDelim);

  // Check if current element is the last element
  static bool isElementLast(const QString &str, const qsizetype startPos,
                            const QString &separator, const QString &txtDelim);

  // Remove extra symbols (spaces, text delimeters...)
  static void removeExtraSymbols(QList<QString> &elements,
                                 const QString &textDelimiter);

public:
  // Function that really reads csv-data and transfer it's data to
  // AbstractProcessor-based processor
  static bool read(QIODevice &ioDevice, Reader::AbstractProcessor &processor,
                   const QString &separator, const QString &textDelimiter,
                   QStringConverter::Encoding codec);
};

// Function that really reads csv-data and transfer it's data to
// AbstractProcessor-based processor
// @input:
// - ioDevice - IO Device containing the csv-formatted data
// - processor - refernce to AbstractProcessor-based object
// - separator - string or character that separate values in a row
// - textDelimiter - string or character that enclose row elements
// - codec - pointer to codec object that would be used for file reading
// @output:
// - bool - result of read operation
bool ReaderPrivate::read(QIODevice &ioDevice,
                         Reader::AbstractProcessor &processor,
                         const QString &separator, const QString &textDelimiter,
                         const QStringConverter::Encoding codec)
{
  if (!checkParams(separator))
  {
    return false;
  }

  // Open IO Device if it was not opened
  if (!ioDevice.isOpen() && !ioDevice.open(QIODevice::ReadOnly))
  {
    qDebug() << __FUNCTION__ << "Error - failed to open IO Device";
    return false;
  }

  QTextStream stream(&ioDevice);
  stream.setEncoding(codec);

  // This list will contain elements of the row if its elements
  // are located on several lines
  QList<QString> row;

  ElementInfo elemInfo;
  auto result = true;
  while (!stream.atEnd())
  {
    auto line = stream.readLine();
    processor.preProcessRawLine(line);
    auto elements = ReaderPrivate::splitElements(line, separator, textDelimiter,
                                                 elemInfo);
    if (elemInfo.isEnded)
    {
      // Current row ends on this line. Check if these elements are
      // end elements of the long row
      if (row.isEmpty())
      {
        // No, these elements constitute the entire row
        if (!processor.processRowElements(elements))
        {
          result = false;
          break;
        }
      }
      else
      {
        // Yes, these elements should be added to the row
        if (!elements.isEmpty())
        {
          row.last().append(elements.takeFirst());
          row << elements;
        }

        if (!processor.processRowElements(row))
        {
          result = false;
          break;
        }

        row.clear();
      }
    }
    else
    {
      // These elements constitute long row that lasts on several lines
      if (!elements.isEmpty())
      {
        if (!row.isEmpty())
        {
          row.last().append(elements.takeFirst());
        }

        row << elements;
      }
    }
  }

  if (!elemInfo.isEnded && !row.isEmpty())
  {
    result = processor.processRowElements(row);
  }

  return result;
}

// Check if file path and separator are valid
// @input:
// - separator - string or character that separate values in a row
// @output:
// - bool - True if file path and separator are valid, otherwise False
bool ReaderPrivate::checkParams(const QString &separator)
{
  if (separator.isEmpty())
  {
    qDebug() << __FUNCTION__ << "Error - separator could not be empty";
    return false;
  }

  return true;
}

// Split string to elements
// @input:
// - line - string with data
// - separator - string or character that separate elements
// - textDelimiter - string that is used as text delimiter
// @output:
// - QList<QString> - list of elements
QList<QString> ReaderPrivate::splitElements(const QString &line,
                                            const QString &separator,
                                            const QString &textDelimiter,
                                            ElementInfo &elemInfo)
{
  // If separator is empty, return whole line. Can't work in this
  // conditions!
  if (separator.isEmpty())
  {
    elemInfo.isEnded = true;
    return (QList<QString>() << line);
  }

  if (line.isEmpty())
  {
    // If previous row was ended, then return empty QList<QString>.
    // Otherwise return list that contains one element - new line symbols
    return elemInfo.isEnded ? QList<QString>() : (QList<QString>() << LF);
  }

  QList<QString> result;
  qsizetype pos = 0;
  while (pos < line.size())
  {
    if (elemInfo.isEnded)
    {
      // This line is a new line, not a continuation of the previous
      // line.
      // Check if element starts with the delimiter symbol
      const auto delimiterPos = line.indexOf(textDelimiter, pos);
      if (delimiterPos == pos)
      {
        pos = delimiterPos + textDelimiter.size();

        // Element starts with the delimiter symbol. It means that
        // this element could contain any number of double
        // delimiters and separator symbols. This element could:
        // 1. Be the first or the middle element. Then it should end
        // with delimiter and the seprator symbols standing next to each
        // other.
        const auto midElemEndPos
            = findMiddleElementPosition(line, pos, separator, textDelimiter);
        if (midElemEndPos > 0)
        {
          const auto length = midElemEndPos - pos;
          result << line.mid(pos, length);
          pos = midElemEndPos + textDelimiter.size() + separator.size();
          continue;
        }

        // 2. Be The last element on the line. Then it should end with
        // delimiter symbol.
        if (isElementLast(line, pos, separator, textDelimiter))
        {
          const auto length = line.size() - textDelimiter.size() - pos;
          result << line.mid(pos, length);
          break;
        }

        // 3. Not ends on this line
        const auto length = line.size() - pos;
        result << line.mid(pos, length);
        elemInfo.isEnded = false;
        break;
      }
      else
      {
        // Element do not starts with the delimiter symbol. It means
        // that this element do not contain double delimiters and it
        // ends at the next separator symbol.
        // Check if line contains separator symbol.
        const auto separatorPos = line.indexOf(separator, pos);
        if (separatorPos >= 0)
        {
          // If line contains separator symbol, then our element
          // located between current position and separator
          // position. Copy it into result list and move
          // current position over the separator position.
          result << line.mid(pos, separatorPos - pos);

          // Special case: if line ends with separator symbol,
          // then at the end of the line we have empty element.
          if (separatorPos == line.size() - separator.size())
          {
            result << QString();
          }

          // Move the current position on to the next element
          pos = separatorPos + separator.size();
        }
        else
        {
          // If line do not contains separator symbol, then
          // this element ends at the end of the string.
          // Copy it into result list and exit the loop.
          result << line.mid(pos);
          break;
        }
      }
    }
    else
    {
      // This line is a continuation of the previous. Last element of the
      // previous line did not end. It started with delimiter symbol.
      // It means that this element could contain any number of double
      // delimiters and separator symbols. This element could:
      // 1. Ends somewhere in the middle of the line. Then it should ends
      // with delimiter and the seprator symbols standing next to each
      // other.
      const auto midElemEndPos
          = findMiddleElementPosition(line, pos, separator, textDelimiter);
      if (midElemEndPos >= 0)
      {
        result << (LF + line.mid(pos, midElemEndPos - pos));
        pos = midElemEndPos + textDelimiter.size() + separator.size();
        elemInfo.isEnded = true;
        continue;
      }

      // 2. Ends at the end of the line. Then it should ends with
      // delimiter symbol.
      if (isElementLast(line, pos, separator, textDelimiter))
      {
        const auto length = line.size() - textDelimiter.size() - pos;
        result << (LF + line.mid(pos, length));
        elemInfo.isEnded = true;
        break;
      }

      // 3. Not ends on this line
      result << (LF + line);
      break;
    }
  }

  removeExtraSymbols(result, textDelimiter);
  return result;
}

// Try to find end position of first or middle element
// @input:
// - str - string with data
// - startPos - start position of the current element in the string
// - separator - string or character that separate elements
// - textDelimiter - string that is used as text delimiter
// @output:
// - qsizetype - end position of the element or -1 if this element is not first
// or middle
qsizetype ReaderPrivate::findMiddleElementPosition(const QString &str,
                                                   const qsizetype &startPos,
                                                   const QString &separator,
                                                   const QString &txtDelim)
{
  const qsizetype ERROR = -1;
  if (str.isEmpty() || startPos < 0 || separator.isEmpty()
      || txtDelim.isEmpty())
  {
    return ERROR;
  }

  const auto elemEndSymbols = txtDelim + separator;
  auto elemEndPos = startPos;
  while (elemEndPos < str.size())
  {
    // Find position of element end symbol
    elemEndPos = str.indexOf(elemEndSymbols, elemEndPos);
    if (elemEndPos < 0)
    {
      // This element could not be the middle element, becaise string
      // do not contains any end symbols
      return ERROR;
    }

    // Check that this is really the end symbols of the
    // element and we don't mix up it with double delimiter
    // and separator. Calc number of delimiter symbols from elemEndPos
    // to startPos that stands together.
    qsizetype numOfDelimiters = 0;
    for (auto pos = elemEndPos; startPos <= pos; --pos, ++numOfDelimiters)
    {
      const auto strRef = str.mid(pos, txtDelim.size());
      if (QString::compare(strRef, txtDelim) != 0)
      {
        break;
      }
    }

    // If we have odd number of delimiter symbols that stand together,
    // then this is the even number of double delimiter symbols + last
    // delimiter symbol. That means that we have found end position of
    // the middle element.
    if (numOfDelimiters % 2 == 1)
    {
      return elemEndPos;
    }
    else
    {
      // Otherwise this is not the end of the middle element and we
      // should try again
      elemEndPos += elemEndSymbols.size();
    }
  }

  return ERROR;
}

// Check if current element is the last element
// @input:
// - str - string with data
// - startPos - start position of the current element in the string
// - separator - string or character that separate elements
// - textDelimiter - string that is used as text delimiter
// @output:
// - bool - True if the current element is the last element of the string,
// False otherwise
bool ReaderPrivate::isElementLast(const QString &str, const qsizetype startPos,
                                  const QString &separator,
                                  const QString &txtDelim)
{
  if (str.isEmpty() || startPos < 0 || separator.isEmpty()
      || txtDelim.isEmpty())
  {
    return false;
  }

  // Check if string ends with text delimiter. If not, then this element
  // do not ends on this line
  if (!str.endsWith(txtDelim))
  {
    return false;
  }

  // Check that this is really the end symbols of the
  // element and we don't mix up it with double delimiter.
  // Calc number of delimiter symbols from end
  // to startPos that stands together.
  qsizetype numOfDelimiters = 0;
  for (auto pos = str.size() - 1; startPos <= pos; --pos, ++numOfDelimiters)
  {
    const auto strRef = str.mid(pos, txtDelim.size());
    if (QString::compare(strRef, txtDelim) != 0)
    {
      break;
    }
  }

  // If we have odd number of delimiter symbols that stand together,
  // then this is the even number of double delimiter symbols + last
  // delimiter symbol. That means that this element is the last on the line.
  return numOfDelimiters % 2 == 1;
}

// Remove extra symbols (spaces, text delimeters...)
// @input:
// - elements - list of row elements
// - textDelimiter - string that is used as text delimiter
void ReaderPrivate::removeExtraSymbols(QList<QString> &elements,
                                       const QString &textDelimiter)
{
  if (elements.isEmpty())
  {
    return;
  }

  const auto doubleTextDelim = textDelimiter + textDelimiter;
  for (auto i = 0; i < elements.size(); ++i)
  {
    const auto str = QStringView{elements.at(i)};
    if (str.isEmpty())
    {
      continue;
    }

    qsizetype startPos = 0, endPos = str.size() - 1;

    // Find first non-space char
    for (; startPos < str.size()
           && str.at(startPos).category() == QChar::Separator_Space;
         ++startPos)
      ;

    // Find last non-space char
    for (; endPos >= 0 && str.at(endPos).category() == QChar::Separator_Space;
         --endPos)
      ;

    if (!textDelimiter.isEmpty())
    {
      // Skip text delimiter symbol if element starts with it
      const auto strStart = str.mid(startPos, textDelimiter.size());
      if (strStart == textDelimiter)
      {
        startPos += textDelimiter.size();
      }

      // Skip text delimiter symbol if element ends with it
      const auto strEnd
          = str.mid(endPos - textDelimiter.size() + 1, textDelimiter.size());
      if (strEnd == textDelimiter)
      {
        endPos -= textDelimiter.size();
      }
    }

    if ((0 < startPos || endPos < str.size() - 1) && startPos <= endPos)
    {
      elements[i] = elements[i].mid(startPos, endPos - startPos + 1);
    }

    // Also replace double text delimiter with one text delimiter symbol
    elements[i].replace(doubleTextDelim, textDelimiter);
  }
}

// ReadToListProcessor - processor that saves rows of elements to list.
class ReadToListProcessor : public Reader::AbstractProcessor
{
public:
  QList<QList<QString>> data;

  bool processRowElements(const QList<QString> &elements) override
  {
    data << elements;
    return true;
  }
};

// Read csv-file and save it's data as strings to QList<QList<QString>>
// @input:
// - filePath - string with absolute path to csv-file
// - separator - string or character that separate elements in a row
// - textDelimiter - string or character that enclose each element in a row
// - codec - pointer to codec object that would be used for file reading
// @output:
// - QList<QList<QString>> - list of values (as strings) from csv-file. In case
// of error will return empty QList<QList<QString>>.
QList<QList<QString>> Reader::readToList(const QString &filePath,
                                         const QString &separator,
                                         const QString &textDelimiter,
                                         const QStringConverter::Encoding codec)
{
  QFile file;
  return openFile(filePath, file)
             ? readToList(file, separator, textDelimiter, codec)
             : QList<QList<QString>>();
}

// Read csv-formatted data from IO Device and save it
// as strings to QList<QList<QString>>
QList<QList<QString>> Reader::readToList(QIODevice &ioDevice,
                                         const QString &separator,
                                         const QString &textDelimiter,
                                         const QStringConverter::Encoding codec)
{
  ReadToListProcessor processor;
  ReaderPrivate::read(ioDevice, processor, separator, textDelimiter, codec);
  return processor.data;
}

// Read csv-file and save it's data to AbstractData-based container class
// @input:
// - filePath - string with absolute path to csv-file
// - data - AbstractData object where all file content will be saved
// - separator - string or character that separate elements in a row
// - textDelimiter - string or character that enclose each element in a row
// - codec - pointer to codec object that would be used for file reading
// @output:
// - bool - True if file was successfully read, otherwise False
bool Reader::readToData(const QString &filePath, AbstractData &data,
                        const QString &separator, const QString &textDelimiter,
                        const QStringConverter::Encoding codec)
{
  QFile file;
  return openFile(filePath, file)
             ? readToData(file, data, separator, textDelimiter, codec)
             : false;
}

// Read csv-formatted data from IO Device and save it
// to AbstractData-based container class
bool Reader::readToData(QIODevice &ioDevice, AbstractData &data,
                        const QString &separator, const QString &textDelimiter,
                        const QStringConverter::Encoding codec)
{
  ReadToListProcessor processor;
  const auto result = ReaderPrivate::read(ioDevice, processor, separator,
                                          textDelimiter, codec);
  if (result)
  {
    for (auto i = 0; i < processor.data.size(); ++i)
    {
      data.addRow(processor.data.at(i));
    }
  }

  return result;
}

// Read csv-file and process it line-by-line
// @input:
// - filePath - string with absolute path to csv-file
// - processor - AbstractProcessor-based object that receives data from
// csv-file line-by-line
// - separator - string or character that separate elements in a row
// - textDelimiter - string or character that enclose each element in a row
// - codec - pointer to codec object that would be used for file reading
// @output:
// - bool - True if file was successfully read, otherwise False
bool Reader::readToProcessor(const QString &filePath,
                             Reader::AbstractProcessor &processor,
                             const QString &separator,
                             const QString &textDelimiter,
                             const QStringConverter::Encoding codec)
{
  QFile file;
  return openFile(filePath, file)
             ? readToProcessor(file, processor, separator, textDelimiter, codec)
             : false;
}

// Read csv-formatted data from IO Device and process it line-by-line
bool Reader::readToProcessor(QIODevice &ioDevice,
                             Reader::AbstractProcessor &processor,
                             const QString &separator,
                             const QString &textDelimiter,
                             const QStringConverter::Encoding codec)
{
  return ReaderPrivate::read(ioDevice, processor, separator, textDelimiter,
                             codec);
}
