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

#include "Misc/ProFeatureNotice.h"

#include <QObject>

#include "Core/DataModel/FrameSupport.h"
#include "Core/License.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

#ifndef BUILD_COMMERCIAL
#  include <QDesktopServices>
#  include <QMessageBox>
#  include <QUrl>

#  include "API/CommandRegistry.h"
#  include "Misc/Utilities.h"
#endif

//--------------------------------------------------------------------------------------------------
// Pro content paragraph
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the paragraph a prompt appends when the refused gesture was a connect on a
 *        project with Pro content: what the project contains, how many, and that removing it is
 *        the free way out. Empty for every other feature id and for a project with none, so a
 *        caller can append the result unconditionally.
 */
QString Misc::ProFeatureNotice::contentDetail(const QString& featureId)
{
  if (featureId != QLatin1String(Core::License::kProContentFeature))
    return QString();

  const auto& project = DataModel::pipelineModules().projectModel;
  const auto summary  = SerialStudio::proContentSummary(project.groups(), project.tableCount());
  if (summary.transforms <= 0 && summary.tables <= 0)
    return QString();

  const QString transforms = QObject::tr("%n dataset transform(s)", "", summary.transforms);
  const QString tables     = QObject::tr("%n Variables table(s)", "", summary.tables);

  QString uses;
  if (summary.transforms > 0 && summary.tables > 0)
    uses = QObject::tr("This project uses %1 and %2.").arg(transforms, tables);
  else
    uses = QObject::tr("This project uses %1.").arg(summary.transforms > 0 ? transforms : tables);

  return QStringLiteral("\n\n") + uses + QStringLiteral(" ")
       + QObject::tr("A project with this content needs Serial Studio Pro to connect. To use it "
                     "for free, remove the transforms and the tables from the project.");
}

//--------------------------------------------------------------------------------------------------
// GPL build handler
//--------------------------------------------------------------------------------------------------

#ifndef BUILD_COMMERCIAL
/**
 * @brief Installs the GPL-build Pro-intent handler and the remote-dispatch probe queued gate sites
 *        consult: a GUI root answers each refused gesture with one notice pointing at the official
 *        build, which carries the trial. Headless roots and remote-origin dispatches stay silent,
 *        and a gesture arriving while the notice is open is dropped, never stacked.
 */
void Misc::ProFeatureNotice::install(const bool headless)
{
  if (headless)
    return;

  Core::License::setRemoteDispatchProbe(
    [] { return API::RemoteDispatchScope::active() != nullptr; });
  Core::License::setProFeatureHandler([](const QString& featureId, Core::License::ProFeatureRetry) {
    static bool s_prompting = false;
    if (s_prompting || API::RemoteDispatchScope::active() != nullptr)
      return;

    ButtonTextMap labels;
    labels[QMessageBox::Yes] = QObject::tr("Get Serial Studio Pro");

    s_prompting      = true;
    const int answer = Misc::Utilities::showMessageBox(
      QObject::tr("This feature requires Serial Studio Pro"),
      QObject::tr("This build includes the GPLv3 feature set only. The official build adds the "
                  "Pro features, with a free 14-day trial.")
        + contentDetail(featureId),
      QMessageBox::Information,
      QObject::tr("Serial Studio Pro"),
      QMessageBox::Yes | QMessageBox::Cancel,
      QMessageBox::Cancel,
      labels);
    s_prompting = false;

    if (answer == QMessageBox::Yes)
      (void)QDesktopServices::openUrl(QUrl(QStringLiteral("https://serial-studio.com/")));
  });
}
#endif
