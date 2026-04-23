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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>
#include <QVector>

namespace Widgets {
/**
 * @brief Multi-LED status indicator panel widget.
 */
class LEDPanel : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             CONSTANT)
  Q_PROPERTY(QStringList titles
             READ titles
             CONSTANT)
  Q_PROPERTY(QStringList colors
             READ colors
             NOTIFY themeChanged)
  Q_PROPERTY(QVector<bool> states
             READ states
             NOTIFY updated)
  // clang-format on

signals:
  void updated();
  void themeChanged();

public:
  explicit LEDPanel(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] const QList<bool>& states() const noexcept;
  [[nodiscard]] const QStringList& colors() const noexcept;
  [[nodiscard]] const QStringList& titles() const noexcept;

private slots:
  void updateData();
  void onThemeChanged();

private:
  int m_index;
  QVector<bool> m_states;
  QStringList m_titles;
  QStringList m_colors;
};

}  // namespace Widgets
