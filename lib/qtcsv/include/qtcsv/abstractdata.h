#ifndef QTCSVABSTRACTDATA_H
#define QTCSVABSTRACTDATA_H

#include "qtcsv_global.h"

namespace QtCSV
{

// AbstractData is a pure abstract container class. Its main purpouse is to
// provide common interface for concrete container classes that could be
// used in processing of csv-files.
//
// You can create concrete Data class with AbstractData as public base class
// and implement functions for:
// - adding new rows of values;
// - getting rows values;
// - clearing all saved information;
// - and so on.
//
// Note, that AbstractData is just an interface for container class, not a
// container class. So you are free to decide how to store
// information in derived classes.
class QTCSVSHARED_EXPORT AbstractData
{
public:
  virtual ~AbstractData() = default;

  // Add new empty row
  virtual void addEmptyRow() = 0;
  // Add new row with specified values
  virtual void addRow(const QList<QString> &values) = 0;
  // Clear all data
  virtual void clear() = 0;
  // Check if there are any rows
  virtual bool isEmpty() const = 0;
  // Get number of rows
  virtual qsizetype rowCount() const = 0;
  // Get values of specified row as list of strings
  virtual QList<QString> rowValues(qsizetype row) const = 0;
};
} // namespace QtCSV

#endif // QTCSVABSTRACTDATA_H
