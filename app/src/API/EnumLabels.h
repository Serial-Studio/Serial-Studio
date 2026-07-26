/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>

#include "SerialStudio.h"

namespace API {
namespace EnumLabels {

/**
 * @brief Returns a short machine-friendly slug for a BusType value (e.g. "uart").
 */
[[nodiscard]] QString busTypeSlug(int value);

/**
 * @brief Returns a human-friendly label for a BusType value (e.g. "UART (serial port)").
 */
[[nodiscard]] QString busTypeLabel(int value);

/**
 * @brief Returns a short slug for a FrameDetection value (e.g. "start-end-delimiter").
 */
[[nodiscard]] QString frameDetectionSlug(int value);

/**
 * @brief Returns a human-friendly label for a FrameDetection value.
 */
[[nodiscard]] QString frameDetectionLabel(int value);

/**
 * @brief Returns a short slug for a DecoderMethod value (e.g. "plain-text").
 */
[[nodiscard]] QString decoderMethodSlug(int value);

/**
 * @brief Returns a human-friendly label for a DecoderMethod value.
 */
[[nodiscard]] QString decoderMethodLabel(int value);

/**
 * @brief Returns a short slug for an OperationMode value (e.g. "project-file").
 */
[[nodiscard]] QString operationModeSlug(int value);

/**
 * @brief Returns a human-friendly label for an OperationMode value.
 */
[[nodiscard]] QString operationModeLabel(int value);

/**
 * @brief Returns a short slug for a ScriptLanguage value (e.g. "lua"). -1 means "inherit".
 */
[[nodiscard]] QString scriptLanguageSlug(int value);

/**
 * @brief Returns a human-friendly label for a ScriptLanguage value (handles -1 = inherit).
 */
[[nodiscard]] QString scriptLanguageLabel(int value);

/**
 * @brief Returns a short slug for a SerialStudio::GroupWidget value.
 */
[[nodiscard]] QString groupWidgetSlug(int value);

/**
 * @brief Returns a human-friendly label for a SerialStudio::GroupWidget value.
 */
[[nodiscard]] QString groupWidgetLabel(int value);

/**
 * @brief Returns a short slug for a SerialStudio::DatasetWidget value.
 */
[[nodiscard]] QString datasetWidgetSlug(int value);

/**
 * @brief Returns a human-friendly label for a SerialStudio::DatasetWidget value.
 */
[[nodiscard]] QString datasetWidgetLabel(int value);

/**
 * @brief Returns a comma-separated list of dataset option flags set in @c value.
 */
[[nodiscard]] QString datasetOptionsLabel(int value);

/**
 * @brief Returns a stable string slug for a DashboardWidget enum value (e.g. "plot").
 */
[[nodiscard]] QString dashboardWidgetSlug(int value);

/**
 * @brief Resolves a DashboardWidget slug back to its enum integer; returns -1 on miss.
 */
[[nodiscard]] int dashboardWidgetFromSlug(const QString& slug);

/**
 * @brief Returns a stable string slug for a single DatasetOption bit (e.g. "fft").
 */
[[nodiscard]] QString datasetOptionSlug(int singleBitValue);

/**
 * @brief Resolves a DatasetOption slug back to its bit value; returns 0 on miss.
 */
[[nodiscard]] int datasetOptionFromSlug(const QString& slug);

/**
 * @brief Splits a DatasetOption bitflag value into the array of individual slug strings.
 */
[[nodiscard]] QStringList datasetOptionsBitsToSlugs(int value);

/**
 * @brief Combines an array of DatasetOption slugs into a single bitflag value.
 */
[[nodiscard]] int datasetOptionsSlugsToBits(const QStringList& slugs);

}  // namespace EnumLabels
}  // namespace API
