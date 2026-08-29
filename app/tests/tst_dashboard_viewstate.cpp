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

#include <QJsonDocument>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

#include "UI/Dashboard/DashboardViewState.h"

// Session view state (spec 0062) and the layout preferences that share its persistence gate.
// Two properties carry the weight. First, a mutator answers true only for a real change: the
// Dashboard emits viewStateChanged off that answer, and a recording bundle debounces on the
// signal, so a repaint that rewrites the same cursor position must stay silent. Second, the
// persistence gate is asymmetric on purpose -- an ephemeral session (the in-app benchmark)
// turns writes off, but the toolbar policy is a global preference and keeps writing.

class DashboardViewStateTest : public QObject {
  Q_OBJECT

private slots:
  void unchangedWidgetValueReportsNoChange();
  void emptyIdentifiersAreRejected();
  void globalValuesFollowTheSameRule();
  void jsonRoundTripsBothScopes();
  void malformedJsonClearsTheState();
  void clearIsIdempotent();
  void layoutValuesClampToTheirFloors();
  void persistenceGateSuppressesLayoutWrites();
  void autoHideToolbarPersistsThroughTheGate();
  void restoreReadsBothLayoutKeyGenerations();
};

//--------------------------------------------------------------------------------------------------
// Session view state
//--------------------------------------------------------------------------------------------------

/**
 * @brief A second write of the same value is not a change, so nothing downstream is notified.
 */
void DashboardViewStateTest::unchangedWidgetValueReportsNoChange()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(state.saveWidgetViewState("plot:7", "zoom", 2.5));
  QVERIFY(!state.saveWidgetViewState("plot:7", "zoom", 2.5));
  QVERIFY(state.saveWidgetViewState("plot:7", "zoom", 3.0));
  QCOMPARE(state.widgetViewState("plot:7").value("zoom").toDouble(), 3.0);
}

/**
 * @brief An empty widget id or key is dropped rather than creating a nameless entry.
 */
void DashboardViewStateTest::emptyIdentifiersAreRejected()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(!state.saveWidgetViewState("", "zoom", 1.0));
  QVERIFY(!state.saveWidgetViewState("plot:7", "", 1.0));
  QVERIFY(!state.saveGlobalViewState("", 1.0));
  QVERIFY(state.globalViewState().isEmpty());
  QVERIFY(state.widgetViewState("plot:7").isEmpty());
}

/**
 * @brief Global values honor the same change contract as per-widget ones.
 */
void DashboardViewStateTest::globalValuesFollowTheSameRule()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(state.saveGlobalViewState("workspace", 2));
  QVERIFY(!state.saveGlobalViewState("workspace", 2));
  QCOMPARE(state.globalViewState().value("workspace").toInt(), 2);
}

/**
 * @brief The bundled document carries both scopes and reloads into the same values.
 */
void DashboardViewStateTest::jsonRoundTripsBothScopes()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(state.saveGlobalViewState("theme", QStringLiteral("midnight")));
  QVERIFY(state.saveWidgetViewState("fft:1", "paused", true));

  const auto json = state.viewStateJson();
  const auto doc  = QJsonDocument::fromJson(json.toUtf8());
  QVERIFY(doc.isObject());
  QCOMPARE(doc.object().value("version").toInt(), 1);

  UI::DashboardViewState restored(settings);
  QVERIFY(restored.setViewStateJson(json));
  QCOMPARE(restored.globalViewState().value("theme").toString(), QStringLiteral("midnight"));
  QCOMPARE(restored.widgetViewState("fft:1").value("paused").toBool(), true);
}

/**
 * @brief A recording with a corrupt bundle must not inherit the previous session's cursors.
 */
void DashboardViewStateTest::malformedJsonClearsTheState()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(state.saveGlobalViewState("theme", QStringLiteral("midnight")));
  QVERIFY(state.setViewStateJson(QStringLiteral("not json at all")));
  QVERIFY(state.globalViewState().isEmpty());
}

/**
 * @brief Clearing an already-empty state reports no change, so no signal is emitted.
 */
void DashboardViewStateTest::clearIsIdempotent()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(!state.clearViewState());
  QVERIFY(state.saveWidgetViewState("bar:2", "cursor", 1));
  QVERIFY(state.clearViewState());
  QVERIFY(!state.clearViewState());
}

//--------------------------------------------------------------------------------------------------
// Layout preferences
//--------------------------------------------------------------------------------------------------

/**
 * @brief The margin floors at 0 and the spacing at -1, the value that welds two borders into
 *        one shared line.
 */
void DashboardViewStateTest::layoutValuesClampToTheirFloors()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  QVERIFY(!state.setLayoutMargin(-40));
  QCOMPARE(state.layoutMargin(), 0);
  QVERIFY(state.setLayoutMargin(12));
  QVERIFY(!state.setLayoutMargin(12));

  QVERIFY(!state.setLayoutSpacing(-9));
  QCOMPARE(state.layoutSpacing(), -1);
  QVERIFY(state.setLayoutSpacing(6));
  QCOMPARE(state.layoutSpacing(), 6);
}

/**
 * @brief An ephemeral session turns persistence off: the live value still moves, the store
 *        does not.
 */
void DashboardViewStateTest::persistenceGateSuppressesLayoutWrites()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  state.setSettingsPersistent(false);
  QVERIFY(state.setLayoutMargin(24));
  QVERIFY(state.setShowActionPanel(false));
  QVERIFY(state.setShowAlignmentGuides(true));

  QCOMPARE(state.layoutMargin(), 24);
  QVERIFY(!settings.contains("Dashboard/LayoutMargin"));
  QVERIFY(!settings.contains("Dashboard/ShowActionPanel"));
  QVERIFY(!settings.contains("Dashboard/ShowAlignmentGuides"));
}

/**
 * @brief The toolbar policy is a global preference and writes even while the gate is closed;
 *        this asymmetry is deliberate and is what the facade's setter relies on.
 */
void DashboardViewStateTest::autoHideToolbarPersistsThroughTheGate()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);
  UI::DashboardViewState state(settings);

  state.setSettingsPersistent(false);
  QVERIFY(state.setAutoHideToolbar(true));
  QCOMPARE(settings.value("Dashboard/AutoHideToolbar").toBool(), true);
}

/**
 * @brief Restore prefers the shared layout keys and falls back to the legacy auto-layout-only
 *        keys they replaced, so an upgraded install keeps its spacing.
 */
void DashboardViewStateTest::restoreReadsBothLayoutKeyGenerations()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  QSettings settings(dir.filePath("ss.ini"), QSettings::IniFormat);

  settings.setValue("Dashboard/AutoLayoutMargin", 9);
  settings.setValue("Dashboard/AutoLayoutSpacing", 4);
  settings.setValue("Dashboard/ShowActionPanel", false);

  UI::DashboardViewState legacy(settings);
  legacy.restoreViewPreferences();
  legacy.restoreLayoutPreferences();
  QCOMPARE(legacy.layoutMargin(), 9);
  QCOMPARE(legacy.layoutSpacing(), 4);
  QCOMPARE(legacy.showActionPanel(), false);

  settings.setValue("Dashboard/LayoutMargin", 30);
  settings.setValue("Dashboard/LayoutSpacing", 2);

  UI::DashboardViewState current(settings);
  current.restoreLayoutPreferences();
  QCOMPARE(current.layoutMargin(), 30);
  QCOMPARE(current.layoutSpacing(), 2);
}

QTEST_APPLESS_MAIN(DashboardViewStateTest)

#include "tst_dashboard_viewstate.moc"
