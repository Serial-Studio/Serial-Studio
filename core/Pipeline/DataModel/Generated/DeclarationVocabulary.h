/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

// AUTO-GENERATED from app/rcc/properties/dataset.json and declaration.json;
// never edit by hand. Regenerate with: python3 scripts/generate-property-registry.py

#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <QLatin1StringView>
#include <QString>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameKeys.h"
#include "Core/DataModel/PropertyValidators.h"

// clang-format off

namespace DataModel::Declaration {

/**
 * @brief Line prefixes (spec 0111): a declaration is the prefix, the wire version digits and one
 *        compact JSON object; the app sends the request and the command lines.
 */
inline constexpr QLatin1StringView kPrefix("#ss");
inline constexpr QLatin1StringView kRequest("#ss?");
inline constexpr QLatin1StringView kCommand("#ss!");

/**
 * @brief The Built-In template a binary declaration's frames parse with (R14).
 */
inline constexpr QLatin1StringView kBinaryTemplate("packed_struct");

/**
 * @brief The wire version this build reads and the library version it ships.
 */
inline constexpr int kWireVersion = 1;
inline constexpr QLatin1StringView kLibraryVersion("1.0.0");

/**
 * @brief Caps applied before any reader runs (spec 0111 R10).
 */
inline constexpr qsizetype kMaxLineBytes = 16384;
inline constexpr qsizetype kMaxStringBytes = 64;
inline constexpr int kMaxGroups = 64;
inline constexpr int kMaxDatasets = 512;
inline constexpr int kMaxControls = 64;

/**
 * @brief Dataset keys a declaration may carry: every wire-included property, then the legacy range
 *        keys its reader falls back to.
 */
inline constexpr Keys::KeyView kDatasetKeys[] = {
  Keys::Title, Keys::HideOnDashboard, Keys::Index, Keys::Units, Keys::Alias, Keys::Color,
  Keys::PltMin, Keys::PltMax, Keys::Graph, Keys::Log, Keys::PltLogX, Keys::PltLogY, Keys::FFT,
  Keys::Waterfall, Keys::FFTBallistics, Keys::FFTBallisticsRelease, Keys::FFTSamples,
  Keys::FFTWindow, Keys::FFTSamplingRate, Keys::FFTLogX, Keys::FFTMin, Keys::FFTMax, Keys::Widget,
  Keys::DisplayTickCount, Keys::DisplayFormat, Keys::DecimalPoints, Keys::WgtMin, Keys::WgtMax,
  Keys::ExtremeHold, Keys::SuppressAlarms, Keys::AlarmsWhileVisible, Keys::AlarmDeadband,
  Keys::LedHigh, Keys::Overview, Keys::Format, Keys::FormatWidth, Keys::FormatSigned,
  Keys::FormatFractionBits, Keys::FormatBitOffset, Keys::FormatBitWidth, Keys::FormatScale,
  Keys::FormatOffset, Keys::LabelsDefault, Keys::Min, Keys::Max,
};

/**
 * @brief Inner keys of one declared alarmBands entry.
 */
inline constexpr Keys::KeyView kAlarmBandsKeys[] = {
  Keys::Min, Keys::Max, Keys::Severity, Keys::Blink, Keys::Color, Keys::Label,
};

/**
 * @brief Inner keys of one declared fftMarkers entry.
 */
inline constexpr Keys::KeyView kFFTMarkersKeys[] = {
  Keys::Frequency, Keys::EndFrequency, Keys::WarningDb, Keys::AlarmDb, Keys::Color, Keys::Label,
};

/**
 * @brief Inner keys of one declared labels entry.
 */
inline constexpr Keys::KeyView kLabelsKeys[] = {
  Keys::Value, Keys::LabelText,
};

/**
 * @brief One nested collection a declared dataset may carry: its key, its inner keys and the most
 *        entries one dataset may declare.
 */
struct SubEntityWire {
  Keys::KeyView key;
  const Keys::KeyView* inner;
  qsizetype innerCount;
  int maxItems;
};

/**
 * @brief Every wire-included sub-entity, in manifest order.
 */
inline constexpr SubEntityWire kSubEntities[] = {
  {Keys::AlarmBands, kAlarmBandsKeys, 6, 16},
  {Keys::FFTMarkers, kFFTMarkersKeys, 6, 16},
  {Keys::Labels, kLabelsKeys, 2, 256},
};

/**
 * @brief Keys a declared frame object may carry (declaration.json).
 */
inline constexpr Keys::KeyView kFrameKeys[] = {
  Keys::Title, Keys::Groups, Keys::Sources,
};

/**
 * @brief Keys a declared group object may carry (declaration.json).
 */
inline constexpr Keys::KeyView kGroupKeys[] = {
  Keys::Title, Keys::Widget, Keys::Datasets, Keys::GroupType, Keys::OutputWidgets,
  Keys::OutputColumns,
};

/**
 * @brief Keys a declared source object may carry (declaration.json).
 */
inline constexpr Keys::KeyView kSourceKeys[] = {
  Keys::FrameStart, Keys::FrameEnd, Keys::FrameDetection, Keys::ChecksumAlgorithm,
  Keys::DecoderMethod, Keys::HexadecimalDelimiters, Keys::FrameParserTemplate,
  Keys::FrameParserParams,
};

/**
 * @brief Keys a declared control object may carry (declaration.json).
 */
inline constexpr Keys::KeyView kControlKeys[] = {
  Keys::Title, Keys::OutputType, Keys::OutputMinValue, Keys::OutputMaxValue, Keys::OutputStepSize,
  Keys::OutputInitialValue, Keys::OutputOnLabel, Keys::OutputOffLabel, Keys::OutputCheckable,
  Keys::OutputInputKind, Keys::OutputDecimals, Keys::OutputUnits, Keys::OutputQuantize,
};

/**
 * @brief Group widgets a declaration may name; code, URL and image widgets never ride the wire.
 */
inline constexpr QLatin1StringView kGroupWidgets[] = {
  QLatin1StringView("datagrid"), QLatin1StringView("multiplot"), QLatin1StringView("accelerometer"),
  QLatin1StringView("gyro"), QLatin1StringView("gps"), QLatin1StringView("plot3d"),
  QLatin1StringView("barpanel"), QLatin1StringView("ledpanel"),
};

/**
 * @brief Values the FftSamples property may take.
 */
inline constexpr int kFftSamplesDomain[] = {
  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144,
};

/**
 * @brief Values the FftWindow property may take.
 */
inline constexpr int kFftWindowDomain[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
};

/**
 * @brief Values the DisplayFormat property may take.
 */
inline constexpr QLatin1StringView kDisplayFormatDomain[] = {
  QLatin1StringView(""), QLatin1StringView("0d"), QLatin1StringView("1d"), QLatin1StringView("2d"),
  QLatin1StringView("3d"), QLatin1StringView("sci"), QLatin1StringView("hex"),
  QLatin1StringView("bin"),
};

/**
 * @brief Values the Format property may take.
 */
inline constexpr int kFormatDomain[] = {
  0, 1, 2, 3,
};

/**
 * @brief True when value is one of domain's entries.
 */
template<typename Value, typename Entry, size_t N>
[[nodiscard]] inline bool inWireDomain(const Value& value, const Entry (&domain)[N])
{
  return std::find(std::begin(domain), std::end(domain), value) != std::end(domain);
}

/**
 * @brief Bounds a declared dataset as the API bounds an update (spec 0111 R10): a value outside its
 *        clamp, domain or validator falls back to the registry default.
 */
inline void sanitizeDatasetWire(Dataset& d)
{
  if (!PropertyHooks::isValidDatasetIndex(d.index))
    d.index = 0;

  if (!PropertyHooks::isValidColor(d.color))
    d.color = QString();

  d.pltMin = std::isfinite(d.pltMin) ? d.pltMin : 0;
  d.pltMax = std::isfinite(d.pltMax) ? d.pltMax : 0;
  d.fftBallisticsRelease = qBound(50, d.fftBallisticsRelease, 5000);
  if (!inWireDomain(d.fftSamples, kFftSamplesDomain))
    d.fftSamples = 256;

  if (!inWireDomain(d.fftWindow, kFftWindowDomain))
    d.fftWindow = 5;

  d.fftMin = std::isfinite(d.fftMin) ? d.fftMin : 0;
  d.fftMax = std::isfinite(d.fftMax) ? d.fftMax : 0;
  d.displayTickCount = qMax(0, d.displayTickCount);
  if (!inWireDomain(d.displayFormat, kDisplayFormatDomain))
    d.displayFormat = QStringLiteral("0d");

  d.decimalPoints = qBound(-1, d.decimalPoints, 15);
  d.wgtMin = std::isfinite(d.wgtMin) ? d.wgtMin : 0;
  d.wgtMax = std::isfinite(d.wgtMax) ? d.wgtMax : 0;
  d.alarmDeadband = std::isfinite(d.alarmDeadband) ? d.alarmDeadband : 0;
  if (!PropertyHooks::isValidAlarmDeadband(d.alarmDeadband))
    d.alarmDeadband = 0;

  d.ledHigh = std::isfinite(d.ledHigh) ? d.ledHigh : 80;
  if (!inWireDomain(d.format, kFormatDomain))
    d.format = 0;

  d.formatWidth = qBound(1, d.formatWidth, 64);
  d.formatFractionBits = qBound(0, d.formatFractionBits, 63);
  d.formatBitOffset = qBound(0, d.formatBitOffset, 63);
  d.formatBitWidth = qBound(1, d.formatBitWidth, 64);
  d.formatScale = std::isfinite(d.formatScale) ? d.formatScale : 1;
  d.formatOffset = std::isfinite(d.formatOffset) ? d.formatOffset : 0;
  if (!PropertyHooks::isValidFormatFraction(d))
    d.formatFractionBits = 0;

  if (!PropertyHooks::isValidFormatBitField(d))
    d.formatBitOffset = 0;

  if (!PropertyHooks::isValidFormatBitField(d))
    d.formatBitWidth = 1;
}

}  // namespace DataModel::Declaration

// clang-format on
