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
 * @brief Output widget that sends a command when clicked.
 */
class Button : public Base {
  Q_OBJECT

public:
  explicit Button(const DataModel::OutputWidget& config, QQuickItem* parent = nullptr);

public slots:
  void click();
};

}  // namespace Output
}  // namespace Widgets
