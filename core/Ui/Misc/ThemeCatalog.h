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

#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QStringList>

#include "Misc/Translator.h"

namespace Misc {

/**
 * @brief Owns which themes exist and what they are called: the bundled themes, the ones installed
 *        as extensions, the "System" entry, and the localized names shown in the UI. Holds no live
 *        theme state and reaches no singleton -- the workspace directory and the UI language are
 *        passed in by ThemeManager.
 */
class ThemeCatalog {
public:
  void loadBuiltInThemes();
  void reloadUserThemes(const QString& themesDir);
  void updateLocalizedNames(Translator::Language language);

  [[nodiscard]] int count() const;
  [[nodiscard]] int indexOf(const QString& title) const;
  [[nodiscard]] QString titleAt(int index) const;
  [[nodiscard]] QJsonObject theme(const QString& title) const;
  [[nodiscard]] bool contains(const QString& title) const;

  [[nodiscard]] const QStringList& titles() const noexcept;
  [[nodiscard]] const QStringList& userThemes() const noexcept;
  [[nodiscard]] const QStringList& localizedNames() const noexcept;

  [[nodiscard]] static QString systemTitle();

private:
  void scanUserThemes(const QString& themesDir);
  void tryLoadUserThemeFile(const QString& subdirPath, const QString& jsonFile);

private:
  QStringList m_titles;
  QStringList m_userThemes;
  QStringList m_localizedNames;
  QMap<QString, QJsonObject> m_themes;
};

}  // namespace Misc
