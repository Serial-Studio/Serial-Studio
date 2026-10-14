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

#include <algorithm>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTest>

#include "API/CommandCapabilities.h"
#include "Core/SerialStudio.h"

using namespace API::CommandCapabilities;

/**
 * @brief Every command name in the committed API snapshot.
 */
static QSet<QString> schemaCommands()
{
  QFile file(QStringLiteral(SS_API_SCHEMA_PATH));
  if (!file.open(QIODevice::ReadOnly))
    return {};

  QSet<QString> names;
  const auto commands = QJsonDocument::fromJson(file.readAll()).array();
  for (const auto& command : commands)
    names.insert(command.toObject().value(QStringLiteral("name")).toString());

  return names;
}

/**
 * @brief The names of the capable table, as QStrings.
 */
static QSet<QString> capableNames()
{
  QSet<QString> out;
  for (const auto& entry : capableCommands())
    out.insert(QString(entry.name));

  return out;
}

/**
 * @brief The names of the plain table, as QStrings.
 */
static QSet<QString> plainNames()
{
  QSet<QString> out;
  for (const auto name : plainCommands())
    out.insert(QString(name));

  return out;
}

/**
 * @brief The command capability classification (spec 0098 AC7): every command in the API
 *        snapshot is classified exactly once, both tables are binary-searchable, arguments add
 *        capabilities exactly where the table says they can, and an unknown name is
 *        unclassified, so a new command fails here before it can reach a remote client.
 */
class TstCommandCapabilityRegistry : public QObject {
  Q_OBJECT

private slots:
  void everySchemaCommandIsClassifiedOnce();
  void tablesNameOnlyRealCommands();
  void tablesAreSortedAndUnique();
  void argumentRefinements_data();
  void argumentRefinements();
  void everyArgumentDependentEntryIsCovered();
  void unknownNamesAreUnclassified();
};

/**
 * @brief AC7: a command in neither table, or in both, fails the sweep by name.
 */
void TstCommandCapabilityRegistry::everySchemaCommandIsClassifiedOnce()
{
  const auto schema  = schemaCommands();
  const auto capable = capableNames();
  const auto plain   = plainNames();
  QVERIFY(!schema.isEmpty());

  for (const auto& name : schema) {
    const bool inCapable = capable.contains(name);
    const bool inPlain   = plain.contains(name);
    QVERIFY2(inCapable || inPlain,
             qPrintable(QStringLiteral("%1 has no capability classification").arg(name)));
    QVERIFY2(!(inCapable && inPlain),
             qPrintable(QStringLiteral("%1 is classified twice").arg(name)));
  }
}

/**
 * @brief A removed or renamed command cannot linger in either table.
 */
void TstCommandCapabilityRegistry::tablesNameOnlyRealCommands()
{
  const auto schema = schemaCommands();
  for (const auto& name : capableNames())
    QVERIFY2(schema.contains(name), qPrintable(name));

  for (const auto& name : plainNames())
    QVERIFY2(schema.contains(name), qPrintable(name));
}

/**
 * @brief The lookup binary-searches both tables, so each must be strictly ascending.
 */
void TstCommandCapabilityRegistry::tablesAreSortedAndUnique()
{
  const auto capable = capableCommands();
  QVERIFY(!capable.empty());
  QVERIFY(std::adjacent_find(capable.begin(),
                             capable.end(),
                             [](const Entry& a, const Entry& b) { return !(a.name < b.name); })
          == capable.end());

  const auto plain = plainCommands();
  QVERIFY(!plain.empty());
  QVERIFY(std::adjacent_find(plain.begin(),
                             plain.end(),
                             [](QLatin1StringView a, QLatin1StringView b) { return !(a < b); })
          == plain.end());
}

/**
 * @brief One row per argument-dependent command: arguments that add its capabilities and
 *        arguments that do not.
 */
void TstCommandCapabilityRegistry::argumentRefinements_data()
{
  QTest::addColumn<QString>("command");
  QTest::addColumn<QJsonObject>("adding");
  QTest::addColumn<QJsonObject>("plain");

  const QJsonObject none{
    {QStringLiteral("title"), QStringLiteral("x")}
  };

  QTest::newRow("dataset.update") << QStringLiteral("project.dataset.update")
                                  << QJsonObject{
                                       {QStringLiteral("transformCode"), QStringLiteral("x")}
  } << none;
  QTest::newRow("group.update") << QStringLiteral("project.group.update")
                                << QJsonObject{
                                     {QStringLiteral("painterCode"), QStringLiteral("x")}
  } << none;
  QTest::newRow("outputWidget.update") << QStringLiteral("project.outputWidget.update")
                                       << QJsonObject{
                                            {QStringLiteral("transmitFunction"), QStringLiteral("x")}
  } << none;
  QTest::newRow("action.update") << QStringLiteral("project.action.update")
                                 << QJsonObject{
                                      {QStringLiteral("txData"), QStringLiteral("x")}
  } << none;
  QTest::newRow("source.setProperty") << QStringLiteral("project.source.setProperty")
                                      << QJsonObject{
                                           {QStringLiteral("key"), QStringLiteral("executable")}
  } << QJsonObject{
    {QStringLiteral("key"), QStringLiteral("baudRate")}
  };
  QTest::newRow("source.setProperties")
    << QStringLiteral("project.source.setProperties")
    << QJsonObject{
         {QStringLiteral("settings"), QJsonObject{{QStringLiteral("arguments"), QStringLiteral("x")}}}
  } << QJsonObject{
    {QStringLiteral("settings"), QJsonObject{{QStringLiteral("baudRate"), 9600}}}
  };

#ifdef BUILD_COMMERCIAL
  const int process = static_cast<int>(SerialStudio::BusType::Process);
  QTest::newRow("io.setBusType") << QStringLiteral("io.setBusType")
                                 << QJsonObject{
                                      {QStringLiteral("busType"), process}
  } << QJsonObject{
    {QStringLiteral("busType"), 0}
  };
  QTest::newRow("source.update") << QStringLiteral("project.source.update")
                                 << QJsonObject{
                                      {QStringLiteral("busType"), process}
  } << QJsonObject{
    {QStringLiteral("busType"), 0}
  };
#endif
}

/**
 * @brief The adding arguments yield the entry's argument-dependent capabilities on top of the
 *        ones it always has; the plain arguments yield only the ones it always has.
 */
void TstCommandCapabilityRegistry::argumentRefinements()
{
  QFETCH(QString, command);
  QFETCH(QJsonObject, adding);
  QFETCH(QJsonObject, plain);

  const auto entries = capableCommands();
  const auto entry =
    std::find_if(entries.begin(), entries.end(), [&](const Entry& e) { return e.name == command; });
  QVERIFY(entry != entries.end());
  QVERIFY(entry->withArguments.toInt() != 0);

  const auto added = of(command, adding);
  const auto base  = of(command, plain);
  QVERIFY(added.has_value());
  QVERIFY(base.has_value());
  QCOMPARE(*added, entry->always | entry->withArguments);
  QCOMPARE(*base, entry->always);
}

/**
 * @brief An argument-dependent entry without a refinement row would never be exercised.
 */
void TstCommandCapabilityRegistry::everyArgumentDependentEntryIsCovered()
{
  QSet<QString> covered = {
    QStringLiteral("project.dataset.update"),
    QStringLiteral("project.group.update"),
    QStringLiteral("project.outputWidget.update"),
    QStringLiteral("project.action.update"),
    QStringLiteral("project.source.setProperty"),
    QStringLiteral("project.source.setProperties"),
    QStringLiteral("io.setBusType"),
    QStringLiteral("project.source.update"),
  };

  for (const auto& entry : capableCommands())
    if (entry.withArguments.toInt() != 0)
      QVERIFY2(covered.contains(QString(entry.name)), qPrintable(QString(entry.name)));
}

/**
 * @brief Unknown and empty names are unclassified; a plain name has no capability.
 */
void TstCommandCapabilityRegistry::unknownNamesAreUnclassified()
{
  QVERIFY(!of(u"no.such.command", {}).has_value());
  QVERIFY(!of(QStringView(), {}).has_value());
  QVERIFY(!of(u"project.batc", {}).has_value());

  const auto plain = of(u"io.getStatus", {});
  QVERIFY(plain.has_value());
  QCOMPARE(*plain, Capabilities(NoCapability));
}

QTEST_GUILESS_MAIN(TstCommandCapabilityRegistry)

#include "tst_command_capability_registry.moc"
