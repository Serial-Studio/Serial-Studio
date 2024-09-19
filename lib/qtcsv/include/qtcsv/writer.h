#ifndef QTCSVWRITER_H
#define QTCSVWRITER_H

#include "qtcsv/qtcsv_global.h"
#include "abstractdata.h"
#include <QIODevice>
#include <QList>
#include <QString>
#include <QStringConverter>

namespace QtCSV
{

class ContentIterator;

// Writer is a data-writer class that works with csv-files and IO Devices.
// As a source of information it requires AbstractData-based container
// class object.
//
// It supports different write methods:
// - WriteMode::REWRITE - if file exist, it will be rewritten
// - WriteMode::APPEND - if file exist, new information will be appended
// to the end of the file.
//
// Also you can specify header and footer for your data.
class QTCSVSHARED_EXPORT Writer
{
public:
  enum class WriteMode
  {
    REWRITE = 0,
    APPEND
  };

  // Write data to csv-file
  static bool write(const QString &filePath, const AbstractData &data,
                    const QString &separator = QString(","),
                    const QString &textDelimiter = QString("\""),
                    WriteMode mode = WriteMode::REWRITE,
                    const QList<QString> &header = {},
                    const QList<QString> &footer = {},
                    QStringConverter::Encoding codec = QStringConverter::Utf8);

  // Write data to IO Device
  static bool write(QIODevice &ioDevice, const AbstractData &data,
                    const QString &separator = QString(","),
                    const QString &textDelimiter = QString("\""),
                    const QList<QString> &header = {},
                    const QList<QString> &footer = {},
                    QStringConverter::Encoding codec = QStringConverter::Utf8);
};
} // namespace QtCSV

#endif // QTCSVWRITER_H
