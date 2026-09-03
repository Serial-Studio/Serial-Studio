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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTest>

#include "API/PathPolicy.h"

/**
 * @brief True when a parameter name is shaped like a file-system path.
 */
static bool looksLikeAPathParam(const QString& param)
{
  static const QSet<QString> kNames = {
    QStringLiteral("executable"),
    QStringLiteral("certificate"),
    QStringLiteral("workingDir"),
    QStringLiteral("key"),
  };

  return param.contains(QStringLiteral("path"), Qt::CaseInsensitive) || kNames.contains(param);
}

/**
 * @brief The four parameters that read like a path and are not one: the file-dialog boolean, a
 *        dataset address inside the project tree, the property-name key of the setProperty verbs,
 *        and system.exec's working directory -- control-script only, defaulting to the project
 *        directory, with a program name resolved from PATH that no allowlist root can hold.
 */
static bool isNotAFileSystemPath(const QString& command, const QString& param)
{
  if (param == QStringLiteral("askPath"))
    return true;

  if (param == QStringLiteral("key"))
    return command != QStringLiteral("io.opcua.setUserCertificate");

  if (param == QStringLiteral("path"))
    return command == QStringLiteral("project.dataset.getByPath")
        || command == QStringLiteral("assistant.dataset.resolve");

  return command == QStringLiteral("system.exec");
}

/**
 * @brief The path policy as command metadata (spec 0075 I3/I7): the declaration table itself, and
 *        the sweep that fails when a command in the committed API snapshot takes a path-shaped
 *        parameter nobody declared -- the shape that let sessions.openDatabase, licensing.
 *        activateOffline and assistant.restore write outside the allowlist.
 */
class TstPathPolicyRegistry : public QObject {
  Q_OBJECT

private slots:
  void declarationsAreWellFormed();
  void everySchemaPathParamIsDeclared();
  void inputsRequireExistenceAndOutputsDoNot();
  void undeclaredCommandsCarryNoPolicy();

private:
  [[nodiscard]] static QJsonArray apiSchema();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The committed API snapshot, empty when it cannot be read from the source tree.
 */
QJsonArray TstPathPolicyRegistry::apiSchema()
{
  QFile file(QStringLiteral(SS_API_SCHEMA_PATH));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return QJsonArray();

  const auto document = QJsonDocument::fromJson(file.readAll());
  file.close();
  return document.array();
}

//--------------------------------------------------------------------------------------------------
// The declaration table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every declared command names at least one parameter, and never the same one twice: a
 *        duplicate would silently apply two policies to one value.
 */
void TstPathPolicyRegistry::declarationsAreWellFormed()
{
  const auto commands = apiSchema();
  QVERIFY2(!commands.isEmpty(), "api-schema.json is missing or empty");

  int declaredCommands = 0;
  for (const auto& value : commands) {
    const auto name     = value.toObject().value(QStringLiteral("name")).toString();
    const auto declared = API::declaredPathParams(name);
    if (declared.isEmpty())
      continue;

    ++declaredCommands;
    QSet<QString> seen;
    for (const auto& policy : declared) {
      QVERIFY2(!policy.name.isEmpty(), qPrintable(name));
      QVERIFY2(!seen.contains(policy.name), qPrintable(name + QLatin1Char('.') + policy.name));
      seen.insert(policy.name);
    }
  }

  QVERIFY2(declaredCommands >= 12, "the declaration table lost commands");
}

/**
 * @brief The sweep: walk every command in the snapshot and fail on a path-shaped parameter that
 *        no declaration covers. A new command that takes a path has to add its row.
 */
void TstPathPolicyRegistry::everySchemaPathParamIsDeclared()
{
  const auto commands = apiSchema();
  QVERIFY2(!commands.isEmpty(), "api-schema.json is missing or empty");

  QStringList undeclared;
  for (const auto& value : commands) {
    const auto entry = value.toObject();
    const auto name  = entry.value(QStringLiteral("name")).toString();
    const auto props = entry.value(QStringLiteral("properties")).toObject();

    QSet<QString> declared;
    for (const auto& policy : API::declaredPathParams(name))
      declared.insert(policy.name);

    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
      if (!looksLikeAPathParam(it.key()) || isNotAFileSystemPath(name, it.key()))
        continue;

      if (!declared.contains(it.key()))
        undeclared.append(name + QLatin1Char('.') + it.key());
    }
  }

  QVERIFY2(undeclared.isEmpty(), qPrintable(undeclared.join(QStringLiteral(", "))));
}

/**
 * @brief An input file must already exist; a destination may not. Getting this backwards either
 *        breaks every save-as or lets an open command create the file it claims to read.
 */
void TstPathPolicyRegistry::inputsRequireExistenceAndOutputsDoNot()
{
  const auto open = API::declaredPathParams(QStringLiteral("project.open"));
  QCOMPARE(open.size(), qsizetype(1));
  QCOMPARE(open.first().name, QStringLiteral("filePath"));
  QCOMPARE(open.first().allowMissing, false);

  const auto save = API::declaredPathParams(QStringLiteral("project.save"));
  QCOMPARE(save.size(), qsizetype(1));
  QCOMPARE(save.first().allowMissing, true);

  const auto certificate = API::declaredPathParams(QStringLiteral("io.opcua.setUserCertificate"));
  QCOMPARE(certificate.size(), qsizetype(2));
  QCOMPARE(certificate.at(0).allowMissing, false);
  QCOMPARE(certificate.at(1).allowMissing, false);

  const auto restore = API::declaredPathParams(QStringLiteral("assistant.restore"));
  QCOMPARE(restore.size(), qsizetype(1));
  QCOMPARE(restore.first().name, QStringLiteral("path"));
}

/**
 * @brief A command that takes no path declares none, so the registry skips the check entirely.
 */
void TstPathPolicyRegistry::undeclaredCommandsCarryNoPolicy()
{
  QVERIFY(API::declaredPathParams(QStringLiteral("meta.listCommands")).isEmpty());
  QVERIFY(API::declaredPathParams(QStringLiteral("system.exec")).isEmpty());
  QVERIFY(API::declaredPathParams(QString()).isEmpty());
}

QTEST_GUILESS_MAIN(TstPathPolicyRegistry)

#include "tst_path_policy_registry.moc"
