/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTest>

#include "AI/Tools/ToolSchemas.h"

using AI::ToolDetail::AssistantToolDef;

/**
 * @brief Structural contract of the assistant.* and fs.* tool schemas served to providers.
 */
class TstToolSchemas : public QObject {
  Q_OBJECT

private slots:
  void rosterIdentity();

  void toolDefIsWellFormed_data();
  void toolDefIsWellFormed();

  void propertyIsWellFormed_data();
  void propertyIsWellFormed();

  void requiredKeysAreDeclared_data();
  void requiredKeysAreDeclared();

  void requiredKeysArePinned_data();
  void requiredKeysArePinned();

  void enumValuesArePinned_data();
  void enumValuesArePinned();

  void multiTypeSelectorsAcceptIdAndAlias_data();
  void multiTypeSelectorsAcceptIdAndAlias();

  void bulkOpsPropertyCarriesItemSchema();

  void namePredicates_data();
  void namePredicates();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns every advertised tool definition, assistant roster first.
 */
static QVector<AssistantToolDef> allDefs()
{
  auto defs = AI::ToolDetail::assistantToolDefs();
  defs.append(AI::ToolDetail::fsToolDefs());
  return defs;
}

/**
 * @brief Returns the input schema of one advertised tool, or an empty object when absent.
 */
static QJsonObject schemaOf(const QString& toolName)
{
  for (const auto& def : allDefs())
    if (def.name == toolName)
      return def.inputSchema;

  return {};
}

/**
 * @brief Returns the JSON Schema type names accepted by the property builders.
 */
static QStringList validTypeNames()
{
  return QStringList{QStringLiteral("object"),
                     QStringLiteral("array"),
                     QStringLiteral("string"),
                     QStringLiteral("integer"),
                     QStringLiteral("number"),
                     QStringLiteral("boolean")};
}

/**
 * @brief Flattens a JSON array of strings into a sorted string list.
 */
static QStringList sortedStrings(const QJsonArray& values)
{
  QStringList out;
  for (const auto& value : values)
    out.append(value.toString());

  out.sort();
  return out;
}

//--------------------------------------------------------------------------------------------------
// Roster identity
//--------------------------------------------------------------------------------------------------

/**
 * @brief Both rosters are non-empty, name-unique, and every name sits under the prefix its
 *        predicate claims; a duplicate would silently shadow a catalog row in listCommands().
 */
void TstToolSchemas::rosterIdentity()
{
  const auto assistant = AI::ToolDetail::assistantToolDefs();
  const auto fs        = AI::ToolDetail::fsToolDefs();
  QVERIFY(!assistant.isEmpty());
  QVERIFY(!fs.isEmpty());

  QSet<QString> seen;
  for (const auto& def : assistant) {
    QVERIFY2(AI::ToolDetail::isAssistantTool(def.name), qPrintable(def.name));
    QVERIFY2(!seen.contains(def.name), qPrintable(def.name));
    seen.insert(def.name);
  }

  for (const auto& def : fs) {
    QVERIFY2(AI::ToolDetail::isFsTool(def.name), qPrintable(def.name));
    QVERIFY2(!seen.contains(def.name), qPrintable(def.name));
    seen.insert(def.name);
  }

  QCOMPARE(seen.size(), assistant.size() + fs.size());
}

//--------------------------------------------------------------------------------------------------
// Tool definition shape
//--------------------------------------------------------------------------------------------------

void TstToolSchemas::toolDefIsWellFormed_data()
{
  QTest::addColumn<QString>("description");
  QTest::addColumn<QJsonObject>("schema");

  for (const auto& def : allDefs())
    QTest::newRow(qPrintable(def.name)) << def.description << def.inputSchema;
}

/**
 * @brief Every advertised tool carries a description and an object-typed schema with a
 *        properties bag, which is the shape every provider adapter serializes blindly.
 */
void TstToolSchemas::toolDefIsWellFormed()
{
  QFETCH(QString, description);
  QFETCH(QJsonObject, schema);

  QVERIFY(!description.isEmpty());
  QCOMPARE(schema.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
  QVERIFY(schema.value(QStringLiteral("properties")).isObject());
  QVERIFY(!schema.value(QStringLiteral("properties")).toObject().isEmpty());
}

void TstToolSchemas::propertyIsWellFormed_data()
{
  QTest::addColumn<QJsonObject>("property");

  for (const auto& def : allDefs()) {
    const auto props = def.inputSchema.value(QStringLiteral("properties")).toObject();
    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
      const auto row = def.name + QLatin1Char('.') + it.key();
      QTest::newRow(qPrintable(row)) << it.value().toObject();
    }
  }
}

/**
 * @brief Each property is a documented JSON Schema fragment: a described value whose type is
 *        one known type name or a two-name union produced by makeMultiTypeProperty().
 */
void TstToolSchemas::propertyIsWellFormed()
{
  QFETCH(QJsonObject, property);

  QVERIFY(!property.value(QStringLiteral("description")).toString().isEmpty());

  const auto type = property.value(QStringLiteral("type"));
  if (type.isString()) {
    QVERIFY2(validTypeNames().contains(type.toString()), qPrintable(type.toString()));
    return;
  }

  QVERIFY(type.isArray());
  const auto names = type.toArray();
  QCOMPARE(names.size(), 2);
  for (const auto& name : names)
    QVERIFY2(validTypeNames().contains(name.toString()), qPrintable(name.toString()));
}

//--------------------------------------------------------------------------------------------------
// Required keys
//--------------------------------------------------------------------------------------------------

void TstToolSchemas::requiredKeysAreDeclared_data()
{
  QTest::addColumn<QJsonObject>("schema");

  for (const auto& def : allDefs())
    QTest::newRow(qPrintable(def.name)) << def.inputSchema;
}

/**
 * @brief A required key that is not also a declared property makes the schema unsatisfiable,
 *        so makeObjectSchema() callers must never name one.
 */
void TstToolSchemas::requiredKeysAreDeclared()
{
  QFETCH(QJsonObject, schema);

  const auto props = schema.value(QStringLiteral("properties")).toObject();
  const auto keys  = schema.value(QStringLiteral("required")).toArray();
  for (const auto& key : keys)
    QVERIFY2(props.contains(key.toString()), qPrintable(key.toString()));

  QVERIFY(!schema.contains(QStringLiteral("required")) || !keys.isEmpty());
}

void TstToolSchemas::requiredKeysArePinned_data()
{
  QTest::addColumn<QString>("tool");
  QTest::addColumn<QStringList>("required");

  QTest::newRow("snapshot takes no required key")
    << QStringLiteral("assistant.snapshot") << QStringList{};
  QTest::newRow("dataset resolve takes no required key")
    << QStringLiteral("assistant.dataset.resolve") << QStringList{};
  QTest::newRow("addTile pins widgetType")
    << QStringLiteral("assistant.workspace.addTile") << QStringList{QStringLiteral("widgetType")};
  QTest::newRow("dryRun pins kind")
    << QStringLiteral("assistant.script.dryRun") << QStringList{QStringLiteral("kind")};
  QTest::newRow("apply pins kind and code")
    << QStringLiteral("assistant.script.apply")
    << QStringList{QStringLiteral("code"), QStringLiteral("kind")};
  QTest::newRow("bulkApply pins ops")
    << QStringLiteral("assistant.project.bulkApply") << QStringList{QStringLiteral("ops")};
  QTest::newRow("memory propose pins category and text")
    << QStringLiteral("assistant.memory.propose")
    << QStringList{QStringLiteral("category"), QStringLiteral("text")};
  QTest::newRow("fs.list takes no required key") << QStringLiteral("fs.list") << QStringList{};
  QTest::newRow("fs.read pins path")
    << QStringLiteral("fs.read") << QStringList{QStringLiteral("path")};
  QTest::newRow("fs.search pins query")
    << QStringLiteral("fs.search") << QStringList{QStringLiteral("query")};
  QTest::newRow("fs.write pins path and content")
    << QStringLiteral("fs.write") << QStringList{QStringLiteral("content"), QStringLiteral("path")};
  QTest::newRow("fs.append pins path and content")
    << QStringLiteral("fs.append")
    << QStringList{QStringLiteral("content"), QStringLiteral("path")};
  QTest::newRow("fs.delete pins path")
    << QStringLiteral("fs.delete") << QStringList{QStringLiteral("path")};
}

/**
 * @brief The required set is the model-visible contract of each tool; widening it silently
 *        breaks callers that already omit the key, narrowing it lets malformed calls through.
 */
void TstToolSchemas::requiredKeysArePinned()
{
  QFETCH(QString, tool);
  QFETCH(QStringList, required);

  const auto schema = schemaOf(tool);
  QVERIFY2(!schema.isEmpty(), qPrintable(tool));
  QCOMPARE(sortedStrings(schema.value(QStringLiteral("required")).toArray()), required);
}

//--------------------------------------------------------------------------------------------------
// Enumerations
//--------------------------------------------------------------------------------------------------

void TstToolSchemas::enumValuesArePinned_data()
{
  QTest::addColumn<QString>("tool");
  QTest::addColumn<QString>("property");
  QTest::addColumn<QStringList>("values");

  QStringList widgets{QStringLiteral("accelerometer"),
                      QStringLiteral("bar"),
                      QStringLiteral("compass"),
                      QStringLiteral("datagrid"),
                      QStringLiteral("fft"),
                      QStringLiteral("gauge"),
                      QStringLiteral("gps"),
                      QStringLiteral("gyroscope"),
                      QStringLiteral("imageview"),
                      QStringLiteral("led"),
                      QStringLiteral("multiplot"),
                      QStringLiteral("notification-log"),
                      QStringLiteral("output-panel"),
                      QStringLiteral("painter"),
                      QStringLiteral("plot"),
                      QStringLiteral("plot3d"),
                      QStringLiteral("waterfall")};
  QStringList kinds{QStringLiteral("end_to_end"),
                    QStringLiteral("frame_parser"),
                    QStringLiteral("output_widget"),
                    QStringLiteral("painter"),
                    QStringLiteral("transform")};

  QTest::newRow("addTile widget slugs")
    << QStringLiteral("assistant.workspace.addTile") << QStringLiteral("widgetType") << widgets;
  QTest::newRow("dryRun script kinds")
    << QStringLiteral("assistant.script.dryRun") << QStringLiteral("kind") << kinds;
  QTest::newRow("apply script kinds")
    << QStringLiteral("assistant.script.apply") << QStringLiteral("kind") << kinds;
}

/**
 * @brief The widget-slug and script-kind enums are the only vocabularies the model may emit,
 *        and every value has a matching branch in the dispatcher, so both lists are pinned.
 */
void TstToolSchemas::enumValuesArePinned()
{
  QFETCH(QString, tool);
  QFETCH(QString, property);
  QFETCH(QStringList, values);

  const auto props = schemaOf(tool).value(QStringLiteral("properties")).toObject();
  const auto prop  = props.value(property).toObject();
  QCOMPARE(prop.value(QStringLiteral("type")).toString(), QStringLiteral("string"));
  QCOMPARE(sortedStrings(prop.value(QStringLiteral("enum")).toArray()), values);
}

void TstToolSchemas::multiTypeSelectorsAcceptIdAndAlias_data()
{
  QTest::addColumn<QString>("tool");

  QTest::newRow("dataset resolver") << QStringLiteral("assistant.dataset.resolve");
  QTest::newRow("script surfaces") << QStringLiteral("assistant.script.dryRun");
}

/**
 * @brief uniqueId is a dual-typed selector: a JSON number is the opaque id, a JSON string is an
 *        editor alias. Collapsing it to one type would make one of the two resolvers unreachable.
 */
void TstToolSchemas::multiTypeSelectorsAcceptIdAndAlias()
{
  QFETCH(QString, tool);

  const auto props = schemaOf(tool).value(QStringLiteral("properties")).toObject();
  const auto types = props.value(QStringLiteral("uniqueId")).toObject();
  QCOMPARE(sortedStrings(types.value(QStringLiteral("type")).toArray()),
           QStringList({QStringLiteral("integer"), QStringLiteral("string")}));
  QVERIFY(!types.value(QStringLiteral("description")).toString().isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Array properties
//--------------------------------------------------------------------------------------------------

/**
 * @brief makeArrayProperty() is used once, for the bulkApply ops list; its items sub-schema is
 *        what stops models from flattening params into the op object.
 */
void TstToolSchemas::bulkOpsPropertyCarriesItemSchema()
{
  const auto schema = schemaOf(QStringLiteral("assistant.project.bulkApply"));
  const auto props  = schema.value(QStringLiteral("properties")).toObject();
  const auto ops    = props.value(QStringLiteral("ops")).toObject();
  QCOMPARE(ops.value(QStringLiteral("type")).toString(), QStringLiteral("array"));

  const auto items = ops.value(QStringLiteral("items")).toObject();
  QCOMPARE(items.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
  QCOMPARE(sortedStrings(items.value(QStringLiteral("required")).toArray()),
           QStringList({QStringLiteral("command"), QStringLiteral("params")}));

  const auto itemProps = items.value(QStringLiteral("properties")).toObject();
  const auto command   = itemProps.value(QStringLiteral("command")).toObject();
  const auto params    = itemProps.value(QStringLiteral("params")).toObject();
  QCOMPARE(command.value(QStringLiteral("type")).toString(), QStringLiteral("string"));
  QCOMPARE(params.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
}

//--------------------------------------------------------------------------------------------------
// Name predicates
//--------------------------------------------------------------------------------------------------

void TstToolSchemas::namePredicates_data()
{
  QTest::addColumn<QString>("name");
  QTest::addColumn<bool>("assistant");
  QTest::addColumn<bool>("filesystem");

  QTest::newRow("assistant tool") << QStringLiteral("assistant.snapshot") << true << false;
  QTest::newRow("fs tool") << QStringLiteral("fs.read") << false << true;
  QTest::newRow("api command") << QStringLiteral("project.snapshot") << false << false;
  QTest::newRow("meta command") << QStringLiteral("meta.listCommands") << false << false;
  QTest::newRow("prefix without dot") << QStringLiteral("assistantThings") << false << false;
  QTest::newRow("fs prefix without dot") << QStringLiteral("fsThings") << false << false;
  QTest::newRow("empty name") << QString() << false << false;
}

/**
 * @brief The two predicates route execution away from API::CommandRegistry, so a name that
 *        merely starts with the letters (and not the dotted prefix) must not match.
 */
void TstToolSchemas::namePredicates()
{
  QFETCH(QString, name);
  QFETCH(bool, assistant);
  QFETCH(bool, filesystem);

  QCOMPARE(AI::ToolDetail::isAssistantTool(name), assistant);
  QCOMPARE(AI::ToolDetail::isFsTool(name), filesystem);
}

QTEST_APPLESS_MAIN(TstToolSchemas)

#include "tst_tool_schemas.moc"
