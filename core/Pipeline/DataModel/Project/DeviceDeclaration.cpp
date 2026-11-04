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

#include "DataModel/Project/DeviceDeclaration.h"

#include <algorithm>
#include <iterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include "Core/DataModel/FrameKeys.h"
#include "Core/DataModel/FrameSupport.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/LineFormat.h"

namespace Declaration = DataModel::Declaration;

static_assert(Declaration::kMaxDatasets <= DataModel::kMaxLineFields,
              "every declared frame position must fit the Quick Plot channel registry");

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief Most digits a wire version may have, and most refused keys one console note names.
 */
static constexpr qsizetype kMaxVersionDigits = 4;
static constexpr qsizetype kMaxListedKeys    = 8;

/**
 * @brief Shortest time between two rebuilds from changed declarations (spec 0111 R10).
 */
static constexpr auto kRebuildInterval = std::chrono::seconds(1);

//--------------------------------------------------------------------------------------------------
// Allow-lists
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when @p key is one of the @p count keys at @p table.
 */
[[nodiscard]] static bool listedIn(const QString& key,
                                   const Keys::KeyView* table,
                                   const qsizetype count) noexcept
{
  SS_ASSERT(table != nullptr, return false);
  SS_ASSERT(count >= 0, return false);
  return std::any_of(table, table + count, [&key](const Keys::KeyView k) { return key == k; });
}

/**
 * @brief True when @p key is one of @p table's keys.
 */
template<size_t N>
[[nodiscard]] static bool listed(const QString& key, const Keys::KeyView (&table)[N]) noexcept
{
  return listedIn(key, table, static_cast<qsizetype>(N));
}

/**
 * @brief True when a declaration may name @p widget as a group widget (or names none).
 */
[[nodiscard]] static bool allowedGroupWidget(const QString& widget) noexcept
{
  const auto* first = std::begin(Declaration::kGroupWidgets);
  const auto* last  = std::end(Declaration::kGroupWidgets);
  SS_ASSERT(first != last, return false);
  return widget.isEmpty() || std::find(first, last, widget) != last;
}

/**
 * @brief The wire-included sub-entity a dataset key names, or null for a plain property key.
 */
[[nodiscard]] static const Declaration::SubEntityWire* subEntityFor(const QString& key) noexcept
{
  SS_ASSERT(std::size(Declaration::kSubEntities) > 0, return nullptr);
  for (const auto& sub : Declaration::kSubEntities)
    if (key == sub.key)
      return &sub;

  return nullptr;
}

/**
 * @brief Records a refused key once for the console note, bounded in count and length so a hostile
 *        line cannot grow the message.
 */
static void rememberKey(QStringList& dropped, const QString& key)
{
  SS_ASSERT(dropped.size() <= kMaxListedKeys, return);
  const auto name = key.left(static_cast<int>(Declaration::kMaxStringBytes));
  if (dropped.size() < kMaxListedKeys && !dropped.contains(name))
    dropped.append(name);
}

/**
 * @brief Copies the keys of @p in that @p table allows; every other key is remembered as refused.
 */
template<size_t N>
[[nodiscard]] static QJsonObject allowed(const QJsonObject& in,
                                         const Keys::KeyView (&table)[N],
                                         QStringList& dropped)
{
  QJsonObject out;
  for (auto it = in.begin(); it != in.end(); ++it)
    if (listed(it.key(), table))
      out.insert(it.key(), it.value());
    else
      rememberKey(dropped, it.key());

  return out;
}

/**
 * @brief True when every string value of @p obj fits the declaration's string cap (UTF-8 bytes).
 */
[[nodiscard]] static bool stringsFit(const QJsonObject& obj)
{
  SS_ASSERT(Declaration::kMaxStringBytes > 0, return false);
  return std::all_of(obj.begin(), obj.end(), [](const QJsonValue& value) {
    return !value.isString() || value.toString().toUtf8().size() <= Declaration::kMaxStringBytes;
  });
}

//--------------------------------------------------------------------------------------------------
// Wire filtering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Limits one sub-entity array to its cap and each entry to its inner keys; fails on a value
 *        that is not an array of objects, too many entries or an over-long string.
 */
[[nodiscard]] static bool filterEntries(const QJsonValue& value,
                                        const Declaration::SubEntityWire& sub,
                                        QJsonArray& entries,
                                        QStringList& dropped,
                                        QString& reason)
{
  SS_ASSERT(sub.inner != nullptr, return false);
  SS_ASSERT(sub.maxItems > 0, return false);
  const auto array = value.toArray();
  if (!value.isArray() || array.size() > sub.maxItems) {
    reason =
      QStringLiteral("%1 must be an array of at most %2 entries").arg(sub.key).arg(sub.maxItems);
    return false;
  }

  for (const auto& item : array) {
    QJsonObject entry;
    const auto raw = item.toObject();
    for (auto it = raw.begin(); it != raw.end(); ++it)
      if (listedIn(it.key(), sub.inner, sub.innerCount))
        entry.insert(it.key(), it.value());
      else
        rememberKey(dropped, it.key());

    if (!item.isObject() || !stringsFit(entry)) {
      reason =
        QStringLiteral("an entry of %1 is not an object or has an over-long string").arg(sub.key);
      return false;
    }

    entries.append(entry);
  }

  return true;
}

/**
 * @brief Limits one dataset object to the wire: registry keys plus the wire-included sub-entities,
 *        each filtered by filterEntries. Fails on an over-long string or a malformed sub-entity.
 */
[[nodiscard]] static bool filterDataset(const QJsonObject& in,
                                        QJsonObject& out,
                                        QStringList& dropped,
                                        QString& reason)
{
  SS_ASSERT(out.isEmpty(), out = QJsonObject());
  SS_ASSERT(std::size(Declaration::kDatasetKeys) > 0, return false);
  for (auto it = in.begin(); it != in.end(); ++it) {
    const auto* sub = subEntityFor(it.key());
    if (sub == nullptr) {
      if (listed(it.key(), Declaration::kDatasetKeys))
        out.insert(it.key(), it.value());
      else
        rememberKey(dropped, it.key());

      continue;
    }

    QJsonArray entries;
    if (!filterEntries(it.value(), *sub, entries, dropped, reason))
      return false;

    out.insert(it.key(), entries);
  }

  if (stringsFit(out))
    return true;

  reason =
    QStringLiteral("a dataset string is longer than %1 bytes").arg(Declaration::kMaxStringBytes);
  return false;
}

//--------------------------------------------------------------------------------------------------
// Envelope and groups
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks the prefix, the version digits, the size cap and the JSON, in that order, and
 *        hands back the top-level object (spec 0111 R10/R11).
 */
[[nodiscard]] static bool readEnvelope(QByteArrayView line, QJsonObject& frame, QString& reason)
{
  const auto text       = DataModel::trimLineAscii(line);
  const qsizetype start = Declaration::kPrefix.size();
  SS_ASSERT(start > 0, return false);
  if (!text.startsWith(Declaration::kPrefix)) {
    reason = QStringLiteral("not a declaration line");
    return false;
  }

  qsizetype cursor = start;
  int version      = 0;
  while (cursor < text.size() && cursor - start < kMaxVersionDigits && text.at(cursor) >= '0'
         && text.at(cursor) <= '9')
    version = version * 10 + (text.at(cursor++) - '0');

  SS_ASSERT(cursor <= text.size(), return false);
  if (cursor == start) {
    reason = QStringLiteral("no wire version after the prefix");
    return false;
  }

  if (version != Declaration::kWireVersion) {
    reason = QStringLiteral("unknown declaration version %1").arg(version);
    return false;
  }

  if (text.size() > Declaration::kMaxLineBytes) {
    reason = QStringLiteral("larger than %1 bytes").arg(Declaration::kMaxLineBytes);
    return false;
  }

  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(text.sliced(cursor).toByteArray(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    reason = QStringLiteral("malformed JSON (%1)").arg(error.errorString());
    return false;
  }

  frame = document.object();
  return true;
}

/**
 * @brief Filters one group object and its datasets to the wire, applying the count and string caps
 *        before the project reader sees it. Sets @p skip for an output group, which version 1 drops
 *        with a note until declared controls land (spec 0111 M5).
 */
[[nodiscard]] static bool filterGroup(const QJsonValue& value,
                                      QJsonObject& group,
                                      bool& skip,
                                      DataModel::DeviceDeclaration& out,
                                      QStringList& dropped)
{
  SS_ASSERT(!skip, skip = false);
  SS_ASSERT(std::size(Declaration::kGroupKeys) > 0, return false);
  group           = allowed(value.toObject(), Declaration::kGroupKeys, dropped);
  const auto name = group.value(Keys::Title).toString().simplified();
  if (!value.isObject() || !stringsFit(group)) {
    out.reason = QStringLiteral("group \"%1\" is malformed or has an over-long string").arg(name);
    return false;
  }

  if (group.value(Keys::GroupType).toInt() != 0 || group.contains(Keys::OutputWidgets)) {
    out.notes.append(
      QStringLiteral("group \"%1\" ignored: declared controls need a newer app").arg(name));
    skip = true;
    return true;
  }

  const auto raw = group.value(Keys::Datasets).toArray();
  if (!group.value(Keys::Datasets).isArray() || raw.size() > Declaration::kMaxDatasets) {
    out.reason = QStringLiteral("group \"%1\" needs an array of at most %2 datasets")
                   .arg(name)
                   .arg(Declaration::kMaxDatasets);
    return false;
  }

  if (!allowedGroupWidget(group.value(Keys::Widget).toString())) {
    out.notes.append(
      QStringLiteral("group \"%1\": widget \"%2\" is not allowed, showing a data grid")
        .arg(name, group.value(Keys::Widget).toString()));
    group.insert(Keys::Widget, QStringLiteral("datagrid"));
  }

  QJsonArray datasets;
  for (const auto& item : raw) {
    QJsonObject dataset;
    if (!item.isObject()) {
      out.reason = QStringLiteral("group \"%1\" has a dataset that is not an object").arg(name);
      return false;
    }

    if (!filterDataset(item.toObject(), dataset, dropped, out.reason))
      return false;

    if (!dataset.isEmpty())
      datasets.append(dataset);
  }

  group.insert(Keys::Datasets, datasets);
  return true;
}

/**
 * @brief Gives a dataset its frame position: the next one, or the earlier position its index names
 *        so two groups can show one channel (spec 0111 R1). An index past the next free position
 *        would leave a gap and is refused.
 */
[[nodiscard]] static bool assignPosition(DataModel::Dataset& dataset, int& next, QString& reason)
{
  SS_ASSERT(next >= 0, return false);
  SS_ASSERT(Declaration::kMaxDatasets >= next, return false);
  if (dataset.index <= 0)
    dataset.index = next + 1;

  if (dataset.index == next + 1) {
    ++next;
  } else if (dataset.index > next) {
    reason = QStringLiteral("dataset \"%1\" names index %2, past the next frame position %3")
               .arg(dataset.title)
               .arg(dataset.index)
               .arg(next + 1);
    return false;
  }

  if (dataset.title.isEmpty())
    dataset.title = QStringLiteral("Channel %1").arg(dataset.index);

  return true;
}

/**
 * @brief Reads one filtered group through the project reader, bounds its datasets with the wire
 *        sanitizer, assigns their frame positions and downgrades a widget whose dataset roles do
 *        not satisfy it to a data grid.
 */
[[nodiscard]] static bool readGroup(const QJsonObject& filtered,
                                    const int groupId,
                                    int& next,
                                    DataModel::DeviceDeclaration& out)
{
  SS_ASSERT(groupId >= 0, return false);
  SS_ASSERT(Declaration::kMaxGroups > groupId, return false);

  DataModel::Group group;
  group.groupId  = groupId;
  group.sourceId = 0;
  if (!DataModel::read(group, filtered)) {
    out.reason = QStringLiteral("group %1 has no title").arg(groupId + 1);
    return false;
  }

  if (group.datasets.empty()) {
    out.notes.append(
      QStringLiteral("group \"%1\" ignored: it declares no dataset").arg(group.title));
    return true;
  }

  for (auto& dataset : group.datasets) {
    Declaration::sanitizeDatasetWire(dataset);
    if (!assignPosition(dataset, next, out.reason))
      return false;
  }

  if (!SerialStudio::groupMeetsWidgetRequirement(group)) {
    out.notes.append(QStringLiteral("group \"%1\": its datasets do not fit widget \"%2\", showing "
                                    "a data grid")
                       .arg(group.title, group.widget));
    group.widget = QStringLiteral("datagrid");
  }

  out.groups.push_back(std::move(group));
  return true;
}

/**
 * @brief A refused declaration: the reason stays, every partial structure goes.
 */
[[nodiscard]] static DataModel::DeviceDeclaration refused(DataModel::DeviceDeclaration out)
{
  SS_ASSERT(!out.reason.isEmpty(), out.reason = QStringLiteral("refused"));
  out.valid    = false;
  out.channels = 0;
  out.groups.clear();
  out.notes.clear();
  SS_ASSERT(out.groups.empty(), return out);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses one declaration line (spec 0111 R1/R10/R11): envelope, allow-list filter, caps,
 *        the project readers, the wire sanitizer and frame positions. A refused line returns
 *        valid = false with the reason and no groups; unknown keys are dropped and named in a note.
 */
DataModel::DeviceDeclaration DataModel::parseDeclaration(QByteArrayView line)
{
  DeviceDeclaration out;
  QJsonObject frame;
  if (!readEnvelope(line, frame, out.reason))
    return refused(out);

  QStringList dropped;
  const auto top  = allowed(frame, Declaration::kFrameKeys, dropped);
  const auto list = top.value(Keys::Groups).toArray();
  out.title       = top.value(Keys::Title).toString().simplified();
  SS_ASSERT(Declaration::kMaxGroups > 0, return refused(out));
  if (out.title.isEmpty() || !stringsFit(top) || list.isEmpty()
      || list.size() > Declaration::kMaxGroups) {
    out.reason =
      QStringLiteral("a declaration needs a title and 1 to %1 groups").arg(Declaration::kMaxGroups);
    return refused(out);
  }

  if (top.contains(Keys::Sources))
    out.notes.append(QStringLiteral("binary layouts (sources) need a newer app and were ignored"));

  int next     = 0;
  int datasets = 0;
  for (const auto& item : list) {
    bool skip = false;
    QJsonObject group;
    if (!filterGroup(item, group, skip, out, dropped))
      return refused(out);

    datasets += static_cast<int>(group.value(Keys::Datasets).toArray().size());
    if (datasets > Declaration::kMaxDatasets) {
      out.reason = QStringLiteral("more than %1 datasets").arg(Declaration::kMaxDatasets);
      return refused(out);
    }

    if (!skip && !readGroup(group, static_cast<int>(out.groups.size()), next, out))
      return refused(out);
  }

  if (!dropped.isEmpty())
    out.notes.append(QStringLiteral("ignored keys: %1").arg(dropped.join(QStringLiteral(", "))));

  if (next <= 0) {
    out.reason = QStringLiteral("the declaration has no dataset");
    return refused(out);
  }

  out.channels = next;
  out.valid    = true;
  return out;
}

//--------------------------------------------------------------------------------------------------
// Canonical comparison
//--------------------------------------------------------------------------------------------------

/**
 * @brief The wire-key view of a structure (spec 0111 R3): the title, then every group, dataset and
 *        control limited to the keys a declaration may carry, so a loaded project and a declaration
 *        compare on exactly what a device can say.
 */
QJsonObject DataModel::wireProjection(const QString& title, const std::vector<Group>& groups)
{
  SS_ASSERT(std::size(Declaration::kGroupKeys) > 0, return QJsonObject());
  SS_ASSERT(std::size(Declaration::kControlKeys) > 0, return QJsonObject());

  QStringList ignored;
  QJsonArray array;
  for (const auto& group : groups) {
    auto obj = allowed(DataModel::serialize(group), Declaration::kGroupKeys, ignored);
    QJsonArray datasets;
    for (const auto& item : obj.value(Keys::Datasets).toArray()) {
      QJsonObject dataset;
      QString reason;
      if (filterDataset(item.toObject(), dataset, ignored, reason))
        datasets.append(dataset);
    }

    QJsonArray controls;
    for (const auto& item : obj.value(Keys::OutputWidgets).toArray())
      controls.append(allowed(item.toObject(), Declaration::kControlKeys, ignored));

    obj.insert(Keys::Datasets, datasets);
    if (obj.contains(Keys::OutputWidgets))
      obj.insert(Keys::OutputWidgets, controls);

    array.append(obj);
  }

  QJsonObject projection;
  projection.insert(Keys::Title, title.simplified());
  projection.insert(Keys::Groups, array);
  return projection;
}

/**
 * @brief The Quick Plot channel registry of a declaration: one channel per frame position, named
 *        and unit-tagged by the first dataset at that position, so a named line maps by title and
 *        a positional one by index (spec 0111 R5). The live lane and the tests share this one map.
 */
std::vector<DataModel::QuickPlotChannel> DataModel::declaredChannels(
  const DeviceDeclaration& declaration)
{
  SS_ASSERT(declaration.valid, return {});
  SS_ASSERT(kMaxLineFields >= declaration.channels, return {});

  std::vector<QuickPlotChannel> channels(static_cast<size_t>(declaration.channels));
  for (const auto& group : declaration.groups) {
    for (const auto& dataset : group.datasets) {
      const bool inRange = dataset.index >= 1 && dataset.index <= declaration.channels;
      SS_ASSERT_LOG(inRange);
      if (!inRange || channels[static_cast<size_t>(dataset.index - 1)].position >= 0)
        continue;

      auto& channel    = channels[static_cast<size_t>(dataset.index - 1)];
      channel.position = dataset.index - 1;
      channel.name     = dataset.title.toUtf8();
      channel.unit     = dataset.units.toUtf8();
    }
  }

  return channels;
}

//--------------------------------------------------------------------------------------------------
// Declaration state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts with no declaration and no rebuild on record.
 */
DataModel::DeclarationState::DeclarationState() : m_rebuilt(false) {}

/**
 * @brief Forgets the declaration and every remembered line; called on connect and on a mode
 *        change, so the app's request brings the declaration back.
 */
void DataModel::DeclarationState::reset()
{
  m_rebuilt = false;
  m_message.clear();
  m_heldLine.clear();
  m_acceptedLine.clear();
  m_rejectedLine.clear();
  m_current = DeviceDeclaration();
}

/**
 * @brief Offers one prefixed line (spec 0111 R4/R10). The bytes of the last accepted, refused or
 *        held line change nothing, so a device that re-declares at reset costs no rebuild; a
 * changed line within a second of the last rebuild (driver time) is held once; anything else is
 *        parsed exactly once and either rebuilds or is refused, the reason in message().
 */
DataModel::DeclarationOutcome DataModel::DeclarationState::offer(QByteArrayView line,
                                                                 const TimePoint timestamp)
{
  const auto text = trimLineAscii(line);
  SS_ASSERT(isDeclarationLine(text), return DeclarationOutcome::Unchanged);
  SS_ASSERT(m_rebuilt || !m_current.valid, return DeclarationOutcome::Unchanged);
  if (text == QByteArrayView(m_acceptedLine) || text == QByteArrayView(m_rejectedLine))
    return DeclarationOutcome::Unchanged;

  if (m_rebuilt && timestamp - m_lastRebuild < kRebuildInterval) {
    if (text == QByteArrayView(m_heldLine))
      return DeclarationOutcome::Unchanged;

    m_heldLine = text.toByteArray();
    m_message  = QStringLiteral("Device declaration held: the dashboard was rebuilt under a "
                                "second ago");
    return DeclarationOutcome::Held;
  }

  m_heldLine.clear();
  auto parsed = parseDeclaration(text);
  if (!parsed.valid) {
    m_rejectedLine = text.toByteArray();
    m_message      = QStringLiteral("Device declaration ignored: %1").arg(parsed.reason);
    return DeclarationOutcome::Rejected;
  }

  int datasets = 0;
  for (const auto& group : parsed.groups)
    datasets += static_cast<int>(group.datasets.size());

  m_message = QStringLiteral("Device declared \"%1\": %2 groups, %3 datasets")
                .arg(parsed.title)
                .arg(static_cast<int>(parsed.groups.size()))
                .arg(datasets);
  if (!parsed.notes.isEmpty())
    m_message += QStringLiteral(" (%1)").arg(parsed.notes.join(QStringLiteral("; ")));

  m_acceptedLine = text.toByteArray();
  m_rejectedLine.clear();
  m_current     = std::move(parsed);
  m_rebuilt     = true;
  m_lastRebuild = timestamp;
  return DeclarationOutcome::Rebuild;
}
