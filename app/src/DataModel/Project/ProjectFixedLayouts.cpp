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

#include "DataModel/Project/ProjectFixedLayouts.h"

#include "DataModel/ProjectModel.h"

namespace DataModel::FixedLayouts {

/**
 * @brief Layout config for fixed three-axis group widgets (Accel/Gyro/GPS/Plot3D).
 */
struct ThreeAxisLayout {
  const char* widgetTag;
  const char* axisWidgets[3];
  QString units[3];
  QString titles[3];
  double wgtMin[3];
  double wgtMax[3];
  bool plt;
};

/**
 * @brief Populates a group with three canonical axis datasets per supplied layout.
 */
static void populateThreeAxisDatasets(Group& grp, int baseIndex, const ThreeAxisLayout& layout)
{
  grp.widget = QString::fromUtf8(layout.widgetTag);

  Dataset axes[3];
  for (int i = 0; i < 3; ++i) {
    axes[i].datasetId = i;
    axes[i].groupId   = grp.groupId;
    axes[i].sourceId  = grp.sourceId;
    axes[i].index     = baseIndex + i;
    axes[i].units     = layout.units[i];
    axes[i].widget    = QString::fromUtf8(layout.axisWidgets[i]);
    axes[i].title     = layout.titles[i];
    axes[i].wgtMin    = layout.wgtMin[i];
    axes[i].wgtMax    = layout.wgtMax[i];
    axes[i].plt       = layout.plt;

    grp.datasets.push_back(axes[i]);
  }
}

/**
 * @brief Fills a group with the three-axis canonical datasets for sensor-style widgets, numbering
 *        them from @p baseIndex. The tr() context stays ProjectModel so the shipped translations
 *        of these axis titles keep resolving.
 */
bool populateFixedLayoutGroup(Group& grp, SerialStudio::GroupWidget widget, int baseIndex)
{
  if (widget == SerialStudio::Accelerometer) {
    // code-verify off
    ThreeAxisLayout layout{
      "accelerometer",
      {                                          "x","y",                                  "z"                                                              },
      {                                       "m/s²", "m/s²",                                       "m/s²"},
      {ProjectModel::tr("Accelerometer %1").arg("X"),
        ProjectModel::tr("Accelerometer %1").arg("Y"),
        ProjectModel::tr("Accelerometer %1").arg("Z")                                                      },
      {                                            0,      0,                                            0},
      {                                            0,      0,                                            0},
      true
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Gyroscope) {
    // code-verify off
    ThreeAxisLayout layout{
      "gyro",
      {                                 "x","y",                       "z"                                                      },
      {                             "deg/s", "deg/s",                            "deg/s"},
      {ProjectModel::tr("Gyro %1").arg("X"),
        ProjectModel::tr("Gyro %1").arg("Y"),
        ProjectModel::tr("Gyro %1").arg("Z")                                             },
      {                                   0,       0,                                  0},
      {                                   0,       0,                                  0},
      true
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::GPS) {
    // code-verify off
    ThreeAxisLayout layout{
      "gps",
      {                       "lat",                         "lon",                        "alt"},
      {                         "°",                           "°",                          "m"},
      {ProjectModel::tr("Latitude"), ProjectModel::tr("Longitude"), ProjectModel::tr("Altitude")},
      {                       -90.0,                        -180.0,                       -500.0},
      {                        90.0,                         180.0,                    1000000.0},
      false
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Plot3D) {
    // code-verify off
    ThreeAxisLayout layout{
      "plot3d",
      {                  "x",                   "y",                   "z"},
      {                   "",                    "",                    ""},
      {ProjectModel::tr("X"), ProjectModel::tr("Y"), ProjectModel::tr("Z")},
      {                    0,                     0,                     0},
      {                    0,                     0,                     0},
      false
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Painter) {
    // code-verify off
    ThreeAxisLayout layout{
      "painter",
      {                   "",                    "",                    ""},
      {                   "",                    "",                    ""},
      {ProjectModel::tr("X"), ProjectModel::tr("Y"), ProjectModel::tr("Z")},
      {                 -100,                  -100,                  -100},
      {                  100,                   100,                   100},
      false
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  return true;
}

}  // namespace DataModel::FixedLayouts
