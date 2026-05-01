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
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Extracts a required non-empty string param or returns an error response.
 */
[[nodiscard]] bool requireString(const QJsonObject& params, const QString& key, QString& out)
{
  if (!params.contains(key))
    return false;

  out = params.value(key).toString();
  return !out.isEmpty();
}

/**
 * @brief Converts a QJsonValue to a QVariant preserving numeric vs string type.
 */
[[nodiscard]] QVariant jsonToVariant(const QJsonValue& v)
{
  if (v.isDouble())
    return QVariant(v.toDouble());

  if (v.isBool())
    return QVariant(v.toBool() ? 1.0 : 0.0);

  if (v.isString())
    return QVariant(v.toString());

  return QVariant(0.0);
}

}  // anonymous namespace

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all data-table commands with the command registry.
 */
void API::Handlers::DataTablesHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Parameterless schema for simple list queries
  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  registry.registerCommand(QStringLiteral("project.tables.list"),
                           QStringLiteral("List all user-defined data tables"),
                           emptySchema,
                           &tablesList);

  // project.tables.get -- name required
  {
    QJsonObject props;
    props[QStringLiteral("name")] = QJsonObject{
      {       QStringLiteral("type"),     QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Table name")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("name")};
    registry.registerCommand(QStringLiteral("project.tables.get"),
                             QStringLiteral("Return the register list for a table (params: name)"),
                             schema,
                             &tableGet);
  }

  // project.tables.add -- optional name (auto-uniquified)
  {
    QJsonObject props;
    props[QStringLiteral("name")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("string")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Desired table name (uniquified on collision)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    registry.registerCommand(
      QStringLiteral("project.tables.add"),
      QStringLiteral("Create a new empty table (params: name=\"Shared Table\")"),
      schema,
      &tableAdd);
  }

  // project.tables.delete
  {
    QJsonObject props;
    props[QStringLiteral("name")] = QJsonObject{
      {       QStringLiteral("type"),     QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Table name")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("name")};
    registry.registerCommand(QStringLiteral("project.tables.delete"),
                             QStringLiteral("Delete a table (params: name)"),
                             schema,
                             &tableDelete);
  }

  // project.tables.rename
  {
    QJsonObject props;
    props[QStringLiteral("oldName")] = QJsonObject{
      {       QStringLiteral("type"),             QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Current table name")}
    };
    props[QStringLiteral("newName")] = QJsonObject{
      {       QStringLiteral("type"),         QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("New table name")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("oldName"), QStringLiteral("newName")};
    registry.registerCommand(QStringLiteral("project.tables.rename"),
                             QStringLiteral("Rename a table (params: oldName, newName)"),
                             schema,
                             &tableRename);
  }

  // project.tables.register.add
  {
    QJsonObject props;
    props[QStringLiteral("table")] = QJsonObject{
      {       QStringLiteral("type"),            QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Owning table name")}
    };
    props[QStringLiteral("name")] = QJsonObject{
      {       QStringLiteral("type"),                          QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Register name (auto-uniquified)")}
    };
    props[QStringLiteral("computed")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("boolean")                                     },
      {QStringLiteral("description"),
       QStringLiteral("true=computed (writable by transforms), false=constant")}
    };
    props[QStringLiteral("defaultValue")] = QJsonObject{
      {       QStringLiteral("type"), QJsonArray{QStringLiteral("number"), QStringLiteral("string")}},
      {QStringLiteral("description"),             QStringLiteral("Default value (number or string)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("table"), QStringLiteral("name")};
    registry.registerCommand(
      QStringLiteral("project.tables.register.add"),
      QStringLiteral("Append a register (params: table, name, computed=true, defaultValue=0)"),
      schema,
      &registerAdd);
  }

  // project.tables.register.delete
  {
    QJsonObject props;
    props[QStringLiteral("table")] = QJsonObject{
      {       QStringLiteral("type"),            QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Owning table name")}
    };
    props[QStringLiteral("name")] = QJsonObject{
      {       QStringLiteral("type"),        QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Register name")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("table"), QStringLiteral("name")};
    registry.registerCommand(QStringLiteral("project.tables.register.delete"),
                             QStringLiteral("Delete a register (params: table, name)"),
                             schema,
                             &registerDelete);
  }

  // project.tables.register.update
  {
    QJsonObject props;
    props[QStringLiteral("table")] = QJsonObject{
      {       QStringLiteral("type"),            QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Owning table name")}
    };
    props[QStringLiteral("name")] = QJsonObject{
      {       QStringLiteral("type"),                QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Current register name")}
    };
    props[QStringLiteral("newName")] = QJsonObject{
      {       QStringLiteral("type"),                       QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("New register name (optional)")}
    };
    props[QStringLiteral("computed")] = QJsonObject{
      {       QStringLiteral("type"),                               QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Switch to computed (true) or constant")}
    };
    props[QStringLiteral("defaultValue")] = QJsonObject{
      {       QStringLiteral("type"), QJsonArray{QStringLiteral("number"), QStringLiteral("string")}},
      {QStringLiteral("description"),             QStringLiteral("Default value (number or string)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("table"), QStringLiteral("name")};
    registry.registerCommand(
      QStringLiteral("project.tables.register.update"),
      QStringLiteral("Update a register (params: table, name, newName?, computed?, defaultValue?)"),
      schema,
      &registerUpdate);
  }
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

  const auto& tables = DataModel::ProjectModel::instance().tables();

  QJsonArray arr;
  for (const auto& t : tables) {
    QJsonObject row;
    row[QStringLiteral("name")]          = t.name;
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

  const auto& tables = DataModel::ProjectModel::instance().tables();
  const auto it =
    std::find_if(tables.begin(), tables.end(), [&name](const auto& t) { return t.name == name; });

  if (it == tables.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Table not found: %1").arg(name));

  // Delegate to ProjectModel's existing summary helper (returns QVariantList)
  const auto registers = DataModel::ProjectModel::instance().registersForTable(name);

  QJsonArray arr = QJsonArray::fromVariantList(registers);

  QJsonObject result;
  result[QStringLiteral("name")]      = name;
  result[QStringLiteral("registers")] = arr;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Table mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new table with the given (optional) name.
 *
 * @return Actual name used after uniquification.
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
 * @brief Deletes a table by name. No-op if not found.
 */
API::CommandResponse API::Handlers::DataTablesHandler::tableDelete(const QString& id,
                                                                   const QJsonObject& params)
{
  QString name;
  if (!requireString(params, QStringLiteral("name"), name))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: name"));

  DataModel::ProjectModel::instance().deleteTable(name);

  QJsonObject result;
  result[QStringLiteral("name")]    = name;
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

  // renameTable silently no-ops on collision; check up front.
  const auto& preTables = pm.tables();
  const bool hasOld     = std::any_of(
    preTables.begin(), preTables.end(), [&oldName](const auto& t) { return t.name == oldName; });
  const bool hasNew = std::any_of(
    preTables.begin(), preTables.end(), [&newName](const auto& t) { return t.name == newName; });
  const bool collides = hasNew && (oldName != newName);

  // Apply only when safe
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

  // defaultValue is canonical; "value" is accepted as a legacy alias
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
 * @brief Removes a register from a table.
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

  DataModel::ProjectModel::instance().deleteRegister(table, name);

  QJsonObject result;
  result[QStringLiteral("table")]   = table;
  result[QStringLiteral("name")]    = name;
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

  // Look up the register to preserve unspecified fields
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

  // Merge caller-supplied fields over the current register's state
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

  DataModel::ProjectModel::instance().updateRegister(table, name, newName, computed, defaultValue);

  QJsonObject result;
  result[QStringLiteral("table")]    = table;
  result[QStringLiteral("name")]     = newName;
  result[QStringLiteral("computed")] = computed;
  result[QStringLiteral("updated")]  = true;
  return CommandResponse::makeSuccess(id, result);
}
