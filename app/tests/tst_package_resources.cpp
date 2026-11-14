/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

#include "API/PathPolicy.h"
#include "Core/Deploy/PackageFormat.h"
#include "Core/Deploy/ResourceExtractor.h"
#include "Core/Runtime.h"

/**
 * @brief Writes @p data to @p path, creating its folder; false when it cannot be written.
 */
static bool writeFile(const QString& path, const QByteArray& data)
{
  QFile file(path);
  return QDir().mkpath(QFileInfo(path).absolutePath()) && file.open(QIODevice::WriteOnly)
      && file.write(data) == data.size();
}

/**
 * @brief Reads a whole file, or returns a null array when it cannot be opened.
 */
static QByteArray readFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return {};

  return file.readAll();
}

/**
 * @brief Package resources (spec 0123 AC1, AC2, AC4, AC7): a resource is carried at its path
 *        relative to the project folder, only resource entries reach disk and none can leave the
 *        session's folder, a leftover folder is swept while a live one is kept, and once sealed
 *        no API path reaches the resources. The packager and the command refusals need a
 *        running app and are not covered here.
 */
class TstPackageResources : public QObject {
  Q_OBJECT

private slots:
  void carriesFilesAndFoldersAtRelativePaths();
  void refusesFilesOutsideTheProjectFolder();
  void extractsOnlyResourceEntries();
  void refusesEntriesThatLeaveTheFolder();
  void reportsAnUnwritableTarget();
  void removesTheFolderWhenTheSessionEnds();
  void sweepsLeftoversAndKeepsLiveFolders();
  void sealedHelpersGuardTheResourcesFolder();
};

/**
 * @brief AC1: a file maps to resources/ plus its relative path, and a folder stands for every
 *        regular file under it.
 */
void TstPackageResources::carriesFilesAndFoldersAtRelativePaths()
{
  QTemporaryDir project;
  QVERIFY(project.isValid());
  const auto root = QFileInfo(project.path()).canonicalFilePath();
  QVERIFY(writeFile(root + QStringLiteral("/Simulator.py"), "print('sim')"));
  QVERIFY(writeFile(root + QStringLiteral("/lib/model.py"), "x = 1"));
  QVERIFY(writeFile(root + QStringLiteral("/lib/data/table.csv"), "a,b"));

  using Deploy::ResourceExtractor;
  QCOMPARE(ResourceExtractor::entryPathFor(root, root + QStringLiteral("/Simulator.py")),
           QStringLiteral("resources/Simulator.py"));
  QCOMPARE(ResourceExtractor::entryPathFor(root, root + QStringLiteral("/lib/data/table.csv")),
           QStringLiteral("resources/lib/data/table.csv"));

  const auto single = ResourceExtractor::filesUnder(root + QStringLiteral("/Simulator.py"));
  QCOMPARE(single, QStringList{root + QStringLiteral("/Simulator.py")});

  const auto folder = ResourceExtractor::filesUnder(root + QStringLiteral("/lib"));
  const QStringList expected{root + QStringLiteral("/lib/data/table.csv"),
                             root + QStringLiteral("/lib/model.py")};
  QCOMPARE(folder, expected);
  QVERIFY(ResourceExtractor::filesUnder(root + QStringLiteral("/missing")).isEmpty());

  QVERIFY(ResourceExtractor::isResourceEntry(QStringLiteral("resources/Simulator.py")));
  QVERIFY(!ResourceExtractor::isResourceEntry(QStringLiteral("resources/")));
  QVERIFY(!ResourceExtractor::isResourceEntry(QStringLiteral("project/Simulator.py")));
  QVERIFY(!ResourceExtractor::isResourceEntry(QStringLiteral("certs/resources/key.pem")));
}

/**
 * @brief AC1: a file outside the project folder, or one the codec cannot name, has no entry path.
 */
void TstPackageResources::refusesFilesOutsideTheProjectFolder()
{
  QTemporaryDir project;
  QTemporaryDir elsewhere;
  QVERIFY(project.isValid() && elsewhere.isValid());
  QVERIFY(writeFile(elsewhere.path() + QStringLiteral("/tool.py"), "pass"));
  QVERIFY(writeFile(project.path() + QStringLiteral("/.session.lock"), "x"));

  using Deploy::ResourceExtractor;
  QVERIFY(
    ResourceExtractor::entryPathFor(project.path(), elsewhere.path() + QStringLiteral("/tool.py"))
      .isEmpty());
  QVERIFY(
    ResourceExtractor::entryPathFor(project.path(), project.path() + QStringLiteral("/absent.py"))
      .isEmpty());
  QVERIFY(ResourceExtractor::entryPathFor(project.path(),
                                          project.path() + QStringLiteral("/.session.lock"))
            .isEmpty());
  QVERIFY(ResourceExtractor::entryPathFor(QString(), project.path()).isEmpty());
}

/**
 * @brief AC2: resource entries land at their relative paths with their bytes, in a folder only
 *        the user can read; project files and certificates in the same list are never written.
 */
void TstPackageResources::extractsOnlyResourceEntries()
{
  QTemporaryDir base;
  QVERIFY(base.isValid());
  const QList<Deploy::FileEntry> files = {
    {QStringLiteral("resources/Simulator.py"), QByteArray("print('sim')")},
    {QStringLiteral("resources/lib/data/table.csv"), QByteArray(4096, 'x')},
    {QStringLiteral("project/logo.png"), QByteArray("image")},
    {QStringLiteral("certs/0-key-client.pem"), QByteArray("secret")},
    {QStringLiteral("extensions/gauge/main.qml"), QByteArray("Item {}")},
  };

  QString error;
  Deploy::ResourceExtractor extractor;
  QVERIFY2(extractor.extract(files, base.path(), error), qPrintable(error));

  const auto folder = extractor.folder();
  QVERIFY(!folder.isEmpty());
  QVERIFY(folder.startsWith(QFileInfo(base.path()).canonicalFilePath() + QLatin1Char('/')));
  QCOMPARE(readFile(folder + QStringLiteral("/Simulator.py")), QByteArray("print('sim')"));
  QCOMPARE(readFile(folder + QStringLiteral("/lib/data/table.csv")), QByteArray(4096, 'x'));

  QStringList written;
  QDirIterator it(folder, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
  while (it.hasNext())
    written.append(QDir(folder).relativeFilePath(it.next()));

  written.sort();
  const QStringList expected{QStringLiteral(".session.lock"),
                             QStringLiteral("Simulator.py"),
                             QStringLiteral("lib/data/table.csv")};
  QCOMPARE(written, expected);

#ifndef Q_OS_WIN
  const auto others = QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup
                    | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;
  QVERIFY(!(QFileInfo(folder).permissions() & others));
#endif
}

/**
 * @brief AC2: an entry whose name would climb out of the folder fails the whole extraction and
 *        leaves nothing behind.
 */
void TstPackageResources::refusesEntriesThatLeaveTheFolder()
{
  QTemporaryDir base;
  QVERIFY(base.isValid());
  const QList<Deploy::FileEntry> files = {
    {           QStringLiteral("resources/ok.py"),  QByteArray("ok")},
    {QStringLiteral("resources/../../escaped.py"), QByteArray("bad")},
  };

  QString error;
  Deploy::ResourceExtractor extractor;
  QVERIFY(!extractor.extract(files, base.path(), error));
  QVERIFY(!error.isEmpty());
  QVERIFY(extractor.folder().isEmpty());
  QVERIFY(!QFileInfo::exists(base.path() + QStringLiteral("/escaped.py")));
  QVERIFY(QDir(base.path()).entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty());

  const QStringList refused{QStringLiteral("resources/.session.lock"),
                            QStringLiteral("resources/COM1"),
                            QStringLiteral("resources/lib/nul.txt"),
                            QStringLiteral("resources/name."),
                            QStringLiteral("resources/name ")};
  for (const auto& name : refused) {
    const QList<Deploy::FileEntry> one = {
      {name, QByteArray("x")}
    };
    Deploy::ResourceExtractor other;
    QVERIFY2(!other.extract(one, base.path(), error), qPrintable(name));
    QVERIFY(QDir(base.path()).entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty());
  }
}

/**
 * @brief AC2 (R6): a base folder that does not exist is reported, not asserted.
 */
void TstPackageResources::reportsAnUnwritableTarget()
{
  QTemporaryDir base;
  QVERIFY(base.isValid());
  const QList<Deploy::FileEntry> files = {
    {QStringLiteral("resources/a.py"), QByteArray("a")},
  };

  QString error;
  Deploy::ResourceExtractor extractor;
  QVERIFY(!extractor.extract(files, base.path() + QStringLiteral("/no/such/folder"), error));
  QVERIFY(!error.isEmpty());
  QVERIFY(extractor.folder().isEmpty());
}

/**
 * @brief R5: the folder goes away with the extractor that owns it.
 */
void TstPackageResources::removesTheFolderWhenTheSessionEnds()
{
  QTemporaryDir base;
  QVERIFY(base.isValid());
  const QList<Deploy::FileEntry> files = {
    {QStringLiteral("resources/a.py"), QByteArray("a")},
  };

  QString folder;
  {
    QString error;
    Deploy::ResourceExtractor extractor;
    QVERIFY2(extractor.extract(files, base.path(), error), qPrintable(error));
    folder = extractor.folder();
    QVERIFY(QFileInfo::exists(folder + QStringLiteral("/a.py")));
  }

  QVERIFY(!QFileInfo::exists(folder));
}

/**
 * @brief AC4: the sweep removes a folder no session holds, keeps one whose session is alive,
 *        keeps one younger than the minimum age, and ignores folders with another name.
 */
void TstPackageResources::sweepsLeftoversAndKeepsLiveFolders()
{
  QTemporaryDir base;
  QVERIFY(base.isValid());
  const QList<Deploy::FileEntry> files = {
    {QStringLiteral("resources/a.py"), QByteArray("a")},
  };

  QString error;
  Deploy::ResourceExtractor live;
  QVERIFY2(live.extract(files, base.path(), error), qPrintable(error));

  const auto leftover  = base.path() + QStringLiteral("/SerialStudio-package-dead01");
  const auto unrelated = base.path() + QStringLiteral("/SomethingElse");
  QVERIFY(writeFile(leftover + QStringLiteral("/lib/a.py"), "a"));
  QVERIFY(writeFile(unrelated + QStringLiteral("/a.py"), "a"));

  Deploy::ResourceExtractor::sweepStale(base.path(), 3600);
  QVERIFY(QFileInfo::exists(leftover));

  Deploy::ResourceExtractor::sweepStale(base.path(), 0);
  QVERIFY(!QFileInfo::exists(leftover));
  QVERIFY(QFileInfo::exists(unrelated));
  QVERIFY(QFileInfo::exists(live.folder() + QStringLiteral("/a.py")));
}

/**
 * @brief AC7: helper programs are off until a seal records them, the first seal wins,
 *        and once sealed no API path may point inside the resources folder while its siblings
 *        stay allowed. Runs last, because a seal never reverts.
 */
void TstPackageResources::sealedHelpersGuardTheResourcesFolder()
{
  qunsetenv("SERIAL_STUDIO_API_ALLOWED_PATHS");
  QVERIFY(!Core::Runtime::sealedHelpers().enabled);
  QVERIFY(Core::Runtime::sealedHelpers().resourceFolder.isEmpty());

  QTemporaryDir base(QDir::tempPath() + QStringLiteral("/tst-package-resources-XXXXXX"));
  QVERIFY(base.isValid());
  const auto resources = base.path() + QStringLiteral("/SerialStudio-package-test");
  const auto sibling   = base.path() + QStringLiteral("/SerialStudio-package-testing");
  QVERIFY(writeFile(resources + QStringLiteral("/lib/a.py"), "a"));
  QVERIFY(writeFile(sibling + QStringLiteral("/a.py"), "a"));
  QVERIFY(API::isPathAllowed(resources + QStringLiteral("/lib/a.py"), false));

  Core::Runtime::SealedHelpers helpers;
  helpers.enabled        = true;
  helpers.identity       = QString(64, QLatin1Char('a'));
  helpers.resourceFolder = QFileInfo(resources).canonicalFilePath();
  Core::Runtime::seal(QStringLiteral(":/ssdeploy-test"), QString(), helpers);
  QVERIFY(Core::Runtime::sealed());
  QVERIFY(Core::Runtime::sealedHelpers().enabled);

  QVERIFY(!API::isPathAllowed(resources, false));
  QVERIFY(!API::isPathAllowed(resources + QStringLiteral("/lib/a.py"), false));
  QVERIFY(!API::isPathAllowed(resources + QStringLiteral("/new.txt"), true));
  QVERIFY(!API::isPathAllowed(resources + QStringLiteral("/lib/../lib/a.py"), false));
  QVERIFY(!API::isPathAllowed(resources.toUpper() + QStringLiteral("/lib/a.py"), true));
  QVERIFY(API::isPathAllowed(sibling + QStringLiteral("/a.py"), false));

  Core::Runtime::seal(
    QStringLiteral(":/ssdeploy-other"), QString(), Core::Runtime::SealedHelpers());
  QVERIFY(Core::Runtime::sealedHelpers().enabled);
  QCOMPARE(Core::Runtime::sealedHelpers().identity, QString(64, QLatin1Char('a')));
}

QTEST_GUILESS_MAIN(TstPackageResources)

#include "tst_package_resources.moc"
