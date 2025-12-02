/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QtTest>
#include "JSON/ProjectModel.h"

class TestProjectModel : public QObject
{
  Q_OBJECT

private slots:
  void testSingleton();
};

void TestProjectModel::testSingleton()
{
  JSON::ProjectModel &instance1 = JSON::ProjectModel::instance();
  JSON::ProjectModel &instance2 = JSON::ProjectModel::instance();
  QCOMPARE(&instance1, &instance2);
}

QTEST_MAIN(TestProjectModel)
#include "test_project_model.moc"
