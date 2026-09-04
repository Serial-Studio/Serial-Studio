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

#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace DataModel {

/**
 * @brief Returns a unique title for a duplicated item using a numbered " (N)" suffix. Every
 *        duplicable project entity (group, dataset, action, output widget, source, table, folder)
 *        names its copy through this one rule, which is why it is a free function rather than a
 *        member of any one owner.
 */
[[nodiscard]] inline QString nextDuplicateTitle(const QString& title, const QStringList& taken)
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

}  // namespace DataModel
