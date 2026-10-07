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

#include <QObject>
#include <QTest>

#include "Core/ModuleConstruction.h"

/**
 * @file tst_module_construction.cpp
 * @brief The composition-root reach guard (spec 0095 M4): a module reached before it is
 *        available is named with the reason, so the three failure shapes the pinned order can
 *        produce stay distinguishable instead of collapsing into one "before adoption" message.
 */

/**
 * @brief Stand-in for the module whose constructor is running.
 */
class EarlyModule : public QObject {
  Q_OBJECT
};

/**
 * @brief Stand-in for a module later in the pinned order.
 */
class LateModule : public QObject {
  Q_OBJECT
};

class ModuleConstructionTest : public QObject {
  Q_OBJECT

private slots:
  void selfReachNamesUnderConstruction();
  void forwardReachNamesTheConstructingModule();
  void reachOutsideASessionSaysSo();
  void scopesNestAndRestore();
};

/**
 * @brief A module's constructor reaching its own instance() is "still under construction".
 */
void ModuleConstructionTest::selfReachNamesUnderConstruction()
{
  const Core::ModuleConstruction::Scope scope(EarlyModule::staticMetaObject);
  const auto reason = Core::ModuleConstruction::unavailableReason(EarlyModule::staticMetaObject);
  QVERIFY(reason.contains("EarlyModule::instance()"));
  QVERIFY(reason.contains("still under construction"));
}

/**
 * @brief Reaching a module the pinned order has not built yet names the module whose constructor
 *        did the reaching.
 */
void ModuleConstructionTest::forwardReachNamesTheConstructingModule()
{
  const Core::ModuleConstruction::Scope scope(EarlyModule::staticMetaObject);
  const auto reason = Core::ModuleConstruction::unavailableReason(LateModule::staticMetaObject);
  QVERIFY(reason.contains("LateModule::instance() reached before LateModule was constructed"));
  QVERIFY(reason.contains("EarlyModule is constructing"));
}

/**
 * @brief Outside the composition root the module is simply not in a session.
 */
void ModuleConstructionTest::reachOutsideASessionSaysSo()
{
  QCOMPARE(Core::ModuleConstruction::constructing(), nullptr);
  const auto reason = Core::ModuleConstruction::unavailableReason(LateModule::staticMetaObject);
  QVERIFY(reason.contains("outside a session"));
}

/**
 * @brief A scope restores the previous mark, so nested construction reports the right module.
 */
void ModuleConstructionTest::scopesNestAndRestore()
{
  {
    const Core::ModuleConstruction::Scope outer(EarlyModule::staticMetaObject);
    {
      const Core::ModuleConstruction::Scope inner(LateModule::staticMetaObject);
      QCOMPARE(Core::ModuleConstruction::constructing(), &LateModule::staticMetaObject);
    }

    QCOMPARE(Core::ModuleConstruction::constructing(), &EarlyModule::staticMetaObject);
  }

  QCOMPARE(Core::ModuleConstruction::constructing(), nullptr);
}

QTEST_APPLESS_MAIN(ModuleConstructionTest)

#include "tst_module_construction.moc"
