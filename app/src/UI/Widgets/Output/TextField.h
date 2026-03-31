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
 * @class Widgets::Output::TextField
 * @brief Output widget with a text input field and send button.
 *
 * Calls transmit(text) where text is the string typed by the user.
 */
class TextField : public Base {
  Q_OBJECT

public:
  explicit TextField(const DataModel::OutputWidget& config, QQuickItem* parent = nullptr);

  Q_INVOKABLE void sendText(const QString& text);
};

}  // namespace Output
}  // namespace Widgets
