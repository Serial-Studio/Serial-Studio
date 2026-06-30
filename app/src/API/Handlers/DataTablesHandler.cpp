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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/DataTablesHandler.h"

#include <QJsonArray>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Extracts a required non-empty string param or returns an error response.
 */
[[nodiscard]] static bool requireString(const QJsonObject& params, const QString& key, QString& out)
{
  if (!params.contains(key))
    return false;

  out = params.value(key).toString();
  return !out.isEmpty();
}

/**
 * @brief Converts a QJsonValue to a QVariant preserving numeric vs string type.
 */
[[nodiscard]] static QVariant jsonToVariant(const QJsonValue& v)
{
  if (v.isDouble())
    return QVariant(SerialStudio::toDouble(v));

  if (v.isBool())
    return QVariant(v.toBool() ? 1.0 : 0.0);

  if (v.isString())
    return QVariant(v.toString());

  return QVariant(0.0);
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the property descriptor for a table register's defaultValue.
 */
[[nodiscard]] static QJsonObject defaultValueProp()
{
  return QJsonObject{
    {       QStringLiteral("type"), QJsonArray{QStringLiteral("number"), QStringLiteral("string")}},
    {QStringLiteral("description"),             QStringLiteral("Default value (number or string)")}
  };
}

/**
 * @brief Register all data-table commands with the command registry.
 */
void API::Handlers::DataTablesHandler::registerCommands()
{
  registerTableQueryCommands();
  registerTableMutationCommands();
  registerRegisterCommands();
}

/**
 * @brief Register read-only table query commands.
 */
void API::Handlers::DataTablesHandler::registerTableQueryCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataTable.list"),
    QStringLiteral("List all user-defined data tables. Each entry has name (bare), path (full "
                   "\"/\"-joined folder path), and registerCount. Pass path to get/getValue/handle "
                   "for a table that lives inside folders."),
    API::emptySchema(),
    &tablesList);
  registry.registerCommand(
    QStringLiteral("project.dataTable.get"),
    QStringLiteral("Return the register list for a table (params: name). name may be the bare "
                   "table name or its full \"/\"-joined folder path."),
    API::makeSchema({
      {QStringLiteral("name"),
       QStringLiteral("string"),
       QStringLiteral("Table name or full folder path")}
  }),
    &tableGet);
  registry.registerCommand(
    QStringLiteral("project.dataTable.getValue"),
    QStringLiteral("Return a register's LIVE runtime value from the data-table store (params: "
                   "table, name). This is the same store the parser/transform tableGet() reads; "
                   "control scripts use it through the tableGet() global."),
    API::makeSchema({
      {QStringLiteral("table"),
       QStringLiteral("string"),
       QStringLiteral("Owning table: full \"/\"-joined folder path (bare name if top-level)")                         },
      { QStringLiteral("name"), QStringLiteral("string"),                              QStringLiteral("Register name")}
  }),
    &valueGet);
  registry.registerCommand(
    QStringLiteral("project.dataTable.handle"),
    QStringLiteral("Resolve a register to a reusable numeric handle for the fast get/set path "
                   "(params: table, name). Returns handle=-1 for an unknown register. Mirrors the "
                   "parser/transform tableHandle() global."),
    API::makeSchema({
      {QStringLiteral("table"),
       QStringLiteral("string"),
       QStringLiteral("Owning table: full \"/\"-joined folder path (bare name if top-level)")                         },
      { QStringLiteral("name"), QStringLiteral("string"),                              QStringLiteral("Register name")}
  }),
    &valueHandle);
  registry.registerCommand(
    QStringLiteral("project.dataTable.getValueH"),
    QStringLiteral("Return a register's LIVE runtime value by handle (params: handle). A stale or "
                   "invalid handle yields found=false. Mirrors tableGetH()."),
    API::makeSchema({
      {QStringLiteral("handle"),
       QStringLiteral("number"),
       QStringLiteral("Handle from project.dataTable.handle")}
  }),
    &valueGetH);
}

/**
 * @brief Register table create/delete/rename mutation commands.
 */
void API::Handlers::DataTablesHandler::registerTableMutationCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataTable.add"),
    QStringLiteral("Create a new empty table (params: name=\"Shared Table\")"),
    API::makeSchema(
      {
  },
      {{QStringLiteral("name"),
        QStringLiteral("string"),
        QStringLiteral("Desired table name (uniquified on collision)")}}),
    &tableAdd);
  registry.registerCommand(
    QStringLiteral("project.dataTable.delete"),
    QStringLiteral("Delete a table and all its registers (params: name). Pass dryRun:true to "
                   "return the register list that WOULD be deleted without committing."),
    API::makeSchema(
      {
        {QStringLiteral("name"), QStringLiteral("string"), QStringLiteral("Table name")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return what would be deleted without committing.")}}),
    &tableDelete);
  registry.registerCommand(
    QStringLiteral("project.dataTable.rename"),
    QStringLiteral("Rename a table (params: oldName, newName)"),
    API::makeSchema({
      {QStringLiteral("oldName"), QStringLiteral("string"), QStringLiteral("Current table name")},
      {QStringLiteral("newName"), QStringLiteral("string"),     QStringLiteral("New table name")}
  }),
    &tableRename);

  QJsonObject valueProps;
  valueProps[QStringLiteral("table")] = QJsonObject{
    {       QStringLiteral("type"),            QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Owning table name")}
  };
  valueProps[QStringLiteral("name")] = QJsonObject{
    {       QStringLiteral("type"),        QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Register name")}
  };
  valueProps[QStringLiteral("value")] = QJsonObject{
    {       QStringLiteral("type"), QJsonArray{QStringLiteral("number"), QStringLiteral("string")}},
    {QStringLiteral("description"),         QStringLiteral("New runtime value (number or string)")}
  };
  QJsonObject valueSchema;
  valueSchema[QStringLiteral("type")]       = QStringLiteral("object");
  valueSchema[QStringLiteral("properties")] = valueProps;
  valueSchema[QStringLiteral("required")] =
    QJsonArray{QStringLiteral("table"), QStringLiteral("name"), QStringLiteral("value")};
  registry.registerCommand(
    QStringLiteral("project.dataTable.setValue"),
    QStringLiteral("Write a register's LIVE runtime value into the data-table store (params: "
                   "table, name, value). Same effect as a parser/transform tableSet(); control "
                   "scripts use it through the tableSet() global. Constant registers reject "
                   "writes."),
    valueSchema,
    &valueSet);

  QJsonObject valueHProps;
  valueHProps[QStringLiteral("handle")] = QJsonObject{
    {       QStringLiteral("type"),                               QStringLiteral("number")},
    {QStringLiteral("description"), QStringLiteral("Handle from project.dataTable.handle")}
  };
  valueHProps[QStringLiteral("value")] = QJsonObject{
    {       QStringLiteral("type"), QJsonArray{QStringLiteral("number"), QStringLiteral("string")}},
    {QStringLiteral("description"),         QStringLiteral("New runtime value (number or string)")}
  };
  QJsonObject valueHSchema;
  valueHSchema[QStringLiteral("type")]       = QStringLiteral("object");
  valueHSchema[QStringLiteral("properties")] = valueHProps;
  valueHSchema[QStringLiteral("required")] =
    QJsonArray{QStringLiteral("handle"), QStringLiteral("value")};
  registry.registerCommand(
    QStringLiteral("project.dataTable.setValueH"),
    QStringLiteral("Write a register's LIVE runtime value by handle (params: handle, value). A "
                   "stale, invalid, or constant-register handle yields written=false. Mirrors "
                   "tableSetH()."),
    valueHSchema,
    &valueSetH);
}

/**
 * @brief Register register-level add/delete/update commands.
 */
void API::Handlers::DataTablesHandler::registerRegisterCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject addProps;
  addProps[QStringLiteral("table")] = QJsonObject{
    {       QStringLiteral("type"),            QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Owning table name")}
  };
  addProps[QStringLiteral("name")] = QJsonObject{
    {       QStringLiteral("type"),                          QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Register name (auto-uniquified)")}
  };
  addProps[QStringLiteral("computed")] = QJsonObject{
    {       QStringLiteral("type"),QStringLiteral("boolean")},
    {QStringLiteral("description"),
     QStringLiteral("true=computed (writable by transforms, persists across frames), "
     "false=constant")    }
  };
  addProps[QStringLiteral("defaultValue")] = defaultValueProp();
  QJsonObject addSchema;
  addSchema[QStringLiteral("type")]       = QStringLiteral("object");
  addSchema[QStringLiteral("properties")] = addProps;
  addSchema[QStringLiteral("required")] =
    QJsonArray{QStringLiteral("table"), QStringLiteral("name")};
  registry.registerCommand(
    QStringLiteral("project.dataTable.addRegister"),
    QStringLiteral("Append a register (params: table, name, computed=true, defaultValue=0)"),
    addSchema,
    &registerAdd);

  registry.registerCommand(
    QStringLiteral("project.dataTable.deleteRegister"),
    QStringLiteral("Delete a register (params: table, name). Pass dryRun:true to return what "
                   "WOULD be deleted without committing."),
    API::makeSchema(
      {
        {QStringLiteral("table"), QStringLiteral("string"), QStringLiteral("Owning table name")},
        { QStringLiteral("name"), QStringLiteral("string"),     QStringLiteral("Register name")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return what would be deleted without committing.")}}),
    &registerDelete);

  QJsonObject updProps;
  updProps[QStringLiteral("table")] = QJsonObject{
    {       QStringLiteral("type"),            QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Owning table name")}
  };
  updProps[QStringLiteral("name")] = QJsonObject{
    {       QStringLiteral("type"),                QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Current register name")}
  };
  updProps[QStringLiteral("newName")] = QJsonObject{
    {       QStringLiteral("type"),                       QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("New register name (optional)")}
  };
  updProps[QStringLiteral("computed")] = QJsonObject{
    {       QStringLiteral("type"),                               QStringLiteral("boolean")},
    {QStringLiteral("description"), QStringLiteral("Switch to computed (true) or constant")}
  };
  updProps[QStringLiteral("defaultValue")] = defaultValueProp();
  QJsonObject updSchema;
  updSchema[QStringLiteral("type")]       = QStringLiteral("object");
  updSchema[QStringLiteral("properties")] = updProps;
  updSchema[QStringLiteral("required")] =
    QJsonArray{QStringLiteral("table"), QStringLiteral("name")};
  registry.registerCommand(
    QStringLiteral("project.dataTable.updateRegister"),
    QStringLiteral("Update a register (params: table, name, newName?, computed?, defaultValue?)"),
    updSchema,
    &registerUpdate);
}

//--------------------------------------------------------------------------------------------------
// Table queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns every user-defined table with its register count.
 */
API::CommandResponse API::Handlers::DataTablesHandler::tablesList(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& tables  = DataModel::ProjectModel::instance().tables();
  const auto& folders = DataModel::ProjectModel::instance().editorTableFolders();

  QJsonArray arr;
  for (const auto& t : tables) {
    QJsonObject row;
    row[QStringLiteral("name")] = t.name;
    row[QStringLiteral("path")] = DataModel::tableFullPath(folders, t.parentFolderId, t.name);
    row[QStringLiteral("registerCount")] = static_cast<int>(t.registers.size());
    arr.append(row);
  }

  QJsonObject result;
  result[QStringLiteral("tables")] = arr;
  result[QStringLiteral("count")]  = static_cast<int>(tables.size());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the register list for a single table.
 */
API::CommandResponse API::Handlers::DataTablesHandler::tableGet(const QString& id,
                                                                const QJsonObject& params)
{
  QString name;
  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  const auto& tables  = DataModel::ProjectModel::instance().tables();
  const auto& folders = DataModel::ProjectModel::instance().editorTableFolders();

  QString path;
  for (const auto& t : tables) {
    const QString full = DataModel::tableFullPath(folders, t.parentFolderId, t.name);
    if (full == name || t.name == name) {
      path = full;
      break;
    }
  }

  if (path.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Table not found: %1").arg(name));

  const auto registers = DataModel::ProjectModel::instance().registersForTable(path);

  QJsonArray arr = QJsonArray::fromVariantList(registers);

  QJsonObject result;
  result[QStringLiteral("name")]      = name;
  result[QStringLiteral("path")]      = path;
  result[QStringLiteral("registers")] = arr;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Table mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new table with the given (optional) name.
 */
API::CommandResponse API::Handlers::DataTablesHandler::tableAdd(const QString& id,
                                                                const QJsonObject& params)
{
  const QString desired =
    params.value(QStringLiteral("name")).toString(QStringLiteral("Shared Table"));

  const QString actual = DataModel::ProjectModel::instance().addTable(desired);

  QJsonObject result;
  result[QStringLiteral("name")]  = actual;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Deletes a table by name. With dryRun:true, reports what would be deleted without
 *        mutating; the delete itself is a no-op if the table is not found.
 */
API::CommandResponse API::Handlers::DataTablesHandler::tableDelete(const QString& id,
                                                                   const QJsonObject& params)
{
  QString name;
  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& tables = pm.tables();
  const auto it =
    std::find_if(tables.begin(), tables.end(), [&name](const auto& t) { return t.name == name; });

  if (it == tables.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Table not found: %1").arg(name));

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  QJsonArray registers;
  for (const auto& r : it->registers)
    registers.append(r.name);

  QJsonObject result;
  result[QStringLiteral("name")]          = name;
  result[QStringLiteral("registerCount")] = static_cast<int>(it->registers.size());
  result[QStringLiteral("registers")]     = registers;

  if (isDryRun) {
    result[QStringLiteral("dryRun")]  = true;
    result[QStringLiteral("deleted")] = false;
    result[QStringLiteral("warning")] = QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to delete the table "
      "and all its registers.");
    return CommandResponse::makeSuccess(id, result);
  }

  pm.deleteTable(name);
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Renames a table. Returns an error if the target name is taken.
 */
API::CommandResponse API::Handlers::DataTablesHandler::tableRename(const QString& id,
                                                                   const QJsonObject& params)
{
  QString oldName;
  QString newName;
  if (!requireString(params, QStringLiteral("oldName"), oldName))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: oldName"));

  if (!requireString(params, QStringLiteral("newName"), newName))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: newName"));

  auto& pm = DataModel::ProjectModel::instance();

  const auto& preTables = pm.tables();
  const bool hasOld     = std::any_of(
    preTables.begin(), preTables.end(), [&oldName](const auto& t) { return t.name == oldName; });
  const bool hasNew = std::any_of(
    preTables.begin(), preTables.end(), [&newName](const auto& t) { return t.name == newName; });
  const bool collides = hasNew && (oldName != newName);

  bool applied = false;
  if (hasOld && !collides) {
    pm.renameTable(oldName, newName);
    const auto& tables = pm.tables();
    applied            = std::any_of(
      tables.begin(), tables.end(), [&newName](const auto& t) { return t.name == newName; });
  }

  QJsonObject result;
  result[QStringLiteral("oldName")] = oldName;
  result[QStringLiteral("newName")] = newName;
  result[QStringLiteral("renamed")] = applied;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Register mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a register to a table.
 */
API::CommandResponse API::Handlers::DataTablesHandler::registerAdd(const QString& id,
                                                                   const QJsonObject& params)
{
  QString table;
  QString name;
  if (!requireString(params, QStringLiteral("table"), table))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: table"));

  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  const bool computed = params.value(QStringLiteral("computed")).toBool(true);

  QVariant defaultValue(0.0);
  if (params.contains(QStringLiteral("defaultValue")))
    defaultValue = jsonToVariant(params.value(QStringLiteral("defaultValue")));
  else if (params.contains(QStringLiteral("value")))
    defaultValue = jsonToVariant(params.value(QStringLiteral("value")));

  DataModel::ProjectModel::instance().addRegister(table, name, computed, defaultValue);

  QJsonObject result;
  result[QStringLiteral("table")]    = table;
  result[QStringLiteral("name")]     = name;
  result[QStringLiteral("computed")] = computed;
  result[QStringLiteral("added")]    = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Removes a register from a table. With dryRun:true, reports what would be deleted
 *        without mutating.
 */
API::CommandResponse API::Handlers::DataTablesHandler::registerDelete(const QString& id,
                                                                      const QJsonObject& params)
{
  QString table;
  QString name;
  if (!requireString(params, QStringLiteral("table"), table))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: table"));

  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& tables = pm.tables();
  const auto tit =
    std::find_if(tables.begin(), tables.end(), [&table](const auto& t) { return t.name == table; });

  if (tit == tables.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Table not found: %1").arg(table));

  const auto rit = std::find_if(tit->registers.begin(),
                                tit->registers.end(),
                                [&name](const auto& r) { return r.name == name; });

  if (rit == tit->registers.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Register not found: %1/%2").arg(table, name));

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  QJsonObject result;
  result[QStringLiteral("table")]    = table;
  result[QStringLiteral("name")]     = name;
  result[QStringLiteral("computed")] = (rit->type == DataModel::RegisterType::Computed);

  if (isDryRun) {
    result[QStringLiteral("dryRun")]  = true;
    result[QStringLiteral("deleted")] = false;
    result[QStringLiteral("warning")] = QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to delete the register.");
    return CommandResponse::makeSuccess(id, result);
  }

  pm.deleteRegister(table, name);
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Updates an existing register. Unspecified fields retain their value.
 */
API::CommandResponse API::Handlers::DataTablesHandler::registerUpdate(const QString& id,
                                                                      const QJsonObject& params)
{
  QString table;
  QString name;
  if (!requireString(params, QStringLiteral("table"), table))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: table"));

  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  const auto& tables = DataModel::ProjectModel::instance().tables();
  const auto tit =
    std::find_if(tables.begin(), tables.end(), [&table](const auto& t) { return t.name == table; });

  if (tit == tables.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Table not found: %1").arg(table));

  const auto rit = std::find_if(tit->registers.begin(),
                                tit->registers.end(),
                                [&name](const auto& r) { return r.name == name; });

  if (rit == tit->registers.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Register not found: %1/%2").arg(table, name));

  const QString newName = params.contains(QStringLiteral("newName"))
                          ? params.value(QStringLiteral("newName")).toString()
                          : rit->name;

  const bool computed = params.contains(QStringLiteral("computed"))
                        ? params.value(QStringLiteral("computed")).toBool()
                        : (rit->type == DataModel::RegisterType::Computed);

  QVariant defaultValue = rit->defaultValue;
  if (params.contains(QStringLiteral("defaultValue")))
    defaultValue = jsonToVariant(params.value(QStringLiteral("defaultValue")));
  else if (params.contains(QStringLiteral("value")))
    defaultValue = jsonToVariant(params.value(QStringLiteral("value")));

  (void)DataModel::ProjectModel::instance().updateRegister(
    table, name, newName, computed, defaultValue);

  QJsonObject result;
  result[QStringLiteral("table")]    = table;
  result[QStringLiteral("name")]     = newName;
  result[QStringLiteral("computed")] = computed;
  result[QStringLiteral("updated")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Runtime register values (FrameBuilder data-table store)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a register's live runtime value from the data-table store, mirroring the
 *        parser/transform tableGet() (control scripts reach the store through this command).
 */
API::CommandResponse API::Handlers::DataTablesHandler::valueGet(const QString& id,
                                                                const QJsonObject& params)
{
  QString table;
  QString name;
  if (!requireString(params, QStringLiteral("table"), table))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: table"));

  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  const auto& store = DataModel::FrameBuilder::instance().tableStore();
  if (!store.isInitialized())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Data-table store not initialized (no project)"));

  const auto* value = store.get(table, name);
  if (!value)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Unknown table or register: %1/%2").arg(table, name));

  QJsonObject result;
  result[QStringLiteral("table")]     = table;
  result[QStringLiteral("name")]      = name;
  result[QStringLiteral("isNumeric")] = value->isNumeric;
  result[QStringLiteral("value")] =
    value->isNumeric ? QJsonValue(value->numericValue) : QJsonValue(value->stringValue);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Writes a register's live runtime value into the data-table store, mirroring the
 *        parser/transform tableSet() value semantics (numeric when parseable, string otherwise).
 */
API::CommandResponse API::Handlers::DataTablesHandler::valueSet(const QString& id,
                                                                const QJsonObject& params)
{
  QString table;
  QString name;
  if (!requireString(params, QStringLiteral("table"), table))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: table"));

  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  if (!params.contains(QStringLiteral("value")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: value"));

  const QVariant v = jsonToVariant(params.value(QStringLiteral("value")));

  DataModel::RegisterValue rv;
  rv.numericValue = SerialStudio::toDouble(v, &rv.isNumeric);
  if (!rv.isNumeric)
    rv.stringValue = v.toString();

  auto& store = DataModel::FrameBuilder::instance().tableStore();
  if (!store.isInitialized())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Data-table store not initialized (no project)"));

  if (!store.set(table, name, rv))
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Cannot write register %1/%2 (unknown, or a constant register)")
        .arg(table, name));

  QJsonObject result;
  result[QStringLiteral("table")]   = table;
  result[QStringLiteral("name")]    = name;
  result[QStringLiteral("written")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Resolves a (table, register) pair to a reusable handle for the marshalled fast path.
 */
API::CommandResponse API::Handlers::DataTablesHandler::valueHandle(const QString& id,
                                                                   const QJsonObject& params)
{
  QString table;
  QString name;
  if (!requireString(params, QStringLiteral("table"), table))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: table"));

  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  const auto& store = DataModel::FrameBuilder::instance().tableStore();
  if (!store.isInitialized())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Data-table store not initialized (no project)"));

  const qint64 handle = store.handleOf(table, name);

  QJsonObject result;
  result[QStringLiteral("table")]  = table;
  result[QStringLiteral("name")]   = name;
  result[QStringLiteral("handle")] = static_cast<double>(handle);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns a register's live value by handle; found=false for a stale or invalid handle.
 */
API::CommandResponse API::Handlers::DataTablesHandler::valueGetH(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("handle")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: handle"));

  const qint64 handle =
    static_cast<qint64>(SerialStudio::toDouble(params.value(QStringLiteral("handle"))));

  const auto& store = DataModel::FrameBuilder::instance().tableStore();
  if (!store.isInitialized())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Data-table store not initialized (no project)"));

  const auto* value = store.getByHandle(handle);

  QJsonObject result;
  result[QStringLiteral("handle")] = static_cast<double>(handle);
  result[QStringLiteral("found")]  = (value != nullptr);
  result[QStringLiteral("value")]  = QJsonValue();
  if (value) {
    result[QStringLiteral("isNumeric")] = value->isNumeric;
    result[QStringLiteral("value")] =
      value->isNumeric ? QJsonValue(value->numericValue) : QJsonValue(value->stringValue);
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Writes a register's live value by handle; written=false for stale/invalid/constant.
 */
API::CommandResponse API::Handlers::DataTablesHandler::valueSetH(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("handle")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: handle"));

  if (!params.contains(QStringLiteral("value")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: value"));

  const qint64 handle =
    static_cast<qint64>(SerialStudio::toDouble(params.value(QStringLiteral("handle"))));
  const QVariant v = jsonToVariant(params.value(QStringLiteral("value")));

  DataModel::RegisterValue rv;
  rv.numericValue = SerialStudio::toDouble(v, &rv.isNumeric);
  if (!rv.isNumeric)
    rv.stringValue = v.toString();

  auto& store = DataModel::FrameBuilder::instance().tableStore();
  if (!store.isInitialized())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Data-table store not initialized (no project)"));

  const bool written = store.setByHandle(handle, rv);

  QJsonObject result;
  result[QStringLiteral("handle")]  = static_cast<double>(handle);
  result[QStringLiteral("written")] = written;
  return CommandResponse::makeSuccess(id, result);
}
