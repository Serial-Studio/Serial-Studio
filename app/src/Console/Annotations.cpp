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

#include "Console/Annotations.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QFile>
#include <QTextStream>
#include <QVariantMap>

#include "DataModel/Scripting/ScriptTemplates.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Interned id 0 is the overflow placeholder every text past kMaxTexts collapses onto
static constexpr int kOverflowTextId = 0;

//--------------------------------------------------------------------------------------------------
// AnnotationModel: construction & Qt model contract
//--------------------------------------------------------------------------------------------------

/**
 * @brief Empty store; the overflow placeholder is interned first so its id is always 0.
 */
Console::AnnotationModel::AnnotationModel(QObject* parent)
  : QAbstractTableModel(parent)
  , m_countDirty(false)
  , m_sortedByStart(true)
  , m_bytesStart(0)
  , m_bytesEnd(0)
  , m_bytes()
  , m_texts()
  , m_rowNames()
  , m_textIds()
  , m_items()
  , m_classes()
  , m_pending()
{
  m_pending.reserve(1024);
  m_texts.append(QStringLiteral("..."));
  m_textIds.insert(m_texts.first(), kOverflowTextId);
}

/**
 * @brief One table row per retained annotation.
 */
int Console::AnnotationModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_items.size());
}

/**
 * @brief Start / End / Length / Row / Class / Text.
 */
int Console::AnnotationModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : ColumnCount;
}

/**
 * @brief Column values for the table view plus named roles for list delegates.
 */
QVariant Console::AnnotationModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_items.size()))
    return {};

  const Annotation& a = m_items[static_cast<std::size_t>(index.row())];
  const int cls       = a.cls;
  const bool clsOk    = cls >= 0 && static_cast<std::size_t>(cls) < m_classes.size();
  const bool rowOk    = a.row >= 0 && a.row < m_rowNames.size();

  switch (role) {
    case StartRole:
      return a.start;
    case EndRole:
      return a.end;
    case LengthRole:
      return a.end - a.start + 1;
    case RowRole:
      return a.row;
    case RowNameRole:
      return rowOk ? m_rowNames.at(a.row) : QString::number(a.row);
    case ClassRole:
      return cls;
    case ClassNameRole:
      return clsOk ? m_classes[static_cast<std::size_t>(cls)].name : QString::number(cls);
    case ClassColorRole:
      return clsOk ? m_classes[static_cast<std::size_t>(cls)].color : QColor(Qt::gray);
    case TextRole:
      return text(index.row(), 0);
    case ShortTextRole:
      return text(index.row(), Annotation::kTextLevels - 1);
    default:
      break;
  }

  if (role != Qt::DisplayRole)
    return {};

  switch (index.column()) {
    case StartColumn:
      return a.start;
    case EndColumn:
      return a.end;
    case LengthColumn:
      return a.end - a.start + 1;
    case RowColumn:
      return rowOk ? m_rowNames.at(a.row) : QString::number(a.row);
    case ClassColumn:
      return clsOk ? m_classes[static_cast<std::size_t>(cls)].name : QString::number(cls);
    case TextColumn:
      return text(index.row(), 0);
    default:
      return {};
  }
}

/**
 * @brief Column titles.
 */
QVariant Console::AnnotationModel::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return {};

  switch (section) {
    case StartColumn:
      return tr("Start");
    case EndColumn:
      return tr("End");
    case LengthColumn:
      return tr("Length");
    case RowColumn:
      return tr("Row");
    case ClassColumn:
      return tr("Class");
    case TextColumn:
      return tr("Text");
    default:
      return {};
  }
}

/**
 * @brief Named roles for QML delegates.
 */
QHash<int, QByteArray> Console::AnnotationModel::roleNames() const
{
  return {
    {Qt::DisplayRole,    "display"},
    {      StartRole,      "start"},
    {        EndRole,        "end"},
    {     LengthRole,     "length"},
    {        RowRole,        "row"},
    {    RowNameRole,    "rowName"},
    {      ClassRole,        "cls"},
    {  ClassNameRole,  "className"},
    { ClassColorRole, "classColor"},
    {       TextRole,       "text"},
    {  ShortTextRole,  "shortText"},
  };
}

//--------------------------------------------------------------------------------------------------
// AnnotationModel: queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retained annotations.
 */
int Console::AnnotationModel::count() const noexcept
{
  return static_cast<int>(m_items.size());
}

/**
 * @brief Absolute offset of the oldest retained byte.
 */
qint64 Console::AnnotationModel::retainedStart() const noexcept
{
  return m_bytesStart;
}

/**
 * @brief Absolute offset one past the newest ingested byte.
 */
qint64 Console::AnnotationModel::retainedEnd() const noexcept
{
  return m_bytesEnd;
}

/**
 * @brief Annotations the store holds before the oldest are dropped.
 */
int Console::AnnotationModel::capacity() const noexcept
{
  return kMaxAnnotations;
}

/**
 * @brief Absolute offset of the oldest retained annotation, -1 when the store is empty. Read
 *        against retainedStart()/retainedEnd() it says whether the decoder is labelling the same
 *        region of the stream the strip is drawing.
 */
qint64 Console::AnnotationModel::labelledStart() const noexcept
{
  return m_items.empty() ? -1 : m_items.front().start;
}

/**
 * @brief Absolute end offset of the newest retained annotation, -1 when the store is empty.
 */
qint64 Console::AnnotationModel::labelledEnd() const noexcept
{
  return m_items.empty() ? -1 : m_items.back().end;
}

/**
 * @brief Decoder-declared row names.
 */
QStringList Console::AnnotationModel::rowNames() const
{
  return m_rowNames;
}

/**
 * @brief Declared classes as `[{name, color}]` for QML pickers and legends.
 */
QVariantList Console::AnnotationModel::classes() const
{
  QVariantList out;
  out.reserve(static_cast<qsizetype>(m_classes.size()));
  for (const auto& spec : m_classes) {
    QVariantMap entry;
    entry.insert(QStringLiteral("name"), spec.name);
    entry.insert(QStringLiteral("color"), spec.color);
    out.append(entry);
  }

  return out;
}

/**
 * @brief Interned strings, overflow placeholder included.
 */
int Console::AnnotationModel::textCount() const noexcept
{
  return static_cast<int>(m_texts.size());
}

/**
 * @brief Retained annotation @p index (callers bound-check through count()).
 */
const Console::Annotation& Console::AnnotationModel::at(int index) const
{
  static const Annotation kEmpty{
    0, 0, 0, 0, {kOverflowTextId, -1, -1}
  };
  if (index < 0 || static_cast<std::size_t>(index) >= m_items.size())
    return kEmpty;

  return m_items[static_cast<std::size_t>(index)];
}

/**
 * @brief Text rendering @p level (0 longest) of annotation @p index, falling back to the nearest
 *        longer rendering the decoder supplied.
 */
QString Console::AnnotationModel::text(int index, int level) const
{
  const Annotation& a = at(index);
  const int clamped   = std::clamp(level, 0, Annotation::kTextLevels - 1);
  for (int l = clamped; l >= 0; --l) {
    const int id = a.texts[l];
    if (id >= 0 && id < m_texts.size())
      return m_texts.at(id);
  }

  return m_texts.first();
}

/**
 * @brief Index of the first annotation that can still overlap @p from: decoders normally emit in
 *        stream order, so the strip skips the history instead of walking it every UI tick; an
 *        out-of-order decoder clears the flag and the scan falls back to the whole store.
 */
std::size_t Console::AnnotationModel::windowFirstIndex(qint64 from) const
{
  if (!m_sortedByStart)
    return 0;

  const auto it =
    std::lower_bound(m_items.begin(), m_items.end(), from, [](const Annotation& a, qint64 value) {
      return a.start < value;
    });

  std::size_t first = static_cast<std::size_t>(std::distance(m_items.begin(), it));
  while (first > 0 && m_items[first - 1].end >= from)
    --first;

  return first;
}

/**
 * @brief Runs of @p row overlapping [from, to], merging neighbours of one class that @p
 *        minSpanBytes cannot separate. Merging is capped at one cluster of that size (a pixel at
 *        the caller's scale), so a wide window decimates into per-pixel marks that still show
 *        density instead of collapsing into one blob per lane.
 */
std::vector<Console::AnnotationModel::SpanRun> Console::AnnotationModel::collectRuns(
  qint64 from, qint64 to, int row, int maxSpans, qint64 minSpanBytes) const
{
  std::vector<SpanRun> runs;
  if (to < from || maxSpans <= 0)
    return runs;

  const qint64 gap = std::max<qint64>(1, minSpanBytes);
  const int limit  = std::min(maxSpans, kMaxAnnotations);
  runs.reserve(static_cast<std::size_t>(std::min(limit, 4096)));

  for (std::size_t i = windowFirstIndex(from);
       i < m_items.size() && runs.size() < static_cast<std::size_t>(limit);
       ++i) {
    const Annotation& a = m_items[i];
    if (a.start > to && m_sortedByStart)
      break;

    if (a.row != row || a.end < from || a.start > to)
      continue;

    if (!runs.empty()) {
      SpanRun& last       = runs.back();
      const bool subPixel = (a.end - a.start + 1) <= gap && (a.start - last.end) <= gap;
      const bool inPixel  = (a.end - last.start + 1) <= gap;
      if (a.cls == last.cls && subPixel && inPixel) {
        last.end = a.end;
        ++last.count;
        continue;
      }
    }

    runs.push_back(SpanRun{a.start, a.end, a.cls, 1, static_cast<int>(i)});
  }

  return runs;
}

/**
 * @brief The runs of @p row as `{start, end, cls, count, color, text, shortText}` maps; the
 *        readable form, used for hover detail rather than per-tick painting.
 */
QVariantList Console::AnnotationModel::trackSpans(
  qint64 from, qint64 to, int row, int maxSpans, qint64 minSpanBytes) const
{
  QVariantList out;
  const auto runs = collectRuns(from, to, row, maxSpans, minSpanBytes);
  out.reserve(static_cast<qsizetype>(runs.size()));

  for (const SpanRun& run : runs) {
    const bool clsOk = run.cls >= 0 && static_cast<std::size_t>(run.cls) < m_classes.size();
    QVariantMap span;
    span.insert(QStringLiteral("start"), run.start);
    span.insert(QStringLiteral("end"), run.end);
    span.insert(QStringLiteral("cls"), run.cls);
    span.insert(QStringLiteral("count"), run.count);
    span.insert(QStringLiteral("color"),
                clsOk ? m_classes[static_cast<std::size_t>(run.cls)].color : QColor(Qt::gray));
    span.insert(QStringLiteral("text"), text(run.firstIndex, 0));
    span.insert(QStringLiteral("shortText"), text(run.firstIndex, Annotation::kTextLevels - 1));
    out.append(span);
  }

  return out;
}

/**
 * @brief The runs of @p row as flat pixel geometry for a canvas: `{geometry, labels, shortLabels,
 *        classes, count}`, where geometry packs (x, width, start, end, class, merged) per run.
 *        One typed array instead of a map and a scene-graph item per mark, which is what the strip
 *        cost before; texts come along only while there are few enough marks to read them.
 */
QVariantMap Console::AnnotationModel::trackStrip(
  qint64 from, qint64 to, int row, int maxSpans, qint64 minSpanBytes, qreal pixels) const
{
  QVariantMap out;
  const auto runs   = collectRuns(from, to, row, maxSpans, minSpanBytes);
  const qreal width = std::max<qreal>(1, pixels);
  const qreal span  = static_cast<qreal>(std::max<qint64>(1, to - from));

  QList<qreal> geometry;
  geometry.reserve(static_cast<qsizetype>(runs.size()) * 6);
  for (const SpanRun& run : runs) {
    const qreal x0 = std::max<qreal>(0, (run.start - from) / span * width);
    const qreal x1 = std::min<qreal>(width, (run.end + 1 - from) / span * width);
    geometry << x0 << std::max<qreal>(1, x1 - x0) << static_cast<qreal>(run.start)
             << static_cast<qreal>(run.end) << static_cast<qreal>(run.cls)
             << static_cast<qreal>(run.count);
  }

  QStringList labels;
  QStringList shortLabels;
  if (runs.size() <= static_cast<std::size_t>(kLabelledSpanBudget)) {
    labels.reserve(static_cast<qsizetype>(runs.size()));
    shortLabels.reserve(static_cast<qsizetype>(runs.size()));
    for (const SpanRun& run : runs) {
      labels << text(run.firstIndex, 0);
      shortLabels << text(run.firstIndex, Annotation::kTextLevels - 1);
    }
  }

  out.insert(QStringLiteral("count"), static_cast<int>(runs.size()));
  out.insert(QStringLiteral("geometry"), QVariant::fromValue(geometry));
  out.insert(QStringLiteral("labels"), labels);
  out.insert(QStringLiteral("shortLabels"), shortLabels);
  return out;
}

/**
 * @brief Concatenated bytes of every retained annotation of class @p cls, in stream order, up
 *        to @p maxBytes; ranges that fell out of the retained byte window are skipped.
 */
QByteArray Console::AnnotationModel::payloadBytes(int cls, int maxBytes) const
{
  QByteArray out;
  if (maxBytes <= 0)
    return out;

  for (const Annotation& a : m_items) {
    if (a.cls != cls || a.start < m_bytesStart || a.end >= m_bytesEnd || a.end < a.start)
      continue;

    const qsizetype offset = static_cast<qsizetype>(a.start - m_bytesStart);
    const qsizetype length = static_cast<qsizetype>(a.end - a.start + 1);
    if (offset < 0 || offset + length > m_bytes.size())
      continue;

    const qsizetype room = static_cast<qsizetype>(maxBytes) - out.size();
    if (room <= 0)
      break;

    out.append(m_bytes.constData() + offset, std::min(length, room));
  }

  return out;
}

/**
 * @brief payloadBytes() as spaced upper-case hex.
 */
QString Console::AnnotationModel::payloadHex(int cls, int maxBytes) const
{
  return QString::fromLatin1(payloadBytes(cls, maxBytes).toHex(' ').toUpper());
}

/**
 * @brief payloadBytes() decoded as UTF-8 (invalid sequences replaced).
 */
QString Console::AnnotationModel::payloadText(int cls, int maxBytes) const
{
  return QString::fromUtf8(payloadBytes(cls, maxBytes));
}

/**
 * @brief Writes every retained annotation as CSV (start, end, length, row, class, text). The
 *        stream is flushed before its status is read: QTextStream buffers, so the final chunk
 *        would otherwise land during destruction and a device error on it would go unreported.
 */
bool Console::AnnotationModel::exportCsv(const QString& path) const
{
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    return false;

  const auto quote = [](const QString& value) {
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
  };

  QTextStream out(&file);
  out << "start,end,length,row,class,text\n";
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    const Annotation& a   = m_items[i];
    const QModelIndex idx = index(static_cast<int>(i), 0);
    out << a.start << ',' << a.end << ',' << (a.end - a.start + 1) << ','
        << quote(data(idx, RowNameRole).toString()) << ','
        << quote(data(idx, ClassNameRole).toString()) << ',' << quote(text(static_cast<int>(i), 0))
        << '\n';
  }

  out.flush();
  return out.status() == QTextStream::Ok && file.error() == QFileDevice::NoError;
}

//--------------------------------------------------------------------------------------------------
// AnnotationModel: mutation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts a decoder's rows and classes (`[{name, color}]` or plain names), clamped to the
 *        table bounds, and drops annotations recorded under the previous layout.
 */
void Console::AnnotationModel::declareLayout(const QStringList& rows,
                                             const QVariantList& classSpecs)
{
  reset();

  m_rowNames = rows.mid(0, kMaxRows);
  if (m_rowNames.isEmpty())
    m_rowNames.append(tr("Annotations"));

  m_classes.clear();
  static const QColor kPalette[] = {
    QColor(0x4e, 0x79, 0xa7),
    QColor(0xf2, 0x8e, 0x2b),
    QColor(0xe1, 0x57, 0x59),
    QColor(0x76, 0xb7, 0xb2),
    QColor(0x59, 0xa1, 0x4f),
    QColor(0xed, 0xc9, 0x48),
    QColor(0xb0, 0x7a, 0xa1),
    QColor(0xff, 0x9d, 0xa7),
  };
  static constexpr int kPaletteSize = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));

  for (int i = 0; i < classSpecs.size() && i < kMaxClasses; ++i) {
    const QVariant& spec = classSpecs.at(i);
    ClassSpec entry;
    entry.color = kPalette[i % kPaletteSize];
    if (spec.canConvert<QVariantMap>()) {
      const auto map = spec.toMap();
      entry.name     = map.value(QStringLiteral("name")).toString();
      const QColor c = QColor::fromString(map.value(QStringLiteral("color")).toString());
      if (c.isValid())
        entry.color = c;
    } else {
      entry.name = spec.toString();
    }

    if (entry.name.isEmpty())
      entry.name = tr("class %1").arg(i);

    m_classes.push_back(entry);
  }

  if (m_classes.empty())
    m_classes.push_back(ClassSpec{tr("data"), kPalette[0]});

  Q_EMIT layoutDeclared();
}

/**
 * @brief Records one annotation from the decoder (JS-callable): a start..end byte range under a
 *        declared row and class with longest-to-shortest texts. Undeclared rows/classes and
 *        malformed ranges are ignored. The record only stages here; commitPending() publishes it,
 *        so a decoder emitting thousands per second costs one model transaction per UI tick.
 */
void Console::AnnotationModel::annotate(
  qint64 start, qint64 end, int row, int cls, const QStringList& texts)
{
  if (end < start || start < 0 || row < 0 || row >= m_rowNames.size() || cls < 0
      || static_cast<std::size_t>(cls) >= m_classes.size())
    return;

  Annotation a;
  a.start = start;
  a.end   = end;
  a.row   = row;
  a.cls   = cls;
  for (int l = 0; l < Annotation::kTextLevels; ++l)
    a.texts[l] = (l < texts.size()) ? internText(texts.at(l)) : -1;

  if (texts.isEmpty())
    a.texts[0] = kOverflowTextId;

  if (m_pending.size() >= static_cast<std::size_t>(kMaxAnnotations))
    m_pending.erase(m_pending.begin(), m_pending.begin() + kMaxAnnotations / 4);

  m_pending.push_back(a);
}

/**
 * @brief Publishes everything staged since the last call as one insertion, trimming the oldest
 *        records when the store would pass its cap. Called at UI cadence: the table, the track
 *        strip and the counters all move in one step instead of per annotation.
 */
void Console::AnnotationModel::commitPending()
{
  if (!m_pending.empty()) {
    const int staged   = static_cast<int>(m_pending.size());
    const int overflow = static_cast<int>(m_items.size()) + staged - kMaxAnnotations;
    if (overflow > 0)
      dropOldest(overflow);

    const int first = static_cast<int>(m_items.size());
    beginInsertRows(QModelIndex(), first, first + staged - 1);
    for (const Annotation& a : m_pending) {
      if (!m_items.empty() && a.start < m_items.back().start)
        m_sortedByStart = false;

      m_items.push_back(a);
    }

    endInsertRows();
    m_pending.clear();
    m_countDirty = true;
  }

  if (m_countDirty) {
    m_countDirty = false;
    Q_EMIT countChanged();
  }
}

/**
 * @brief Appends raw stream bytes to the bounded retained window (payload extraction reads it)
 *        and advances the absolute offset counter; annotations older than the window drop. The
 *        count signal is deferred to commitPending() so a fast source does not re-evaluate every
 *        bound QML expression per chunk.
 */
void Console::AnnotationModel::ingestBytes(const QByteArray& bytes)
{
  if (bytes.isEmpty())
    return;

  m_bytes.append(bytes);
  m_bytesEnd += bytes.size();
  if (m_bytes.size() > kMaxRetainedBytes) {
    const qsizetype excess = m_bytes.size() - kMaxRetainedBytes;
    m_bytes.remove(0, excess);
    m_bytesStart += excess;
    trimToRetainedBytes();
  }

  m_countDirty = true;
}

/**
 * @brief Drops every annotation and retained byte; the layout stays declared.
 */
void Console::AnnotationModel::reset()
{
  beginResetModel();
  m_items.clear();
  m_pending.clear();
  m_texts = QStringList{QStringLiteral("...")};
  m_textIds.clear();
  m_textIds.insert(m_texts.first(), kOverflowTextId);
  m_bytes.clear();
  m_bytesStart    = 0;
  m_bytesEnd      = 0;
  m_countDirty    = false;
  m_sortedByStart = true;
  endResetModel();
  Q_EMIT countChanged();
}

/**
 * @brief Interns @p text: same string, same id; past kMaxTexts everything maps onto id 0.
 */
int Console::AnnotationModel::internText(const QString& text)
{
  const auto it = m_textIds.constFind(text);
  if (it != m_textIds.cend())
    return it.value();

  if (m_texts.size() >= kMaxTexts)
    return kOverflowTextId;

  const int id = m_texts.size();
  m_texts.append(text);
  m_textIds.insert(text, id);
  return id;
}

/**
 * @brief Removes annotations that end before the retained byte window (they can no longer be
 *        extracted). The prefix walk stops at the first survivor only while the store is known
 *        sorted by start; commitPending clears that flag for a decoder that annotates backwards,
 *        and an out-of-order item behind a survivor would otherwise never be trimmed.
 */
void Console::AnnotationModel::trimToRetainedBytes()
{
  int drop = 0;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    if (m_items[i].end >= m_bytesStart) {
      if (m_sortedByStart)
        break;

      continue;
    }

    if (drop < static_cast<int>(i))
      break;

    ++drop;
  }

  if (drop > 0)
    dropOldest(drop);
}

/**
 * @brief Removes the oldest @p count annotations as one model operation; the store is a deque so
 *        trimming costs the dropped records, not a copy of everything that survives.
 */
void Console::AnnotationModel::dropOldest(int count)
{
  const int available = static_cast<int>(m_items.size());
  const int n         = std::clamp(count, 0, available);
  if (n == 0)
    return;

  beginRemoveRows(QModelIndex(), 0, n - 1);
  m_items.erase(m_items.begin(), m_items.begin() + n);
  endRemoveRows();
}

//--------------------------------------------------------------------------------------------------
// AnnotationFilter
//--------------------------------------------------------------------------------------------------

/**
 * @brief Unfiltered proxy; filters are -1 (any) until set.
 */
Console::AnnotationFilter::AnnotationFilter(QObject* parent)
  : QSortFilterProxyModel(parent), m_rowFilter(-1), m_classFilter(-1)
{
  setDynamicSortFilter(true);
}

/**
 * @brief Tracks the source model so a re-declared layout can clear the filters. Without this the
 *        proxy keeps filtering on an index the new layout may not have, emptying the table while
 *        the combos - rebuilt from the new lists - read "All rows" and "All classes".
 */
void Console::AnnotationFilter::setSourceModel(QAbstractItemModel* source)
{
  QSortFilterProxyModel::setSourceModel(source);

  auto* model = qobject_cast<AnnotationModel*>(source);
  if (!model)
    return;

  connect(model, &AnnotationModel::layoutDeclared, this, [this] {
    setRowFilter(-1);
    setClassFilter(-1);
  });
}

/**
 * @brief Row filter, -1 = any.
 */
int Console::AnnotationFilter::rowFilter() const noexcept
{
  return m_rowFilter;
}

/**
 * @brief Class filter, -1 = any.
 */
int Console::AnnotationFilter::classFilter() const noexcept
{
  return m_classFilter;
}

/**
 * @brief Keeps only annotations of @p row (-1 = any).
 */
void Console::AnnotationFilter::setRowFilter(int row)
{
  if (m_rowFilter == row)
    return;

  beginFilterChange();
  m_rowFilter = row;
  endFilterChange(QSortFilterProxyModel::Direction::Rows);
  Q_EMIT filterChanged();
}

/**
 * @brief Keeps only annotations of class @p cls (-1 = any).
 */
void Console::AnnotationFilter::setClassFilter(int cls)
{
  if (m_classFilter == cls)
    return;

  beginFilterChange();
  m_classFilter = cls;
  endFilterChange(QSortFilterProxyModel::Direction::Rows);
  Q_EMIT filterChanged();
}

/**
 * @brief Row and class filters, both optional.
 */
bool Console::AnnotationFilter::filterAcceptsRow(int sourceRow, const QModelIndex& parent) const
{
  const auto* source = sourceModel();
  if (!source)
    return false;

  const QModelIndex idx = source->index(sourceRow, 0, parent);
  if (m_rowFilter >= 0 && source->data(idx, AnnotationModel::RowRole).toInt() != m_rowFilter)
    return false;

  return m_classFilter < 0
      || source->data(idx, AnnotationModel::ClassRole).toInt() == m_classFilter;
}

//--------------------------------------------------------------------------------------------------
// AnnotationDecoder
//--------------------------------------------------------------------------------------------------

/**
 * @brief Idle decoder bound to @p model; nothing runs until setCode() + setEnabled().
 */
Console::AnnotationDecoder::AnnotationDecoder(AnnotationModel* model, QObject* parent)
  : QObject(parent)
  , m_model(model)
  , m_engine()
  , m_watchdog()
  , m_viewers()
  , m_decodeFn()
  , m_context()
  , m_code()
  , m_lastError()
  , m_carry()
  , m_carryOffset(0)
  , m_errorCount(0)
  , m_enabled(false)
  , m_compiled(false)
  , m_failed(false)
  , m_inFeed(false)
{
  SS_ASSERT_LOG(model != nullptr);
}

/**
 * @brief Releases the engine and watchdog.
 */
Console::AnnotationDecoder::~AnnotationDecoder()
{
  teardownEngine();
}

/**
 * @brief The bundled decoder templates as `[{file, name, default}]`, in manifest order.
 */
QVariantList Console::AnnotationDecoder::templates() const
{
  QVariantList out;
  const auto definitions = DataModel::loadScriptTemplateManifest(
    QStringLiteral(":/scripts/annotations/templates.json"), "AnnotationDecoder");

  out.reserve(definitions.size());
  for (const auto& definition : definitions) {
    QVariantMap entry;
    entry.insert(QStringLiteral("file"), definition.file);
    entry.insert(QStringLiteral("name"), definition.name);
    entry.insert(QStringLiteral("default"), definition.isDefault);
    out.append(entry);
  }

  return out;
}

/**
 * @brief Source of the named template, empty when the resource is missing.
 */
QString Console::AnnotationDecoder::templateCode(const QString& file) const
{
  return DataModel::readTextResource(DataModel::templateResourcePath(
    QStringLiteral(":/scripts/annotations"), file, QStringLiteral(".js")));
}

/**
 * @brief True while at least one annotation view is on screen. Decoding is gated on it: the
 *        panel is the only consumer of the store, so a closed one earns the console stream a
 *        free ride instead of a retained-window copy plus a decode() call per chunk.
 */
bool Console::AnnotationDecoder::active() const noexcept
{
  return !m_viewers.isEmpty();
}

/**
 * @brief True while chunks are decoded.
 */
bool Console::AnnotationDecoder::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief True when the script compiled and declared a layout.
 */
bool Console::AnnotationDecoder::compiled() const noexcept
{
  return m_compiled;
}

/**
 * @brief True after a compile or runtime failure until the script is re-applied.
 */
bool Console::AnnotationDecoder::failed() const noexcept
{
  return m_failed;
}

/**
 * @brief The script text last applied.
 */
const QString& Console::AnnotationDecoder::code() const noexcept
{
  return m_code;
}

/**
 * @brief Last compile / runtime error message.
 */
const QString& Console::AnnotationDecoder::lastError() const noexcept
{
  return m_lastError;
}

/**
 * @brief Failures since construction.
 */
quint64 Console::AnnotationDecoder::errorCount() const noexcept
{
  return m_errorCount;
}

/**
 * @brief Compiles a new decoder script; a failed compile leaves the decoder disabled with the
 *        error exposed. An empty script tears everything down.
 */
void Console::AnnotationDecoder::setCode(const QString& code)
{
  m_code = code;
  teardownEngine();
  m_failed = false;
  m_lastError.clear();

  if (m_code.trimmed().isEmpty()) {
    m_compiled = false;
    m_enabled  = false;
    Q_EMIT stateChanged();
    return;
  }

  QString error;
  m_compiled = compile(error);
  if (!m_compiled)
    fail(error);
  else
    Q_EMIT stateChanged();
}

/**
 * @brief Arms or disarms decoding; enabling a failed decoder retries the compile.
 */
void Console::AnnotationDecoder::setEnabled(bool enabled)
{
  if (enabled && !m_compiled)
    setCode(m_code);

  const bool next = enabled && m_compiled;
  if (m_enabled == next)
    return;

  m_enabled = next;
  Q_EMIT stateChanged();
}

/**
 * @brief Registers or drops @p viewer as an on-screen annotation view; decoding runs only while
 *        one is registered, and each is guarded by destroyed() so a torn-down console window
 *        cannot wedge the gate open. Resuming drops the carry: its bytes predate the pause, and
 *        splicing them onto what arrives after hands decode() a frame never seen on the wire.
 */
void Console::AnnotationDecoder::setViewerActive(QObject* viewer, bool active)
{
  SS_ASSERT(viewer != nullptr, return);

  if (active == m_viewers.contains(viewer))
    return;

  if (!active) {
    QObject::disconnect(m_viewers.take(viewer));
    Q_EMIT stateChanged();
    return;
  }

  if (m_viewers.isEmpty()) {
    m_carry.clear();
    m_carryOffset = m_model ? m_model->retainedEnd() : 0;
  }

  const auto guard = connect(viewer, &QObject::destroyed, this, [this, viewer]() {
    m_viewers.remove(viewer);
    Q_EMIT stateChanged();
  });

  m_viewers.insert(viewer, guard);
  Q_EMIT stateChanged();
}

/**
 * @brief Chunk-rate entry. The carry is appended to and trimmed in place rather than regrown,
 *        but a chunk still costs one QV4 ArrayBuffer that shares it: measure before calling this
 *        allocation-free. size is explicit because bytes.length costs a string-to-index
 *        conversion per script-loop iteration; the QJSValues release before fail() frees engine.
 */
void Console::AnnotationDecoder::feed(const QByteArray& bytes)
{
  SS_ASSERT(m_model != nullptr, return);

  if (bytes.isEmpty() || m_viewers.isEmpty())
    return;

  m_model->ingestBytes(bytes);

  if (!m_enabled || !m_compiled || m_inFeed || !m_engine || !m_watchdog)
    return;

  m_inFeed = true;

  m_carry.append(bytes);
  const qint64 offset  = m_carryOffset;
  const qsizetype size = m_carry.size();

  bool timedOut = false;
  bool hadError = false;
  QString errorMessage;
  qint64 consumed = size;

  {
    QJSValueList args;
    args << m_engine->toScriptValue(m_carry) << QJSValue(static_cast<double>(offset)) << m_context
         << QJSValue(static_cast<double>(size));

    const QJSValue result = m_watchdog->call(m_decodeFn, args);
    timedOut              = m_watchdog->lastCallTimedOut();
    hadError              = !timedOut && result.isError();

    if (hadError)
      errorMessage = result.property(QStringLiteral("message")).toString();

    else if (!timedOut && result.isNumber()) {
      const double returned = result.toNumber();
      SS_ASSERT_LOG(std::isfinite(returned));
      if (std::isfinite(returned))
        consumed = static_cast<qint64>(returned);
    }
  }

  if (timedOut || hadError) {
    fail(timedOut ? tr("decode() exceeded %1 ms").arg(kWatchdogMs) : errorMessage);
    m_inFeed = false;
    return;
  }

  consumed = std::clamp<qint64>(consumed, 0, size);
  m_carry.remove(0, static_cast<qsizetype>(consumed));

  if (m_carry.size() > kMaxCarry) {
    const qsizetype dropped = m_carry.size() - kMaxCarry;
    m_carry.remove(0, dropped);
    consumed += dropped;
  }

  m_carryOffset = offset + consumed;
  m_inFeed      = false;
}

/**
 * @brief Drops the carry-over and the model's annotations (a reconnect or clear). The store is
 *        emptied BEFORE the offset is re-read: reading first left the decoder annotating at the
 *        old stream position while the byte window restarted at zero, so every later annotation
 *        landed past the window, escaped trimming and never reached the track strip.
 */
void Console::AnnotationDecoder::reset()
{
  m_carry.clear();
  if (m_model)
    m_model->reset();

  m_carryOffset = m_model ? m_model->retainedEnd() : 0;
}

/**
 * @brief Evaluates the script in a fresh engine and binds `decoder.decode`, `decoder.rows`,
 *        `decoder.classes`; the model is exposed as the `ctx` argument (annotate()).
 */
bool Console::AnnotationDecoder::compile(QString& error)
{
  m_engine   = std::make_unique<QJSEngine>();
  m_watchdog = std::make_unique<DataModel::JsWatchdog>(
    m_engine.get(), kWatchdogMs, QStringLiteral("annotation decoder"));

  const QJSValue evaluated = m_engine->evaluate(m_code);
  if (evaluated.isError()) {
    error = tr("Line %1: %2")
              .arg(evaluated.property(QStringLiteral("lineNumber")).toInt())
              .arg(evaluated.property(QStringLiteral("message")).toString());
    return false;
  }

  const QJSValue decoder = m_engine->globalObject().property(QStringLiteral("decoder"));
  if (!decoder.isObject()) {
    error = tr("the script must define a global 'decoder' object");
    return false;
  }

  m_decodeFn = decoder.property(QStringLiteral("decode"));
  if (!m_decodeFn.isCallable()) {
    error = tr("'decoder.decode(bytes, offset, ctx, size)' is not a function");
    return false;
  }

  if (!readLayout(error))
    return false;

  QJSEngine::setObjectOwnership(m_model, QJSEngine::CppOwnership);
  m_context = m_engine->newQObject(m_model);
  m_carry.clear();
  m_carryOffset = m_model->retainedEnd();
  return true;
}

/**
 * @brief Reads `decoder.rows` (names) and `decoder.classes` (`{name, color}` or names) into the
 *        model layout.
 */
bool Console::AnnotationDecoder::readLayout(QString& error)
{
  const QJSValue decoder = m_engine->globalObject().property(QStringLiteral("decoder"));

  QStringList rows;
  const QJSValue rowsValue = decoder.property(QStringLiteral("rows"));
  if (rowsValue.isArray()) {
    const int n = rowsValue.property(QStringLiteral("length")).toInt();
    for (int i = 0; i < n && i < AnnotationModel::kMaxRows; ++i)
      rows.append(rowsValue.property(static_cast<quint32>(i)).toString());
  }

  QVariantList classes;
  const QJSValue classValue = decoder.property(QStringLiteral("classes"));
  if (classValue.isArray()) {
    const int n = classValue.property(QStringLiteral("length")).toInt();
    for (int i = 0; i < n && i < AnnotationModel::kMaxClasses; ++i)
      classes.append(classValue.property(static_cast<quint32>(i)).toVariant());
  }

  if (rows.isEmpty() || classes.isEmpty()) {
    error = tr("'decoder.rows' and 'decoder.classes' must be non-empty arrays");
    return false;
  }

  m_model->declareLayout(rows, classes);
  return true;
}

/**
 * @brief Records a failure, counts it, and disables decoding until the script is re-applied.
 */
void Console::AnnotationDecoder::fail(const QString& error)
{
  ++m_errorCount;
  m_lastError = error.isEmpty() ? tr("unknown decoder error") : error;
  m_failed    = true;
  m_enabled   = false;
  m_compiled  = false;
  teardownEngine();
  qWarning() << "[Console] annotation decoder disabled:" << m_lastError;
  Q_EMIT stateChanged();
}

/**
 * @brief Releases the engine (watchdog first: it holds a raw engine pointer).
 */
void Console::AnnotationDecoder::teardownEngine()
{
  m_decodeFn = QJSValue();
  m_context  = QJSValue();
  m_watchdog.reset();
  m_engine.reset();
}
