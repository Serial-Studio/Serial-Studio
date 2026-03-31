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
 * @class Widgets::Output::Toggle
 * @brief Output widget with an on/off switch.
 *
 * Calls transmit(1) when toggled on and transmit(0) when toggled off.
 */
class Toggle : public Base {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool checked
             READ  isChecked
             WRITE setChecked
             NOTIFY checkedChanged)
  // clang-format on

Q_SIGNALS:
  void checkedChanged();

public:
  explicit Toggle(const DataModel::OutputWidget& config, QQuickItem* parent = nullptr);

  [[nodiscard]] bool isChecked() const noexcept;

public Q_SLOTS:
  void setChecked(bool checked);

private:
  bool m_checked;
};

}  // namespace Output
}  // namespace Widgets
