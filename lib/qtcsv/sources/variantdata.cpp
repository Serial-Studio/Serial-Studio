#include "include/qtcsv/variantdata.h"

using namespace QtCSV;

// Check if all values are convertable to strings
// @input:
// - values - list of values
// @output:
// - bool - True if all values are convertable to strings, False otherwise
bool isConvertableToString(const QList<QVariant> &values)
{
  for (auto iter = values.constBegin(); iter != values.constEnd(); ++iter)
  {
    if (!(*iter).canConvert<QString>())
    {
      return false;
    }
  }

  return true;
}

// Transform QList<QString> to QList<QVariant>
// @input:
// - values - list of strings
// @output:
// - QList<QVariant> - list of the same strings, but converted to QVariants
QList<QVariant> toListOfVariants(const QList<QString> &values)
{
  QList<QVariant> list;
  for (auto iter = values.constBegin(); iter != values.constEnd(); ++iter)
  {
    list << QVariant(*iter);
  }

  return list;
}

VariantData::VariantData(const VariantData &other)
  : m_values(other.m_values)
{
}

VariantData &VariantData::operator=(const VariantData &other)
{
  m_values = other.m_values;
  return *this;
}

bool VariantData::operator==(const VariantData &other) const
{
  return m_values == other.m_values;
}

// Add new empty row
void VariantData::addEmptyRow()
{
  m_values << QList<QVariant>();
}

// Add new row with one value
// @input:
// - value - value that is supposed to be written to the new row.
// If value is empty, empty row will be added. Value must be convertable to a
// QString!
// @output:
// - bool - True if new row was successfully added, else False
bool VariantData::addRow(const QVariant &value)
{
  if (!value.canConvert<QString>())
  {
    return false;
  }

  m_values << (QList<QVariant>() << value);
  return true;
}

// Add new row with list of values
// @input:
// - values - list of values. If list is empty, empty row will be added.
// Values must be convertable to a QString!
// @output:
// - bool - True if new row was successfully added, else False
bool VariantData::addRow(const QList<QVariant> &values)
{
  if (!isConvertableToString(values))
  {
    return false;
  }

  m_values << values;
  return true;
}

// Add new row with specified values (as strings)
// @input:
// - values - list of strings. If list is empty, empty row will be added.
void VariantData::addRow(const QList<QString> &values)
{
  m_values << toListOfVariants(values);
}

// Clear all data
void VariantData::clear()
{
  m_values.clear();
}

// Insert new row at index position 'row'.
// @input:
// - row - index of row. If 'row' is 0, the value will be set as first row.
// If 'row' is >= rowCount(), the value will be added as new last row.
// - value - value that is supposed to be written to the new row. Value must be
// convertable to a QString!
// @output:
// - bool - True if row was inserted, False otherwise
bool VariantData::insertRow(const qsizetype row, const QVariant &value)
{
  return insertRow(row, (QList<QVariant>() << value));
}

// Insert new row at index position 'row'.
// @input:
// - row - index of row. If 'row' is 0, the value will be set as first row.
// If 'row' is >= rowCount(), the values will be added as new last row.
// - values - list of strings that are supposed to be written to the new row
// @output:
// - bool - True if row was inserted, False otherwise
bool VariantData::insertRow(const qsizetype row, const QList<QString> &values)
{
  return insertRow(row, toListOfVariants(values));
}

// Insert new row at index position 'row'.
// @input:
// - row - index of row. If 'row' is 0, the value will be set as first row.
// If 'row' is >= rowCount(), the values will be added as new last row.
// - values - list of values that are supposed to be written to the new row.
// Values must be convertable to a QString!
// @output:
// - bool - True if row was inserted, False otherwise
bool VariantData::insertRow(const qsizetype row, const QList<QVariant> &values)
{
  if (!isConvertableToString(values))
  {
    return false;
  }

  m_values.insert(qBound(0, row, m_values.size()), values);
  return true;
}

// Check if there are any rows
// @output:
// - bool - True if there are any rows, else False
bool VariantData::isEmpty() const
{
  return m_values.isEmpty();
}

// Remove the row at index position 'row'
// @input:
// - row - index of row to remove. 'row' must be a valid index position
// (i.e., 0 <= row < rowCount()). Otherwise function will do nothing.
void VariantData::removeRow(const qsizetype row)
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
// - value - value that is supposed to be written instead of the 'old' values.
// Value must be convertable to QString!
// @output:
// - bool - True if row was replaced, else False
bool VariantData::replaceRow(const qsizetype row, const QVariant &value)
{
  return replaceRow(row, (QList<QVariant>() << value));
}

// Replace the row at index position 'row' with new row.
// @input:
// - row - index of row that should be replaced. 'row' must be
// a valid index position (i.e., 0 <= row < rowCount()).
// - values - values that are supposed to be written instead of the 'old'
// values.
// @output:
// - bool - True if row was replaced, else False
bool VariantData::replaceRow(const qsizetype row, const QList<QString> &values)
{
  return replaceRow(row, toListOfVariants(values));
}

// Replace the row at index position 'row' with new row.
// @input:
// - row - index of row that should be replaced. 'row' must be
// a valid index position (i.e., 0 <= row < rowCount()).
// - values - values that are supposed to be written instead of the 'old'
// values. Values must be convertable to a QString!
// @output:
// - bool - True if row was replaced, else False
bool VariantData::replaceRow(const qsizetype row, const QList<QVariant> &values)
{
  if (!isConvertableToString(values))
  {
    return false;
  }

  m_values.replace(row, values);
  return true;
}

// Reserve space for 'size' rows.
// @input:
// - size - number of rows to reserve in memory. If 'size' is smaller than the
// current number of rows, function will do nothing.
void VariantData::reserve(const qsizetype size)
{
  m_values.reserve(size);
}

// Get number of rows
// @output:
// - qsizetype - current number of rows
qsizetype VariantData::rowCount() const
{
  return m_values.size();
}

// Get values (as list of strings) of specified row
// @input:
// - row - valid number of the row
// @output:
// - QList<QString> - values of the row. If row have invalid value, function
// will return empty QList<QString>.
QList<QString> VariantData::rowValues(const qsizetype row) const
{
  if (row < 0 || rowCount() <= row)
  {
    return {};
  }

  QList<QString> values;
  for (int i = 0; i < m_values.at(row).size(); ++i)
  {
    values << m_values.at(row).at(i).toString();
  }

  return values;
}

// Add new row that would contain one value
VariantData &VariantData::operator<<(const QVariant &value)
{
  addRow(value);
  return *this;
}

// Add new row with specified values
VariantData &VariantData::operator<<(const QList<QVariant> &values)
{
  addRow(values);
  return *this;
}

// Add new row with specified values
VariantData &VariantData::operator<<(const QList<QString> &values)
{
  addRow(values);
  return *this;
}
