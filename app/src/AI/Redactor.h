/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace AI {

/**
 * @brief Redacts secrets from strings before they enter the model context.
 */
class Redactor {
public:
  [[nodiscard]] static bool scrub(QString& text);
  [[nodiscard]] static QJsonObject scrubObject(const QJsonObject& obj, int depth = 0);
  [[nodiscard]] static QJsonArray scrubArray(const QJsonArray& arr, int depth = 0);
};

}  // namespace AI
