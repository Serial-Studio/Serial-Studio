#ifndef QTCSVCONTENTITERATOR_H
#define QTCSVCONTENTITERATOR_H

#include "include/qtcsv/abstractdata.h"
#include <QList>
#include <QString>

namespace QtCSV
{

// ContentIterator is a class that holds references to containers with
// information. Its main purpose:
// - to separate information into a chunks and
// - to return these chunks one by one to the client.
// You can use this class as forward-iterator that can go (only once) from
// the beginning to the end of the data.
// You can use this class with csv-writer class. ContentIterator will join
// elements of one row with separator symbol and then join rows with
// new line symbol.
class ContentIterator
{
  const AbstractData &m_data;
  const QString &m_separator;
  const QString &m_textDelimiter;
  const QList<QString> &m_header;
  const QList<QString> &m_footer;
  const qsizetype m_chunkSize;
  qsizetype m_dataRow;
  bool m_atEnd;

  // Compose row string from values
  QString composeRow(const QList<QString> &values) const;

public:
  ContentIterator(const AbstractData &data, const QString &separator,
                  const QString &textDelimiter, const QList<QString> &header,
                  const QList<QString> &footer, qsizetype chunkSize = 1000);

  // Check if content contains information
  bool isEmpty() const;
  // Check if content still has chunks of information to return
  bool hasNext() const;
  // Get next chunk of information
  QString getNext();
};
} // namespace QtCSV

#endif // QTCSVCONTENTITERATOR_H
