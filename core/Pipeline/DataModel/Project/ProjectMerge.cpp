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

#include "DataModel/Project/ProjectMerge.h"

#include <QJsonArray>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The double-quoted Lua literal the importers emit for a table name, so a rename can be
 *        applied to generated code by exact string replacement.
 */
[[nodiscard]] static QString quotedLua(const QString& text)
{
  QString escaped = text;
  escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"))
    .replace(QLatin1Char('"'), QLatin1String("\\\""))
    .replace(QLatin1Char('\n'), QLatin1String("\\n"));

  return QLatin1Char('"') + escaped + QLatin1Char('"');
}

/**
 * @brief Rewrites every renamed table's quoted literal inside one script.
 */
static void applyTableRenames(QString& code, const QMap<QString, QString>& renames)
{
  for (auto it = renames.constBegin(); it != renames.constEnd(); ++it)
    code.replace(quotedLua(it.key()), quotedLua(it.value()));
}

/**
 * @brief Reads the imported sources and assigns each a free sourceId.
 */
static void remapSources(const QJsonObject& imported,
                         const DataModel::ProjectMerge::Base& base,
                         DataModel::ProjectMerge::Result& out,
                         QMap<int, int>& sourceMap)
{
  const auto sources = imported.value(Keys::Sources).toArray();
  for (qsizetype i = 0; i < sources.size(); ++i) {
    DataModel::Source source;
    if (!DataModel::read(source, sources.at(i).toObject()))
      continue;

    const int newId = base.nextSourceId + static_cast<int>(out.sources.size());
    sourceMap.insert(source.sourceId, newId);
    source.sourceId = newId;
    out.sources.push_back(std::move(source));
  }
}

/**
 * @brief Reads the imported groups, allocating fresh group and dataset uniqueIds and pointing every
 *        entity at its remapped source.
 */
static void remapGroups(const QJsonObject& imported,
                        const DataModel::ProjectMerge::Base& base,
                        const QMap<int, int>& sourceMap,
                        DataModel::ProjectMerge::Result& out,
                        QMap<int, int>& groupMap,
                        QMap<int, int>& datasetMap)
{
  const auto groups = imported.value(Keys::Groups).toArray();
  for (qsizetype g = 0; g < groups.size(); ++g) {
    DataModel::Group group;
    if (!DataModel::read(group, groups.at(g).toObject()))
      continue;

    const int position = base.groupCount + static_cast<int>(out.groups.size());
    groupMap.insert(group.uniqueId, out.nextUniqueId);
    group.uniqueId = out.nextUniqueId++;
    group.groupId  = position;
    group.sourceId = sourceMap.value(group.sourceId, group.sourceId);

    for (size_t d = 0; d < group.datasets.size(); ++d) {
      auto& dataset = group.datasets[d];
      if (dataset.uniqueId >= 0)
        datasetMap.insert(dataset.uniqueId, out.nextUniqueId);

      dataset.uniqueId  = out.nextUniqueId++;
      dataset.groupId   = position;
      dataset.datasetId = static_cast<int>(d);
      dataset.sourceId  = group.sourceId;
    }

    for (auto& widget : group.outputWidgets) {
      widget.groupId  = position;
      widget.sourceId = sourceMap.value(widget.sourceId, widget.sourceId);
    }

    out.groups.push_back(std::move(group));
  }
}

/**
 * @brief Reads the imported tables, renaming any whose name the document already uses.
 */
static void remapTables(const QJsonObject& imported,
                        const DataModel::ProjectMerge::Base& base,
                        const QString& label,
                        DataModel::ProjectMerge::Result& out)
{
  QSet<QString> taken = base.tableNames;
  const auto tables   = imported.value(Keys::Tables).toArray();
  for (const auto& value : tables) {
    DataModel::TableDef table;
    if (!DataModel::read(table, value.toObject()))
      continue;

    const QString name = DataModel::ProjectMerge::freeName(table.name, taken, label);
    if (name != table.name)
      out.renamedTables.insert(table.name, name);

    table.name           = name;
    table.parentFolderId = -1;
    taken.insert(name);
    out.tables.push_back(std::move(table));
  }
}

/**
 * @brief Reads the imported workspaces onto free ids and titles; refs keep only their identity,
 *        the ordinal is rebound by the workspace rebind once the groups are in the document.
 */
static void remapWorkspaces(const QJsonObject& imported,
                            const DataModel::ProjectMerge::Base& base,
                            const QString& label,
                            const QMap<int, int>& groupMap,
                            const QMap<int, int>& datasetMap,
                            DataModel::ProjectMerge::Result& out)
{
  QSet<QString> titles     = base.workspaceTitles;
  const auto workspaces    = imported.value(Keys::Workspaces).toArray();
  const int firstWorkspace = qMax(base.nextWorkspaceId, WorkspaceIds::UserStart);
  for (const auto& value : workspaces) {
    DataModel::Workspace workspace;
    if (!DataModel::read(workspace, value.toObject()))
      continue;

    workspace.workspaceId    = firstWorkspace + static_cast<int>(out.workspaces.size());
    workspace.parentFolderId = -1;
    workspace.title          = DataModel::ProjectMerge::freeName(workspace.title, titles, label);
    titles.insert(workspace.title);

    for (auto& ref : workspace.widgetRefs) {
      ref.groupUniqueId   = groupMap.value(ref.groupUniqueId, -1);
      ref.datasetUniqueId = datasetMap.value(ref.datasetUniqueId, -1);
      ref.relativeIndex   = -1;
    }

    out.workspaces.push_back(std::move(workspace));
  }
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @p wanted when free, else "<wanted> (<label>)", then numbered variants.
 */
QString DataModel::ProjectMerge::freeName(const QString& wanted,
                                          const QSet<QString>& taken,
                                          const QString& label)
{
  if (!taken.contains(wanted))
    return wanted;

  const QString suffixed = QStringLiteral("%1 (%2)").arg(wanted, label);
  if (!taken.contains(suffixed))
    return suffixed;

  constexpr int kMaxAttempts = 1000;
  for (int n = 2; n < kMaxAttempts; ++n) {
    const QString numbered = QStringLiteral("%1 (%2 %3)").arg(wanted, label, QString::number(n));
    if (!taken.contains(numbered))
      return numbered;
  }

  SS_ASSERT_LOG(false);
  return suffixed;
}

/**
 * @brief Rewrites the imported project against @p base; see the header. Table renames are also
 *        applied to the imported parser and transform code, which reference tables by the quoted
 *        literal the importers generate.
 */
DataModel::ProjectMerge::Result DataModel::ProjectMerge::remap(const QJsonObject& imported,
                                                               const Base& base,
                                                               const QString& label)
{
  SS_ASSERT_LOG(base.nextUniqueId >= 1);
  SS_ASSERT_LOG(base.groupCount >= 0);

  Result out;
  out.nextUniqueId = base.nextUniqueId;

  QMap<int, int> sourceMap;
  QMap<int, int> groupMap;
  QMap<int, int> datasetMap;
  remapSources(imported, base, out, sourceMap);
  remapGroups(imported, base, sourceMap, out, groupMap, datasetMap);
  remapTables(imported, base, label, out);
  remapWorkspaces(imported, base, label, groupMap, datasetMap, out);

  if (out.renamedTables.isEmpty())
    return out;

  for (auto& source : out.sources)
    applyTableRenames(source.frameParserCode, out.renamedTables);

  for (auto& group : out.groups)
    for (auto& dataset : group.datasets)
      applyTableRenames(dataset.transformCode, out.renamedTables);

  return out;
}
