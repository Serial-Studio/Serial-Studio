/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Sessions/ReportOptionsModel.h"

#ifdef BUILD_COMMERCIAL

#  include <QSet>

#  include "SSAssert.h"

static const QString kFolderKind  = QStringLiteral("folder");
static const QString kGroupKind   = QStringLiteral("group");
static const QString kDatasetKind = QStringLiteral("dataset");

/**
 * @brief Builds an empty picker; the dialog fills it when the session's column list arrives.
 */
Sessions::ReportOptionsModel::ReportOptionsModel(QObject* parent)
  : QAbstractListModel(parent), m_visibleRowCount(0), m_selectedDatasetCount(0)
{}

//--------------------------------------------------------------------------------------------------
// Model interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief The number of flattened rows, visible or not; the delegate hides the collapsed ones.
 */
int Sessions::ReportOptionsModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return static_cast<int>(m_rows.size());
}

/**
 * @brief One row field by role.
 */
QVariant Sessions::ReportOptionsModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() >= m_rows.size())
    return {};

  const auto& row = m_rows.at(index.row());
  switch (role) {
    case KindRole:
      return row.kind;
    case NodeIdRole:
      return row.nodeId;
    case ParentIdRole:
      return row.parentId;
    case DepthRole:
      return row.depth;
    case ExpandedRole:
      return row.expanded;
    case HasChildrenRole:
      return row.hasChildren;
    case RowVisibleRole:
      return row.rowVisible;
    case LabelRole:
      return row.label;
    case SourceLabelRole:
      return row.sourceLabel;
    case CheckStateRole:
      return row.checkState;
    case UniqueIdRole:
      return row.uniqueId;
    case CheckedRole:
      return row.checked;
    default:
      break;
  }

  return {};
}

/**
 * @brief Role names, matching the property names the delegate already binds.
 */
QHash<int, QByteArray> Sessions::ReportOptionsModel::roleNames() const
{
  static const QHash<int, QByteArray> kRoles = {
    {       KindRole,        "kind"},
    {     NodeIdRole,      "nodeId"},
    {   ParentIdRole,    "parentId"},
    {      DepthRole,       "depth"},
    {   ExpandedRole,    "expanded"},
    {HasChildrenRole, "hasChildren"},
    { RowVisibleRole,  "rowVisible"},
    {      LabelRole,       "label"},
    {SourceLabelRole, "sourceLabel"},
    { CheckStateRole,  "checkState"},
    {   UniqueIdRole,    "uniqueId"},
    {    CheckedRole,     "checked"},
  };

  return kRoles;
}

/**
 * @brief The flattened row count; QML binds this the way it bound ListModel::count.
 */
int Sessions::ReportOptionsModel::count() const noexcept
{
  return static_cast<int>(m_rows.size());
}

/**
 * @brief Rows the collapse or search state currently shows.
 */
int Sessions::ReportOptionsModel::visibleRowCount() const noexcept
{
  return m_visibleRowCount;
}

/**
 * @brief Checked dataset leaves; the export button is gated on this being non-zero.
 */
int Sessions::ReportOptionsModel::selectedDatasetCount() const noexcept
{
  return m_selectedDatasetCount;
}

//--------------------------------------------------------------------------------------------------
// Building the tree
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every row; the dialog calls this when it opens for another session.
 */
void Sessions::ReportOptionsModel::clear()
{
  beginResetModel();
  m_rows.clear();
  m_indexByNode.clear();
  m_folders.clear();
  m_groupsByKey.clear();
  m_itemsByKey.clear();
  m_search.clear();
  m_visibleRowCount      = 0;
  m_selectedDatasetCount = 0;
  endResetModel();

  Q_EMIT countChanged();
  Q_EMIT countsChanged();
}

/**
 * @brief Flattens the session's columns into the folder / group / dataset tree the picker shows.
 *        A group's source title is only displayed when the SAME group name arrives from more than
 *        one source, which is what keeps a single-source recording free of redundant chrome.
 */
void Sessions::ReportOptionsModel::build(const QVariantList& datasets,
                                         const QVariantMap& folderByGroup)
{
  beginResetModel();
  m_rows.clear();
  m_indexByNode.clear();
  m_folders.clear();
  m_groupsByKey.clear();
  m_itemsByKey.clear();

  QHash<QString, QSet<QString>> sourcesPerGroup;
  for (const auto& entry : datasets) {
    const auto map = entry.toMap();
    sourcesPerGroup[map.value(QStringLiteral("group")).toString()].insert(
      map.value(QStringLiteral("sourceTitle")).toString());
  }

  QStringList order;
  for (const auto& entry : datasets) {
    const auto map    = entry.toMap();
    const auto group  = map.value(QStringLiteral("group")).toString();
    const auto source = map.value(QStringLiteral("sourceTitle")).toString();
    const auto key    = QString::number(source.size()) + QLatin1Char(':') + source + group;

    if (!m_groupsByKey.contains(key)) {
      const int distinct = sourcesPerGroup.value(group).size();

      QVariantMap header;
      header[QStringLiteral("group")]       = group;
      header[QStringLiteral("sourceTitle")] = source;
      header[QStringLiteral("showSource")]  = distinct > 1 && !source.isEmpty();
      header[QStringLiteral("folderPath")]  = folderByGroup.value(group).toString();
      m_groupsByKey.insert(key, header);
      order.append(key);
    }

    m_itemsByKey[key].append(entry);
  }

  m_folders.insert(QString(), FolderNode{QString(), {}, {}});
  for (const auto& key : std::as_const(order)) {
    const auto path     = m_groupsByKey.value(key).value(QStringLiteral("folderPath")).toString();
    const auto segments = path.isEmpty() ? QStringList() : path.split(QLatin1Char('/'));

    QString parent;
    for (const auto& segment : segments) {
      const QString full = parent.isEmpty() ? segment : (parent + QLatin1Char('/') + segment);
      if (!m_folders.contains(full)) {
        m_folders.insert(full, FolderNode{full, {}, {}});
        m_folders[parent].childOrder.append(segment);
      }

      parent = full;
    }

    m_folders[parent].groups.append(key);
  }

  emitFolderNode(QString(), 0);

  for (int i = 0; i < m_rows.size(); ++i)
    m_indexByNode.insert(m_rows.at(i).nodeId, i);

  endResetModel();

  Q_EMIT countChanged();
  recomputeVisibility();
  refreshSelectedCount();
}

/**
 * @brief Depth-first emit of one folder node: nested subfolders first (recursively), then the
 *        groups it holds, then each group's datasets.
 */
void Sessions::ReportOptionsModel::emitFolderNode(const QString& path, const int depth)
{
  SS_ASSERT(depth >= 0, return);
  SS_ASSERT(m_folders.contains(path), return);

  const auto node        = m_folders.value(path);
  const QString parentId = path.isEmpty() ? QString() : (QStringLiteral("F:") + path);

  for (const auto& segment : node.childOrder) {
    const QString full = path.isEmpty() ? segment : (path + QLatin1Char('/') + segment);
    const auto child   = m_folders.value(full);

    Row row;
    row.kind        = kFolderKind;
    row.nodeId      = QStringLiteral("F:") + full;
    row.parentId    = parentId;
    row.depth       = depth;
    row.hasChildren = !child.childOrder.isEmpty() || !child.groups.isEmpty();
    row.rowVisible  = depth == 0;
    row.label       = segment;
    m_rows.append(row);

    emitFolderNode(full, depth + 1);
  }

  for (const auto& key : node.groups) {
    const auto header = m_groupsByKey.value(key);
    const auto items  = m_itemsByKey.value(key);

    Row group;
    group.kind        = kGroupKind;
    group.nodeId      = QStringLiteral("G:") + key;
    group.parentId    = parentId;
    group.depth       = depth;
    group.hasChildren = !items.isEmpty();
    group.rowVisible  = depth == 0;
    group.label       = header.value(QStringLiteral("group")).toString();
    if (header.value(QStringLiteral("showSource")).toBool())
      group.sourceLabel = header.value(QStringLiteral("sourceTitle")).toString();

    m_rows.append(group);

    for (const auto& item : items) {
      const auto map   = item.toMap();
      const auto title = map.value(QStringLiteral("title")).toString();
      const auto units = map.value(QStringLiteral("units")).toString();

      Row leaf;
      leaf.kind       = kDatasetKind;
      leaf.uniqueId   = map.value(QStringLiteral("uniqueId")).toInt();
      leaf.nodeId     = QStringLiteral("D:") + QString::number(leaf.uniqueId);
      leaf.parentId   = group.nodeId;
      leaf.depth      = depth + 1;
      leaf.checkState = Qt::Unchecked;
      leaf.label =
        units.isEmpty() ? title : (title + QStringLiteral(" (") + units + QLatin1Char(')'));
      m_rows.append(leaf);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Visibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a search query (already lower-cased by the caller) and re-derives visibility.
 */
void Sessions::ReportOptionsModel::setSearch(const QString& query)
{
  if (m_search == query)
    return;

  m_search = query;
  recomputeVisibility();
}

/**
 * @brief Re-derives row visibility from the expansion state, or from the query when one is set.
 */
void Sessions::ReportOptionsModel::recomputeVisibility()
{
  if (m_search.isEmpty())
    recomputeCollapsedVisibility();
  else
    recomputeSearchVisibility();
}

/**
 * @brief Collapsed view: a row shows only when every ancestor is expanded. One forward pass, since
 *        a parent always precedes its children in the flattened order.
 */
void Sessions::ReportOptionsModel::recomputeCollapsedVisibility()
{
  int shownCount = 0;
  QHash<QString, bool> shown;
  for (int i = 0; i < m_rows.size(); ++i) {
    auto& row          = m_rows[i];
    const bool visible = row.parentId.isEmpty() ? true : shown.value(row.parentId, false);
    if (row.rowVisible != visible) {
      row.rowVisible = visible;
      publishRowChange(i, {RowVisibleRole});
    }

    shown.insert(row.nodeId, visible && row.expanded);
    if (visible)
      ++shownCount;
  }

  if (m_visibleRowCount == shownCount)
    return;

  m_visibleRowCount = shownCount;
  Q_EMIT countsChanged();
}

/**
 * @brief Search view: every matching row shows, plus its ancestors. The ancestor walk is a hash
 *        lookup per hop, so a deep tree costs its depth and not a scan.
 */
void Sessions::ReportOptionsModel::recomputeSearchVisibility()
{
  QList<bool> visible(m_rows.size(), false);
  for (int i = 0; i < m_rows.size(); ++i) {
    if (!m_rows.at(i).label.toLower().contains(m_search))
      continue;

    visible[i]  = true;
    QString pid = m_rows.at(i).parentId;
    for (int guard = 0; guard < m_rows.size() && !pid.isEmpty(); ++guard) {
      const int idx = indexOfNode(pid);
      if (idx < 0)
        break;

      visible[idx] = true;
      pid          = m_rows.at(idx).parentId;
    }
  }

  int shownCount = 0;
  for (int i = 0; i < m_rows.size(); ++i) {
    if (m_rows.at(i).rowVisible != visible.at(i)) {
      m_rows[i].rowVisible = visible.at(i);
      publishRowChange(i, {RowVisibleRole});
    }

    if (visible.at(i))
      ++shownCount;
  }

  if (m_visibleRowCount == shownCount)
    return;

  m_visibleRowCount = shownCount;
  Q_EMIT countsChanged();
}

//--------------------------------------------------------------------------------------------------
// Expansion and checking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggles the expansion of a folder or group row; a leaf is ignored.
 */
void Sessions::ReportOptionsModel::toggleExpanded(const int row)
{
  if (row < 0 || row >= m_rows.size() || !m_rows.at(row).hasChildren)
    return;

  m_rows[row].expanded = !m_rows.at(row).expanded;
  publishRowChange(row, {ExpandedRole});
  recomputeVisibility();
}

/**
 * @brief Expands or collapses every folder and group node at once.
 */
void Sessions::ReportOptionsModel::setAllExpanded(const bool expanded)
{
  for (int i = 0; i < m_rows.size(); ++i) {
    if (!m_rows.at(i).hasChildren || m_rows.at(i).expanded == expanded)
      continue;

    m_rows[i].expanded = expanded;
    publishRowChange(i, {ExpandedRole});
  }

  recomputeVisibility();
}

/**
 * @brief Checks or unchecks a node and its whole subtree (the contiguous rows of greater depth),
 *        then refreshes every ancestor's tri-state.
 */
void Sessions::ReportOptionsModel::setSubtreeChecked(const int row, const bool checked)
{
  if (row < 0 || row >= m_rows.size())
    return;

  const int baseDepth        = m_rows.at(row).depth;
  const QString baseParentId = m_rows.at(row).parentId;

  applyRowChecked(row, checked);
  for (int i = row + 1; i < m_rows.size() && m_rows.at(i).depth > baseDepth; ++i)
    applyRowChecked(i, checked);

  recomputeAncestors(baseParentId);
  refreshSelectedCount();
}

/**
 * @brief Writes the checked state onto one row, respecting its kind: a leaf carries `checked`, a
 *        container carries the tri-state `checkState` the delegate's checkbox binds.
 */
void Sessions::ReportOptionsModel::applyRowChecked(const int row, const bool checked)
{
  SS_ASSERT(row >= 0 && row < m_rows.size(), return);

  if (m_rows.at(row).kind == kDatasetKind) {
    m_rows[row].checked = checked;
    publishRowChange(row, {CheckedRole});
    return;
  }

  m_rows[row].checkState = checked ? Qt::Checked : Qt::Unchecked;
  publishRowChange(row, {CheckStateRole});
}

/**
 * @brief Walks up the parent chain, recomputing each container's tri-state.
 */
void Sessions::ReportOptionsModel::recomputeAncestors(const QString& parentId)
{
  QString pid = parentId;
  for (int guard = 0; guard < m_rows.size() && !pid.isEmpty(); ++guard) {
    const int idx = indexOfNode(pid);
    if (idx < 0)
      break;

    recomputeNode(idx);
    pid = m_rows.at(idx).parentId;
  }
}

/**
 * @brief Recomputes one container's tri-state from the dataset leaves it holds. A container with
 *        no leaves keeps its state rather than reading as unchecked.
 */
void Sessions::ReportOptionsModel::recomputeNode(const int headerIndex)
{
  SS_ASSERT(headerIndex >= 0 && headerIndex < m_rows.size(), return);
  if (m_rows.at(headerIndex).kind == kDatasetKind)
    return;

  int total         = 0;
  int checked       = 0;
  const int baseDep = m_rows.at(headerIndex).depth;
  for (int i = headerIndex + 1; i < m_rows.size() && m_rows.at(i).depth > baseDep; ++i) {
    if (m_rows.at(i).kind != kDatasetKind)
      continue;

    ++total;
    if (m_rows.at(i).checked)
      ++checked;
  }

  if (total == 0)
    return;

  const int state = (checked == 0)     ? Qt::Unchecked
                  : (checked == total) ? Qt::Checked
                                       : Qt::PartiallyChecked;
  if (m_rows.at(headerIndex).checkState == state)
    return;

  m_rows[headerIndex].checkState = state;
  publishRowChange(headerIndex, {CheckStateRole});
}

/**
 * @brief Refreshes the count of checked dataset leaves, which gates the export button.
 */
void Sessions::ReportOptionsModel::refreshSelectedCount()
{
  int selected = 0;
  for (const auto& row : std::as_const(m_rows))
    if (row.kind == kDatasetKind && row.checked)
      ++selected;

  if (m_selectedDatasetCount == selected)
    return;

  m_selectedDatasetCount = selected;
  Q_EMIT countsChanged();
}

/**
 * @brief The unique ids of every checked dataset, in wire order, for the export options.
 */
QVariantList Sessions::ReportOptionsModel::selectedUniqueIds() const
{
  QVariantList ids;
  for (const auto& row : std::as_const(m_rows))
    if (row.kind == kDatasetKind && row.checked)
      ids.append(row.uniqueId);

  return ids;
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Row index of a node id, or -1; the map is rebuilt whenever the tree is.
 */
int Sessions::ReportOptionsModel::indexOfNode(const QString& nodeId) const
{
  return m_indexByNode.value(nodeId, -1);
}

/**
 * @brief Notifies the view that one row's roles changed.
 */
void Sessions::ReportOptionsModel::publishRowChange(const int row, const QList<int>& roles)
{
  SS_ASSERT(row >= 0 && row < m_rows.size(), return);
  SS_ASSERT_LOG(!roles.isEmpty());

  const auto idx = index(row, 0);
  Q_EMIT dataChanged(idx, idx, roles);
}

#endif  // BUILD_COMMERCIAL
