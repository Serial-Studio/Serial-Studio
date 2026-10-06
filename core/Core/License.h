/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <functional>
#include <QString>
#include <QtGlobal>

/**
 * @file License.h
 * @brief The licensing facts every layer may read without reaching the licensing module: four
 *        atomics the composition root writes at licensing-block construction and on every real
 *        activation transition (spec 0077). Readers on the message or block path load these; the
 *        HMAC-backed token check stays in the root, and the LicenseStateChanged topic carries the
 *        change for subscribers that must re-derive.
 */

namespace Core::License {
/**
 * @brief Resumes the refused gesture once the entitlement lands; invoked at most once.
 */
using ProFeatureRetry = std::function<void()>;

/**
 * @brief Root-installed Pro-intent sink: feature id plus the optional resume closure.
 */
using ProFeatureHandler = std::function<void(const QString&, ProFeatureRetry)>;

[[nodiscard]] bool activated() noexcept;
[[nodiscard]] quint8 tier() noexcept;
[[nodiscard]] bool trialExpired() noexcept;
[[nodiscard]] int trialDaysRemaining() noexcept;

void set(bool activated, quint8 tier, bool trialExpired, int trialDaysRemaining) noexcept;
void setProFeatureHandler(ProFeatureHandler handler);
void setRemoteDispatchProbe(std::function<bool()> probe);
[[nodiscard]] bool remoteDispatchActive();
void requestProFeature(const QString& featureId, ProFeatureRetry retry = {});
}  // namespace Core::License
