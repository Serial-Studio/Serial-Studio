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

#include "Console/WelcomeText.h"

#include <QFile>
#include <QObject>

#include "Core/LanguageTable.h"
#include "Core/License.h"

//--------------------------------------------------------------------------------------------------
// Welcome text
//--------------------------------------------------------------------------------------------------

/**
 * @brief The ordinal the root publishes for a trial entitlement; anything above it is a paid tier.
 */
static constexpr quint8 kTrialTier = 2;

/**
 * @brief Resolves the resource path of the welcome text variant this build and licence get:
 *        paid tiers read pro, an active trial reads trial, and every unentitled state
 *        (first run, declined, expired) reads unlicensed (spec 0092).
 */
[[nodiscard]] static QString welcomeTextPath(const QString& lang)
{
#ifdef BUILD_COMMERCIAL
  if (Core::License::activated()) {
    if (Core::License::tier() > kTrialTier)
      return ":/messages/pro/Welcome_" + lang + ".txt";

    return ":/messages/trial/Welcome_" + lang + ".txt";
  }

  return ":/messages/unlicensed/Welcome_" + lang + ".txt";
#else
  return ":/messages/gpl3/Welcome_" + lang + ".txt";
#endif
}

/**
 * @brief The one-line passive licensing status (spec 0092 R4): trial days while a trial runs,
 *        the expired note afterwards, nothing for paid, first-run or GPL builds. Riding the
 *        welcome text keeps it regenerating on every LicenseStateChanged reprint.
 */
[[nodiscard]] static QString licenseStatusLine()
{
#ifdef BUILD_COMMERCIAL
  const int trial_days = Core::License::trialDaysRemaining();
  if (Core::License::activated() && Core::License::tier() <= kTrialTier && trial_days >= 0)
    return QObject::tr("Pro trial: %n day(s) remaining.", "", trial_days) + "\n";

  if (Core::License::trialExpired() && !Core::License::activated())
    return QObject::tr("Pro trial expired. Free features remain fully functional.") + "\n";
#endif

  return QString();
}

/**
 * @brief Returns the localized welcome guide for @p language, with a trailing newline.
 */
QString Console::welcomeConsoleText(const Misc::Translator::Language language)
{
  const auto lang = Misc::LanguageTable::welcomeCode(language);

  QString text = QObject::tr("Failed to load welcome text :(");
  QFile file(welcomeTextPath(lang));
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    file.close();
  }

  return text + "\n" + licenseStatusLine();
}
