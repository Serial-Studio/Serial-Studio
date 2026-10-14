/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#ifdef BUILD_COMMERCIAL

#  include "Core/Deploy/ResourceTree.h"

#  include <algorithm>
#  include <QDebug>
#  include <QHash>
#  include <QResource>
#  include <QtEndian>
#  include <vector>

#  include "Core/Deploy/PackageFormat.h"
#  include "Core/SSAssert.h"

namespace Deploy::ResourceTreeDetail {

/**
 * @brief One directory or file of the tree being written; children index into the node list.
 */
struct Node {
  QString name;
  bool directory      = false;
  qsizetype fileIndex = -1;
  std::vector<int> children;
};

/**
 * @brief Nodes plus a full-path index, so a path maps to exactly one directory or file.
 */
struct Builder {
  std::vector<Node> nodes;
  QHash<QString, int> paths;
};

}  // namespace Deploy::ResourceTreeDetail

using namespace Deploy;
using namespace Deploy::ResourceTreeDetail;

static constexpr quint32 kRccVersion      = 2;
static constexpr quint32 kRccHeaderBytes  = 20;
static constexpr quint16 kFlagDirectory   = 0x02;
static constexpr quint16 kTerritoryAny    = 0;
static constexpr quint16 kLanguageC       = 1;
static constexpr char kRccMagic[]         = "qres";
static constexpr qsizetype kRccMagicBytes = 4;

static QList<QByteArray> s_retainedTrees;

//--------------------------------------------------------------------------------------------------
// Encoding primitives
//--------------------------------------------------------------------------------------------------

/**
 * @brief Qt's resource name hash (qt_hash over UTF-16 code units), which lookups binary-search.
 */
[[nodiscard]] static quint32 resourceHash(QStringView name) noexcept
{
  quint32 h = 0;
  for (const QChar c : name) {
    h  = (h << 4) + c.unicode();
    h ^= (h & 0xf0000000) >> 23;
    h &= 0x0fffffff;
  }

  return h;
}

/**
 * @brief Appends a big-endian 16-bit value.
 */
static void appendU16(QByteArray& out, const quint16 value)
{
  char bytes[2];
  qToBigEndian<quint16>(value, bytes);
  out.append(bytes, 2);
}

/**
 * @brief Appends a big-endian 32-bit value.
 */
static void appendU32(QByteArray& out, const quint32 value)
{
  char bytes[4];
  qToBigEndian<quint32>(value, bytes);
  out.append(bytes, 4);
}

/**
 * @brief Appends a big-endian 64-bit value.
 */
static void appendU64(QByteArray& out, const quint64 value)
{
  char bytes[8];
  qToBigEndian<quint64>(value, bytes);
  out.append(bytes, 8);
}

//--------------------------------------------------------------------------------------------------
// Tree construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the directory node for path, creating it under parent; -1 if a file owns it.
 */
[[nodiscard]] static int ensureDirectory(Builder& builder,
                                         const QString& path,
                                         const QString& name,
                                         const int parent)
{
  SS_ASSERT(parent >= 0 && static_cast<size_t>(parent) < builder.nodes.size(), return -1);

  const auto existing = builder.paths.constFind(path);
  if (existing != builder.paths.constEnd())
    return builder.nodes[static_cast<size_t>(*existing)].directory ? *existing : -1;

  const auto index = static_cast<int>(builder.nodes.size());
  builder.nodes.push_back(Node{name, true, -1, {}});
  builder.nodes[static_cast<size_t>(parent)].children.push_back(index);
  builder.paths.insert(path, index);
  return index;
}

/**
 * @brief Adds a file node under parent; false when the path is already taken.
 */
[[nodiscard]] static bool addFile(Builder& builder,
                                  const QString& path,
                                  const QString& name,
                                  const int parent,
                                  const qsizetype fileIndex)
{
  SS_ASSERT(parent >= 0 && static_cast<size_t>(parent) < builder.nodes.size(), return false);

  if (builder.paths.contains(path))
    return false;

  const auto index = static_cast<int>(builder.nodes.size());
  builder.nodes.push_back(Node{name, false, fileIndex, {}});
  builder.nodes[static_cast<size_t>(parent)].children.push_back(index);
  builder.paths.insert(path, index);
  return true;
}

/**
 * @brief Builds the directory/file node graph; false on an unsafe path or a path conflict.
 */
[[nodiscard]] static bool buildNodes(const QList<FileEntry>& files, Builder& builder)
{
  builder.nodes.push_back(Node{QString(), true, -1, {}});
  for (qsizetype i = 0; i < files.size(); ++i) {
    const auto& path = files[i].path;
    if (!isSafeEntryPath(path))
      return false;

    const auto parts = path.split(QLatin1Char('/'));
    int parent       = 0;
    QString prefix;
    for (qsizetype p = 0; p + 1 < parts.size() && parent >= 0; ++p) {
      prefix = prefix.isEmpty() ? parts[p] : prefix + QLatin1Char('/') + parts[p];
      parent = ensureDirectory(builder, prefix, parts[p], parent);
    }

    if (parent < 0 || !addFile(builder, path, parts.last(), parent, i))
      return false;
  }

  return true;
}

/**
 * @brief Sorts every directory's children by name hash and lists nodes breadth first, so each
 *        directory's children occupy consecutive node indices as the lookup requires.
 */
[[nodiscard]] static std::vector<int> breadthFirstOrder(std::vector<Node>& nodes)
{
  const auto byHash = [&nodes](const int a, const int b) {
    return resourceHash(nodes[static_cast<size_t>(a)].name)
         < resourceHash(nodes[static_cast<size_t>(b)].name);
  };

  for (auto& node : nodes)
    std::stable_sort(node.children.begin(), node.children.end(), byHash);

  std::vector<int> order{0};
  order.reserve(nodes.size());
  for (size_t i = 0; i < order.size() && i < nodes.size(); ++i)
    for (const int child : nodes[static_cast<size_t>(order[i])].children)
      order.push_back(child);

  return order;
}

//--------------------------------------------------------------------------------------------------
// Blob writers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the names blob (u16 length, u32 hash, UTF-16BE) and records each offset.
 */
static void writeNames(const std::vector<Node>& nodes,
                       const std::vector<int>& order,
                       QByteArray& names,
                       std::vector<quint32>& nameOffsets)
{
  for (const int index : order) {
    if (index == 0)
      continue;

    const auto& name                        = nodes[static_cast<size_t>(index)].name;
    nameOffsets[static_cast<size_t>(index)] = static_cast<quint32>(names.size());
    appendU16(names, static_cast<quint16>(name.size()));
    appendU32(names, resourceHash(name));
    for (const QChar c : name)
      appendU16(names, c.unicode());
  }
}

/**
 * @brief Writes the data blob (u32 length, bytes) for every file and records each offset.
 */
static void writeData(const std::vector<Node>& nodes,
                      const std::vector<int>& order,
                      const QList<FileEntry>& files,
                      QByteArray& data,
                      std::vector<quint32>& dataOffsets)
{
  for (const int index : order) {
    const auto& node = nodes[static_cast<size_t>(index)];
    if (node.directory)
      continue;

    const auto& bytes                       = files[node.fileIndex].data;
    dataOffsets[static_cast<size_t>(index)] = static_cast<quint32>(data.size());
    appendU32(data, static_cast<quint32>(bytes.size()));
    data.append(bytes);
  }
}

/**
 * @brief Writes the v2 tree: name offset, flags, then children or locale+data, then mtime 0.
 */
static void writeTree(const std::vector<Node>& nodes,
                      const std::vector<int>& order,
                      const std::vector<quint32>& nameOffsets,
                      const std::vector<quint32>& dataOffsets,
                      QByteArray& tree)
{
  std::vector<int> position(nodes.size(), 0);
  for (size_t i = 0; i < order.size(); ++i)
    position[static_cast<size_t>(order[i])] = static_cast<int>(i);

  for (const int index : order) {
    const auto& node = nodes[static_cast<size_t>(index)];
    appendU32(tree, nameOffsets[static_cast<size_t>(index)]);
    appendU16(tree, node.directory ? kFlagDirectory : 0);
    if (node.directory) {
      const int first = node.children.empty() ? 0 : position[static_cast<size_t>(node.children[0])];
      appendU32(tree, static_cast<quint32>(node.children.size()));
      appendU32(tree, static_cast<quint32>(first));
    } else {
      appendU16(tree, kTerritoryAny);
      appendU16(tree, kLanguageC);
      appendU32(tree, dataOffsets[static_cast<size_t>(index)]);
    }

    appendU64(tree, 0);
  }
}

//--------------------------------------------------------------------------------------------------
// Construction / destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an unmounted tree.
 */
Deploy::ResourceTree::ResourceTree() = default;

/**
 * @brief Unregisters the tree. If Qt still holds a reference into it, the bytes are retained
 *        for the rest of the process instead of freed under a live reader.
 */
Deploy::ResourceTree::~ResourceTree()
{
  if (m_mapRoot.isEmpty())
    return;

  const auto* bytes = static_cast<const uchar*>(static_cast<const void*>(m_data.constData()));
  if (QResource::unregisterResource(bytes, m_mapRoot))
    return;

  qWarning() << "[ResourceTree] still referenced at teardown; retaining" << m_mapRoot;
  s_retainedTrees.append(std::move(m_data));
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes an RCC v2 tree for the given files; empty on an unsafe or conflicting path.
 */
QByteArray Deploy::ResourceTree::build(const QList<FileEntry>& files)
{
  if (static_cast<quint64>(files.size()) > kMaxFiles)
    return {};

  Builder builder;
  if (!buildNodes(files, builder))
    return {};

  const auto order = breadthFirstOrder(builder.nodes);
  SS_ASSERT(order.size() == builder.nodes.size(), return {});

  QByteArray names;
  QByteArray data;
  QByteArray tree;
  std::vector<quint32> nameOffsets(builder.nodes.size(), 0);
  std::vector<quint32> dataOffsets(builder.nodes.size(), 0);
  writeNames(builder.nodes, order, names, nameOffsets);
  writeData(builder.nodes, order, files, data, dataOffsets);
  writeTree(builder.nodes, order, nameOffsets, dataOffsets, tree);

  const quint32 dataStart  = kRccHeaderBytes;
  const quint32 namesStart = dataStart + static_cast<quint32>(data.size());
  const quint32 treeStart  = namesStart + static_cast<quint32>(names.size());

  QByteArray out;
  out.reserve(static_cast<qsizetype>(treeStart) + tree.size());
  out.append(kRccMagic, kRccMagicBytes);
  appendU32(out, kRccVersion);
  appendU32(out, treeStart);
  appendU32(out, dataStart);
  appendU32(out, namesStart);
  out.append(data);
  out.append(names);
  out.append(tree);
  return out;
}

/**
 * @brief Whether the tree is registered with Qt's resource system.
 */
bool Deploy::ResourceTree::isMounted() const noexcept
{
  return !m_root.isEmpty();
}

/**
 * @brief The ":/ssdeploy-<id>" prefix consumers read through; empty until mounted.
 */
const QString& Deploy::ResourceTree::root() const noexcept
{
  return m_root;
}

/**
 * @brief Builds and registers the tree once under ":/ssdeploy-<id>"; id is a single safe segment.
 */
bool Deploy::ResourceTree::mount(const QList<FileEntry>& files, const QString& id)
{
  SS_ASSERT(m_root.isEmpty(), return false);

  if (!isSafeEntryPath(id) || id.contains(QLatin1Char('/')))
    return false;

  m_data = build(files);
  if (m_data.isEmpty())
    return false;

  const QString root = kMountRootPrefix + id;
  const auto* bytes  = static_cast<const uchar*>(static_cast<const void*>(m_data.constData()));
  if (!QResource::registerResource(bytes, root.mid(1))) {
    m_data.clear();
    return false;
  }

  m_mapRoot = root.mid(1);
  m_root    = root;
  return true;
}

#endif  // BUILD_COMMERCIAL
