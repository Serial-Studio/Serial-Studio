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

#pragma once

#include <array>
#include <QLocale>
#include <QString>
#include <QStringList>

#include "Misc/Translator.h"

namespace Misc {

/**
 * @brief Everything the application knows about one shipped UI language: the .qm/theme key, the
 *        welcome-text code, the locale it maps to, its native display name, and its script
 *        direction.
 */
struct LanguageEntry {
  bool rightToLeft;
  Translator::Language language;
  QLocale::Language locale;
  QLocale::Territory territory;
  const char* qmName;
  const char* welcomeCode;
  const char* nativeName;
};

/**
 * @brief The single source of truth for the languages Serial Studio ships. Adding a language is
 *        one row here plus its .qm and welcome text; the enum order and the table order are the
 *        same, so index and Translator::Language are interchangeable.
 */
namespace LanguageTable {
inline constexpr int kLanguageCount = 21;

[[nodiscard]] const std::array<LanguageEntry, kLanguageCount>& entries();
[[nodiscard]] const LanguageEntry& entryFor(Translator::Language language);
[[nodiscard]] Translator::Language languageForLocale(QLocale::Language locale);

[[nodiscard]] QLocale localeFor(Translator::Language language);
[[nodiscard]] QString qmName(Translator::Language language);
[[nodiscard]] QString welcomeCode(Translator::Language language);
[[nodiscard]] QStringList nativeNames();
}  // namespace LanguageTable

}  // namespace Misc
