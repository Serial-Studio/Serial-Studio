/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "UI/Widgets/Output/Base.h"

namespace Widgets {
namespace Output {

/**
 * @class Widgets::Output::Slider
 * @brief Output widget with a slider that sends scaled numeric values.
 *
 * Calls transmit(value) where value is clamped to [minValue, maxValue].
 */
class Slider : public Base {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(double currentValue
             READ  currentValue
             WRITE setCurrentValue
             NOTIFY currentValueChanged)
  // clang-format on

Q_SIGNALS:
  void currentValueChanged();

public:
  explicit Slider(const DataModel::OutputWidget& config, QQuickItem* parent = nullptr);

  [[nodiscard]] double currentValue() const noexcept;

public Q_SLOTS:
  void setCurrentValue(double value);

private:
  double m_currentValue;
};

}  // namespace Output
}  // namespace Widgets
