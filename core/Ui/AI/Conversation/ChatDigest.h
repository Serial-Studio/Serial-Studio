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

#include <QString>
#include <QVariantList>

namespace AI::ChatDigest {

[[nodiscard]] QString firstUserText(const QVariantList& uiMessages);

[[nodiscard]] QString buildHandoffDigest(const QVariantList& uiMessages, int maxChars);

void downgradeStaleToolCards(QVariantList& uiMessages);

}  // namespace AI::ChatDigest
