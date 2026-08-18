/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QTest>

#include "CSV/SparseRowMerger.h"

// The single-file sparse CSV body (spec 0055 R6, AC5). Two properties carry it: exactly one row
// per distinct sample instant with nothing forward-filled, and rows in strict time order across
// sources -- which is what the bounded reorder window buys and what a downstream tool assumes.

//--------------------------------------------------------------------------------------------------
// Formatter stubs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Export.cpp owns these in production; the suite supplies the same semantics so the merge
 *        can be verified without the file, workspace and session machinery around it.
 */
void CSV::appendCsvDouble(QByteArray& dst, double value, bool fixed, int precision)
{
  dst += QByteArray::number(value, fixed ? 'f' : 'g', precision);
}

QByteArray CSV::escapeCsvBytes(const QString& field)
{
  QString out = field.simplified();
  if (!out.contains(QChar(',')) && !out.contains(QChar('"')))
    return out.toUtf8();

  out.replace(QChar('"'), QStringLiteral("\"\""));
  return QStringLiteral("\"%1\"").arg(out).toUtf8();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a schema over @p uniqueIds, in the ascending order buildExportSchema produces.
 */
static DataModel::ExportSchema makeSchema(const std::vector<int>& uniqueIds)
{
  DataModel::ExportSchema schema;
  for (int i = 0; i < static_cast<int>(uniqueIds.size()); ++i) {
    DataModel::ExportColumn column;
    column.uniqueId = uniqueIds[static_cast<std::size_t>(i)];
    schema.columns.push_back(column);
    schema.uniqueIdToColumnIndex.insert(column.uniqueId, i);
  }

  return schema;
}

/**
 * @brief Builds a block carrying @p values for @p uniqueId, one sample per entry.
 */
static DataModel::DataBlockPtr makeBlock(int sourceId,
                                         int uniqueId,
                                         const std::vector<double>& values,
                                         bool text = false)
{
  auto block      = std::make_shared<DataModel::DataBlock>();
  block->sourceId = sourceId;
  block->samples  = static_cast<qsizetype>(values.size());

  DataModel::BlockColumn column;
  column.uniqueId = uniqueId;
  column.hasText  = text;
  column.values   = values;
  if (text) {
    column.numeric.assign(values.size(), 1);
    for (const double value : values)
      column.text.push_back(QString::number(value));
  }

  block->columns.push_back(std::move(column));
  return block;
}

/**
 * @brief Collects the rows a flush emits, split into fields.
 */
class RowSink {
public:
  [[nodiscard]] CSV::SparseRowMerger::RowSink sink()
  {
    return [this](const QByteArray& row) {
      m_rows.push_back(row.trimmed());
    };
  }

  [[nodiscard]] const std::vector<QByteArray>& rows() const noexcept { return m_rows; }

  [[nodiscard]] QList<QByteArray> fields(std::size_t index) const
  {
    return m_rows[index].split(',');
  }

private:
  std::vector<QByteArray> m_rows;
};

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class CsvSparseWriterTest : public QObject {
  Q_OBJECT

private slots:
  void oneRowPerDistinctInstant();
  void unsampledCellsStayEmpty();
  void equalInstantsCoalesceAcrossSources();
  void rowsAreStrictlyTimeOrdered();
  void windowHoldsBackRecentRows();
  void finalFlushDrainsEverything();
  void textColumnsAreEscaped();
};

/**
 * @brief Every sample instant produces exactly one row, and no instant produces two.
 */
void CsvSparseWriterTest::oneRowPerDistinctInstant()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10, 20}));
  merger.addBlock(makeBlock(0, 10, {1.0, 2.0, 3.0}), {100, 200, 300});

  RowSink sink;
  merger.flush(std::numeric_limits<qint64>::max(), sink.sink());
  QCOMPARE(sink.rows().size(), std::size_t(3));
}

/**
 * @brief A dataset not sampled at an instant leaves its cell empty; nothing is forward-filled,
 *        which is the whole contract of the sparse layout (AC5).
 */
void CsvSparseWriterTest::unsampledCellsStayEmpty()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10, 20}));
  merger.addBlock(makeBlock(0, 10, {1.0, 2.0}), {100, 200});
  merger.addBlock(makeBlock(1, 20, {9.0}), {150});

  RowSink sink;
  merger.flush(std::numeric_limits<qint64>::max(), sink.sink());
  QCOMPARE(sink.rows().size(), std::size_t(3));

  QCOMPARE(sink.fields(0).at(1), QByteArray("1"));
  QVERIFY(sink.fields(0).at(2).isEmpty());
  QVERIFY(sink.fields(1).at(1).isEmpty());
  QCOMPARE(sink.fields(1).at(2), QByteArray("9"));
  QCOMPARE(sink.fields(2).at(1), QByteArray("2"));
  QVERIFY(sink.fields(2).at(2).isEmpty());
}

/**
 * @brief Two sources sampling the same instant share one row rather than emitting two.
 */
void CsvSparseWriterTest::equalInstantsCoalesceAcrossSources()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10, 20}));
  merger.addBlock(makeBlock(0, 10, {1.0}), {500});
  merger.addBlock(makeBlock(1, 20, {2.0}), {500});

  RowSink sink;
  merger.flush(std::numeric_limits<qint64>::max(), sink.sink());

  QCOMPARE(sink.rows().size(), std::size_t(1));
  QCOMPARE(sink.fields(0).at(1), QByteArray("1"));
  QCOMPARE(sink.fields(0).at(2), QByteArray("2"));
}

/**
 * @brief A block arriving after a later-stamped one still merges into ascending order: that is
 *        what the reorder window exists for.
 */
void CsvSparseWriterTest::rowsAreStrictlyTimeOrdered()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10, 20}));
  merger.addBlock(makeBlock(0, 10, {1.0, 2.0}), {300, 400});
  merger.addBlock(makeBlock(1, 20, {5.0, 6.0}), {100, 350});

  RowSink sink;
  merger.flush(std::numeric_limits<qint64>::max(), sink.sink());
  QCOMPARE(sink.rows().size(), std::size_t(4));

  double previous = -1.0;
  for (const auto& row : sink.rows()) {
    const double seconds = row.split(',').at(0).toDouble();
    QVERIFY(seconds > previous);
    previous = seconds;
  }
}

/**
 * @brief Instants newer than the cutoff stay buffered, so a late block can still merge ahead of
 *        them on the next pass.
 */
void CsvSparseWriterTest::windowHoldsBackRecentRows()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10}));
  merger.addBlock(makeBlock(0, 10, {1.0, 2.0, 3.0}), {100, 200, 300});

  RowSink sink;
  merger.flush(150, sink.sink());
  QCOMPARE(sink.rows().size(), std::size_t(1));
  QVERIFY(!merger.empty());
  QCOMPARE(merger.newestNs(), static_cast<qint64>(300));
}

/**
 * @brief The close-time flush drains whatever the window still holds, so the tail of a recording
 *        is never lost.
 */
void CsvSparseWriterTest::finalFlushDrainsEverything()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10}));
  merger.addBlock(makeBlock(0, 10, {1.0, 2.0, 3.0}), {100, 200, 300});

  RowSink held;
  merger.flush(150, held.sink());

  RowSink rest;
  merger.flush(std::numeric_limits<qint64>::max(), rest.sink());

  QCOMPARE(rest.rows().size(), std::size_t(2));
  QVERIFY(merger.empty());
}

/**
 * @brief A textual column is written through the escape path, not as a formatted double.
 */
void CsvSparseWriterTest::textColumnsAreEscaped()
{
  CSV::SparseRowMerger merger;
  merger.setSchema(makeSchema({10}));

  auto block     = makeBlock(0, 10, {0.0}, true);
  auto& column   = const_cast<DataModel::BlockColumn&>(block->columns.front());
  column.text[0] = QStringLiteral("a,b");

  merger.addBlock(block, {100});

  RowSink sink;
  merger.flush(std::numeric_limits<qint64>::max(), sink.sink());
  QVERIFY(sink.rows().front().contains("\"a,b\""));
}

QTEST_APPLESS_MAIN(CsvSparseWriterTest)

#include "tst_csv_sparse_writer.moc"
