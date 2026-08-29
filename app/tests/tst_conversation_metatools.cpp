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
#include <QString>
#include <QStringList>
#include <QTest>

#include "AI/Conversation/MetaToolCatalog.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief The meta-tool surface the assistant advertises to the model on every request.
 *
 * A malformed input_schema is not a local defect: providers validate the tool list up front and
 * reject the whole request, and a silently dropped enum leaves the model guessing skill and
 * recipe ids. These cases pin the schema shapes, the surface roster, and the two flags that
 * reshape the curated command list.
 */
class TstConversationMetaTools : public QObject {
  Q_OBJECT

private slots:
  void stringProp_omitsEmptyEnum();
  void stringProp_carriesEnumValues();

  void objectSchema_declaresPropertyAndRequirement();
  void objectSchema_omitsRequiredWhenOptional();

  void makeMetaTool_usesProviderFieldNames();

  void metaTools_advertisesTheFullRoster();
  void metaTools_everyToolHasAWellFormedSchema();
  void metaTools_echoesRecipeAndSkillEnums();
  void metaTools_requiredFieldsAreDeclaredProperties();

  void essentials_fullSurfaceCarriesWorkspaceEditing();
  void essentials_smallSurfaceDropsExactlyFourTools();
  void essentials_memoryFlagAppendsProposal();

  void remap_renamesInputSchema();
  void remap_fillsMissingSchemaDefaults();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

static QStringList toolNames(const QJsonArray& tools)
{
  QStringList names;
  for (const auto& v : tools)
    names.append(v.toObject().value(QStringLiteral("name")).toString());

  return names;
}

static QJsonObject toolNamed(const QJsonArray& tools, const QString& name)
{
  for (const auto& v : tools) {
    const auto obj = v.toObject();
    if (obj.value(QStringLiteral("name")).toString() == name)
      return obj;
  }
  return {};
}

static QStringList enumOf(const QJsonObject& tool, const QString& property)
{
  const auto values = tool.value(QStringLiteral("input_schema"))
                        .toObject()
                        .value(QStringLiteral("properties"))
                        .toObject()
                        .value(property)
                        .toObject()
                        .value(QStringLiteral("enum"))
                        .toArray();

  QStringList out;
  for (const auto& v : values)
    out.append(v.toString());

  return out;
}

static QStringList sampleTasks()
{
  return {QStringLiteral("add_painter"), QStringLiteral("build_dashboard")};
}

static QStringList sampleSkills()
{
  return {QStringLiteral("project_basics"), QStringLiteral("frame_parsers")};
}

//--------------------------------------------------------------------------------------------------
// Schema primitives
//--------------------------------------------------------------------------------------------------

void TstConversationMetaTools::stringProp_omitsEmptyEnum()
{
  const auto prop = AI::MetaToolCatalog::stringProp(QStringLiteral("a description"));

  QCOMPARE(prop.value(QStringLiteral("type")).toString(), QStringLiteral("string"));
  QCOMPARE(prop.value(QStringLiteral("description")).toString(), QStringLiteral("a description"));
  QVERIFY(!prop.contains(QStringLiteral("enum")));
}

void TstConversationMetaTools::stringProp_carriesEnumValues()
{
  const QJsonArray values{QStringLiteral("one"), QStringLiteral("two")};
  const auto prop = AI::MetaToolCatalog::stringProp(QStringLiteral("pick"), values);

  QCOMPARE(prop.value(QStringLiteral("enum")).toArray(), values);
}

void TstConversationMetaTools::objectSchema_declaresPropertyAndRequirement()
{
  const auto inner = AI::MetaToolCatalog::stringProp(QStringLiteral("the name"));
  const auto schema =
    AI::MetaToolCatalog::objectSchemaWithProperty(QStringLiteral("name"), inner, true);

  QCOMPARE(schema.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
  QCOMPARE(schema.value(QStringLiteral("properties")).toObject().value(QStringLiteral("name")),
           QJsonValue(inner));
  QCOMPARE(schema.value(QStringLiteral("required")).toArray(), QJsonArray{QStringLiteral("name")});
}

void TstConversationMetaTools::objectSchema_omitsRequiredWhenOptional()
{
  const auto schema = AI::MetaToolCatalog::objectSchemaWithProperty(
    QStringLiteral("name"), AI::MetaToolCatalog::stringProp(QString()), false);

  QVERIFY(!schema.contains(QStringLiteral("required")));
  QVERIFY(schema.value(QStringLiteral("properties")).toObject().contains(QStringLiteral("name")));
}

void TstConversationMetaTools::makeMetaTool_usesProviderFieldNames()
{
  QJsonObject schema;
  schema[QStringLiteral("type")] = QStringLiteral("object");

  const auto tool = AI::MetaToolCatalog::makeMetaTool(
    QStringLiteral("meta.thing"), QStringLiteral("does a thing"), schema);

  QCOMPARE(tool.value(QStringLiteral("name")).toString(), QStringLiteral("meta.thing"));
  QCOMPARE(tool.value(QStringLiteral("description")).toString(), QStringLiteral("does a thing"));
  QCOMPARE(tool.value(QStringLiteral("input_schema")).toObject(), schema);
  QVERIFY(!tool.contains(QStringLiteral("inputSchema")));
}

//--------------------------------------------------------------------------------------------------
// Assembled meta surface
//--------------------------------------------------------------------------------------------------

void TstConversationMetaTools::metaTools_advertisesTheFullRoster()
{
  const auto tools = AI::MetaToolCatalog::metaTools(sampleTasks(), sampleSkills());

  const QStringList expected{QStringLiteral("meta.listCategories"),
                             QStringLiteral("meta.snapshot"),
                             QStringLiteral("meta.listCommands"),
                             QStringLiteral("meta.search"),
                             QStringLiteral("meta.describeCommand"),
                             QStringLiteral("meta.executeCommand"),
                             QStringLiteral("meta.fetchHelp"),
                             QStringLiteral("meta.fetchScriptingDocs"),
                             QStringLiteral("meta.howTo"),
                             QStringLiteral("meta.loadSkill"),
                             QStringLiteral("meta.searchDocs")};

  QCOMPARE(toolNames(tools), expected);
}

void TstConversationMetaTools::metaTools_everyToolHasAWellFormedSchema()
{
  const auto tools = AI::MetaToolCatalog::metaTools(sampleTasks(), sampleSkills());

  for (const auto& v : tools) {
    const auto tool = v.toObject();
    const auto name = tool.value(QStringLiteral("name")).toString();
    QVERIFY2(name.startsWith(QStringLiteral("meta.")), qPrintable(name));
    QVERIFY2(!tool.value(QStringLiteral("description")).toString().isEmpty(), qPrintable(name));

    const auto schema = tool.value(QStringLiteral("input_schema")).toObject();
    QVERIFY2(schema.value(QStringLiteral("type")).toString() == QStringLiteral("object"),
             qPrintable(name));
    QVERIFY2(schema.value(QStringLiteral("properties")).isObject(), qPrintable(name));
  }
}

void TstConversationMetaTools::metaTools_echoesRecipeAndSkillEnums()
{
  const auto tools = AI::MetaToolCatalog::metaTools(sampleTasks(), sampleSkills());

  QCOMPARE(enumOf(toolNamed(tools, QStringLiteral("meta.howTo")), QStringLiteral("task")),
           sampleTasks());
  QCOMPARE(enumOf(toolNamed(tools, QStringLiteral("meta.loadSkill")), QStringLiteral("name")),
           sampleSkills());

  const auto kinds =
    enumOf(toolNamed(tools, QStringLiteral("meta.fetchScriptingDocs")), QStringLiteral("kind"));
  QVERIFY(kinds.contains(QStringLiteral("frame_parser_js")));
  QVERIFY(kinds.contains(QStringLiteral("sdk_lua")));

  const auto empty = AI::MetaToolCatalog::metaTools(QStringList(), QStringList());
  QVERIFY(enumOf(toolNamed(empty, QStringLiteral("meta.howTo")), QStringLiteral("task")).isEmpty());
}

/**
 * @brief A required field that names no declared property is the schema defect providers
 *        reject the whole tool list over, so every required entry is checked against the
 *        properties object it belongs to.
 */
void TstConversationMetaTools::metaTools_requiredFieldsAreDeclaredProperties()
{
  const auto tools = AI::MetaToolCatalog::metaTools(sampleTasks(), sampleSkills());

  for (const auto& v : tools) {
    const auto tool   = v.toObject();
    const auto schema = tool.value(QStringLiteral("input_schema")).toObject();
    const auto props  = schema.value(QStringLiteral("properties")).toObject();
    for (const auto& r : schema.value(QStringLiteral("required")).toArray())
      QVERIFY2(props.contains(r.toString()),
               qPrintable(tool.value(QStringLiteral("name")).toString() + QLatin1Char('/')
                          + r.toString()));
  }
}

//--------------------------------------------------------------------------------------------------
// Curated command surface
//--------------------------------------------------------------------------------------------------

void TstConversationMetaTools::essentials_fullSurfaceCarriesWorkspaceEditing()
{
  const auto full = AI::MetaToolCatalog::essentialToolNames(false, false);

  QVERIFY(full.contains(QStringLiteral("project.workspace.addWidget")));
  QVERIFY(full.contains(QStringLiteral("project.workspace.removeWidget")));
  QVERIFY(full.contains(QStringLiteral("project.workspace.setCustomizeMode")));
  QVERIFY(full.contains(QStringLiteral("project.dataset.setOptions")));
  QVERIFY(!full.contains(QStringLiteral("assistant.memory.propose")));
  QCOMPARE(full.count(QStringLiteral("fs.read")), 1);
}

void TstConversationMetaTools::essentials_smallSurfaceDropsExactlyFourTools()
{
  const auto full  = AI::MetaToolCatalog::essentialToolNames(false, false);
  const auto small = AI::MetaToolCatalog::essentialToolNames(true, false);

  QCOMPARE(small.size(), full.size() - 4);
  QVERIFY(!small.contains(QStringLiteral("project.workspace.addWidget")));
  QVERIFY(!small.contains(QStringLiteral("project.workspace.removeWidget")));
  QVERIFY(!small.contains(QStringLiteral("project.workspace.setCustomizeMode")));
  QVERIFY(!small.contains(QStringLiteral("project.dataset.setOptions")));
  QVERIFY(small.contains(QStringLiteral("project.workspace.list")));
  QVERIFY(small.contains(QStringLiteral("project.dataset.add")));
}

void TstConversationMetaTools::essentials_memoryFlagAppendsProposal()
{
  const auto withMemory = AI::MetaToolCatalog::essentialToolNames(false, true);
  QCOMPARE(withMemory.count(QStringLiteral("assistant.memory.propose")), 1);
  QCOMPARE(withMemory.constLast(), QStringLiteral("assistant.memory.propose"));

  const auto smallWithMemory = AI::MetaToolCatalog::essentialToolNames(true, true);
  QCOMPARE(smallWithMemory.size(), withMemory.size() - 4);
  QVERIFY(smallWithMemory.contains(QStringLiteral("assistant.memory.propose")));
}

//--------------------------------------------------------------------------------------------------
// Dispatcher remapping
//--------------------------------------------------------------------------------------------------

void TstConversationMetaTools::remap_renamesInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("id")] = AI::MetaToolCatalog::stringProp(QStringLiteral("an id"));
  QJsonObject schema;
  schema[QStringLiteral("type")]       = QStringLiteral("object");
  schema[QStringLiteral("properties")] = props;

  QJsonObject raw;
  raw[QStringLiteral("name")]        = QStringLiteral("project.dataset.add");
  raw[QStringLiteral("description")] = QStringLiteral("adds a dataset");
  raw[QStringLiteral("inputSchema")] = schema;

  const auto tool = AI::MetaToolCatalog::remapDispatcherTool(raw);

  QCOMPARE(tool.value(QStringLiteral("name")).toString(), QStringLiteral("project.dataset.add"));
  QCOMPARE(tool.value(QStringLiteral("description")).toString(), QStringLiteral("adds a dataset"));
  QCOMPARE(tool.value(QStringLiteral("input_schema")).toObject(), schema);
  QVERIFY(!tool.contains(QStringLiteral("inputSchema")));
}

void TstConversationMetaTools::remap_fillsMissingSchemaDefaults()
{
  QJsonObject raw;
  raw[QStringLiteral("name")]        = QStringLiteral("io.getStatus");
  raw[QStringLiteral("description")] = QStringLiteral("status");

  const auto schema =
    AI::MetaToolCatalog::remapDispatcherTool(raw).value(QStringLiteral("input_schema")).toObject();

  QCOMPARE(schema.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
  QVERIFY(schema.value(QStringLiteral("properties")).isObject());
  QVERIFY(schema.value(QStringLiteral("properties")).toObject().isEmpty());
}

QTEST_APPLESS_MAIN(TstConversationMetaTools)

#include "tst_conversation_metatools.moc"
