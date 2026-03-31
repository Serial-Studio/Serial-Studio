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
 * @class Widgets::Output::Button
 * @brief Output widget that sends a command when clicked.
 *
 * Calls transmit(1) on each click. The JS function decides what to send.
 */
class Button : public Base {
  Q_OBJECT

public:
  explicit Button(const DataModel::OutputWidget& config, QQuickItem* parent = nullptr);

  Q_INVOKABLE void click();
};

}  // namespace Output
}  // namespace Widgets
