#ifndef QTCSVSTRINGDATA_H
#define QTCSVSTRINGDATA_H

#include "qtcsv/abstractdata.h"
#include "qtcsv/qtcsv_global.h"
#include <QList>
#include <QString>

namespace QtCSV
{

// StringData is a simple container class. It implements interface of
// AbstractData class and uses strings to store information. Also it
// provides basic functions for working with rows.
class QTCSVSHARED_EXPORT StringData : public AbstractData
{
  QList<QList<QString>> m_values;

public:
  StringData() = default;
  StringData(const StringData &other);
  StringData &operator=(const StringData &other);
  ~StringData() override = default;

  bool operator==(const StringData &other) const;

  // Add new empty row
  void addEmptyRow() override;
  // Add new row with one value
  void addRow(const QString &value);
  // Add new row with specified values (as strings)
  void addRow(const QList<QString> &values) override;
  // Clear all data
  void clear() override;
  // Insert new row at index position 'row'
  void insertRow(qsizetype row, const QString &value);
  void insertRow(qsizetype row, const QList<QString> &values);

  // Check if there are any data
  bool isEmpty() const override;
  // Remove the row at index position 'row'
  void removeRow(qsizetype row);
  // Replace the row at index position 'row' with new row
  void replaceRow(qsizetype row, const QString &value);
  void replaceRow(qsizetype row, const QList<QString> &values);

  // Reserve space for 'size' rows
  void reserve(qsizetype size);
  // Get number of rows
  qsizetype rowCount() const override;
  // Get values (as list of strings) of specified row
  QList<QString> rowValues(qsizetype row) const override;

  // Add new row that would contain one value
  StringData &operator<<(const QString &value);
  // Add new row with specified values
  StringData &operator<<(const QList<QString> &values);
};

inline bool operator!=(const StringData &left, const StringData &right)
{
  return !(left == right);
}
} // namespace QtCSV

#endif // QTCSVSTRINGDATA_H
