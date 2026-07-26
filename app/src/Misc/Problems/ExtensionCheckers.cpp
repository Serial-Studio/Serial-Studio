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

#include "Misc/Problems/ExtensionCheckers.h"

#include <QCoreApplication>
#include <QString>

#include "DataModel/ProjectModel.h"
#include "Misc/ProblemCenter.h"
#include "UI/WidgetExtensions.h"

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Finding  = Misc::ProblemCenter::Finding;
using Severity = Misc::ProblemCenter::Severity;

static const QString kJumpGroup   = QStringLiteral("group");
static const QString kJumpDataset = QStringLiteral("dataset");

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated text for the shared "Problems" translation context.
 */
[[nodiscard]] static QString trProblem(const char* text)
{
  return QCoreApplication::translate("Problems", text);
}

/**
 * @brief Assembles one finding for a project entity that names a widget package.
 */
[[nodiscard]] static Finding makeEntityFinding(Severity severity,
                                               const QString& code,
                                               const QString& title,
                                               const QString& explanation,
                                               const QString& remedy,
                                               int entityUniqueId,
                                               const QString& jump)
{
  Finding finding;
  finding.severity       = severity;
  finding.entityUniqueId = entityUniqueId;
  finding.code           = code;
  finding.jump           = jump;
  finding.title          = title;
  finding.remedy         = remedy;
  finding.explanation    = explanation;
  return finding;
}

/**
 * @brief Reports one entity whose widget string names a package that is not installed, or one the
 *        user has not consented to run. Built-in widget strings and empty selections are skipped by
 *        the caller, so anything reaching here is an extension reference.
 */
static void checkEntityReference(const QString& widget,
                                 const QString& entityTitle,
                                 int uniqueId,
                                 const QString& jump,
                                 QList<Finding>& out)
{
  static auto& catalog = UI::WidgetExtensions::instance();

  if (!catalog.contains(widget)) {
    out.append(makeEntityFinding(
      Misc::ProblemCenter::Error,
      QStringLiteral("widget-not-installed"),
      trProblem("Project uses a widget extension that is not installed"),
      trProblem("\"%1\" is set to the widget extension \"%2\", which is not installed, so the "
                "dashboard shows a placeholder instead.")
        .arg(entityTitle, widget),
      trProblem("Install the extension from the extension manager, or choose another widget."),
      uniqueId,
      jump));
    return;
  }

  if (!catalog.consentGranted(widget))
    out.append(makeEntityFinding(
      Misc::ProblemCenter::Warning,
      QStringLiteral("widget-consent-required"),
      trProblem("Widget extension is waiting for your permission"),
      trProblem("\"%1\" uses the widget extension \"%2\". Extensions run with the same privileges "
                "as Serial Studio itself, so it stays inactive until you allow it.")
        .arg(entityTitle, widget),
      trProblem("Open the widget and allow the extension to run, or choose another widget."),
      uniqueId,
      jump));
}

//--------------------------------------------------------------------------------------------------
// Checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Republishes the catalog's load-time rejections: malformed manifests, incompatible host
 *        ranges, reserved identifiers, missing QML files, and unresolved dependencies.
 */
static void checkCatalogFindings(QList<Finding>& out)
{
  static auto& catalog = UI::WidgetExtensions::instance();
  out.append(catalog.findings());
}

/**
 * @brief Walks the project for group and dataset widget selections that name a widget package,
 *        reporting the ones that cannot render. Built-in widget strings are reserved identifiers,
 *        so they never reach the catalog lookup.
 */
static void checkProjectReferences(QList<Finding>& out)
{
  static auto& project = DataModel::ProjectModel::instance();

  const auto& groups = project.groups();
  for (const auto& group : groups) {
    if (!group.widget.isEmpty() && !UI::WidgetExtensions::isReservedId(group.widget))
      checkEntityReference(group.widget, group.title, group.uniqueId, kJumpGroup, out);

    for (const auto& dataset : group.datasets)
      if (!dataset.widget.isEmpty() && !UI::WidgetExtensions::isReservedId(dataset.widget))
        checkEntityReference(dataset.widget, dataset.title, dataset.uniqueId, kJumpDataset, out);
  }
}

/**
 * @brief Runs both widget-extension checks in one pass, so the panel groups them under a single
 *        checker id.
 */
static void checkWidgetExtensions(QList<Finding>& out)
{
  checkCatalogFindings(out);
  checkProjectReferences(out);
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the widget-extension checker, which re-runs whenever the project changes and on
 *        an explicit request; the catalog itself only changes on install, uninstall, or rescan.
 */
void Misc::ExtensionCheckers::registerAll()
{
  static auto& center   = Misc::ProblemCenter::instance();
  const quint8 triggers = Misc::ProblemCenter::ProjectChanged | Misc::ProblemCenter::OnDemand;

  center.registerChecker(QStringLiteral("extension.widget"), triggers, checkWidgetExtensions);
}
