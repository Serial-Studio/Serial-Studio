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

#include <QSet>
#include <QString>
#include <QTest>
#include <vector>

#include "DataModel/Project/ProjectFolders.h"

// The folder-tree templates are duck-typed on folderId/parentFolderId/title, so the suite drives
// them through a local POD instead of a project entity: the arithmetic under test is the same one
// the workspace, group and table trees all share, and none of it needs a ProjectModel.

/**
 * @brief Minimal stand-in for WorkspaceFolder/GroupFolder/TableFolder.
 */
struct TestFolder {
  int folderId;
  int parentFolderId;
  QString title;
};

/**
 * @brief Builds a folder vector from {id, parent, title} triples.
 */
static std::vector<TestFolder> tree()
{
  return {
    {1, -1,       QStringLiteral("root")},
    {2,  1,      QStringLiteral("child")},
    {3,  2, QStringLiteral("grandchild")},
    {4, -1,    QStringLiteral("sibling")}
  };
}

/**
 * @brief Cycle safety, subtree containment and path building for the shared folder-tree math.
 */
class TstProjectFolders : public QObject {
  Q_OBJECT

private slots:
  void folderExistsFindsOnlyRealIds();
  void folderParentIdWalksOneLevel();
  void folderIsSelfOrDescendantAcceptsSelfAndSubtree();
  void folderIsSelfOrDescendantRejectsOutsiders();
  void folderIsSelfOrDescendantTerminatesOnCycle();
  void folderHasSelectedAncestorSkipsSelf();
  void folderHasSelectedAncestorTerminatesOnCycle();
  void folderDisplayPathJoinsRootToLeaf();
  void folderDisplayPathStopsAtMissingParent();
};

//--------------------------------------------------------------------------------------------------
// Existence and parentage
//--------------------------------------------------------------------------------------------------

/**
 * @brief folderExists() is the guard every "file into folder N" path uses before accepting N.
 */
void TstProjectFolders::folderExistsFindsOnlyRealIds()
{
  const auto folders = tree();

  QVERIFY(DataModel::folderExists(folders, 1));
  QVERIFY(DataModel::folderExists(folders, 3));
  QVERIFY(!DataModel::folderExists(folders, 99));
  QVERIFY(!DataModel::folderExists(folders, -1));
}

/**
 * @brief A top-level folder and an unknown id both report -1, which is what makes -1 usable as the
 *        "top level" sentinel throughout the folder API.
 */
void TstProjectFolders::folderParentIdWalksOneLevel()
{
  const auto folders = tree();

  QCOMPARE(DataModel::folderParentId(folders, 3), 2);
  QCOMPARE(DataModel::folderParentId(folders, 2), 1);
  QCOMPARE(DataModel::folderParentId(folders, 1), -1);
  QCOMPARE(DataModel::folderParentId(folders, 99), -1);
}

//--------------------------------------------------------------------------------------------------
// Subtree containment (the cyclic-move guard)
//--------------------------------------------------------------------------------------------------

/**
 * @brief A folder is its own descendant for this predicate, because re-parenting a folder into
 *        itself is exactly the move the guard exists to reject.
 */
void TstProjectFolders::folderIsSelfOrDescendantAcceptsSelfAndSubtree()
{
  const auto folders = tree();

  QVERIFY(DataModel::folderIsSelfOrDescendant(folders, 1, 1));
  QVERIFY(DataModel::folderIsSelfOrDescendant(folders, 1, 2));
  QVERIFY(DataModel::folderIsSelfOrDescendant(folders, 1, 3));
  QVERIFY(DataModel::folderIsSelfOrDescendant(folders, 2, 3));
}

/**
 * @brief A sibling branch, a parent, and the top level are all outside the subtree.
 */
void TstProjectFolders::folderIsSelfOrDescendantRejectsOutsiders()
{
  const auto folders = tree();

  QVERIFY(!DataModel::folderIsSelfOrDescendant(folders, 2, 1));
  QVERIFY(!DataModel::folderIsSelfOrDescendant(folders, 1, 4));
  QVERIFY(!DataModel::folderIsSelfOrDescendant(folders, 3, 4));
  QVERIFY(!DataModel::folderIsSelfOrDescendant(folders, 1, -1));
}

/**
 * @brief A hand-edited file can hand the loader a parent cycle; the walk is bounded by the folder
 *        count so it terminates instead of hanging the sanitizer.
 */
void TstProjectFolders::folderIsSelfOrDescendantTerminatesOnCycle()
{
  const std::vector<TestFolder> cyclic = {
    {1, 2, QStringLiteral("a")},
    {2, 1, QStringLiteral("b")}
  };

  QVERIFY(DataModel::folderIsSelfOrDescendant(cyclic, 1, 2));
  QVERIFY(!DataModel::folderIsSelfOrDescendant(cyclic, 3, 2));
}

//--------------------------------------------------------------------------------------------------
// Selected-ancestor test (bulk duplicate skips nested folders)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The predicate looks at ancestors only: a folder selected on its own is not "covered" by
 *        itself, which is what lets a lone folder in a multi-selection still be duplicated.
 */
void TstProjectFolders::folderHasSelectedAncestorSkipsSelf()
{
  const auto folders = tree();

  const QSet<int> onlyRoot = {1};
  QVERIFY(!DataModel::folderHasSelectedAncestor(folders, 1, onlyRoot));
  QVERIFY(DataModel::folderHasSelectedAncestor(folders, 2, onlyRoot));
  QVERIFY(DataModel::folderHasSelectedAncestor(folders, 3, onlyRoot));
  QVERIFY(!DataModel::folderHasSelectedAncestor(folders, 4, onlyRoot));

  const QSet<int> onlyChild = {2};
  QVERIFY(!DataModel::folderHasSelectedAncestor(folders, 1, onlyChild));
  QVERIFY(DataModel::folderHasSelectedAncestor(folders, 3, onlyChild));
}

/**
 * @brief Same bound as the containment walk: a cycle must not spin the bulk-duplicate pass.
 */
void TstProjectFolders::folderHasSelectedAncestorTerminatesOnCycle()
{
  const std::vector<TestFolder> cyclic = {
    {1, 2, QStringLiteral("a")},
    {2, 1, QStringLiteral("b")}
  };

  const QSet<int> chosen = {9};
  QVERIFY(!DataModel::folderHasSelectedAncestor(cyclic, 1, chosen));
}

//--------------------------------------------------------------------------------------------------
// Display paths (the table accessor key)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The path is built root-first and slash-joined; it is the key scripts address tables by,
 *        so the ordering is load-bearing rather than cosmetic.
 */
void TstProjectFolders::folderDisplayPathJoinsRootToLeaf()
{
  const auto folders = tree();

  QCOMPARE(DataModel::folderDisplayPath(folders, 3), QStringLiteral("root/child/grandchild"));
  QCOMPARE(DataModel::folderDisplayPath(folders, 2), QStringLiteral("root/child"));
  QCOMPARE(DataModel::folderDisplayPath(folders, 1), QStringLiteral("root"));
  QCOMPARE(DataModel::folderDisplayPath(folders, -1), QString());
}

/**
 * @brief A dangling parent truncates the path instead of dropping the segment or looping.
 */
void TstProjectFolders::folderDisplayPathStopsAtMissingParent()
{
  const std::vector<TestFolder> orphaned = {
    {5, 99, QStringLiteral("orphan")}
  };

  QCOMPARE(DataModel::folderDisplayPath(orphaned, 5), QStringLiteral("orphan"));
  QCOMPARE(DataModel::folderDisplayPath(orphaned, 99), QString());
}

QTEST_APPLESS_MAIN(TstProjectFolders)

#include "tst_project_folders.moc"
