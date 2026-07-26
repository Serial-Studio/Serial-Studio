/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <algorithm>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <vector>

#include "DataModel/Frame.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "SerialStudio.h"

namespace DataModel {

/**
 * @brief Returns true when a folder with @p id exists in any folder vector.
 */
template<typename Folder>
bool folderExists(const std::vector<Folder>& folders, int id)
{
  return std::any_of(
    folders.begin(), folders.end(), [id](const auto& f) { return f.folderId == id; });
}

/**
 * @brief Returns true when @p candidate is @p folderId or sits inside its subtree.
 */
template<typename Folder>
bool folderIsSelfOrDescendant(const std::vector<Folder>& folders, int folderId, int candidate)
{
  const int kMax = static_cast<int>(folders.size());
  int p          = candidate;
  for (int i = 0; i <= kMax && p != -1; ++i) {
    if (p == folderId)
      return true;

    int parent = -1;
    for (const auto& f : folders)
      if (f.folderId == p) {
        parent = f.parentFolderId;
        break;
      }

    p = parent;
  }

  return false;
}

/**
 * @brief Returns a unique title for a duplicated item using a numbered " (N)" suffix.
 */
inline QString nextDuplicateTitle(const QString& title, const QStringList& taken)
{
  static const QRegularExpression kSuffixRe(QStringLiteral("^(.*?)\\s*\\((\\d+)\\)\\s*$"));

  QString base        = title;
  const auto stripped = kSuffixRe.match(title);
  if (stripped.hasMatch())
    base = stripped.captured(1).trimmed();

  const QString basePattern = QStringLiteral("^") + QRegularExpression::escape(base)
                            + QStringLiteral("(?:\\s*\\((\\d+)\\))?\\s*$");
  const QRegularExpression baseRe(basePattern);

  int maxN = -1;
  for (const auto& t : taken) {
    const auto m = baseRe.match(t);
    if (!m.hasMatch())
      continue;

    const QString suffix = m.captured(1);
    if (suffix.isEmpty()) {
      maxN = qMax(maxN, 0);
      continue;
    }

    bool ok     = false;
    const int n = suffix.toInt(&ok);
    if (ok)
      maxN = qMax(maxN, n);
  }

  if (maxN < 0)
    return QStringLiteral("%1 (1)").arg(base);

  return QStringLiteral("%1 (%2)").arg(base, QString::number(maxN + 1));
}

/**
 * @brief Seeds a source with the default parser: the Native CSV (delimited/comma) template.
 * Switching to a scripting language converts the template via the equivalence mapping.
 */
inline void seedDefaultFrameParser(DataModel::Source& source)
{
  source.frameParserLanguage = static_cast<int>(SerialStudio::Native);
  source.frameParserTemplate = DataModel::defaultNativeTemplateId();

  const auto* tmpl = DataModel::nativeTemplateById(source.frameParserTemplate);
  if (tmpl)
    source.frameParserParams = DataModel::nativeTemplateDefaults(*tmpl);
}

}  // namespace DataModel
