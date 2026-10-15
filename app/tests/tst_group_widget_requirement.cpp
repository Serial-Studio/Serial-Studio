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

#include <QStringList>
#include <QTest>

#include "Core/SerialStudio.h"

// Group widget requirements (spec 0100 R15/R16): the editor warning and the Problem Center finding
// share one rule, and it must match what the dashboard code binds. The accelerometer and gyroscope
// read exactly three tagged datasets, the 3D plot needs x/y/z, the map lat/lon; order never
// matters.

using SerialStudio::groupMeetsRequirement;

/**
 * @brief Pins SerialStudio::groupWidgetRequirement() and groupMeetsRequirement().
 */
class GroupWidgetRequirementTest : public QObject {
  Q_OBJECT

private slots:
  void accelerometerNeedsExactlyXyz();
  void accelerometerIgnoresOrderAndCase();
  void gyroscopeAcceptsAxisOrAttitudeTags();
  void plot3dNeedsXyzAndToleratesExtras();
  void gpsNeedsLatAndLonAltOptional();
  void listWidgetsNeedOneDataset_data();
  void listWidgetsNeedOneDataset();
  void freeFormWidgetsAcceptAnything();
};

/**
 * @brief Three datasets tagged x, y and z; two, four, or a missing tag fail.
 */
void GroupWidgetRequirementTest::accelerometerNeedsExactlyXyz()
{
  const auto w = SerialStudio::Accelerometer;
  QVERIFY(groupMeetsRequirement(w, {"x", "y", "z"}));
  QVERIFY(!groupMeetsRequirement(w, {"x", "y"}));
  QVERIFY(!groupMeetsRequirement(w, {"x", "y", "z", "x"}));
  QVERIFY(!groupMeetsRequirement(w, {"x", "y", ""}));
}

/**
 * @brief Binding is by tag, so order and letter case do not matter.
 */
void GroupWidgetRequirementTest::accelerometerIgnoresOrderAndCase()
{
  QVERIFY(groupMeetsRequirement(SerialStudio::Accelerometer, {"Z", "x", " Y "}));
}

/**
 * @brief The gyroscope takes x/y/z or pitch/roll/yaw for each axis.
 */
void GroupWidgetRequirementTest::gyroscopeAcceptsAxisOrAttitudeTags()
{
  const auto w = SerialStudio::Gyroscope;
  QVERIFY(groupMeetsRequirement(w, {"x", "y", "z"}));
  QVERIFY(groupMeetsRequirement(w, {"pitch", "roll", "yaw"}));
  QVERIFY(!groupMeetsRequirement(w, {"pitch", "roll", "roll"}));
}

/**
 * @brief The 3D plot needs x, y and z; an extra dataset is ignored, not an error.
 */
void GroupWidgetRequirementTest::plot3dNeedsXyzAndToleratesExtras()
{
  const auto w = SerialStudio::Plot3D;
  QVERIFY(groupMeetsRequirement(w, {"x", "y", "z"}));
  QVERIFY(groupMeetsRequirement(w, {"x", "y", "z", ""}));
  QVERIFY(!groupMeetsRequirement(w, {"x", "y"}));
}

/**
 * @brief The map needs latitude and longitude; altitude is optional.
 */
void GroupWidgetRequirementTest::gpsNeedsLatAndLonAltOptional()
{
  const auto w = SerialStudio::GPS;
  QVERIFY(groupMeetsRequirement(w, {"lat", "lon"}));
  QVERIFY(groupMeetsRequirement(w, {"lat", "lon", "alt"}));
  QVERIFY(!groupMeetsRequirement(w, {"lat", "alt"}));
  QVERIFY(!groupMeetsRequirement(w, {"lat"}));
}

/**
 * @brief Rows: the list-style group widgets.
 */
void GroupWidgetRequirementTest::listWidgetsNeedOneDataset_data()
{
  QTest::addColumn<int>("widget");
  QTest::newRow("datagrid") << static_cast<int>(SerialStudio::DataGrid);
  QTest::newRow("multiplot") << static_cast<int>(SerialStudio::MultiPlot);
  QTest::newRow("barpanel") << static_cast<int>(SerialStudio::BarPanel);
}

/**
 * @brief Data grid, multi-plot and bar panel need at least one dataset, of any tag.
 */
void GroupWidgetRequirementTest::listWidgetsNeedOneDataset()
{
  QFETCH(int, widget);
  const auto w = static_cast<SerialStudio::GroupWidget>(widget);
  QVERIFY(!groupMeetsRequirement(w, {}));
  QVERIFY(groupMeetsRequirement(w, {""}));
  QVERIFY(groupMeetsRequirement(w, {"", "", "", ""}));
}

/**
 * @brief Image view, canvas, web view and "no widget" impose nothing.
 */
void GroupWidgetRequirementTest::freeFormWidgetsAcceptAnything()
{
  for (const auto w : {SerialStudio::ImageView,
                       SerialStudio::Painter,
                       SerialStudio::WebView,
                       SerialStudio::NoGroupWidget}) {
    QVERIFY(groupMeetsRequirement(w, {}));
    QVERIFY(groupMeetsRequirement(w, {"x"}));
  }
}

QTEST_APPLESS_MAIN(GroupWidgetRequirementTest)

#include "tst_group_widget_requirement.moc"
