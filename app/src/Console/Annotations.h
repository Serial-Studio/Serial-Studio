/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <QAbstractTableModel>
#include <QByteArray>
#include <QColor>
#include <QHash>
#include <QJSEngine>
#include <QJSValue>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <vector>

#include "DataModel/Scripting/JsWatchdog.h"

namespace Console {

/**
 * @brief One decoded byte-range annotation (spec 0059): absolute stream offsets (inclusive),
 *        the decoder-declared row and class, and up to kTextLevels interned text renderings from
 *        longest to shortest.
 */
struct Annotation {
  static constexpr int kTextLevels = 3;

  qint64 start;
  qint64 end;
  qint32 row;
  qint32 cls;
  qint32 texts[kTextLevels];
};

/**
 * @brief Bounded annotation store over the raw byte stream the Console receives: Annotation
 *        records, an interned text table, the decoder-declared rows/classes and a bounded copy of
 *        the raw bytes (payload extraction). Doubles as the QAbstractTableModel behind the table
 *        view. GUI thread only, chunk cadence, never per byte on the frame pipeline.
 */
class AnnotationModel : public QAbstractTableModel {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             NOTIFY countChanged)
  Q_PROPERTY(qint64 retainedStart
             READ retainedStart
             NOTIFY countChanged)
  Q_PROPERTY(qint64 retainedEnd
             READ retainedEnd
             NOTIFY countChanged)
  Q_PROPERTY(int capacity
             READ capacity
             CONSTANT)
  Q_PROPERTY(qint64 labelledStart
             READ labelledStart
             NOTIFY countChanged)
  Q_PROPERTY(qint64 labelledEnd
             READ labelledEnd
             NOTIFY countChanged)
  Q_PROPERTY(QStringList rowNames
             READ rowNames
             NOTIFY layoutDeclared)
  Q_PROPERTY(QVariantList classes
             READ classes
             NOTIFY layoutDeclared)
  // clang-format on

signals:
  void countChanged();
  void layoutDeclared();

public:
  enum Column {
    StartColumn = 0,
    EndColumn,
    LengthColumn,
    RowColumn,
    ClassColumn,
    TextColumn,
    ColumnCount
  };
  Q_ENUM(Column)

  enum Roles {
    StartRole = Qt::UserRole + 1,
    EndRole,
    LengthRole,
    RowRole,
    RowNameRole,
    ClassRole,
    ClassNameRole,
    ClassColorRole,
    TextRole,
    ShortTextRole
  };

  static constexpr int kMaxAnnotations         = 65536;
  static constexpr int kMaxTexts               = 4096;
  static constexpr qsizetype kMaxRetainedBytes = 1 << 20;
  static constexpr int kMaxRows                = 16;
  static constexpr int kMaxClasses             = 64;

  explicit AnnotationModel(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QVariant headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int capacity() const noexcept;
  [[nodiscard]] qint64 labelledEnd() const noexcept;
  [[nodiscard]] qint64 retainedEnd() const noexcept;
  [[nodiscard]] qint64 labelledStart() const noexcept;
  [[nodiscard]] qint64 retainedStart() const noexcept;
  [[nodiscard]] QStringList rowNames() const;
  [[nodiscard]] QVariantList classes() const;
  [[nodiscard]] int textCount() const noexcept;
  [[nodiscard]] const Annotation& at(int index) const;
  [[nodiscard]] QString text(int index, int level) const;

  [[nodiscard]] Q_INVOKABLE QVariantList
  trackSpans(qint64 from, qint64 to, int row, int maxSpans, qint64 minSpanBytes = 1) const;
  [[nodiscard]] Q_INVOKABLE QVariantMap trackStrip(
    qint64 from, qint64 to, int row, int maxSpans, qint64 minSpanBytes, qreal pixels) const;
  [[nodiscard]] Q_INVOKABLE QByteArray payloadBytes(int cls, int maxBytes) const;
  [[nodiscard]] Q_INVOKABLE QString payloadHex(int cls, int maxBytes) const;
  [[nodiscard]] Q_INVOKABLE QString payloadText(int cls, int maxBytes) const;
  [[nodiscard]] Q_INVOKABLE bool exportCsv(const QString& path) const;

public slots:
  void reset();
  void commitPending();
  void ingestBytes(const QByteArray& bytes);
  void declareLayout(const QStringList& rows, const QVariantList& classSpecs);
  void annotate(qint64 start, qint64 end, int row, int cls, const QStringList& texts);

private:
  struct ClassSpec {
    QString name;
    QColor color;
  };

  /**
   * @brief One drawn mark: a run of annotations of the same class that the caller's scale cannot
   *        tell apart, plus the index of the first one (its texts label the run).
   */
  struct SpanRun {
    qint64 start;
    qint64 end;
    int cls;
    int count;
    int firstIndex;
  };

  static constexpr int kLabelledSpanBudget = 256;

  void dropOldest(int count);
  void trimToRetainedBytes();
  [[nodiscard]] int internText(const QString& text);
  [[nodiscard]] std::size_t windowFirstIndex(qint64 from) const;
  [[nodiscard]] std::vector<SpanRun> collectRuns(
    qint64 from, qint64 to, int row, int maxSpans, qint64 minSpanBytes) const;

private:
  bool m_countDirty;
  bool m_sortedByStart;
  qint64 m_bytesStart;
  qint64 m_bytesEnd;
  QByteArray m_bytes;
  QStringList m_texts;
  QStringList m_rowNames;
  QHash<QString, int> m_textIds;
  std::deque<Annotation> m_items;
  std::vector<ClassSpec> m_classes;
  std::vector<Annotation> m_pending;
};

/**
 * @brief Row/class filter over an AnnotationModel for the tabular view (-1 = any).
 */
class AnnotationFilter : public QSortFilterProxyModel {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int rowFilter
             READ rowFilter
             WRITE setRowFilter
             NOTIFY filterChanged)
  Q_PROPERTY(int classFilter
             READ classFilter
             WRITE setClassFilter
             NOTIFY filterChanged)
  // clang-format on

signals:
  void filterChanged();

public:
  explicit AnnotationFilter(QObject* parent = nullptr);

  [[nodiscard]] int rowFilter() const noexcept;
  [[nodiscard]] int classFilter() const noexcept;

public slots:
  void setRowFilter(int row);
  void setClassFilter(int cls);

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex& parent) const override;

private:
  int m_rowFilter;
  int m_classFilter;
};

/**
 * @brief Runs a user JavaScript decoder over the Console's raw byte stream at chunk cadence: the
 *        script defines a global `decoder` object with `rows`, `classes` and
 *        `decode(bytes, offset, ctx)` (returns the bytes consumed; unconsumed bytes carry over,
 *        bounded). Errors disable the decoder until re-applied; every call runs under a watchdog.
 */
class AnnotationDecoder : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool enabled
             READ enabled
             WRITE setEnabled
             NOTIFY stateChanged)
  Q_PROPERTY(bool compiled
             READ compiled
             NOTIFY stateChanged)
  Q_PROPERTY(bool failed
             READ failed
             NOTIFY stateChanged)
  Q_PROPERTY(QString code
             READ code
             NOTIFY stateChanged)
  Q_PROPERTY(QString lastError
             READ lastError
             NOTIFY stateChanged)
  Q_PROPERTY(quint64 errorCount
             READ errorCount
             NOTIFY stateChanged)
  // clang-format on

signals:
  void stateChanged();

public:
  static constexpr int kWatchdogMs     = 200;
  static constexpr qsizetype kMaxCarry = 4096;

  explicit AnnotationDecoder(AnnotationModel* model, QObject* parent = nullptr);
  ~AnnotationDecoder() override;

  [[nodiscard]] Q_INVOKABLE QVariantList templates() const;
  [[nodiscard]] Q_INVOKABLE QString templateCode(const QString& file) const;

  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool compiled() const noexcept;
  [[nodiscard]] bool failed() const noexcept;
  [[nodiscard]] const QString& code() const noexcept;
  [[nodiscard]] const QString& lastError() const noexcept;
  [[nodiscard]] quint64 errorCount() const noexcept;

public slots:
  void setCode(const QString& code);
  void setEnabled(bool enabled);
  void feed(const QByteArray& bytes);
  void reset();

private:
  [[nodiscard]] bool compile(QString& error);
  [[nodiscard]] bool readLayout(QString& error);
  void fail(const QString& error);
  void teardownEngine();

private:
  AnnotationModel* m_model;
  std::unique_ptr<QJSEngine> m_engine;
  std::unique_ptr<DataModel::JsWatchdog> m_watchdog;
  QJSValue m_decodeFn;
  QJSValue m_context;
  QString m_code;
  QString m_lastError;
  QByteArray m_carry;
  qint64 m_carryOffset;
  quint64 m_errorCount;
  bool m_enabled;
  bool m_compiled;
  bool m_failed;
  bool m_inFeed;
};

}  // namespace Console
