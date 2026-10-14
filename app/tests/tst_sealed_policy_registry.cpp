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

#include "API/SealedPolicy.h"

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
 * @brief A table as a set of QStrings.
 */
static QSet<QString> toSet(std::span<const QLatin1StringView> table)
{
  QSet<QString> out;
  for (const auto name : table)
    out.insert(QString(name));

  return out;
}

/**
 * @brief The sealed-session command policy (spec 0096 AC6): every command in the API snapshot is
 *        classified exactly once, the allowed table is binary-searchable, and anything unknown is
 *        refused, so a new command fails here instead of leaking from a package session.
 */
class TstSealedPolicyRegistry : public QObject {
  Q_OBJECT

private slots:
  void everySchemaCommandIsClassifiedOnce();
  void tablesNameOnlyRealCommands();
  void allowedTableIsSortedAndUnique();
  void lookupMatchesTheTables();
};

/**
 * @brief AC6: a command in neither table, or in both, fails the sweep by name.
 */
void TstSealedPolicyRegistry::everySchemaCommandIsClassifiedOnce()
{
  const auto schema  = schemaCommands();
  const auto allowed = toSet(API::SealedPolicy::allowedCommands());
  const auto denied  = toSet(API::SealedPolicy::deniedCommands());
  QVERIFY(!schema.isEmpty());

  for (const auto& name : schema) {
    const bool inAllowed = allowed.contains(name);
    const bool inDenied  = denied.contains(name);
    QVERIFY2(inAllowed || inDenied,
             qPrintable(QStringLiteral("%1 has no sealed-session classification").arg(name)));
    QVERIFY2(!(inAllowed && inDenied),
             qPrintable(QStringLiteral("%1 is both allowed and denied").arg(name)));
  }
}

/**
 * @brief A removed or renamed command cannot linger in either table.
 */
void TstSealedPolicyRegistry::tablesNameOnlyRealCommands()
{
  const auto schema = schemaCommands();
  for (const auto name : API::SealedPolicy::allowedCommands())
    QVERIFY2(schema.contains(QString(name)), qPrintable(QString(name)));

  for (const auto name : API::SealedPolicy::deniedCommands())
    QVERIFY2(schema.contains(QString(name)), qPrintable(QString(name)));
}

/**
 * @brief The lookup binary-searches the allowed table, so it must be strictly ascending.
 */
void TstSealedPolicyRegistry::allowedTableIsSortedAndUnique()
{
  const auto allowed = API::SealedPolicy::allowedCommands();
  QVERIFY(!allowed.empty());
  QVERIFY(std::adjacent_find(allowed.begin(),
                             allowed.end(),
                             [](QLatin1StringView a, QLatin1StringView b) { return !(a < b); })
          == allowed.end());

  QCOMPARE(toSet(API::SealedPolicy::deniedCommands()).size(),
           static_cast<qsizetype>(API::SealedPolicy::deniedCommands().size()));
}

/**
 * @brief Every allowed name is found, every denied and unknown name is refused.
 */
void TstSealedPolicyRegistry::lookupMatchesTheTables()
{
  for (const auto name : API::SealedPolicy::allowedCommands())
    QVERIFY2(API::SealedPolicy::allowedWhenSealed(QString(name)), qPrintable(QString(name)));

  for (const auto name : API::SealedPolicy::deniedCommands())
    QVERIFY2(!API::SealedPolicy::allowedWhenSealed(QString(name)), qPrintable(QString(name)));

  QVERIFY(!API::SealedPolicy::allowedWhenSealed(u"project.exportJson"));
  QVERIFY(!API::SealedPolicy::allowedWhenSealed(u"no.such.command"));
  QVERIFY(!API::SealedPolicy::allowedWhenSealed(QStringView()));
}

QTEST_GUILESS_MAIN(TstSealedPolicyRegistry)

#include "tst_sealed_policy_registry.moc"
