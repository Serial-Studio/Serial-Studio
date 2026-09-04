/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

namespace Misc {

/**
 * @brief Resolves fixed UI icons by (category, name, pixel size) from the
 *        resource tree at :/icons/<category>/<tier>/<name>.svg (spec 0028).
 */
class IconRegistry : public QObject {
  Q_OBJECT

private:
  explicit IconRegistry();
  IconRegistry(IconRegistry&&)                 = delete;
  IconRegistry(const IconRegistry&)            = delete;
  IconRegistry& operator=(IconRegistry&&)      = delete;
  IconRegistry& operator=(const IconRegistry&) = delete;

public:
  [[nodiscard]] static IconRegistry& instance();

  [[nodiscard]] Q_INVOKABLE QString icon(const QString& category,
                                         const QString& name,
                                         int px) const;
  [[nodiscard]] Q_INVOKABLE QString iconById(const QString& id, int px) const;
  [[nodiscard]] QString iconPath(const QString& category, const QString& name, int px) const;

private:
  [[nodiscard]] QString missingIcon(const QString& category, const QString& name) const;
  [[nodiscard]] QString resolve(const QString& category, const QString& name, int px) const;

private:
  mutable QSet<QString> m_warned;
  QHash<QString, QHash<QString, QList<int>>> m_catalog;
};

}  // namespace Misc
