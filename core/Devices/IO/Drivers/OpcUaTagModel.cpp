/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/OpcUaTagModel.h"

#include <QLoggingCategory>
#include <QUuid>

#include "Core/SSAssert.h"
#include "SerialStudio.h"

Q_DECLARE_LOGGING_CATEGORY(lcOpcUa)

static constexpr const char* kObjectsFolder = "ns=0;i=85";
static constexpr int kOpcUaMaxDepth         = 32;
static constexpr int kMaxUnitsInFlight      = 8;
static constexpr int kMaxProbesInFlight     = 8;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the empty model rooted at the Objects folder.
 */
IO::Drivers::OpcUaTagModel::OpcUaTagModel(QObject* parent)
  : QAbstractItemModel(parent)
  , m_session(nullptr)
  , m_root(std::make_unique<Node>())
  , m_nextToken(1)
  , m_unitsInFlight(0)
  , m_probesInFlight(0)
  , m_pendingBrowses(0)
  , m_lastBusy(false)
{
  m_root->folder     = true;
  m_root->expandable = true;
  m_root->nodeId     = QString::fromLatin1(kObjectsFolder);
  m_root->name       = QStringLiteral("Objects");
}

/**
 * @brief Retires every outstanding node handle.
 */
IO::Drivers::OpcUaTagModel::~OpcUaTagModel()
{
  setSession(nullptr);
}

/**
 * @brief Lends (or withdraws) the connected browse session; withdrawing resets the tree. Every
 *        browse and read reply arrives on the session, so the wiring follows the lease.
 */
void IO::Drivers::OpcUaTagModel::setSession(OpcUaSession* session)
{
  if (m_session == session)
    return;

  if (m_session)
    disconnect(m_session, nullptr, this, nullptr);

  m_session = session;
  clear();

  if (!m_session)
    return;

  connect(
    m_session, &OpcUaSession::readFinished, this, &IO::Drivers::OpcUaTagModel::onAttributesRead);
  connect(
    m_session, &OpcUaSession::browseFinished, this, &IO::Drivers::OpcUaTagModel::onBrowseReply);
}

/**
 * @brief Drops every fetched row after retiring every in-flight handle, so no reply can reach a
 *        freed Node; the root stays and refetches on demand.
 */
void IO::Drivers::OpcUaTagModel::clear()
{
  m_pendingReads.clear();
  m_browseTokens.clear();
  m_unitTargets.clear();
  m_probeQueue.clear();
  m_unitQueue.clear();
  m_index.clear();
  m_unitsInFlight  = 0;
  m_probesInFlight = 0;
  m_pendingBrowses = 0;

  beginResetModel();
  m_root->children.clear();
  m_root->fetched  = false;
  m_root->fetching = false;
  m_root->checked  = false;
  endResetModel();

  publishBusy();
  Q_EMIT selectionChanged();
}

/**
 * @brief Remembers tags to show checked once their rows appear (project round-trip, R13).
 */
void IO::Drivers::OpcUaTagModel::preselect(const QList<OpcUaTag>& tags)
{
  m_preselected = tags;
}

/**
 * @brief True while a browse or an attribute read is outstanding.
 */
bool IO::Drivers::OpcUaTagModel::busy() const
{
  return m_pendingBrowses > 0 || !m_pendingReads.isEmpty();
}

/**
 * @brief Emits busyChanged() only on a real transition.
 */
void IO::Drivers::OpcUaTagModel::publishBusy()
{
  const bool now = busy();
  if (now == m_lastBusy)
    return;

  m_lastBusy = now;
  Q_EMIT busyChanged();
}

//--------------------------------------------------------------------------------------------------
// QAbstractItemModel
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves an index to its node (root for an invalid index).
 */
IO::Drivers::OpcUaTagModel::Node* IO::Drivers::OpcUaTagModel::nodeAt(const QModelIndex& index) const
{
  if (!index.isValid())
    return m_root.get();

  auto* node = static_cast<Node*>(index.internalPointer());
  SS_ASSERT(node != nullptr, return m_root.get());
  return node;
}

/**
 * @brief Builds the model index for a node (invalid for root).
 */
QModelIndex IO::Drivers::OpcUaTagModel::indexOf(Node* node) const
{
  SS_ASSERT(node != nullptr, return {});
  if (node == m_root.get() || !node->parent)
    return {};

  return createIndex(node->row, 0, node);
}

/**
 * @brief Child index under a parent (column 0 only).
 */
QModelIndex IO::Drivers::OpcUaTagModel::index(int row, int column, const QModelIndex& parent) const
{
  if (column != 0 || row < 0)
    return {};

  const Node* node = nodeAt(parent);
  if (static_cast<size_t>(row) >= node->children.size())
    return {};

  return createIndex(row, 0, node->children[row].get());
}

/**
 * @brief Parent index of a row.
 */
QModelIndex IO::Drivers::OpcUaTagModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return {};

  const Node* node = nodeAt(child);
  return indexOf(node->parent);
}

/**
 * @brief Fetched child count.
 */
int IO::Drivers::OpcUaTagModel::rowCount(const QModelIndex& parent) const
{
  return static_cast<int>(nodeAt(parent)->children.size());
}

/**
 * @brief Single-column tree.
 */
int IO::Drivers::OpcUaTagModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 1;
}

/**
 * @brief An expandable node claims children until its own browse proves otherwise, so the tree
 *        stays lazy: nothing is fetched before the user expands it.
 */
bool IO::Drivers::OpcUaTagModel::hasChildren(const QModelIndex& parent) const
{
  const Node* node = nodeAt(parent);
  if (node->fetched)
    return !node->children.empty();

  return node->expandable;
}

/**
 * @brief True for an unfetched expandable node while a client is lent.
 */
bool IO::Drivers::OpcUaTagModel::canFetchMore(const QModelIndex& parent) const
{
  const Node* node = nodeAt(parent);
  return m_session && node->expandable && !node->fetched && !node->fetching;
}

/**
 * @brief Expansion hook: browses that one node, never its children.
 */
void IO::Drivers::OpcUaTagModel::fetchMore(const QModelIndex& parent)
{
  if (canFetchMore(parent))
    browse(nodeAt(parent));
}

/**
 * @brief QML role names.
 */
QHash<int, QByteArray> IO::Drivers::OpcUaTagModel::roleNames() const
{
  static const QHash<int, QByteArray> k_roles = {
    {       NameRole,        "name"},
    {     NodeIdRole,      "nodeId"},
    {   TypeCodeRole,    "typeCode"},
    {     AccessRole,      "access"},
    {    CheckedRole,     "checked"},
    { SelectableRole,  "selectable"},
    {     FolderRole,      "folder"},
    {   ArrayLenRole,    "arrayLen"},
    {DescriptionRole, "description"},
    {       UnitRole,        "unit"},
    {Qt::DisplayRole,     "display"},
  };

  return k_roles;
}

/**
 * @brief Row data; checked is tri-state for folders (0 none, 1 partial, 2 all).
 */
QVariant IO::Drivers::OpcUaTagModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return {};

  const Node* node = nodeAt(index);
  switch (role) {
    case Qt::DisplayRole:
    case NameRole:
      return node->name;
    case NodeIdRole:
      return node->nodeId;
    case TypeCodeRole:
      return node->folder ? QString() : OpcUaWire::codeFromType(node->type);
    case AccessRole:
      return node->folder ? QString() : (node->readable ? tr("read") : tr("no read"));
    case SelectableRole:
      return selectable(node);
    case FolderRole:
      return node->folder;
    case ArrayLenRole:
      return node->arrayLen;
    case DescriptionRole:
      return node->description;
    case UnitRole:
      return node->unit;
    case CheckedRole: {
      if (!node->folder)
        return node->checked ? 2 : 0;

      int checked = 0, selectableCount = 0;
      countSelectable(node, checked, selectableCount);
      if (checked == 0)
        return node->checked ? 2 : 0;

      return checked == selectableCount ? 2 : 1;
    }
    default:
      return {};
  }
}

/**
 * @brief Only the checked role is writable.
 */
bool IO::Drivers::OpcUaTagModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != CheckedRole)
    return false;

  setChecked(index, value.toBool());
  return true;
}

//--------------------------------------------------------------------------------------------------
// Selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief A variable is selectable when readable and of a supported wire type.
 */
bool IO::Drivers::OpcUaTagModel::selectable(const Node* node) const noexcept
{
  SS_ASSERT(node != nullptr, return false);
  return !node->folder && node->readable && node->type != OpcUaWire::Type::Invalid;
}

/**
 * @brief Checks a variable, or marks a folder so every selectable variable under it (now or as
 *        it is fetched later) follows. A folder never triggers a browse: the tree stays lazy.
 */
void IO::Drivers::OpcUaTagModel::setChecked(const QModelIndex& index, const bool checked)
{
  Node* node = nodeAt(index);
  if (node == m_root.get() && !index.isValid())
    return;

  if (!node->folder) {
    if (!selectable(node) || node->checked == checked)
      return;

    node->checked = checked;
    if (checked)
      queueUnits(node);
    else
      for (Node* up = node->parent; up; up = up->parent)
        up->checked = false;
  } else {
    node->checked = checked;
    applySelectAll(node);
  }

  Q_EMIT dataChanged(index, index, {CheckedRole});
  for (Node* up = node->parent; up && up != m_root.get(); up = up->parent) {
    const auto upIndex = indexOf(up);
    Q_EMIT dataChanged(upIndex, upIndex, {CheckedRole});
  }

  Q_EMIT selectionChanged();
}

/**
 * @brief Propagates a folder's check state to its fetched descendants; unfetched folders keep
 *        the mark and apply it to their children when the user expands them.
 */
void IO::Drivers::OpcUaTagModel::applySelectAll(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  SS_ASSERT(node->folder, return);

  for (auto& child : node->children) {
    if (child->folder) {
      child->checked = node->checked;
      applySelectAll(child.get());
      continue;
    }

    const bool tick = node->checked && selectable(child.get());
    if (tick && !child->checked)
      queueUnits(child.get());

    child->checked = tick;
  }

  if (node->children.empty())
    return;

  const auto first = indexOf(node->children.front().get());
  const auto last  = indexOf(node->children.back().get());
  Q_EMIT dataChanged(first, last, {CheckedRole});
}

/**
 * @brief Checks every selectable variable already fetched under the root.
 */
void IO::Drivers::OpcUaTagModel::selectAllReadable()
{
  m_root->checked = true;
  applySelectAll(m_root.get());
  Q_EMIT selectionChanged();
}

/**
 * @brief Counts the checked and the selectable fetched variables under a folder (tri-state).
 */
void IO::Drivers::OpcUaTagModel::countSelectable(const Node* node,
                                                 int& checked,
                                                 int& selectableCount) const
{
  SS_ASSERT(node != nullptr, return);
  for (const auto& child : node->children) {
    if (child->folder) {
      countSelectable(child.get(), checked, selectableCount);
      continue;
    }

    if (!selectable(child.get()))
      continue;

    ++selectableCount;
    if (child->checked)
      ++checked;
  }
}

/**
 * @brief Counts checked variables and the wire indices they occupy.
 */
void IO::Drivers::OpcUaTagModel::countSelected(const Node* node, int& count, int& indices) const
{
  SS_ASSERT(node != nullptr, return);
  for (const auto& child : node->children) {
    if (child->folder)
      countSelected(child.get(), count, indices);
    else if (child->checked) {
      ++count;
      indices += qMax(1, child->arrayLen);
    }
  }
}

/**
 * @brief Number of checked variables.
 */
int IO::Drivers::OpcUaTagModel::selectedCount() const
{
  int count = 0, indices = 0;
  countSelected(m_root.get(), count, indices);
  return count;
}

/**
 * @brief Wire indices the checked variables occupy (arrays expand).
 */
int IO::Drivers::OpcUaTagModel::selectedIndices() const
{
  int count = 0, indices = 0;
  countSelected(m_root.get(), count, indices);
  return indices;
}

/**
 * @brief True above the soft tag limit (pane banner).
 */
bool IO::Drivers::OpcUaTagModel::overSoftLimit() const
{
  return selectedIndices() > OpcUaWire::kSoftTagLimit;
}

/**
 * @brief Flattens the checked variables into tags in tree order.
 */
void IO::Drivers::OpcUaTagModel::collectSelected(const Node* node, QList<OpcUaTag>& out) const
{
  SS_ASSERT(node != nullptr, return);
  for (const auto& child : node->children) {
    if (child->folder) {
      collectSelected(child.get(), out);
      continue;
    }

    if (!child->checked)
      continue;

    OpcUaTag tag;
    tag.nodeId   = child->nodeId;
    tag.name     = child->name;
    tag.path     = child->path;
    tag.unit     = child->unit;
    tag.type     = child->type;
    tag.arrayLen = child->arrayLen;
    tag.min      = child->euMin;
    tag.max      = child->euMax;
    out.append(tag);
  }
}

/**
 * @brief The checked variables as tags in tree order.
 */
QList<IO::Drivers::OpcUaTag> IO::Drivers::OpcUaTagModel::selectedTags() const
{
  QList<OpcUaTag> out;
  collectSelected(m_root.get(), out);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Headless access (API)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lookup of a fetched row by node id through the flat index.
 */
IO::Drivers::OpcUaTagModel::Node* IO::Drivers::OpcUaTagModel::findNode(Node* from,
                                                                       const QString& nodeId) const
{
  SS_ASSERT(from != nullptr, return nullptr);
  if (from->nodeId == nodeId)
    return from;

  return m_index.value(nodeId, nullptr);
}

/**
 * @brief True when a row with this node id has been fetched in the current browse session.
 */
bool IO::Drivers::OpcUaTagModel::hasSeen(const QString& nodeId) const
{
  SS_ASSERT_LOG(!nodeId.isEmpty());
  return findNode(m_root.get(), nodeId) != nullptr;
}

/**
 * @brief The fetched children of a node as JSON rows (empty when unknown or not yet fetched).
 */
QJsonArray IO::Drivers::OpcUaTagModel::childrenJson(const QString& nodeId) const
{
  QJsonArray out;
  const Node* node = nodeId.isEmpty() ? m_root.get() : findNode(m_root.get(), nodeId);
  if (!node)
    return out;

  for (const auto& child : node->children) {
    out.append(QJsonObject{
      {         QStringLiteral("id"),                                                    child->nodeId},
      {       QStringLiteral("name"),                                                      child->name},
      {     QStringLiteral("folder"),                                                    child->folder},
      { QStringLiteral("expandable"),                                                child->expandable},
      {          QStringLiteral("t"), child->folder ? QString() : OpcUaWire::codeFromType(child->type)},
      {   QStringLiteral("readable"),                                                  child->readable},
      { QStringLiteral("selectable"),                                          selectable(child.get())},
      {          QStringLiteral("n"),                                                  child->arrayLen},
      {       QStringLiteral("unit"),                                                      child->unit},
      {       QStringLiteral("path"),                                                      child->path},
      {QStringLiteral("description"),                                               child->description},
      {    QStringLiteral("fetched"),                                                   child->fetched},
    });
  }

  return out;
}

/**
 * @brief Starts a browse of any node id, fetched or not; false when no client is lent.
 */
bool IO::Drivers::OpcUaTagModel::fetchNode(const QString& nodeId)
{
  if (!m_session)
    return false;

  Node* node = nodeId.isEmpty() ? m_root.get() : findNode(m_root.get(), nodeId);
  if (!node)
    return false;

  if (node->fetched || node->fetching)
    return true;

  browse(node);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Browsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a browsed node id onto this server's namespace table; a reference that names a
 *        namespace URI (aggregating servers) would otherwise carry a meaningless index.
 */
QString IO::Drivers::OpcUaTagModel::resolveNodeId(const OpcUaTypes::ReferenceRow& row) const
{
  if (row.namespaceUri.isEmpty() || !m_session)
    return row.nodeId;

  const int index = m_session->namespaceArray().indexOf(row.namespaceUri);
  if (index < 0)
    return row.nodeId;

  const auto identifier = row.nodeId.section(QLatin1Char(';'), 1);
  if (identifier.isEmpty())
    return row.nodeId;

  return QStringLiteral("ns=%1;%2").arg(QString::number(index), identifier);
}

/**
 * @brief Sends one browse through the session and registers what its reply will mean.
 */
quint32 IO::Drivers::OpcUaTagModel::issueBrowse(Node* node, Purpose purpose)
{
  SS_ASSERT(node != nullptr, return 0);
  SS_ASSERT(m_session != nullptr, return 0);

  OpcUaTypes::BrowseQuery query;
  query.token         = m_nextToken++;
  query.kind          = purpose == Purpose::Units ? OpcUaTypes::ReferenceKind::HasProperty
                                                  : OpcUaTypes::ReferenceKind::Hierarchical;
  query.nodeClassMask = purpose == Purpose::Units
                        ? OpcUaTypes::NodeClass::Variable
                        : OpcUaTypes::NodeClass::Object | OpcUaTypes::NodeClass::Variable;

  PendingBrowse pending;
  pending.node    = node;
  pending.purpose = purpose;
  m_browseTokens.insert(query.token, pending);

  if (m_session->browse(node->nodeId, query))
    return query.token;

  m_browseTokens.remove(query.token);
  return 0;
}

/**
 * @brief Routes a browse reply to the pass that asked for it.
 */
void IO::Drivers::OpcUaTagModel::onBrowseReply(quint32 token,
                                               const QString& nodeId,
                                               const QList<OpcUaTypes::ReferenceRow>& children,
                                               OpcUaTypes::StatusCode status)
{
  Q_UNUSED(nodeId)
  const auto pending = m_browseTokens.take(token);
  if (!pending.node)
    return;

  if (pending.purpose == Purpose::Level) {
    onBrowseFinished(pending.node, children, status);
    return;
  }

  if (pending.purpose == Purpose::Probe) {
    onProbeFinished(pending.node, children, status);
    return;
  }

  if (OpcUaTypes::isGood(status))
    onUnitsBrowsed(pending.node, children);

  --m_unitsInFlight;
  pumpUnitQueue();
}

/**
 * @brief Issues one browse for a node; the reply lands in onBrowseFinished().
 */
void IO::Drivers::OpcUaTagModel::browse(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  if (!m_session || node->fetching || node->fetched)
    return;

  int depth = 0;
  for (const Node* up = node; up; up = up->parent)
    ++depth;

  if (depth > kOpcUaMaxDepth)
    return;

  node->fetching = true;
  ++m_pendingBrowses;

  if (issueBrowse(node, Purpose::Level) == 0)
    onBrowseFinished(node, {}, OpcUaTypes::kStatusBadInternal);

  publishBusy();
}

/**
 * @brief Builds one child row from a browse reference, honoring the preselection list. The
 *        expandable flag starts on the likely answer (Objects have children, Variables do not)
 *        and the probe corrects the exceptions, such as a struct Variable.
 */
std::unique_ptr<IO::Drivers::OpcUaTagModel::Node> IO::Drivers::OpcUaTagModel::makeChild(
  Node* parent, const OpcUaTypes::ReferenceRow& row) const
{
  SS_ASSERT(parent != nullptr, return std::make_unique<Node>());

  auto child        = std::make_unique<Node>();
  child->parent     = parent;
  child->row        = static_cast<int>(parent->children.size());
  child->folder     = row.nodeClass != OpcUaTypes::NodeClass::Variable;
  child->expandable = child->folder;
  child->nodeId     = resolveNodeId(row);
  child->name       = row.displayName.isEmpty() ? row.browseName : row.displayName;
  child->path =
    parent == m_root.get() ? parent->name : parent->path + QLatin1Char('/') + parent->name;

  if (parent->checked && parent->folder)
    child->inherited = true;

  child->checked = isPreselected(child->nodeId);

  return child;
}

/**
 * @brief Inserts the browsed children and issues ONE batched attribute read for the whole level;
 *        nothing below this node is fetched until the user expands it.
 */
void IO::Drivers::OpcUaTagModel::onBrowseFinished(Node* node,
                                                  const QList<OpcUaTypes::ReferenceRow>& children,
                                                  OpcUaTypes::StatusCode status)
{
  SS_ASSERT(node != nullptr, return);
  node->fetching   = false;
  node->fetched    = true;
  m_pendingBrowses = qMax(0, m_pendingBrowses - 1);

  if (!OpcUaTypes::isGood(status)) {
    Q_EMIT browseError(node->nodeId, OpcUaSession::describeStatus(status));
    publishBusy();
    return;
  }

  const auto parentIndex = indexOf(node);
  if (!children.isEmpty()) {
    beginInsertRows(parentIndex, 0, children.size() - 1);
    for (const auto& ref : children) {
      auto child = makeChild(node, ref);
      m_index.insert(child->nodeId, child.get());
      node->children.push_back(std::move(child));
    }
    endInsertRows();
  }

  readLevel(node);

  for (auto& child : node->children)
    queueProbe(child.get());

  expandPreselected(node);
  refreshAncestors(node);

  Q_EMIT dataChanged(parentIndex, parentIndex);
  Q_EMIT selectionChanged();
  publishBusy();
}

/**
 * @brief The has-children probe's verdict: OPC UA carries no such hint in a browse result, and
 *        guessing leaves dead expanders behind.
 */
void IO::Drivers::OpcUaTagModel::onProbeFinished(Node* node,
                                                 const QList<OpcUaTypes::ReferenceRow>& children,
                                                 OpcUaTypes::StatusCode status)
{
  SS_ASSERT(node != nullptr, return);
  node->probing = false;
  node->probed  = true;
  if (OpcUaTypes::isGood(status) && !node->fetched)
    node->expandable = !children.isEmpty();

  const auto index = indexOf(node);
  Q_EMIT dataChanged(index, index);

  --m_probesInFlight;
  pumpProbeQueue();
}

/**
 * @brief True when the project's tag list already holds this node id.
 */
bool IO::Drivers::OpcUaTagModel::isPreselected(const QString& nodeId) const
{
  for (const auto& tag : m_preselected)
    if (tag.nodeId == nodeId)
      return true;

  return false;
}

/**
 * @brief The full browse path of a node, in the shape stored on a committed tag.
 */
QString IO::Drivers::OpcUaTagModel::pathOf(const Node* node) const
{
  SS_ASSERT(node != nullptr, return {});
  if (node == m_root.get())
    return node->name;

  return node->path + QLatin1Char('/') + node->name;
}

/**
 * @brief Follows ONLY the branches that hold preselected tags, so reopening the picker shows the
 *        project's selection ticked without crawling the rest of the address space.
 */
void IO::Drivers::OpcUaTagModel::expandPreselected(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  if (m_preselected.isEmpty())
    return;

  for (auto& child : node->children) {
    if (!child->folder || child->fetched || child->fetching)
      continue;

    const auto branch = pathOf(child.get());
    for (const auto& tag : m_preselected) {
      if (tag.path != branch && !tag.path.startsWith(branch + QLatin1Char('/')))
        continue;

      Q_EMIT expandRequested(indexOf(child.get()));
      browse(child.get());
      break;
    }
  }
}

/**
 * @brief One batched Read for every variable of a freshly fetched level, chunked so a server's
 *        MaxNodesPerRead is never exceeded.
 */
void IO::Drivers::OpcUaTagModel::readLevel(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  if (!m_session)
    return;

  static const QList<OpcUaTypes::NodeAttribute> k_attributes = {
    OpcUaTypes::NodeAttribute::DataType,
    OpcUaTypes::NodeAttribute::ValueRank,
    OpcUaTypes::NodeAttribute::ArrayDimensions,
    OpcUaTypes::NodeAttribute::AccessLevel,
    OpcUaTypes::NodeAttribute::UserAccessLevel,
    OpcUaTypes::NodeAttribute::Description,
    OpcUaTypes::NodeAttribute::Value,
  };

  const int chunk = qMax(1, m_session->readLimit() / static_cast<int>(k_attributes.size()));

  QStringList nodeIds;
  for (auto& child : node->children) {
    if (child->folder || child->attributesRead)
      continue;

    m_pendingReads.insert(child->nodeId, child.get());
    nodeIds.append(child->nodeId);
    if (nodeIds.size() < chunk)
      continue;

    (void)m_session->readAttributes(nodeIds, k_attributes, Token::LevelRead);
    nodeIds.clear();
  }

  if (!nodeIds.isEmpty())
    (void)m_session->readAttributes(nodeIds, k_attributes, Token::LevelRead);
}

/**
 * @brief Routes a batched Read reply by the token the request carried. Routing on node id alone
 *        let a ticked variable's own EngineeringUnits child divert that variable's level read.
 */
void IO::Drivers::OpcUaTagModel::onAttributesRead(quint32 token,
                                                  const QList<OpcUaTypes::ReadRow>& rows,
                                                  OpcUaTypes::StatusCode status)
{
  Q_UNUSED(status)

  if (token == Token::UnitRead) {
    for (const auto& row : rows)
      applyUnitValue(m_unitTargets.take(row.nodeId), row.value, row.status);

    publishBusy();
    return;
  }

  if (token != Token::LevelRead)
    return;

  QList<Node*> touched;
  for (const auto& row : rows) {
    Node* node = m_pendingReads.value(row.nodeId, nullptr);
    if (!node)
      continue;

    applyAttribute(node, row.attribute, row.value, row.status);
    if (!touched.contains(node))
      touched.append(node);
  }

  for (auto* node : touched) {
    m_pendingReads.remove(node->nodeId);
    finishAttributes(node);
  }

  if (!touched.isEmpty())
    Q_EMIT selectionChanged();

  publishBusy();
}

/**
 * @brief Applies one attribute of a variable row.
 */
void IO::Drivers::OpcUaTagModel::applyAttribute(Node* node,
                                                OpcUaTypes::NodeAttribute attribute,
                                                const QVariant& value,
                                                OpcUaTypes::StatusCode status)
{
  SS_ASSERT(node != nullptr, return);
  if (!OpcUaTypes::isGood(status))
    return;

  switch (attribute) {
    case OpcUaTypes::NodeAttribute::DataType:
      node->type = wireTypeFromDataTypeId(value.toString());
      break;
    case OpcUaTypes::NodeAttribute::Description:
      node->description = value.toString();
      break;
    case OpcUaTypes::NodeAttribute::AccessLevel:
    case OpcUaTypes::NodeAttribute::UserAccessLevel:
      node->readable = (value.toUInt() & OpcUaTypes::kAccessLevelCurrentRead) != 0;
      break;
    case OpcUaTypes::NodeAttribute::ArrayDimensions: {
      const auto dims = value.toList();
      if (!dims.isEmpty())
        node->arrayLen = qBound(1, dims.first().toInt(), OpcUaWire::kMaxTags);

      break;
    }
    case OpcUaTypes::NodeAttribute::Value: {
      if (node->type == OpcUaWire::Type::Invalid)
        node->type = wireTypeFromValue(value);

      if (value.typeId() == QMetaType::QVariantList && node->arrayLen <= 1)
        node->arrayLen = qBound(1, static_cast<int>(value.toList().size()), OpcUaWire::kMaxTags);

      break;
    }
    default:
      break;
  }
}

/**
 * @brief Republishes the check state of every ancestor: a folder's tri-state is derived from its
 *        descendants, so a level arriving deeper down changes what the folders above must show.
 */
void IO::Drivers::OpcUaTagModel::refreshAncestors(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  for (Node* up = node; up && up != m_root.get(); up = up->parent) {
    const auto index = indexOf(up);
    Q_EMIT dataChanged(index, index, {CheckedRole});
  }
}

/**
 * @brief Publishes a row once its attributes landed, and inherits a parent folder's tick.
 */
void IO::Drivers::OpcUaTagModel::finishAttributes(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  node->attributesRead = true;

  if (!selectable(node))
    node->checked = false;
  else if (!node->checked)
    node->checked = isPreselected(node->nodeId);
  else if (node->inherited && !node->checked) {
    node->checked   = true;
    node->inherited = false;
    queueUnits(node);
  }

  const auto index = indexOf(node);
  Q_EMIT dataChanged(index, index);
  refreshAncestors(node->parent);
}

//--------------------------------------------------------------------------------------------------
// Engineering units (ticked tags only)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Queues the one-Browse probe that decides whether a row really has children. OPC UA
 *        carries no such hint in a browse result, and guessing leaves dead expanders behind.
 */
void IO::Drivers::OpcUaTagModel::queueProbe(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  if (node->probed || node->probing || node->fetched || m_probeQueue.contains(node))
    return;

  m_probeQueue.enqueue(node);
  pumpProbeQueue();
}

/**
 * @brief Keeps a bounded number of probes in flight so a wide level cannot flood the server.
 */
void IO::Drivers::OpcUaTagModel::pumpProbeQueue()
{
  while (m_session && m_probesInFlight < kMaxProbesInFlight && !m_probeQueue.isEmpty()) {
    Node* node = m_probeQueue.dequeue();
    if (!node || node->probed || node->fetched)
      continue;

    ++m_probesInFlight;
    node->probing = true;
    if (issueBrowse(node, Purpose::Probe) != 0)
      continue;

    node->probing = false;
    node->probed  = true;
    --m_probesInFlight;
  }
}

/**
 * @brief Queues a units/range lookup for a ticked variable; browsing every variable's properties
 *        up front is what makes a picker unusable on a large server, so it happens on demand.
 */
void IO::Drivers::OpcUaTagModel::queueUnits(Node* node)
{
  SS_ASSERT(node != nullptr, return);
  if (node->euResolved || m_unitQueue.contains(node))
    return;

  m_unitQueue.enqueue(node);
  pumpUnitQueue();
}

/**
 * @brief Keeps a bounded number of property browses in flight.
 */
void IO::Drivers::OpcUaTagModel::pumpUnitQueue()
{
  while (m_session && m_unitsInFlight < kMaxUnitsInFlight && !m_unitQueue.isEmpty()) {
    Node* node = m_unitQueue.dequeue();
    if (!node || node->euResolved)
      continue;

    ++m_unitsInFlight;
    node->euResolved = true;
    if (issueBrowse(node, Purpose::Units) == 0)
      --m_unitsInFlight;
  }
}

/**
 * @brief Reads EngineeringUnits / EURange out of a variable's properties.
 */
void IO::Drivers::OpcUaTagModel::onUnitsBrowsed(Node* node,
                                                const QList<OpcUaTypes::ReferenceRow>& children)
{
  SS_ASSERT(node != nullptr, return);
  if (!m_session)
    return;

  QStringList nodeIds;
  for (const auto& ref : children) {
    const bool range = ref.browseName == QLatin1String("EURange");
    if (!range && ref.browseName != QLatin1String("EngineeringUnits"))
      continue;

    UnitTarget target;
    target.node  = node;
    target.range = range;

    const auto nodeId = resolveNodeId(ref);
    m_unitTargets.insert(nodeId, target);
    nodeIds.append(nodeId);
  }

  if (!nodeIds.isEmpty())
    (void)m_session->readAttributes(nodeIds, {OpcUaTypes::NodeAttribute::Value}, Token::UnitRead);
}

/**
 * @brief Applies one property value to the variable that asked for it. EURange carries {low,
 *        high}: both bounds become the generated dataset's plot minimum and maximum.
 */
void IO::Drivers::OpcUaTagModel::applyUnitValue(const UnitTarget& target,
                                                const QVariant& value,
                                                OpcUaTypes::StatusCode status)
{
  if (!target.node || !OpcUaTypes::isGood(status))
    return;

  if (!target.range)
    target.node->unit = value.toString();
  else {
    const auto bounds = value.toList();
    if (bounds.size() >= 2) {
      target.node->euMin = SerialStudio::toDouble(bounds.at(0));
      target.node->euMax = SerialStudio::toDouble(bounds.at(1));
    }
  }

  const auto index = indexOf(target.node);
  Q_EMIT dataChanged(index, index, {UnitRole});
}

//--------------------------------------------------------------------------------------------------
// Type resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a namespace-0 DataType id onto the wire vocabulary, including the common subtypes
 *        (Duration, UtcTime, Enumeration, ...) that vendors declare instead of the base type.
 */
IO::Drivers::OpcUaWire::Type IO::Drivers::OpcUaTagModel::wireTypeFromDataTypeId(
  const QString& dataTypeId) noexcept
{
  using OpcUaWire::Type;
  static const QHash<int, Type> k_map = {
    {  1, Type::Bool},
    {  2,   Type::I8},
    {  3,   Type::U8},
    {  4,  Type::I16},
    {  5,  Type::U16},
    {  6,  Type::I32},
    {  7,  Type::U32},
    {  8,  Type::I64},
    {  9,  Type::U64},
    { 10,  Type::F32},
    { 11,  Type::F64},
    { 12,  Type::Str},
    { 13,  Type::Str},
    { 14,  Type::Str},
    { 15,  Type::Str},
    { 16,  Type::Str},
    { 17,  Type::Str},
    { 18,  Type::Str},
    { 19,  Type::U32},
    { 20,  Type::Str},
    { 21,  Type::Str},
    { 26,  Type::F64},
    { 27,  Type::I64},
    { 28,  Type::U64},
    { 29,  Type::I32},
    {288,  Type::U32},
    {289,  Type::U64},
    {290,  Type::F64},
    {291,  Type::Str},
    {292,  Type::Str},
    {293,  Type::Str},
    {294,  Type::Str},
  };

  if (!dataTypeId.startsWith(QLatin1String("ns=0;i=")))
    return Type::Invalid;

  bool ok         = false;
  const int ident = dataTypeId.mid(7).toInt(&ok);
  if (!ok)
    return Type::Invalid;

  return k_map.value(ident, Type::Invalid);
}

/**
 * @brief Falls back to the value's own type when the DataType id is a vendor subtype the table
 *        does not know; anything printable ends up as a string channel rather than unselectable.
 */
IO::Drivers::OpcUaWire::Type IO::Drivers::OpcUaTagModel::wireTypeFromValue(
  const QVariant& value) noexcept
{
  using OpcUaWire::Type;

  auto probe = value;
  if (probe.typeId() == QMetaType::QVariantList) {
    const auto list = probe.toList();
    if (list.isEmpty())
      return Type::Invalid;

    probe = list.first();
  }

  switch (probe.typeId()) {
    case QMetaType::Bool:
      return Type::Bool;
    case QMetaType::SChar:
      return Type::I8;
    case QMetaType::UChar:
      return Type::U8;
    case QMetaType::Short:
      return Type::I16;
    case QMetaType::UShort:
      return Type::U16;
    case QMetaType::Int:
      return Type::I32;
    case QMetaType::UInt:
      return Type::U32;
    case QMetaType::LongLong:
      return Type::I64;
    case QMetaType::ULongLong:
      return Type::U64;
    case QMetaType::Float:
      return Type::F32;
    case QMetaType::Double:
      return Type::F64;
    case QMetaType::QString:
    case QMetaType::QDateTime:
    case QMetaType::QUuid:
    case QMetaType::QByteArray:
      return Type::Str;
    default:
      break;
  }

  return Type::Invalid;
}
