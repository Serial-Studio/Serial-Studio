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

#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include "API/ScriptCommandPolicy.h"

// What a project's own scripts are refused (spec 0120 R10). The 2026-11-12 rescan found a script
// could add an extension repository and install from it, change the licence, and save over another
// project so that project inherited its launch consent. A loaded project is trusted, so these are
// refused outright, never prompted.

namespace Policy = API::ScriptCommandPolicy;

class ScriptCommandPolicyTest : public QObject {
  Q_OBJECT

private slots:
  void everyListedCommandIsRefused_data();
  void everyListedCommandIsRefused();
  void ordinaryCommandsRun();
  void savingTheProjectsOwnFileRuns();
  void savingWithoutAPathRuns();
  void savingToAnotherFileIsRefused();
  void savingToAPathWithNoProjectFileIsRefused();
  void theOwnFileWithoutItsSuffixRuns();
  void aPathTheSaverWouldExtendIsRefused();
  void aCaseVariantOfTheOwnFileIsRefused();
  void savingThroughADialogIsRefused();
  void aBareSaveWithNoProjectFileIsRefused();
};

/**
 * @brief The nine extension and licensing commands.
 */
void ScriptCommandPolicyTest::everyListedCommandIsRefused_data()
{
  QTest::addColumn<QString>("name");
  for (const auto& refused : Policy::kRefused)
    QTest::newRow(refused.data()) << QString(refused);
}

/**
 * @brief Each is refused with an error that names it.
 */
void ScriptCommandPolicyTest::everyListedCommandIsRefused()
{
  QFETCH(QString, name);
  const auto refusal = Policy::refusal(name, QJsonObject(), QStringLiteral("/p/a.ssproj"));
  QVERIFY(refusal.has_value());
  QVERIFY(refusal->contains(name));
}

/**
 * @brief Everything else keeps today's trust, including the read-only extension and licence
 *        queries.
 */
void ScriptCommandPolicyTest::ordinaryCommandsRun()
{
  const QString project = QStringLiteral("/p/a.ssproj");
  QVERIFY(!Policy::refusal(QStringLiteral("io.writeData"), {}, project).has_value());
  QVERIFY(!Policy::refusal(QStringLiteral("extensions.list"), {}, project).has_value());
  QVERIFY(!Policy::refusal(QStringLiteral("licensing.getStatus"), {}, project).has_value());
  QVERIFY(!Policy::refusal(QStringLiteral("project.open"), {}, project).has_value());
}

/**
 * @brief A script saving its own project, by its own path, still works.
 */
void ScriptCommandPolicyTest::savingTheProjectsOwnFileRuns()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto own = dir.filePath(QStringLiteral("own.ssproj"));
  QFile file(own);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.close();

  const QJsonObject params{
    {QStringLiteral("filePath"), own}
  };
  QVERIFY(!Policy::refusal(QStringLiteral("project.save"), params, own).has_value());
}

/**
 * @brief Omitting the path saves the project's own file.
 */
void ScriptCommandPolicyTest::savingWithoutAPathRuns()
{
  QVERIFY(!Policy::refusal(QStringLiteral("project.save"), {}, QStringLiteral("/p/a.ssproj"))
             .has_value());
}

/**
 * @brief Saving over any other file is refused: that file's launch consent would follow.
 */
void ScriptCommandPolicyTest::savingToAnotherFileIsRefused()
{
  const QJsonObject params{
    {QStringLiteral("filePath"), QStringLiteral("/p/trusted.ssproj")}
  };
  const auto refusal =
    Policy::refusal(QStringLiteral("project.save"), params, QStringLiteral("/p/a.ssproj"));
  QVERIFY(refusal.has_value());
}

/**
 * @brief A project that has no file yet cannot be saved over a path a script chose.
 */
void ScriptCommandPolicyTest::savingToAPathWithNoProjectFileIsRefused()
{
  const QJsonObject params{
    {QStringLiteral("filePath"), QStringLiteral("/p/trusted.ssproj")}
  };
  QVERIFY(Policy::refusal(QStringLiteral("project.save"), params, QString()).has_value());
}

/**
 * @brief The saver appends ".ssproj", so the project's own path without it is still its own file.
 */
void ScriptCommandPolicyTest::theOwnFileWithoutItsSuffixRuns()
{
  const QJsonObject params{
    {QStringLiteral("filePath"), QStringLiteral("/p/a")}
  };
  QVERIFY(!Policy::refusal(QStringLiteral("project.save"), params, QStringLiteral("/p/a.ssproj"))
             .has_value());
}

/**
 * @brief A project opened from a legacy .json file names its own path, but the saver would write
 *        "a.json.ssproj", another file.
 */
void ScriptCommandPolicyTest::aPathTheSaverWouldExtendIsRefused()
{
  const QJsonObject params{
    {QStringLiteral("filePath"), QStringLiteral("/p/a.json")}
  };
  QVERIFY(Policy::refusal(QStringLiteral("project.save"), params, QStringLiteral("/p/a.json"))
            .has_value());
}

/**
 * @brief Case is compared exactly, so a case variant fails closed on a case-sensitive volume.
 */
void ScriptCommandPolicyTest::aCaseVariantOfTheOwnFileIsRefused()
{
  const QJsonObject params{
    {QStringLiteral("filePath"), QStringLiteral("/p/A.ssproj")}
  };
  QVERIFY(Policy::refusal(QStringLiteral("project.save"), params, QStringLiteral("/p/a.ssproj"))
            .has_value());
}

/**
 * @brief askPath opens a Save dialog: the file is not the project's own, and a script calling it
 *        in a loop would prompt once per call.
 */
void ScriptCommandPolicyTest::savingThroughADialogIsRefused()
{
  const QJsonObject params{
    {QStringLiteral("askPath"), true}
  };
  QVERIFY(Policy::refusal(QStringLiteral("project.save"), params, QStringLiteral("/p/a.ssproj"))
            .has_value());
}

/**
 * @brief A bare save of a project with no file yet opens the Save dialog too.
 */
void ScriptCommandPolicyTest::aBareSaveWithNoProjectFileIsRefused()
{
  QVERIFY(Policy::refusal(QStringLiteral("project.save"), {}, QString()).has_value());
}

QTEST_APPLESS_MAIN(ScriptCommandPolicyTest)

#include "tst_script_command_policy.moc"
