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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QRegularExpression>
#include <QString>
#include <QTest>

#include "Misc/IconRegistryLegacy.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Known-answer inputs
//--------------------------------------------------------------------------------------------------

// One row per sampled prefix in the 381-entry generated table (kLegacyIcons in
// IconRegistryLegacy.cpp), spot-checked at authoring time against app/rcc/icons: every mapped
// target below exists on disk.
static const QString kMigratedTarget("qrc:/icons/code/24/copy.svg");
static const QString kKnownKey("qrc:/icons/code-editor/copy.svg");

/**
 * @brief Byte-level contract of Misc::legacyIconPath() and the generated table it dispatches
 *        through.
 */
class TstLegacyIcons : public QObject {
  Q_OBJECT

private slots:
  void unknownInputReturnedUnchanged();

  void knownPairs_data();
  void knownPairs();

  void uppercasedKeyIsUnmapped();
  void migratedTargetFedBackUnchanged();

  void sampledTargetsMatchQrcShape_data();
  void sampledTargetsMatchQrcShape();
};

//--------------------------------------------------------------------------------------------------
// Identity fallback
//--------------------------------------------------------------------------------------------------

/**
 * @brief An icon URL that never shipped in the pre-spec-0028 tree passes through unchanged, and so
 *        does the empty string: a project file with no icon key must not gain one.
 */
void TstLegacyIcons::unknownInputReturnedUnchanged()
{
  const QString unknown(QStringLiteral("qrc:/icons/nonexistent/made-up.svg"));
  const QString notAnIcon(QStringLiteral("not-an-icon-url"));

  QCOMPARE(Misc::legacyIconPath(QString()), QString());
  QCOMPARE(Misc::legacyIconPath(unknown), unknown);
  QCOMPARE(Misc::legacyIconPath(notAnIcon), notAnIcon);
}

//--------------------------------------------------------------------------------------------------
// Known-answer pairs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Ten pairs sampled across ten distinct pre-migration prefixes (code-editor, console, csd,
 *        dashboard-buttons, dashboard-large, dashboard-small, database, taskbar, toolbar,
 *        licensing); every target was spot-checked on disk under app/rcc/icons at authoring time.
 */
void TstLegacyIcons::knownPairs_data()
{
  QTest::addColumn<QString>("legacy");
  QTest::addColumn<QString>("expected");

  QTest::newRow("code-editor/copy")
    << QStringLiteral("qrc:/icons/code-editor/copy.svg") << kMigratedTarget;
  QTest::newRow("console/clear") << QStringLiteral("qrc:/icons/console/clear.svg")
                                 << QStringLiteral("qrc:/icons/console/32/clear.svg");
  QTest::newRow("csd/close") << QStringLiteral("qrc:/icons/csd/close.svg")
                             << QStringLiteral("qrc:/icons/window/32/close.svg");
  QTest::newRow("dashboard-buttons/pause")
    << QStringLiteral("qrc:/icons/dashboard-buttons/pause.svg")
    << QStringLiteral("qrc:/icons/commands/16/pause.svg");
  QTest::newRow("dashboard-large/gauge") << QStringLiteral("qrc:/icons/dashboard-large/gauge.svg")
                                         << QStringLiteral("qrc:/icons/widgets/32/gauge.svg");
  QTest::newRow("dashboard-small/gauge") << QStringLiteral("qrc:/icons/dashboard-small/gauge.svg")
                                         << QStringLiteral("qrc:/icons/widgets/16/gauge.svg");
  QTest::newRow("database/close") << QStringLiteral("qrc:/icons/database/close.svg")
                                  << QStringLiteral("qrc:/icons/database/32/close.svg");
  QTest::newRow("taskbar/clock") << QStringLiteral("qrc:/icons/taskbar/clock.svg")
                                 << QStringLiteral("qrc:/icons/widgets/16/clock.svg");
  QTest::newRow("toolbar/csv") << QStringLiteral("qrc:/icons/toolbar/csv.svg")
                               << QStringLiteral("qrc:/icons/commands/16/csv.svg");
  QTest::newRow("licensing/key") << QStringLiteral("qrc:/icons/licensing/key.svg")
                                 << QStringLiteral("qrc:/icons/licensing/16/key.svg");
}

void TstLegacyIcons::knownPairs()
{
  QFETCH(QString, legacy);
  QFETCH(QString, expected);

  QCOMPARE(Misc::legacyIconPath(legacy), expected);
}

//--------------------------------------------------------------------------------------------------
// Case sensitivity and idempotence
//--------------------------------------------------------------------------------------------------

/**
 * @brief The generated table is a QHash<QString, QString>, which compares case-sensitively, so an
 *        uppercased known key is a miss and falls through to the identity default.
 */
void TstLegacyIcons::uppercasedKeyIsUnmapped()
{
  const auto uppercased = kKnownKey.toUpper();

  QVERIFY(uppercased != kKnownKey);
  QCOMPARE(Misc::legacyIconPath(uppercased), uppercased);
}

/**
 * @brief A path already on the post-migration tree is not itself a table key, so feeding a mapped
 *        value back in is a no-op: re-running the migration on an already-migrated project is safe.
 */
void TstLegacyIcons::migratedTargetFedBackUnchanged()
{
  QCOMPARE(Misc::legacyIconPath(kMigratedTarget), kMigratedTarget);
}

//--------------------------------------------------------------------------------------------------
// Target shape
//--------------------------------------------------------------------------------------------------

void TstLegacyIcons::sampledTargetsMatchQrcShape_data()
{
  QTest::addColumn<QString>("target");

  QTest::newRow("code/24/copy") << kMigratedTarget;
  QTest::newRow("console/32/clear") << QStringLiteral("qrc:/icons/console/32/clear.svg");
  QTest::newRow("window/32/close") << QStringLiteral("qrc:/icons/window/32/close.svg");
  QTest::newRow("commands/16/pause") << QStringLiteral("qrc:/icons/commands/16/pause.svg");
  QTest::newRow("widgets/32/gauge") << QStringLiteral("qrc:/icons/widgets/32/gauge.svg");
  QTest::newRow("widgets/16/gauge") << QStringLiteral("qrc:/icons/widgets/16/gauge.svg");
  QTest::newRow("database/32/close") << QStringLiteral("qrc:/icons/database/32/close.svg");
  QTest::newRow("widgets/16/clock") << QStringLiteral("qrc:/icons/widgets/16/clock.svg");
  QTest::newRow("commands/16/csv") << QStringLiteral("qrc:/icons/commands/16/csv.svg");
  QTest::newRow("licensing/16/key") << QStringLiteral("qrc:/icons/licensing/16/key.svg");
}

/**
 * @brief Every sampled mapped value fits qrc:/icons/<category>/<tier>/<name>.svg with the tier
 *        segment one of the four registry sizes (buttons/ is the icon tree's own tier-exempt
 *        category and would also pass here, though none of the sampled targets land in it).
 */
void TstLegacyIcons::sampledTargetsMatchQrcShape()
{
  QFETCH(QString, target);

  static const QRegularExpression kShape(
    QStringLiteral("^qrc:/icons/[a-z0-9-]+/(16|24|32|48|buttons)/[a-z0-9.-]+\\.svg$"));

  const auto match = kShape.match(target);
  QVERIFY(match.hasMatch());

  const auto tier = match.captured(1);
  QVERIFY(tier == QStringLiteral("buttons") || tier == QStringLiteral("16")
          || tier == QStringLiteral("24") || tier == QStringLiteral("32")
          || tier == QStringLiteral("48"));
}

QTEST_APPLESS_MAIN(TstLegacyIcons)

#include "tst_legacy_icons.moc"
