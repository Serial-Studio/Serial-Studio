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

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "Core/SimdLevel.h"

namespace Misc {
/**
 * @brief The user-facing side of the kernel lane selector (spec 0081): persists the chosen level
 *        under App/SimdLevel as a stable id, feeds the Startup-tab combobox, and resolves the
 *        startup level from the preference and an optional --simd pin. Root-owned, never a
 *        singleton: the benchmark and selftest roots only need the static apply.
 */
class SimdSettings : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString currentLevel
             READ currentLevel
             WRITE setCurrentLevel
             NOTIFY currentLevelChanged)
  Q_PROPERTY(QVariantList availableLevels
             READ availableLevels
             CONSTANT)
  // clang-format on

signals:
  void currentLevelChanged();

public:
  explicit SimdSettings(QObject* parent = nullptr);
  SimdSettings(SimdSettings&&)                 = delete;
  SimdSettings(const SimdSettings&)            = delete;
  SimdSettings& operator=(SimdSettings&&)      = delete;
  SimdSettings& operator=(const SimdSettings&) = delete;

  static void applyConfiguredLevel(const QString& pinnedId);
  [[nodiscard]] static QString levelLabel(DSP::SimdLevel level);

  [[nodiscard]] const QString& currentLevel() const noexcept;
  [[nodiscard]] const QVariantList& availableLevels() const noexcept;

public slots:
  void setCurrentLevel(const QString& id);

private:
  [[nodiscard]] static QString normalizedId(const QString& id);
  [[nodiscard]] static bool applyId(const QString& id);
  [[nodiscard]] static const char* settingsKey() noexcept;
  [[nodiscard]] static QVariantMap levelEntry(const QString& id, const QString& label);
  [[nodiscard]] static QString idString(DSP::SimdLevel level);
  void rebuildAvailableLevels();

private:
  QString m_currentLevel;
  QSettings m_settings;
  QVariantList m_availableLevels;
};
}  // namespace Misc
