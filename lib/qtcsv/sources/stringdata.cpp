#include "include/qtcsv/stringdata.h"

using namespace QtCSV;

StringData::StringData(const StringData &other)
  : m_values(other.m_values)
{
}

StringData &StringData::operator=(const StringData &other)
{
  m_values = other.m_values;
  return *this;
}

bool StringData::operator==(const StringData &other) const
{
  return m_values == other.m_values;
}

// Add new empty row
void StringData::addEmptyRow()
{
  m_values << QList<QString>();
}

// Add new row with one value
// @input:
// - value - value that is supposed to be written to the new row
void StringData::addRow(const QString &value)
{
  m_values << (QList<QString>() << value);
}

// Add new row with specified values (as strings)
// @input:
// - values - list of strings. If list is empty, it will be interpreted
// as empty line
void StringData::addRow(const QList<QString> &values)
{
  m_values << values;
}

// Clear all data
void StringData::clear()
{
  m_values.clear();
}

// Insert new row at index position 'row'.
// @input:
// - row - index of row. If 'row' is 0, the value will be set as first row.
// If 'row' is >= rowCount(), the value will be added as new last row.
// - value - value that is supposed to be written to the new row
void StringData::insertRow(const qsizetype row, const QString &value)
{
  insertRow(row, (QList<QString>() << value));
}

// Insert new row at index position 'row'.
// @input:
// - row - index of row. If 'row' is 0, the values will be set as first row.
// If 'row' is >= rowCount(), the values will be added as new last row.
// - values - list of strings
void StringData::insertRow(const qsizetype row, const QList<QString> &values)
{
  m_values.insert(qBound(0, row, m_values.size()), values);
}

// Check if there are any rows
// @output:
// - bool - True if there are any rows, else False
bool StringData::isEmpty() const
{
  return m_values.isEmpty();
}

// Remove the row at index position 'row'.
// @input:
// - row - index of row to remove. 'row' must be a valid index position
// (i.e., 0 <= row < rowCount()). Otherwise function will do nothing.
void StringData::removeRow(const qsizetype row)
{
  if (row >= 0 && row < m_values.size())
  {
    m_values.removeAt(row);
  }
}

// Replace the row at index position 'row' with new row.
// @input:
// - row - index of row that should be replaced. 'row' must be
// a valid index position (i.e., 0 <= row < rowCount()).
// - value - value that is supposed to be written instead of the 'old' values
void StringData::replaceRow(const qsizetype row, const QString &value)
{
  replaceRow(row, (QList<QString>() << value));
}

// Replace the row at index position 'row' with new row.
// @input:
// - row - index of row that should be replaced. 'row' must be
// a valid index position (i.e., 0 <= row < rowCount()).
// - values - list of strings that is supposed to be written instead of the
// 'old' values
void StringData::replaceRow(const qsizetype row, const QList<QString> &values)
{
  m_values.replace(row, values);
}

// Reserve space for 'size' rows.
// @input:
// - size - number of rows to reserve in memory. If 'size' is smaller than the
// current number of rows, function will do nothing.
void StringData::reserve(const qsizetype size)
{
  m_values.reserve(size);
}

// Get number of rows
// @output:
// - qsizetype - current number of rows
qsizetype StringData::rowCount() const
{
  return m_values.size();
}

// Get values (as list of strings) of specified row
// @input:
// - row - valid number of row
// @output:
// - QList<QString> - values of row. If row is invalid number, function will
// return empty list.
QList<QString> StringData::rowValues(const qsizetype row) const
{
  if (row < 0 || rowCount() <= row)
  {
    return {};
  }

  return m_values.at(row);
}

// Add new row that would contain one value
StringData &StringData::operator<<(const QString &value)
{
  addRow(value);
  return *this;
}

// Add new row with specified values
StringData &StringData::operator<<(const QList<QString> &values)
{
  addRow(values);
  return *this;
}
