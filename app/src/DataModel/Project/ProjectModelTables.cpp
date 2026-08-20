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

#include <algorithm>
#include <QApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Data-table CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a table's full folder-qualified path (the editor handle + script accessor key).
 */
QString DataModel::ProjectModel::tablePathFor(const DataModel::TableDef& table) const
{
  return DataModel::tableFullPath(m_tableFolders, table.parentFolderId, table.name);
}

/**
 * @brief Resolves a table by its full folder-qualified path; returns -1 when not found.
 */
int DataModel::ProjectModel::findTableIndexByPath(const QString& tablePath) const
{
  for (size_t i = 0; i < m_tables.size(); ++i)
    if (tablePathFor(m_tables[i]) == tablePath)
      return static_cast<int>(i);

  return -1;
}

/**
 * @brief Adds a new empty data table with a leaf name unique within @p parentFolderId; returns its
 *        full folder-qualified path.
 */
QString DataModel::ProjectModel::addTable(const QString& name, int parentFolderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Table")};
  if (parentFolderId != -1 && !folderExists(m_tableFolders, parentFolderId))
    parentFolderId = -1;

  QString base = name.simplified().remove(QLatin1Char('/'));
  if (base.isEmpty())
    base = tr("Shared Table");

  QString unique     = base;
  int suffix         = 2;
  const auto hasLeaf = [this, parentFolderId](const QString& n) {
    for (const auto& t : m_tables)
      if (t.parentFolderId == parentFolderId && t.name == n)
        return true;

    return false;
  };

  while (hasLeaf(unique))
    unique = QStringLiteral("%1 %2").arg(base, QString::number(suffix++));

  DataModel::TableDef table;
  table.name           = unique;
  table.parentFolderId = parentFolderId;
  m_tables.push_back(table);
  setModified(true);
  Q_EMIT tablesChanged();
  return tablePathFor(table);
}

/**
 * @brief Deletes the table with the given full path.
 */
void DataModel::ProjectModel::deleteTable(const QString& name)
{
  const ProjectUndoScope undo_scope{*this, tr("Delete Table")};
  const int idx = findTableIndexByPath(name);
  if (idx < 0)
    return;

  m_tables.erase(m_tables.begin() + idx);
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Renames a table's leaf name (no-op if the new leaf collides within the same folder).
 */
void DataModel::ProjectModel::renameTable(const QString& oldName, const QString& newName)
{
  const ProjectUndoScope undo_scope{*this, tr("Rename Table")};
  const QString n = newName.simplified().remove(QLatin1Char('/'));
  if (n.isEmpty())
    return;

  const int idx = findTableIndexByPath(oldName);
  if (idx < 0)
    return;

  const int parent = m_tables[static_cast<size_t>(idx)].parentFolderId;
  for (size_t i = 0; i < m_tables.size(); ++i)
    if (static_cast<int>(i) != idx && m_tables[i].parentFolderId == parent && m_tables[i].name == n)
      return;

  m_tables[static_cast<size_t>(idx)].name = n;
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Pushes a copy of @p src into folder @p parentFolderId with a leaf name unique within it;
 *        the copy keeps its name when free, otherwise takes a nextDuplicateTitle suffix. Registers
 *        are deep-copied. Silent (no signal): the batch caller emits tablesChanged once.
 */
void DataModel::ProjectModel::appendTableCopyToFolder(const DataModel::TableDef& src,
                                                      int parentFolderId)
{
  QStringList taken;
  for (const auto& t : m_tables)
    if (t.parentFolderId == parentFolderId)
      taken.append(t.name);

  DataModel::TableDef copy = src;
  copy.parentFolderId      = parentFolderId;
  copy.name = taken.contains(src.name) ? nextDuplicateTitle(src.name, taken) : src.name;
  m_tables.push_back(std::move(copy));
}

/**
 * @brief Deep-copies the table at @p tablePath into the same folder with a nextDuplicateTitle name.
 */
void DataModel::ProjectModel::duplicateTableByPath(const QString& tablePath)
{
  const int idx = findTableIndexByPath(tablePath);
  if (idx < 0)
    return;

  const DataModel::TableDef src = m_tables[static_cast<size_t>(idx)];

  QStringList taken;
  for (const auto& t : m_tables)
    if (t.parentFolderId == src.parentFolderId)
      taken.append(t.name);

  DataModel::TableDef copy = src;
  copy.name                = nextDuplicateTitle(src.name, taken);
  m_tables.push_back(std::move(copy));

  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Appends a register to @p table with a unique name.
 */
void DataModel::ProjectModel::addRegister(const QString& table,
                                          const QString& registerName,
                                          bool computed,
                                          const QVariant& defaultValue)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Variable")};
  const int idx = findTableIndexByPath(table);
  if (idx < 0)
    return;

  auto it = m_tables.begin() + idx;

  QString base = registerName.simplified();
  if (base.isEmpty())
    base = tr("variable");

  QString unique     = base;
  int suffix         = 2;
  const auto hasName = [it](const QString& n) {
    for (const auto& r : it->registers)
      if (r.name == n)
        return true;

    return false;
  };

  while (hasName(unique))
    unique = QStringLiteral("%1_%2").arg(base, QString::number(suffix++));

  DataModel::RegisterDef reg;
  reg.name         = unique;
  reg.type         = computed ? RegisterType::Computed : RegisterType::Constant;
  reg.defaultValue = defaultValue.isValid() ? defaultValue : QVariant(0.0);
  it->registers.push_back(reg);

  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Removes a register from the specified table.
 */
void DataModel::ProjectModel::deleteRegister(const QString& table, const QString& registerName)
{
  const ProjectUndoScope undo_scope{*this, tr("Delete Variable")};
  const int idx = findTableIndexByPath(table);
  if (idx < 0)
    return;

  auto it = m_tables.begin() + idx;

  auto rit = std::find_if(it->registers.begin(),
                          it->registers.end(),
                          [&registerName](const auto& r) { return r.name == registerName; });

  if (rit == it->registers.end())
    return;

  it->registers.erase(rit);
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Updates an existing register -- rename, retype, and/or default value.
 *        Returns true only when the update was applied and tablesChanged() was
 *        emitted; false on every validation-failure path so callers can key
 *        refresh-suppression off confirmed success rather than emit timing.
 */
bool DataModel::ProjectModel::updateRegister(const QString& table,
                                             const QString& registerName,
                                             const QString& newName,
                                             bool computed,
                                             const QVariant& defaultValue)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Variable")};
  const int idx = findTableIndexByPath(table);
  if (idx < 0)
    return false;

  auto it = m_tables.begin() + idx;

  const QString n = newName.simplified();
  if (n.isEmpty())
    return false;

  if (n != registerName) {
    for (const auto& r : it->registers)
      if (r.name == n)
        return false;
  }

  for (auto& r : it->registers) {
    if (r.name == registerName) {
      r.name         = n;
      r.type         = computed ? RegisterType::Computed : RegisterType::Constant;
      r.defaultValue = defaultValue.isValid() ? defaultValue : r.defaultValue;
      setModified(true);
      Q_EMIT tablesChanged();
      return true;
    }
  }

  return false;
}

/**
 * @brief Returns the register list of a table as a QVariantList for QML.
 */
QVariantList DataModel::ProjectModel::registersForTable(const QString& table) const
{
  QVariantList result;
  const int idx = findTableIndexByPath(table);
  if (idx < 0)
    return result;

  auto it = m_tables.begin() + idx;

  for (const auto& r : it->registers) {
    QVariantMap row;
    row["name"] = r.name;
    row["type"] =
      r.type == RegisterType::Computed ? QStringLiteral("computed") : QStringLiteral("constant");
    row["value"]     = r.defaultValue;
    row["valueType"] = r.defaultValue.typeId() == QMetaType::Double ? QStringLiteral("number")
                                                                    : QStringLiteral("string");
    result.append(row);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// QInputDialog wrappers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a new shared table name and appends it on accept,
 * deferring the tree selection via singleShot(0) so the queued tablesChanged
 * tree rebuild lands before the new row is selected.
 */
void DataModel::ProjectModel::promptAddTable()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Shared Table"), tr("Name:"), QLineEdit::Normal, tr("Shared Table"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const QString added = addTable(name.trimmed());
  QTimer::singleShot(0, this, [added] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectUserTable(added);
  });
}

/**
 * @brief Prompts for a new name for an existing table.
 */
void DataModel::ProjectModel::promptRenameTable(const QString& oldName)
{
  const int idx = findTableIndexByPath(oldName);
  if (idx < 0)
    return;

  const QString currentLeaf = m_tables[static_cast<size_t>(idx)].name;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Table"), tr("Name:"), QLineEdit::Normal, currentLeaf, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == currentLeaf)
    return;

  renameTable(oldName, name.trimmed());
}

/**
 * @brief Prompts for a register name and type, then appends the register with a
 *        zero default (numeric). Users can edit the value inline afterwards.
 */
void DataModel::ProjectModel::promptAddRegister(const QString& table)
{
  if (table.isEmpty())
    return;

  bool okName           = false;
  const QString regName = QInputDialog::getText(nullptr,
                                                tr("New Variable"),
                                                tr("Name:"),
                                                QLineEdit::Normal,
                                                QStringLiteral("variable"),
                                                &okName);

  if (!okName || regName.trimmed().isEmpty())
    return;

  addRegister(table, regName.trimmed(), true, QVariant(0.0));
}

/**
 * @brief Prompts for a new name for an existing register.
 */
void DataModel::ProjectModel::promptRenameRegister(const QString& table,
                                                   const QString& registerName)
{
  if (table.isEmpty() || registerName.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Variable"), tr("Name:"), QLineEdit::Normal, registerName, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == registerName)
    return;

  const int idx = findTableIndexByPath(table);
  if (idx < 0)
    return;

  for (const auto& r : m_tables[static_cast<size_t>(idx)].registers) {
    if (r.name == registerName) {
      (void)updateRegister(
        table, registerName, name.trimmed(), r.type == RegisterType::Computed, r.defaultValue);
      return;
    }
  }
}

/**
 * @brief Asks the user to confirm before deleting a table.
 */
void DataModel::ProjectModel::confirmDeleteTable(const QString& name)
{
  if (name.isEmpty())
    return;

  const int idx      = findTableIndexByPath(name);
  const QString leaf = idx >= 0 ? m_tables[static_cast<size_t>(idx)].name : name;
  int registerCount  = 0;
  if (idx >= 0)
    registerCount = static_cast<int>(m_tables[static_cast<size_t>(idx)].registers.size());

  const QString informative =
    registerCount == 0
      ? tr("This action cannot be undone.")
      : tr("This removes %1 variable(s) along with the table. This action cannot be undone.")
          .arg(registerCount);

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(leaf),
                                                     informative,
                                                     QMessageBox::Warning,
                                                     tr("Delete Table"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteTable(name);
}

/**
 * @brief Asks the user to confirm before deleting a register.
 */
void DataModel::ProjectModel::confirmDeleteRegister(const QString& table,
                                                    const QString& registerName)
{
  if (table.isEmpty() || registerName.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(registerName),
                                                     tr("This action cannot be undone."),
                                                     QMessageBox::Warning,
                                                     tr("Delete Variable"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteRegister(table, registerName);
}

/**
 * @brief Exports a table's registers to a CSV file chosen by the user, with the
 * register permission written as read-write/read-only to match the editor UI.
 */
void DataModel::ProjectModel::exportTableToCsv(const QString& tableName)
{
  const int idx = findTableIndexByPath(tableName);
  if (idx < 0)
    return;

  const auto it = m_tables.begin() + idx;

  static auto& workspaceManager = Misc::WorkspaceManager::instance();

  const auto path = QFileDialog::getSaveFileName(
    nullptr,
    tr("Export Table"),
    QStringLiteral("%1/%2.csv").arg(workspaceManager.path("CSV"), it->name),
    tr("CSV files (*.csv)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);
  out << "name,permissions,value\n";
  for (const auto& reg : it->registers) {
    const auto permissions = (reg.type == DataModel::RegisterType::Computed)
                             ? QStringLiteral("read-write")
                             : QStringLiteral("read-only");

    auto val = reg.defaultValue.toString();
    if (val.contains(',') || val.contains('"') || val.contains('\n'))
      val = QStringLiteral("\"%1\"").arg(val.replace('"', "\"\""));

    out << reg.name << ',' << permissions << ',' << val << '\n';
  }

  file.close();
}

/**
 * @brief Imports registers from a CSV file into an existing table, accepting both the
 * read-write/read-only permission terms and the legacy computed/constant type values.
 */
void DataModel::ProjectModel::importTableFromCsv(const QString& tableName)
{
  const int idx = findTableIndexByPath(tableName);
  if (idx < 0)
    return;

  auto it = m_tables.begin() + idx;

  static auto& workspaceManager = Misc::WorkspaceManager::instance();

  const auto path = QFileDialog::getOpenFileName(
    nullptr, tr("Import Table"), workspaceManager.path("CSV"), tr("CSV files (*.csv)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  const ProjectUndoScope undo_scope{*this, tr("Import Table CSV")};

  QTextStream in(&file);

  if (!in.atEnd())
    in.readLine();

  constexpr int kMaxImportRows = 1'000'000;

  int imported = 0;
  int rowsRead = 0;
  while (!in.atEnd() && rowsRead < kMaxImportRows) {
    ++rowsRead;
    const auto line = in.readLine().trimmed();
    if (line.isEmpty())
      continue;

    const auto parts = line.split(',');
    if (parts.size() < 3)
      continue;

    const auto name     = parts[0].trimmed();
    const auto permsStr = parts[1].trimmed().toLower();
    auto valStr         = parts.mid(2).join(',').trimmed();
    if (valStr.size() >= 2 && valStr.startsWith('"') && valStr.endsWith('"')) {
      valStr = valStr.mid(1, valStr.size() - 2);
      valStr.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }

    if (name.isEmpty())
      continue;

    const bool computed =
      (permsStr == QStringLiteral("read-write")) || (permsStr == QStringLiteral("read/write"))
      || (permsStr == QStringLiteral("rw")) || (permsStr == QStringLiteral("computed"));

    bool isNumeric              = false;
    const double dval           = SerialStudio::toDouble(valStr, &isNumeric);
    const QVariant defaultValue = isNumeric ? QVariant(dval) : QVariant(valStr);

    auto regIt = std::find_if(it->registers.begin(),
                              it->registers.end(),
                              [&](const DataModel::RegisterDef& r) { return r.name == name; });

    if (regIt != it->registers.end()) {
      regIt->type =
        computed ? DataModel::RegisterType::Computed : DataModel::RegisterType::Constant;
      regIt->defaultValue = defaultValue;
    } else {
      DataModel::RegisterDef reg;
      reg.name = name;
      reg.type = computed ? DataModel::RegisterType::Computed : DataModel::RegisterType::Constant;
      reg.defaultValue = defaultValue;
      it->registers.push_back(reg);
    }

    ++imported;
  }

  file.close();

  if (imported > 0) {
    setModified(true);
    Q_EMIT tablesChanged();
  }
}
