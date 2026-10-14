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
#include <QFile>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QScopedPointer>
#include <QTest>

#include "Core/Deploy/ResourceTree.h"

/**
 * @brief Reads a whole resource file, or returns a null array when it cannot be opened.
 */
static QByteArray readResource(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return {};

  return file.readAll();
}

/**
 * @brief The in-memory RCC tree (spec 0096 T3): Qt's own resource code must read back every
 *        entry, list folders, and resolve a multi-file QML extension over qrc.
 */
class TstDeployResourceTree : public QObject {
  Q_OBJECT

private slots:
  void readsBackNestedAndUnicodeEntries();
  void servesTheFullEntryCap();
  void loadsMultiFileQmlOverQrc();
  void rejectsConflictsAndUnsafePaths();
  void unmountsOnDestruction();
};

/**
 * @brief Nested folders and non-ASCII names read back byte-identical and list through QDir.
 */
void TstDeployResourceTree::readsBackNestedAndUnicodeEntries()
{
  const QList<Deploy::FileEntry> files = {
    {QStringLiteral("sounds/alarm.wav"), QByteArray("RIFF....WAVE")},
    {QStringLiteral("images/logo.png"), QByteArray(1000, 'x')},
    {QStringLiteral("images/deep/a/b/c.txt"), QByteArray("deep")},
    {QStringLiteral("unicode/été-日本.txt"), QByteArray("unicode")},
  };

  Deploy::ResourceTree tree;
  QVERIFY(tree.mount(files, QStringLiteral("aaaa000000000001")));
  QCOMPARE(tree.root(), QStringLiteral(":/ssdeploy-aaaa000000000001"));

  for (const auto& file : files)
    QCOMPARE(readResource(tree.root() + QLatin1Char('/') + file.path), file.data);

  const auto listing = QDir(tree.root() + QStringLiteral("/images"))
                         .entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);
  QCOMPARE(listing, QStringList({QStringLiteral("deep"), QStringLiteral("logo.png")}));
  QVERIFY(!QFile::exists(tree.root() + QStringLiteral("/missing.txt")));
}

/**
 * @brief 4096 entries (the codec's cap) all resolve.
 */
void TstDeployResourceTree::servesTheFullEntryCap()
{
  QList<Deploy::FileEntry> files;
  for (int i = 0; i < 4096; ++i)
    files.append(
      {QStringLiteral("bulk/f%1.bin").arg(i, 4, 10, QLatin1Char('0')), QByteArray::number(i)});

  Deploy::ResourceTree tree;
  QVERIFY(tree.mount(files, QStringLiteral("aaaa000000000002")));
  QCOMPARE(QDir(tree.root() + QStringLiteral("/bulk")).entryList(QDir::Files).size(), 4096);
  for (const auto& file : files)
    QCOMPARE(readResource(tree.root() + QLatin1Char('/') + file.path), file.data);
}

/**
 * @brief A widget QML that instantiates a sibling type loads from the tree over qrc.
 */
void TstDeployResourceTree::loadsMultiFileQmlOverQrc()
{
  const QList<Deploy::FileEntry> files = {
    {QStringLiteral("extensions/gauge/widget.qml"),
     QByteArray("import QtQml\nQtObject { property var inner: Inner { }\n"
     "property int answer: inner.value }\n")                          },
    { QStringLiteral("extensions/gauge/Inner.qml"),
     QByteArray("import QtQml\nQtObject { property int value: 42 }\n")},
  };

  Deploy::ResourceTree tree;
  QVERIFY(tree.mount(files, QStringLiteral("aaaa000000000003")));

  QQmlEngine engine;
  QQmlComponent component(
    &engine,
    QUrl(QStringLiteral("qrc") + tree.root() + QStringLiteral("/extensions/gauge/widget.qml")));
  QScopedPointer<QObject> object(component.create());
  QVERIFY2(!object.isNull(), qPrintable(component.errorString()));
  QCOMPARE(object->property("answer").toInt(), 42);
}

/**
 * @brief A path used as both a file and a folder, a duplicate, or an unsafe path builds nothing.
 */
void TstDeployResourceTree::rejectsConflictsAndUnsafePaths()
{
  QVERIFY(Deploy::ResourceTree::build(
            {
              {  QStringLiteral("a"), QByteArray("x")},
              {QStringLiteral("a/b"), QByteArray("y")}
  })
            .isEmpty());
  QVERIFY(Deploy::ResourceTree::build(
            {
              {QStringLiteral("a/b"), QByteArray("x")},
              {  QStringLiteral("a"), QByteArray("y")}
  })
            .isEmpty());
  QVERIFY(Deploy::ResourceTree::build(
            {
              {QStringLiteral("a"), QByteArray("x")},
              {QStringLiteral("a"), QByteArray("y")}
  })
            .isEmpty());
  QVERIFY(Deploy::ResourceTree::build({
                                        {QStringLiteral("../escape"), QByteArray("x")}
  })
            .isEmpty());

  Deploy::ResourceTree tree;
  QVERIFY(!tree.mount(
    {
      {QStringLiteral("ok.txt"), QByteArray("x")}
  },
    QStringLiteral("a/b")));
  QVERIFY(!tree.isMounted());
}

/**
 * @brief Destroying the owner removes its root from the resource system.
 */
void TstDeployResourceTree::unmountsOnDestruction()
{
  const QString path = QStringLiteral(":/ssdeploy-aaaa000000000005/x.txt");
  {
    Deploy::ResourceTree tree;
    QVERIFY(tree.mount(
      {
        {QStringLiteral("x.txt"), QByteArray("x")}
    },
      QStringLiteral("aaaa000000000005")));
    QVERIFY(QFile::exists(path));
  }

  QVERIFY(!QFile::exists(path));
}

QTEST_GUILESS_MAIN(TstDeployResourceTree)

#include "tst_deploy_resource_tree.moc"
