#ifndef QTCSVREADER_H
#define QTCSVREADER_H

#include "qtcsv/qtcsv_global.h"
#include "abstractdata.h"
#include <QIODevice>
#include <QList>
#include <QString>
#include <QStringConverter>

namespace QtCSV
{

// Reader class is a file reader that work with csv-files. It needs an
// absolute path to the csv-file that you are going to read or
// some IO Device with csv-formatted data.
//
// Additionally you can specify:
// - a separator character (or string) that is used as separator of row
// values. Default separator is comma (",");
// - text delimiter character (or string) that encloses each element in a
// row. Typical delimiter characters: none (""), quote ("'")
// and double quotes ("\"");
// - text codec.
//
// Reader can save (or transfer) information to:
// - QList<QList<QString>>, where each QList<QString> contains values
// of one row;
// - AbstractData-based container class;
// - AbstractProcessor-based object.
class QTCSVSHARED_EXPORT Reader
{
public:
  // AbstractProcessor is a class that could be used to process csv-data
  // line by line
  class QTCSVSHARED_EXPORT AbstractProcessor
  {
  public:
    virtual ~AbstractProcessor() = default;

    // Preprocess one raw line from a file
    // @input:
    // editable_line - raw line from a file
    virtual void preProcessRawLine(QString & /*editable_line*/) {}

    // Process one row worth of elements
    // @input:
    // - elements - list of row elements
    // @output:
    // bool - True if elements was processed successfully, False in case
    // of error. If process() return False, the csv-file will be stopped
    // reading
    virtual bool processRowElements(const QList<QString> &elements) = 0;
  };

  // Read csv-file and save it's data as strings to QList<QList<QString>>
  static QList<QList<QString>>
  readToList(const QString &filePath, const QString &separator = QString(","),
             const QString &textDelimiter = QString("\""),
             QStringConverter::Encoding codec = QStringConverter::Utf8);

  // Read csv-formatted data from IO Device and save it
  // as strings to QList<QList<QString>>
  static QList<QList<QString>>
  readToList(QIODevice &ioDevice, const QString &separator = QString(","),
             const QString &textDelimiter = QString("\""),
             QStringConverter::Encoding codec = QStringConverter::Utf8);

  // Read csv-file and save it's data to AbstractData-based container
  // class
  static bool readToData(const QString &filePath, AbstractData &data,
                         const QString &separator = QString(","),
                         const QString &textDelimiter = QString("\""),
                         QStringConverter::Encoding codec
                         = QStringConverter::Utf8);

  // Read csv-formatted data from IO Device and save it
  // to AbstractData-based container class
  static bool readToData(QIODevice &ioDevice, AbstractData &data,
                         const QString &separator = QString(","),
                         const QString &textDelimiter = QString("\""),
                         QStringConverter::Encoding codec
                         = QStringConverter::Utf8);

  // Read csv-file and process it line-by-line
  static bool
  readToProcessor(const QString &filePath, AbstractProcessor &processor,
                  const QString &separator = QString(","),
                  const QString &textDelimiter = QString("\""),
                  QStringConverter::Encoding codec = QStringConverter::Utf8);

  // Read csv-formatted data from IO Device and process it line-by-line
  static bool readToProcessor(QIODevice &ioDevice, AbstractProcessor &processor,
                              const QString &separator = QString(","),
                              const QString &textDelimiter = QString("\""),
                              QStringConverter::Encoding codec
                              = QStringConverter::Utf8);
};
} // namespace QtCSV

#endif // QTCSVREADER_H
