#ifndef QTCSVVARIANTDATA_H
#define QTCSVVARIANTDATA_H

#include "qtcsv/abstractdata.h"
#include "qtcsv/qtcsv_global.h"
#include <QList>
#include <QString>
#include <QVariant>

namespace QtCSV
{

// VariantData is a simple container class. It implements interface of
// AbstractData class. It uses QVariant to hold information, so data could
// be of almost any type - integral, strings and so on. There is only one
// limitation - QVariant must be convertible to string (because,
// obviously, if we want to save information to CSV file, we would need to
// convert it to plain-text form). So don't forget to see docs of QVariant
// before you start using this class.
class QTCSVSHARED_EXPORT VariantData : public AbstractData
{
  QList<QList<QVariant>> m_values;

public:
  VariantData() = default;
  VariantData(const VariantData &other);
  VariantData &operator=(const VariantData &other);
  ~VariantData() override = default;

  bool operator==(const VariantData &other) const;

  // Add new empty row
  void addEmptyRow() override;
  // Add new row with one value
  bool addRow(const QVariant &value);
  // Add new row with specified values
  bool addRow(const QList<QVariant> &values);
  // Add new row with specified values (as strings)
  void addRow(const QList<QString> &values) override;
  // Clear all data
  void clear() override;
  // Insert new row at index position 'row'
  bool insertRow(qsizetype row, const QVariant &value);
  bool insertRow(qsizetype row, const QList<QString> &values);
  bool insertRow(qsizetype row, const QList<QVariant> &values);

  // Check if there are any data
  bool isEmpty() const override;
  // Remove the row at index position 'row'
  void removeRow(qsizetype row);
  // Replace the row at index position 'row' with new row
  bool replaceRow(qsizetype row, const QVariant &value);
  bool replaceRow(qsizetype row, const QList<QString> &values);
  bool replaceRow(qsizetype row, const QList<QVariant> &values);

  // Reserve space for 'size' rows
  void reserve(qsizetype size);
  // Get number of rows
  qsizetype rowCount() const override;
  // Get values (as list of strings) of specified row
  QList<QString> rowValues(qsizetype row) const override;

  // Add new row that would contain one value
  VariantData &operator<<(const QVariant &value);
  // Add new row with specified values
  VariantData &operator<<(const QList<QString> &values);
  VariantData &operator<<(const QList<QVariant> &values);
};

inline bool operator!=(const VariantData &left, const VariantData &right)
{
  return !(left == right);
}
} // namespace QtCSV

#endif // QTCSVVARIANTDATA_H
