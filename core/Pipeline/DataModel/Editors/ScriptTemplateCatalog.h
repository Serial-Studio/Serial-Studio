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

#include <QString>
#include <QStringList>

namespace DataModel {

/**
 * @brief Localized view of one script-template manifest: the display names, the resolved resource
 *        paths and which entry is the default. Rebuilt on a language change.
 */
class ScriptTemplateCatalog {
public:
  ScriptTemplateCatalog(QString manifestPath,
                        QString resourceRoot,
                        QString suffix,
                        const char* translationContext);

  void reload();

  [[nodiscard]] bool isEmpty() const noexcept;
  [[nodiscard]] QString defaultFile() const;
  [[nodiscard]] const QStringList& names() const noexcept;
  [[nodiscard]] const QStringList& files() const noexcept;

private:
  const char* m_context;
  QString m_suffix;
  QString m_defaultFile;
  QString m_manifestPath;
  QString m_resourceRoot;
  QStringList m_names;
  QStringList m_files;
};

[[nodiscard]] QString defaultScriptTemplateCode(const QString& manifestPath,
                                                const QString& resourceRoot,
                                                const QString& suffix,
                                                const char* translationContext);

}  // namespace DataModel
