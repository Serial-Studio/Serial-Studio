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
 * @brief Output widget that sends a command when clicked, latching on/off when checkable.
 */
class Button : public Base {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool checkable
             READ isCheckable
             CONSTANT)
  Q_PROPERTY(bool checked
             READ isChecked
             WRITE setChecked
             NOTIFY checkedChanged)
  // clang-format on

signals:
  void checkedChanged();

public:
  Button(const DataModel::OutputWidget& config,
         TransmitTarget target,
         QQuickItem* parent = nullptr);

  [[nodiscard]] bool isChecked() const noexcept;
  [[nodiscard]] bool isCheckable() const noexcept;

public slots:
  void click();
  void setChecked(bool checked);

protected:
  void applyStateVerdict(const StateBinding::Verdict& verdict) override;

private:
  bool m_checked;
  bool m_checkable;
};

}  // namespace Output
}  // namespace Widgets
