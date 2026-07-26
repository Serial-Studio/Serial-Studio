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

// AUTO-GENERATED from app/rcc/properties/dataset.json; never edit by hand.

// Regenerate with: python3 scripts/generate-property-registry.py

#include "DataModel/Frame.h"

#include <QJsonArray>

#include "SerialStudio.h"
#include "DataModel/Project/PropertyHooks.h"

// clang-format off

namespace DataModel {

//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the declared flags of a dataset into its project-JSON object.
 */
static void writeDatasetFlags(QJsonObject& obj, const Dataset& d)
{
  if (d.virtual_)
    obj.insert(Keys::Virtual, true);

  if (d.hideOnDashboard)
    obj.insert(Keys::HideOnDashboard, true);

  obj.insert(Keys::Graph, d.plt);
  obj.insert(Keys::Log, d.log);
  if (d.pltLogX)
    obj.insert(Keys::PltLogX, true);

  if (d.pltLogY)
    obj.insert(Keys::PltLogY, true);

  obj.insert(Keys::FFT, d.fft);
  if (d.waterfall)
    obj.insert(Keys::Waterfall, true);

  if (d.fftBallistics)
    obj.insert(Keys::FFTBallistics, true);

  if (d.fftLogX)
    obj.insert(Keys::FFTLogX, true);

  obj.insert(Keys::LED, d.led);
  if (d.overviewDisplay)
    obj.insert(Keys::Overview, true);

  if (!d.enabled)
    obj.insert(Keys::Disabled, true);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the declared numbers of a dataset into its project-JSON object.
 */
static void writeDatasetNumbers(QJsonObject& obj, const Dataset& d)
{
  obj.insert(Keys::Index, d.index);
  obj.insert(Keys::PltMin, qMin(d.pltMin, d.pltMax));
  obj.insert(Keys::PltMax, qMax(d.pltMin, d.pltMax));
  obj.insert(Keys::XAxis, d.xAxisId);
  if (d.fftBallistics)
    obj.insert(Keys::FFTBallisticsRelease, d.fftBallisticsRelease);

  if (d.waterfallYAxis != 0)
    obj.insert(Keys::WaterfallYAxis, d.waterfallYAxis);

  obj.insert(Keys::FFTSamples, d.fftSamples);
  obj.insert(Keys::FFTWindow, d.fftWindow);
  obj.insert(Keys::FFTSamplingRate, d.fftSamplingRate);
  obj.insert(Keys::FFTMin, qMin(d.fftMin, d.fftMax));
  obj.insert(Keys::FFTMax, qMax(d.fftMin, d.fftMax));
  if (d.displayTickCount > 0)
    obj.insert(Keys::DisplayTickCount, d.displayTickCount);

  if (d.decimalPoints >= 0)
    obj.insert(Keys::DecimalPoints, d.decimalPoints);

  obj.insert(Keys::WgtMin, qMin(d.wgtMin, d.wgtMax));
  obj.insert(Keys::WgtMax, qMax(d.wgtMin, d.wgtMax));
  obj.insert(Keys::LedHigh, d.ledHigh);
  if (!d.transformCode.isEmpty())
    obj.insert(Keys::TransformLanguage, d.transformLanguage);

  obj.insert(Keys::GroupId, d.groupId);
  obj.insert(Keys::DatasetId, d.datasetId);
  if (d.uniqueId >= 0)
    obj.insert(Keys::UniqueId, d.uniqueId);

  obj.insert(Keys::NumericValue, d.numericValue);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the declared strings of a dataset into its project-JSON object.
 */
static void writeDatasetStrings(QJsonObject& obj, const Dataset& d)
{
  obj.insert(Keys::Title, d.title.simplified());
  obj.insert(Keys::Units, d.units.simplified());
  if (!d.alias.isEmpty())
    obj.insert(Keys::Alias, d.alias);

  if (!d.color.isEmpty())
    obj.insert(Keys::Color, d.color);

  obj.insert(Keys::Widget, d.widget.simplified());
  if (!d.displayFormat.isEmpty())
    obj.insert(Keys::DisplayFormat, d.displayFormat);

  if (!d.transformCode.isEmpty())
    obj.insert(Keys::TransformCode, d.transformCode);

  obj.insert(Keys::Value, d.value.simplified());
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the nested alarm-band and frequency-marker collections when they are non-empty.
 */
static void writeDatasetSubEntities(QJsonObject& obj, const Dataset& d)
{
  if (!d.alarmBands.empty()) {
    QJsonArray alarm_bands;
    for (const auto& entry : d.alarmBands)
      alarm_bands.append(serialize(entry));

    obj.insert(Keys::AlarmBands, alarm_bands);
  }

  if (!d.fftMarkers.empty()) {
    QJsonArray fft_markers;
    for (const auto& entry : d.fftMarkers)
      fft_markers.append(serialize(entry));

    obj.insert(Keys::FFTMarkers, fft_markers);
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the declared flags of a dataset from its project-JSON object.
 */
static void readDatasetFlags(Dataset& d, const QJsonObject& obj)
{
  d.virtual_ = ss_jsr(obj, Keys::Virtual, false).toBool();
  d.hideOnDashboard = ss_jsr(obj, Keys::HideOnDashboard, false).toBool();
  d.plt = ss_jsr(obj, Keys::Graph, false).toBool();
  d.log = ss_jsr(obj, Keys::Log, false).toBool();
  d.pltLogX = ss_jsr(obj, Keys::PltLogX, false).toBool();
  d.pltLogY = ss_jsr(obj, Keys::PltLogY, false).toBool();
  d.fft = ss_jsr(obj, Keys::FFT, false).toBool();
  d.waterfall = ss_jsr(obj, Keys::Waterfall, false).toBool();
  d.fftBallistics = ss_jsr(obj, Keys::FFTBallistics, false).toBool();
  d.fftLogX = ss_jsr(obj, Keys::FFTLogX, false).toBool();
  d.led = ss_jsr(obj, Keys::LED, false).toBool();
  d.overviewDisplay = ss_jsr(obj, Keys::Overview, false).toBool();
  d.enabled = !ss_jsr(obj, Keys::Disabled, false).toBool();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the declared numbers of a dataset from its project-JSON object.
 */
static void readDatasetNumbers(Dataset& d, const QJsonObject& obj)
{
  d.index = ss_jsr(obj, Keys::Index, 0).toInt();
  d.pltMin = SerialStudio::toDouble(ss_jsr(obj, Keys::PltMin, 0));
  d.pltMax = SerialStudio::toDouble(ss_jsr(obj, Keys::PltMax, 0));
  d.xAxisId = ss_jsr(obj, Keys::XAxis, -2).toInt();
  d.fftBallisticsRelease = ss_jsr(obj, Keys::FFTBallisticsRelease, 300).toInt();
  d.waterfallYAxis = ss_jsr(obj, Keys::WaterfallYAxis, 0).toInt();
  d.fftSamples = ss_jsr(obj, Keys::FFTSamples, 256).toInt();
  d.fftWindow = ss_jsr(obj, Keys::FFTWindow, 5).toInt();
  d.fftSamplingRate = ss_jsr(obj, Keys::FFTSamplingRate, 100).toInt();
  d.fftMin = SerialStudio::toDouble(ss_jsr(obj, Keys::FFTMin, 0));
  d.fftMax = SerialStudio::toDouble(ss_jsr(obj, Keys::FFTMax, 0));
  d.displayTickCount = ss_jsr(obj, Keys::DisplayTickCount, 5).toInt();
  d.decimalPoints = ss_jsr(obj, Keys::DecimalPoints, -1).toInt();
  d.wgtMin = SerialStudio::toDouble(ss_jsr(obj, Keys::WgtMin, 0));
  d.wgtMax = SerialStudio::toDouble(ss_jsr(obj, Keys::WgtMax, 0));
  d.ledHigh = SerialStudio::toDouble(ss_jsr(obj, Keys::LedHigh, 80));
  d.transformLanguage = ss_jsr(obj, Keys::TransformLanguage, -1).toInt();
  d.uniqueId = ss_jsr(obj, Keys::UniqueId, -1).toInt();
  d.sourceId = ss_jsr(obj, Keys::DatasetSourceId, 0).toInt();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the declared strings of a dataset from its project-JSON object.
 */
static void readDatasetStrings(Dataset& d, const QJsonObject& obj)
{
  d.title = ss_jsr(obj, Keys::Title, "").toString().simplified();
  d.units = ss_jsr(obj, Keys::Units, "").toString().simplified();
  d.alias = ss_jsr(obj, Keys::Alias, "").toString().simplified();
  d.color = ss_jsr(obj, Keys::Color, "").toString().simplified();
  d.widget = ss_jsr(obj, Keys::Widget, "").toString().simplified();
  d.displayFormat = ss_jsr(obj, Keys::DisplayFormat, "0d").toString();
  d.transformCode = ss_jsr(obj, Keys::TransformCode, "").toString();
  d.value = ss_jsr(obj, Keys::Value, "").toString().simplified();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the declared post-read steps: colour validation, derived numeric value, FFT window
 *        clamp, legacy range fallbacks, nested entities and range normalization.
 */
static void finalizeDatasetRead(Dataset& d, const QJsonObject& obj)
{
  if (!PropertyHooks::isValidColor(d.color))
    d.color.clear();

  if (!d.value.isEmpty())
    d.numericValue = SerialStudio::toDouble(d.value, &d.isNumeric);

  if (!PropertyHooks::isValidFftWindow(d.fftWindow))
    d.fftWindow = 5;

  if (!obj.contains(Keys::PltMin) || !obj.contains(Keys::PltMax)) {
    d.pltMin = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
    d.pltMax = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  }

  if (!obj.contains(Keys::FFTMin) || !obj.contains(Keys::FFTMax)) {
    d.fftMin = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
    d.fftMax = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  }

  if (!obj.contains(Keys::WgtMin) || !obj.contains(Keys::WgtMax)) {
    d.wgtMin = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
    d.wgtMax = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  }

  readDatasetAlarmBands(d, obj);
  readDatasetFrequencyMarkers(d, obj);
  normalizeDatasetRanges(d);
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes a Dataset to a QJsonObject.
 */
QJsonObject DataModel::serialize(const Dataset& d)
{
  QJsonObject obj;
  writeDatasetFlags(obj, d);
  writeDatasetNumbers(obj, d);
  writeDatasetStrings(obj, d);
  writeDatasetSubEntities(obj, d);
  return obj;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Deserializes a Dataset from a QJsonObject.
 */
bool DataModel::read(Dataset& d, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  readDatasetFlags(d, obj);
  readDatasetNumbers(d, obj);
  readDatasetStrings(d, obj);
  finalizeDatasetRead(d, obj);
  return true;
}

// clang-format on
