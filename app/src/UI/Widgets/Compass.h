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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>

namespace Widgets {
/**
 * @brief Data model for a compass/heading display (0-360 deg, fixed rose).
 */
class Compass : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool alarmsDefined
             READ alarmsDefined
             CONSTANT)
  Q_PROPERTY(bool alarmTriggered
             READ alarmTriggered
             NOTIFY updated)
  Q_PROPERTY(double value
             READ value
             NOTIFY updated)
  Q_PROPERTY(QString cardinal
             READ cardinal
             NOTIFY updated)
  Q_PROPERTY(QString title
             READ title
             CONSTANT)
  Q_PROPERTY(QString units
             READ units
             CONSTANT)
  Q_PROPERTY(QString displayFormat
             READ displayFormat
             CONSTANT)
  // clang-format on

signals:
  void updated();

public:
  explicit Compass(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] bool alarmsDefined() const noexcept;
  [[nodiscard]] bool alarmTriggered() const noexcept;
  [[nodiscard]] double value() const noexcept;
  [[nodiscard]] const QString& cardinal() const noexcept;
  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] const QString& units() const noexcept;
  [[nodiscard]] const QString& displayFormat() const noexcept;

private slots:
  void updateData();

private:
  [[nodiscard]] QString cardinalDirection(double angle) const;

  int m_index;
  double m_value;
  QString m_title;
  QString m_units;
  QString m_displayFormat;
  QString m_cardinal;
};
}  // namespace Widgets
