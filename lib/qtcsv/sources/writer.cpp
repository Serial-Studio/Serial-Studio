#include "include/qtcsv/writer.h"
#include "sources/contentiterator.h"
#include "sources/filechecker.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QTextStream>
#include <limits>

using namespace QtCSV;

// Class TempFileHandler is a helper class. Its main purpose is to delete file
// on destruction. It is like a "smart pointer" but for temporary file. When you
// create object of class TempFileHandler, you must specify absolute path
// to the (temp) file (as a string). When object will be about to destroy, it
// will try to remove specified file.
class TempFileHandler
{
  QString m_filePath;

public:
  explicit TempFileHandler(const QString &filePath)
    : m_filePath(filePath)
  {
  }
  ~TempFileHandler() { QFile::remove(m_filePath); }
};

class WriterPrivate
{
public:
  // Append information to the file
  static bool appendToFile(const QString &filePath, ContentIterator &content,
                           QStringConverter::Encoding codec);

  // Overwrite file with new information
  static bool overwriteFile(const QString &filePath, ContentIterator &content,
                            QStringConverter::Encoding codec);

  // Write to IO Device
  static bool writeToIODevice(QIODevice &ioDevice, ContentIterator &content,
                              QStringConverter::Encoding codec);

  // Create unique name for the temporary file
  static QString getTempFileName();
};

// Append information to the file
// @input:
// - filePath - string with absolute path to csv-file
// - content - not empty handler of content for csv-file
// - codec - pointer to codec object that would be used for file writing
// @output:
// - bool - True if data was appended to the file, otherwise False
bool WriterPrivate::appendToFile(const QString &filePath,
                                 ContentIterator &content,
                                 const QStringConverter::Encoding codec)
{
  if (filePath.isEmpty() || content.isEmpty())
  {
    qDebug() << __FUNCTION__ << "Error - invalid arguments";
    return false;
  }

  QFile csvFile(filePath);
  if (!csvFile.open(QIODevice::Append | QIODevice::Text))
  {
    qDebug() << __FUNCTION__
             << "Error - can't open file:" << csvFile.fileName();
    return false;
  }

  const auto result = writeToIODevice(csvFile, content, codec);
  csvFile.close();

  return result;
}

// Overwrite file with new information
// @input:
// - filePath - string with absolute path to csv-file
// - content - not empty handler of content for csv-file
// - codec - pointer to codec object that would be used for file writing
// @output:
// - bool - True if file was overwritten with new data, otherwise False
bool WriterPrivate::overwriteFile(const QString &filePath,
                                  ContentIterator &content,
                                  const QStringConverter::Encoding codec)
{
  // Create path to the unique temporary file
  const auto tempFileName = getTempFileName();
  if (tempFileName.isEmpty())
  {
    qDebug() << __FUNCTION__
             << "Error - failed to create unique name for temp file";
    return false;
  }

  TempFileHandler handler(tempFileName);

  // Write information to the temporary file
  if (!appendToFile(tempFileName, content, codec))
  {
    return false;
  }

  // Remove "old" file if it exists
  if (QFile::exists(filePath) && !QFile::remove(filePath))
  {
    qDebug() << __FUNCTION__ << "Error - failed to remove file" << filePath;
    return false;
  }

  // Copy "new" file (temporary file) to the destination path (replace
  // "old" file)
  if (!QFile::copy(tempFileName, filePath))
  {
    qDebug() << __FUNCTION__ << "Error - failed to copy temp file to"
             << filePath;
    return false;
  }

  return true;
}

// Write csv data to IO Device
// @input:
// - iodevice - IO Device to write data to
// - content - not empty handler of content for csv-file
// - codec - pointer to codec object that would be used for file writing
// @output:
// - bool - True if data could be written to the IO Device
bool WriterPrivate::writeToIODevice(QIODevice &ioDevice,
                                    ContentIterator &content,
                                    const QStringConverter::Encoding codec)
{
  if (content.isEmpty())
  {
    qDebug() << __FUNCTION__ << "Error - invalid arguments";
    return false;
  }

  // Open IO Device if it was not opened
  if (!ioDevice.isOpen() && !ioDevice.open(QIODevice::Append | QIODevice::Text))
  {
    qDebug() << __FUNCTION__ << "Error - failed to open IO Device";
    return false;
  }

  QTextStream stream(&ioDevice);
  stream.setEncoding(codec);
  while (content.hasNext())
  {
    stream << content.getNext();
  }

  stream.flush();
  return stream.status() == QTextStream::Ok;
}

// Create unique name for the temporary file
// @output:
// - QString - string with the absolute path to the temporary file that is not
// exist yet. If function failed to create unique path, it will return empty
// string.
QString WriterPrivate::getTempFileName()
{
  const auto nameTemplate
      = QDir::tempPath() + "/qtcsv_"
        + QString::number(QCoreApplication::applicationPid()) + "_%1.csv";

  for (auto counter = 0; counter < std::numeric_limits<int>::max(); ++counter)
  {
    QString name = nameTemplate.arg(
        QString::number(QRandomGenerator::global()->generate()));

    if (!QFile::exists(name))
    {
      return name;
    }
  }

  return QString();
}

// Write data to csv-file
// @input:
// - filePath - string with absolute path to csv-file
// - data - not empty AbstractData object that contains information that should
// be written to csv-file
// - separator - string or character that would separate values in a row
// (line) in csv-file
// - textDelimiter - string or character that enclose each element in a row
// - mode - write mode of the file
// - header - strings that will be written at the beginning of the file in
// one line. separator will be used as delimiter character.
// - footer - strings that will be written at the end of the file in
// one line. separator will be used as delimiter character.
// - codec - pointer to codec object that would be used for file writing
// @output:
// - bool - True if data was written to the file, otherwise False
bool Writer::write(const QString &filePath, const AbstractData &data,
                   const QString &separator, const QString &textDelimiter,
                   const WriteMode mode, const QList<QString> &header,
                   const QList<QString> &footer,
                   const QStringConverter::Encoding codec)
{
  if (filePath.isEmpty())
  {
    qDebug() << __FUNCTION__ << "Error - empty path to file";
    return false;
  }

  if (data.isEmpty())
  {
    qDebug() << __FUNCTION__ << "Error - empty data";
    return false;
  }

  if (false == CheckFile(filePath))
  {
    qDebug() << __FUNCTION__ << "Error - wrong file path/name:" << filePath;
    return false;
  }

  ContentIterator content(data, separator, textDelimiter, header, footer);
  switch (mode)
  {
    case WriteMode::APPEND:
      return WriterPrivate::appendToFile(filePath, content, codec);
      break;
    case WriteMode::REWRITE:
    default:
      return WriterPrivate::overwriteFile(filePath, content, codec);
  }

  return false;
}

// Write data to IO Device
// @input:
// - ioDevice - IO Device
// - data - not empty AbstractData object that contains information that should
// be written to IO Device
// - separator - string or character that would separate values in a row
// - textDelimiter - string or character that enclose each element in a row
// - header - strings that will be written at the beginning of the csv-data in
// one line. separator will be used as delimiter character.
// - footer - strings that will be written at the end of the csv-data in
// one line. separator will be used as delimiter character.
// - codec - pointer to codec object that would be used for data writing
// @output:
// - bool - True if data was written to the IO Device, otherwise False
bool Writer::write(QIODevice &ioDevice, const AbstractData &data,
                   const QString &separator, const QString &textDelimiter,
                   const QList<QString> &header, const QList<QString> &footer,
                   const QStringConverter::Encoding codec)
{
  if (data.isEmpty())
  {
    qDebug() << __FUNCTION__ << "Error - empty data";
    return false;
  }

  ContentIterator content(data, separator, textDelimiter, header, footer);
  return WriterPrivate::writeToIODevice(ioDevice, content, codec);
}
