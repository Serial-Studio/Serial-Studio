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

namespace AI::TokenBudget {

/// Bytes of serialized JSON charged to one estimated token.
inline constexpr int kBytesPerToken = 4;

/**
 * @brief The provider-window inputs the history budgeter reads, passed in rather than
 *        resolved, so the trimming rule is exercisable without a live Provider.
 */
struct Window {
  int contextWindowTokens;
  int maxOutputTokens;
  int systemReserveTokens;
};

[[nodiscard]] int estimateTokens(const QJsonArray& blocks);

[[nodiscard]] int historyBudget(const Window& window, const QJsonArray& tools);

[[nodiscard]] QJsonArray budgetedHistory(const QJsonArray& history, int budget);

}  // namespace AI::TokenBudget
