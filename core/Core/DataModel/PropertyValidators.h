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

#include <QString>

/**
 * @file PropertyValidators.h
 * @brief The dataset-property validators the generated readers and the frame value types call
 *        (spec 0036). They reach no project state, which is what lets the value types live in
 *        Core: the one check that needs Qt Gui, the colour parse, is a hook the composition root
 *        installs (QColor::fromString), and an unbound hook accepts every colour so a bare Core
 *        link never rejects a document.
 */

namespace DataModel::PropertyHooks {

/**
 * @brief The colour parse the root installs (QColor::fromString behind it).
 */
using ColorValidator = bool (*)(const QString& color);

/**
 * @brief The action-icon URL resolver the root installs (the icon engine behind it).
 */
using ActionIconResolver = QString (*)(const QString& icon);

void setColorValidator(ColorValidator validator) noexcept;
void setActionIconResolver(ActionIconResolver resolver) noexcept;

[[nodiscard]] QString resolveActionIcon(const QString& icon);

[[nodiscard]] bool isValidColor(const QString& color);
[[nodiscard]] bool isValidDatasetIndex(int index);
[[nodiscard]] bool isValidFftWindow(int window);
[[nodiscard]] bool isValidTransformLanguage(int language);
[[nodiscard]] bool isValidAlarmDeadband(double deadband);
}  // namespace DataModel::PropertyHooks
