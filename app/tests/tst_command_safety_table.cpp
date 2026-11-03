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
#include "API/CommandSafety.h"

using namespace API::CommandSafety;

static constexpr int kDirectoryNameLimit = 64;

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
 * @brief The assistant-only names the table may carry without a schema entry.
 */
static QSet<QString> assistantOnlyNames()
{
  QSet<QString> out;
  for (const auto name : assistantTools())
    out.insert(QString(name));

  return out;
}

/**
 * @brief Whether the remote gate refuses the name on classification alone.
 */
static bool isControlScriptOnly(const QString& name)
{
  const auto capabilities = API::CommandCapabilities::of(name, {});
  return capabilities && capabilities->testFlag(API::CommandCapabilities::ControlScriptOnly);
}

/**
 * @brief The generated command-safety table (spec 0112 AC2): every command in the API snapshot
 *        has a tier, the table names only real commands or assistant-only verbs, it is
 *        binary-searchable, no name exceeds the directory's limit, the tool list omits exactly
 *        the blocked tier and the control-script-only class, and the hints and titles are what
 *        the tier and the dotted name imply.
 */
class TstCommandSafetyTable : public QObject {
  Q_OBJECT

private slots:
  void everySchemaCommandHasATier();
  void tableNamesOnlyRealCommands();
  void tableIsSortedAndUnique();
  void namesFitTheDirectoryLimit();
  void omittedToolsAreExactlyBlockedAndControlScriptOnly();
  void hintsFollowTheTier();
  void unknownNamesStayListed();
  void titles_data();
  void titles();
};

/**
 * @brief AC2: a registered command with no tier fails the sweep by name.
 */
void TstCommandSafetyTable::everySchemaCommandHasATier()
{
  const auto schema = schemaCommands();
  QVERIFY(!schema.isEmpty());

  for (const auto& name : schema)
    QVERIFY2(tierOf(name).has_value(),
             qPrintable(QStringLiteral("%1 has no safety tier").arg(name)));
}

/**
 * @brief A removed or renamed command cannot linger in the table.
 */
void TstCommandSafetyTable::tableNamesOnlyRealCommands()
{
  const auto schema    = schemaCommands();
  const auto assistant = assistantOnlyNames();
  QVERIFY(!assistant.isEmpty());

  for (const auto& entry : entries()) {
    const auto name = QString(entry.name);
    QVERIFY2(schema.contains(name) || assistant.contains(name), qPrintable(name));
  }

  for (const auto& name : assistant)
    QVERIFY2(tierOf(name).has_value(), qPrintable(name));
}

/**
 * @brief The lookup binary-searches the table, so it must be strictly ascending.
 */
void TstCommandSafetyTable::tableIsSortedAndUnique()
{
  const auto table = entries();
  QVERIFY(!table.empty());
  QVERIFY(std::adjacent_find(table.begin(),
                             table.end(),
                             [](const Entry& a, const Entry& b) { return !(a.name < b.name); })
          == table.end());
}

/**
 * @brief The Claude directory rejects a tool name over 64 characters.
 */
void TstCommandSafetyTable::namesFitTheDirectoryLimit()
{
  for (const auto& entry : entries())
    QVERIFY2(entry.name.size() <= kDirectoryNameLimit, qPrintable(QString(entry.name)));
}

/**
 * @brief Listing is discovery: the tool list drops the blocked tier and the names the gate
 *        refuses on classification alone, and nothing else.
 */
void TstCommandSafetyTable::omittedToolsAreExactlyBlockedAndControlScriptOnly()
{
  const QSet<QString> expected = {
    QStringLiteral("licensing.activate"),
    QStringLiteral("licensing.activateOffline"),
    QStringLiteral("licensing.deactivate"),
    QStringLiteral("licensing.enableTrial"),
    QStringLiteral("licensing.setLicense"),
    QStringLiteral("licensing.validate"),
    QStringLiteral("system.exec"),
    QStringLiteral("system.kill"),
    QStringLiteral("system.runningProcesses"),
  };

  QSet<QString> omitted;
  for (const auto& name : schemaCommands()) {
    const bool blocked = tierOf(name) == Tier::Blocked;
    const bool listed  = isListedTool(name);
    QCOMPARE(listed, !(blocked || isControlScriptOnly(name)));
    if (!listed)
      omitted.insert(name);
  }

  QCOMPARE(omitted, expected);
}

/**
 * @brief Safe reads as read-only, Confirm as a plain mutation, and AlwaysConfirm, Blocked or
 *        a device-gated name as destructive.
 */
void TstCommandSafetyTable::hintsFollowTheTier()
{
  QCOMPARE(tierOf(u"io.getStatus"), Tier::Safe);
  QCOMPARE(hintsFor(u"io.getStatus").read_only, true);
  QCOMPARE(hintsFor(u"io.getStatus").destructive, false);

  QCOMPARE(tierOf(u"project.dataset.update"), Tier::Confirm);
  QCOMPARE(hintsFor(u"project.dataset.update").read_only, false);
  QCOMPARE(hintsFor(u"project.dataset.update").destructive, false);

  QCOMPARE(tierOf(u"project.dataset.delete"), Tier::AlwaysConfirm);
  QCOMPARE(hintsFor(u"project.dataset.delete").read_only, false);
  QCOMPARE(hintsFor(u"project.dataset.delete").destructive, true);

  QCOMPARE(tierOf(u"licensing.activate"), Tier::Blocked);
  QCOMPARE(hintsFor(u"licensing.activate").destructive, true);

  QVERIFY(isDeviceGated(u"io.connect"));
  QVERIFY(!isDeviceGated(u"io.getStatus"));
  QCOMPARE(hintsFor(u"io.connect").read_only, false);
  QCOMPARE(hintsFor(u"io.connect").destructive, true);
}

/**
 * @brief A name the table lacks is a plain mutation, never read-only, and the list does not
 *        drop it (the gate refuses the call instead).
 */
void TstCommandSafetyTable::unknownNamesStayListed()
{
  QVERIFY(!tierOf(u"no.such.command").has_value());
  QVERIFY(!tierOf(QStringView()).has_value());
  QVERIFY(!isDeviceGated(u"no.such.command"));
  QCOMPARE(hintsFor(u"no.such.command").read_only, false);
  QCOMPARE(hintsFor(u"no.such.command").destructive, false);
  QVERIFY(isListedTool(u"no.such.command"));
}

/**
 * @brief Twelve dotted names and the titles they must read as.
 */
void TstCommandSafetyTable::titles_data()
{
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("title");

  QTest::newRow("status") << QStringLiteral("io.getStatus") << QStringLiteral("IO / Get Status");
  QTest::newRow("transform") << QStringLiteral("project.dataset.setTransformCode")
                             << QStringLiteral("Project / Dataset / Set Transform Code");
  QTest::newRow("opcua") << QStringLiteral("io.opcua.trustServer")
                         << QStringLiteral("IO / OPC UA / Trust Server");
  QTest::newRow("iec104") << QStringLiteral("io.iec104.getPoints")
                          << QStringLiteral("IO / IEC 104 / Get Points");
  QTest::newRow("eip") << QStringLiteral("io.eip.addTag")
                       << QStringLiteral("IO / EtherNet/IP / Add Tag");
  QTest::newRow("csv") << QStringLiteral("csvExport.setInterval")
                       << QStringLiteral("CSV Export / Set Interval");
  QTest::newRow("layout") << QStringLiteral("ui.window.setLayoutPattern")
                          << QStringLiteral("UI / Window / Set Layout Pattern");
  QTest::newRow("s7") << QStringLiteral("io.s7.setProperty")
                      << QStringLiteral("IO / S7 / Set Property");
  QTest::newRow("fft") << QStringLiteral("project.dataset.getFFTMarkers")
                       << QStringLiteral("Project / Dataset / Get FFT Markers");
  QTest::newRow("vt100") << QStringLiteral("console.setVt100Emulation")
                         << QStringLiteral("Console / Set VT100 Emulation");
  QTest::newRow("canfd") << QStringLiteral("io.canbus.setCanFd")
                         << QStringLiteral("IO / CAN Bus / Set CAN FD");
  QTest::newRow("mdf4") << QStringLiteral("mdf4Export.getStatus")
                        << QStringLiteral("MDF4 Export / Get Status");
}

/**
 * @brief The title is a pure function of the name and differs from it.
 */
void TstCommandSafetyTable::titles()
{
  QFETCH(QString, name);
  QFETCH(QString, title);

  QCOMPARE(humanizeToolName(name), title);
  QVERIFY(humanizeToolName(name) != name);
}

QTEST_GUILESS_MAIN(TstCommandSafetyTable)

#include "tst_command_safety_table.moc"
