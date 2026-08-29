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
#include <QString>
#include <QStringList>

namespace DataModel {

/**
 * @brief The frame-parser template catalog: the shipped JS/Lua template manifest, the compiled-in
 *        native template registry, and the mapping between them that makes a language switch land
 *        on the equivalent template instead of stale wrong-language code. Owns no engine and
 *        resolves no singleton beyond the translator, so the mapping is exercisable standalone.
 */
class ParserTemplateCatalog {
public:
  ParserTemplateCatalog();

  [[nodiscard]] static QString defaultCode(int language);
  [[nodiscard]] static int detectNativeTemplate(const QString& descriptor);
  [[nodiscard]] static QString scriptResource(const QString& file, int language);
  [[nodiscard]] static QString fileForNativeTemplate(const QString& templateId,
                                                     const QJsonObject& params);
  [[nodiscard]] static bool nativeEquivalentForFile(const QString& file,
                                                    QString& templateId,
                                                    QJsonObject& params);

  [[nodiscard]] int fileCount() const noexcept;
  [[nodiscard]] int defaultFileIndex() const;
  [[nodiscard]] int detect(const QString& code) const;
  [[nodiscard]] const QStringList& files() const noexcept;
  [[nodiscard]] const QStringList& names() const noexcept;
  [[nodiscard]] const QStringList& nativeNames() const noexcept;
  [[nodiscard]] QString codeForIndex(int index, int language) const;

  void reload();

private:
  QString m_defaultFile;
  QStringList m_files;
  QStringList m_names;
  QStringList m_nativeNames;
};

}  // namespace DataModel
