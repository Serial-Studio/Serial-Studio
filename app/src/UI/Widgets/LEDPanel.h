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
 * @class Widgets::LEDPanel
 * @brief Multi-LED status indicator panel widget.
 *
 * The LEDPanel class provides a visual panel of multiple LED indicators, each
 * with its own state (on/off), color, and title. This is ideal for displaying
 * multiple boolean status indicators or binary flags in a compact,
 * easy-to-read format.
 *
 * Key Features:
 * - **Multiple Indicators**: Display many LED states simultaneously
 * - **Individual Titles**: Each LED has an associated descriptive label
 * - **Theme-Aware Colors**: LED colors automatically adapt to the current
 *   theme
 * - **State Tracking**: Each LED maintains an independent on/off state
 * - **Compact Display**: Efficiently presents many binary indicators
 *
 * Typical Use Cases:
 * - System status indicators (errors, warnings, ready states)
 * - Digital I/O pin visualization
 * - Multi-channel sensor threshold indicators
 * - Relay/actuator state monitoring
 * - Binary flag/bit field visualization
 *
 * @note The number of LEDs is determined by the associated dataset group and
 *       is fixed after initialization.
 */
class LEDPanel : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(int count READ count CONSTANT)
  Q_PROPERTY(QStringList titles READ titles CONSTANT)
  Q_PROPERTY(QVector<bool> states READ states NOTIFY updated)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)

signals:
  void updated();
  void themeChanged();

public:
  explicit LEDPanel(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] int count() const;
  [[nodiscard]] const QList<bool>& states() const;
  [[nodiscard]] const QStringList& colors() const;
  [[nodiscard]] const QStringList& titles() const;

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
