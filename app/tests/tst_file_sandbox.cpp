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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>

#include "AI/Conversation/AsyncToolRunner.h"
#include "AI/FileSandbox.h"
#include "Core/Bus/MessageBus.h"
#include "Core/IconRegistry.h"
#include "Core/Services.h"
#include "Core/TimerEvents.h"
#include "Core/Translator.h"
#include "Core/WorkspaceManager.h"

// The sandbox is a singleton over the live workspace, so the fixture points WorkspaceManager at a
// temporary directory for the whole suite and every case works inside it. The sandbox reads the
// workspace through the root-bound service set (spec 0077), so the fixture plays composition root
// and binds one over the same four Core singletons ModuleManager::bootstrapCoreServices() uses.

/**
 * @brief Pins the assistant's filesystem trust boundary -- what a model-supplied path may reach
 *        for reads, the narrower 'AI/' root writes are confined to, and the escape shapes both
 *        must refuse -- plus the worker lane the two read tools now run on (spec 0075, J3/M10)
 *        and the read-only source root behind the 'source/' prefix (spec 0078, AC4/AC5).
 */
class TstFileSandbox : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  void readInsideWorkspaceSucceeds();
  void readOutsideWorkspaceIsRefused();
  void traversalPathIsRefused();
  void writeOutsideAiRootIsRefused();
  void writeInsideAiRootSucceeds();
  void deleteRefusesTheWriteRootItself();
  void droppedPathAuthorizesOneFile();
  void searchFindsAMatchAndReportsCounts();
  void sourcePrefixListsTheBundle();
  void sourcePrefixReadsAFile();
  void dotSlashReachesAShadowedWorkspaceFolder();
  void sourceScopedSearchFindsOnlySourceHits();
  void defaultSearchDoesNotReachTheSource();
  void writeUnderSourcePrefixIsRefused();
  void resourceSchemePathIsRefusedForWrite();
  void asyncRunnerTakesOnlyTheReadTools();
  void asyncRunnerReportsResultWithItsGeneration();

private:
  QTemporaryDir m_workspace;
  QTemporaryDir m_outside;
  QTemporaryDir m_source;
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Redirects the workspace at a temporary tree and seeds the files every case reads.
 */
void TstFileSandbox::initTestCase()
{
  QVERIFY(m_workspace.isValid());
  QVERIFY(m_outside.isValid());

  static Core::Bus::MessageBus bus;
  static Core::Services services{bus,
                                 Misc::Translator::instance(),
                                 Misc::TimerEvents::instance(),
                                 Misc::WorkspaceManager::instance(),
                                 Misc::IconRegistry::instance()};
  Core::bindServices(&services);

  Misc::WorkspaceManager::instance().setTemporaryPath(m_workspace.path());
  QVERIFY(QDir(m_workspace.path()).mkpath(QStringLiteral("AI")));

  QFile inside(m_workspace.path() + QStringLiteral("/notes.txt"));
  QVERIFY(inside.open(QIODevice::WriteOnly));
  inside.write("alpha needle beta\n");
  inside.close();

  QFile outside(m_outside.path() + QStringLiteral("/secret.txt"));
  QVERIFY(outside.open(QIODevice::WriteOnly));
  outside.write("private\n");
  outside.close();

  QVERIFY(m_source.isValid());
  QVERIFY(QDir(m_source.path()).mkpath(QStringLiteral("core")));
  QFile source(m_source.path() + QStringLiteral("/core/Example.cpp"));
  QVERIFY(source.open(QIODevice::WriteOnly));
  source.write("int checksumSentinel = 1;\n");
  source.close();
  AI::FileSandbox::instance().setSourceRoot(QFileInfo(m_source.path()).canonicalFilePath());
}

/**
 * @brief Restores the real workspace and releases the service set so nothing else in the run
 *        inherits the fixture's tree or its binding.
 */
void TstFileSandbox::cleanupTestCase()
{
  AI::FileSandbox::instance().clearDroppedPaths();
  AI::FileSandbox::instance().setSourceRoot(
    QString::fromLatin1(AI::FileSandbox::kSourceResourceRoot));
  Misc::WorkspaceManager::instance().clearTemporaryPath();
  Core::bindServices(nullptr);
}

//--------------------------------------------------------------------------------------------------
// Read policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief A workspace-relative path resolves and returns its bytes.
 */
void TstFileSandbox::readInsideWorkspaceSucceeds()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("notes.txt"));

  const auto out = AI::FileSandbox::instance().read(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QVERIFY(out.value(QStringLiteral("content")).toString().contains(QStringLiteral("needle")));
}

/**
 * @brief An absolute path outside the workspace is refused with the sandbox reason.
 */
void TstFileSandbox::readOutsideWorkspaceIsRefused()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), m_outside.path() + QStringLiteral("/secret.txt"));

  const auto out = AI::FileSandbox::instance().read(args);
  QVERIFY(!out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("error")).toString(), QStringLiteral("outside_sandbox"));
}

/**
 * @brief A relative traversal cannot climb out of the workspace.
 */
void TstFileSandbox::traversalPathIsRefused()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("../../etc/hosts"));

  const auto out = AI::FileSandbox::instance().read(args);
  QVERIFY(!out.value(QStringLiteral("ok")).toBool());
}

//--------------------------------------------------------------------------------------------------
// Write policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief A write that resolves outside 'AI/' is refused even though the read root allows it.
 */
void TstFileSandbox::writeOutsideAiRootIsRefused()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), m_workspace.path() + QStringLiteral("/notes.txt"));
  args.insert(QStringLiteral("content"), QStringLiteral("overwritten"));

  const auto out = AI::FileSandbox::instance().write(args);
  QVERIFY(!out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("error")).toString(), QStringLiteral("outside_sandbox"));

  QFile file(m_workspace.path() + QStringLiteral("/notes.txt"));
  QVERIFY(file.open(QIODevice::ReadOnly));
  QVERIFY(QString::fromUtf8(file.readAll()).contains(QStringLiteral("needle")));
}

/**
 * @brief A bare relative path lands under the 'AI/' write root.
 */
void TstFileSandbox::writeInsideAiRootSucceeds()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("note.md"));
  args.insert(QStringLiteral("content"), QStringLiteral("hello"));

  const auto out = AI::FileSandbox::instance().write(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QVERIFY(QFile::exists(m_workspace.path() + QStringLiteral("/AI/note.md")));
}

/**
 * @brief The write root itself cannot be deleted by a tool call.
 */
void TstFileSandbox::deleteRefusesTheWriteRootItself()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("."));

  const auto out = AI::FileSandbox::instance().remove(args);
  QVERIFY(!out.value(QStringLiteral("ok")).toBool());
  QVERIFY(QDir(m_workspace.path() + QStringLiteral("/AI")).exists());
}

//--------------------------------------------------------------------------------------------------
// Session allow-list
//--------------------------------------------------------------------------------------------------

/**
 * @brief A user-dropped file becomes readable for the session and drops out again on clear.
 */
void TstFileSandbox::droppedPathAuthorizesOneFile()
{
  const auto path = m_outside.path() + QStringLiteral("/secret.txt");
  auto& sandbox   = AI::FileSandbox::instance();

  QJsonObject args;
  args.insert(QStringLiteral("path"), path);
  QVERIFY(!sandbox.read(args).value(QStringLiteral("ok")).toBool());

  QVERIFY(!sandbox.registerDroppedPath(path).isEmpty());
  QVERIFY(sandbox.read(args).value(QStringLiteral("ok")).toBool());

  sandbox.clearDroppedPaths();
  QVERIFY(!sandbox.read(args).value(QStringLiteral("ok")).toBool());
}

//--------------------------------------------------------------------------------------------------
// Search
//--------------------------------------------------------------------------------------------------

/**
 * @brief Search reports its hits and the files it scanned, bounded to the read roots.
 */
void TstFileSandbox::searchFindsAMatchAndReportsCounts()
{
  QJsonObject args;
  args.insert(QStringLiteral("query"), QStringLiteral("needle"));

  const auto out = AI::FileSandbox::instance().search(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QVERIFY(out.value(QStringLiteral("count")).toInt() >= 1);
  QVERIFY(out.value(QStringLiteral("filesScanned")).toInt() >= 1);
}

//--------------------------------------------------------------------------------------------------
// Source root (spec 0078)
//--------------------------------------------------------------------------------------------------

/**
 * @brief 'source' lists the bundle root and reports its entries under the same prefix.
 */
void TstFileSandbox::sourcePrefixListsTheBundle()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("source"));
  args.insert(QStringLiteral("recursive"), true);

  const auto out = AI::FileSandbox::instance().list(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("path")).toString(), QStringLiteral("source"));

  QStringList paths;
  const auto entries = out.value(QStringLiteral("entries")).toArray();
  for (const auto& entry : entries)
    paths.append(entry.toObject().value(QStringLiteral("path")).toString());

  QVERIFY(paths.contains(QStringLiteral("source/core")));
  QVERIFY(paths.contains(QStringLiteral("source/core/Example.cpp")));
}

/**
 * @brief A 'source/...' path reads the bundled file and echoes the prefixed display path; a
 *        '..' that climbs out of the prefix is normalized into the workspace lane before the
 *        prefix check, and a resource path outside the root is refused by containment.
 */
void TstFileSandbox::sourcePrefixReadsAFile()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("source/core/Example.cpp"));

  const auto out = AI::FileSandbox::instance().read(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("path")).toString(), QStringLiteral("source/core/Example.cpp"));
  QVERIFY(
    out.value(QStringLiteral("content")).toString().contains(QStringLiteral("checksumSentinel")));

  QJsonObject climbed;
  climbed.insert(QStringLiteral("path"), QStringLiteral("source/../notes.txt"));
  const auto workspace = AI::FileSandbox::instance().read(climbed);
  QVERIFY(workspace.value(QStringLiteral("ok")).toBool());
  QCOMPARE(workspace.value(QStringLiteral("path")).toString(), QStringLiteral("notes.txt"));

  QJsonObject elsewhere;
  elsewhere.insert(QStringLiteral("path"), QStringLiteral(":/elsewhere/secret.txt"));
  const auto refused = AI::FileSandbox::instance().read(elsewhere);
  QVERIFY(!refused.value(QStringLiteral("ok")).toBool());
  QCOMPARE(refused.value(QStringLiteral("error")).toString(), QStringLiteral("outside_sandbox"));
}

/**
 * @brief A real workspace folder named 'source' is shadowed by the prefix but stays reachable
 *        through a raw './source' spelling, which opts out before path normalization.
 */
void TstFileSandbox::dotSlashReachesAShadowedWorkspaceFolder()
{
  QVERIFY(QDir(m_workspace.path()).mkpath(QStringLiteral("source")));
  QFile real(m_workspace.path() + QStringLiteral("/source/real.txt"));
  QVERIFY(real.open(QIODevice::WriteOnly));
  real.write("workspace, not bundle\n");
  real.close();

  QJsonObject shadowed;
  shadowed.insert(QStringLiteral("path"), QStringLiteral("source/real.txt"));
  const auto bundle = AI::FileSandbox::instance().read(shadowed);
  QVERIFY(!bundle.value(QStringLiteral("ok")).toBool());

  QJsonObject dotted;
  dotted.insert(QStringLiteral("path"), QStringLiteral("./source/real.txt"));
  const auto out = AI::FileSandbox::instance().read(dotted);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QVERIFY(out.value(QStringLiteral("content"))
            .toString()
            .contains(QStringLiteral("workspace, not bundle")));
}

/**
 * @brief A search scoped to 'source' walks only the bundle: the workspace needle is absent and
 *        hits carry the prefixed path.
 */
void TstFileSandbox::sourceScopedSearchFindsOnlySourceHits()
{
  QJsonObject args;
  args.insert(QStringLiteral("query"), QStringLiteral("Sentinel"));
  args.insert(QStringLiteral("path"), QStringLiteral("source"));

  const auto out = AI::FileSandbox::instance().search(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("count")).toInt(), 1);

  const auto hit = out.value(QStringLiteral("hits")).toArray().at(0).toObject();
  QCOMPARE(hit.value(QStringLiteral("file")).toString(), QStringLiteral("source/core/Example.cpp"));

  QJsonObject workspace;
  workspace.insert(QStringLiteral("query"), QStringLiteral("needle"));
  workspace.insert(QStringLiteral("path"), QStringLiteral("source"));
  QCOMPARE(AI::FileSandbox::instance().search(workspace).value(QStringLiteral("count")).toInt(), 0);
}

/**
 * @brief Without a scope the walk is the workspace plus dropped paths exactly as before: a
 *        needle that lives only in the bundle is not found.
 */
void TstFileSandbox::defaultSearchDoesNotReachTheSource()
{
  QJsonObject args;
  args.insert(QStringLiteral("query"), QStringLiteral("checksumSentinel"));

  const auto out = AI::FileSandbox::instance().search(args);
  QVERIFY(out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("count")).toInt(), 0);
}

/**
 * @brief Every writing verb refuses a path under the source prefix with the read-only reason,
 *        and nothing lands under 'AI/source/'.
 */
void TstFileSandbox::writeUnderSourcePrefixIsRefused()
{
  auto& sandbox = AI::FileSandbox::instance();

  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("source/core/Example.cpp"));
  args.insert(QStringLiteral("content"), QStringLiteral("tampered"));

  const auto written = sandbox.write(args);
  QVERIFY(!written.value(QStringLiteral("ok")).toBool());
  QCOMPARE(written.value(QStringLiteral("error")).toString(), QStringLiteral("read_only_root"));

  const auto appended = sandbox.append(args);
  QCOMPARE(appended.value(QStringLiteral("error")).toString(), QStringLiteral("read_only_root"));

  const auto removed = sandbox.remove(args);
  QCOMPARE(removed.value(QStringLiteral("error")).toString(), QStringLiteral("read_only_root"));

  QVERIFY(!QDir(m_workspace.path() + QStringLiteral("/AI/source")).exists());

  QFile file(m_source.path() + QStringLiteral("/core/Example.cpp"));
  QVERIFY(file.open(QIODevice::ReadOnly));
  QVERIFY(!QString::fromUtf8(file.readAll()).contains(QStringLiteral("tampered")));
}

/**
 * @brief A resource-scheme path is refused for writing before any join with the write root.
 */
void TstFileSandbox::resourceSchemePathIsRefusedForWrite()
{
  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral(":/source/core/Example.cpp"));
  args.insert(QStringLiteral("content"), QStringLiteral("tampered"));

  const auto out = AI::FileSandbox::instance().write(args);
  QVERIFY(!out.value(QStringLiteral("ok")).toBool());
  QCOMPARE(out.value(QStringLiteral("error")).toString(), QStringLiteral("read_only_root"));
}

//--------------------------------------------------------------------------------------------------
// Worker lane
//--------------------------------------------------------------------------------------------------

/**
 * @brief Only the two read-only tools take the worker lane; every writing tool stays inline so
 *        its effect is ordered against the rest of the turn.
 */
void TstFileSandbox::asyncRunnerTakesOnlyTheReadTools()
{
  QVERIFY(AI::AsyncToolRunner::handles(QStringLiteral("fs.read")));
  QVERIFY(AI::AsyncToolRunner::handles(QStringLiteral("fs.search")));
  QVERIFY(!AI::AsyncToolRunner::handles(QStringLiteral("fs.write")));
  QVERIFY(!AI::AsyncToolRunner::handles(QStringLiteral("fs.delete")));
  QVERIFY(!AI::AsyncToolRunner::handles(QStringLiteral("project.save")));
}

/**
 * @brief A queued read completes on the caller's thread and echoes the turn generation back, so
 *        a superseded turn can drop the result instead of answering a tool_use it never made.
 */
void TstFileSandbox::asyncRunnerReportsResultWithItsGeneration()
{
  AI::AsyncToolRunner runner;
  QSignalSpy done(&runner, &AI::AsyncToolRunner::toolFinished);

  QJsonObject args;
  args.insert(QStringLiteral("path"), QStringLiteral("notes.txt"));
  runner.run(QStringLiteral("call-1"), QStringLiteral("fs.read"), args, 7);

  QVERIFY(done.wait(5000));
  QCOMPARE(done.count(), 1);
  QCOMPARE(done.at(0).at(0).toString(), QStringLiteral("call-1"));
  QCOMPARE(done.at(0).at(4).toULongLong(), static_cast<qulonglong>(7));

  const auto reply = done.at(0).at(3).toJsonObject();
  QVERIFY(reply.value(QStringLiteral("ok")).toBool());
  QVERIFY(reply.value(QStringLiteral("content")).toString().contains(QStringLiteral("needle")));
}

QTEST_GUILESS_MAIN(TstFileSandbox)

#include "tst_file_sandbox.moc"
