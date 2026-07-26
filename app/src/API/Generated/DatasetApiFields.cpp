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

#include "API/Handlers/ProjectHandler.h"

#include <QJsonArray>
#include <optional>

#include "DataModel/Project/PropertyHooks.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

// clang-format off

namespace PropertyHooks = DataModel::PropertyHooks;

namespace API::Handlers {

/**
 * @brief Returns the first present spelling of a declared field and records every present spelling
 *        as consumed, so the unknown-field warning cannot be forgotten.
 */
static QString takeDatasetField(const QJsonObject& params,
                                QSet<QString>& consumed,
                                const QStringList& names)
{
  QString found;
  for (const auto& name : names) {
    if (!params.contains(name))
      continue;

    consumed.insert(name);
    if (found.isEmpty())
      found = name;
  }

  return found;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared string fields onto d (part 1 of 2); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetStringFields1(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  static auto& project = DataModel::ProjectModel::instance();

  const auto key_title = takeDatasetField(params, consumed, {Keys::Title});
  if (!key_title.isEmpty()) {
    d.title = params.value(key_title).toString();
    rebuildTree = true;
  }

  const auto key_units = takeDatasetField(params, consumed, {Keys::Units});
  if (!key_units.isEmpty())
    d.units = params.value(key_units).toString();

  const auto key_alias = takeDatasetField(params, consumed, {Keys::Alias});
  if (!key_alias.isEmpty()) {
    const auto candidate = params.value(key_alias).toString().simplified();
    if (!candidate.isEmpty()
        && PropertyHooks::aliasInUseByOtherDataset(project, candidate, d.uniqueId))
      return QStringLiteral("Alias '%1' is already used by another dataset; aliases must be "
                            "unique across the project").arg(candidate);

    d.alias = candidate;
  }

  const auto key_color = takeDatasetField(params, consumed, {Keys::Color});
  if (!key_color.isEmpty()) {
    const auto candidate = params.value(key_color).toString().simplified();
    if (!PropertyHooks::isValidColor(candidate))
      return QStringLiteral("Invalid color '%1': use '#rrggbb' or a valid color name; empty "
                            "string restores the automatic theme color").arg(candidate);

    d.color = candidate;
  }

  const auto key_widget = takeDatasetField(params, consumed, {Keys::Widget});
  if (!key_widget.isEmpty()) {
    d.widget = params.value(key_widget).toString();
    rebuildTree = true;
  }

  const auto key_display_format = takeDatasetField(params, consumed, {Keys::DisplayFormat});
  if (!key_display_format.isEmpty())
    d.displayFormat = params.value(key_display_format).toString();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared string fields onto d (part 2 of 2); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetStringFields2(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  Q_UNUSED(rebuildTree);

  const auto key_transform_code = takeDatasetField(params, consumed, {Keys::TransformCode});
  if (!key_transform_code.isEmpty())
    d.transformCode = params.value(key_transform_code).toString();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared number fields onto d (part 1 of 3); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetNumberFields1(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  const auto key_index = takeDatasetField(params, consumed, {Keys::Index});
  if (!key_index.isEmpty()) {
    const auto candidate = params.value(key_index).toInt();
    if (!PropertyHooks::isValidDatasetIndex(candidate))
      return QStringLiteral("Invalid index: must be >= 0 (0 = unassigned, 1+ = parser slot)");

    d.index = candidate;
    rebuildTree = true;
  }

  const auto key_plt_min = takeDatasetField(
    params, consumed,
    {QStringLiteral("pltMin"), Keys::PltMin});
  if (!key_plt_min.isEmpty())
    d.pltMin = SerialStudio::toDouble(params.value(key_plt_min));

  const auto key_plt_max = takeDatasetField(
    params, consumed,
    {QStringLiteral("pltMax"), Keys::PltMax});
  if (!key_plt_max.isEmpty())
    d.pltMax = SerialStudio::toDouble(params.value(key_plt_max));

  const auto key_x_axis_id = takeDatasetField(
    params, consumed,
    {QStringLiteral("xAxisId"), Keys::XAxis});
  if (!key_x_axis_id.isEmpty())
    d.xAxisId = params.value(key_x_axis_id).toInt();

  const auto key_fft_ballistics_release = takeDatasetField(
    params, consumed,
    {Keys::FFTBallisticsRelease});
  if (!key_fft_ballistics_release.isEmpty())
    d.fftBallisticsRelease = qBound(50, params.value(key_fft_ballistics_release).toInt(), 5000);

  const auto key_waterfall_y_axis = takeDatasetField(params, consumed, {Keys::WaterfallYAxis});
  if (!key_waterfall_y_axis.isEmpty())
    d.waterfallYAxis = params.value(key_waterfall_y_axis).toInt();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared number fields onto d (part 2 of 3); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetNumberFields2(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  Q_UNUSED(rebuildTree);

  const auto key_fft_samples = takeDatasetField(params, consumed, {Keys::FFTSamples});
  if (!key_fft_samples.isEmpty())
    d.fftSamples = params.value(key_fft_samples).toInt();

  const auto key_fft_window = takeDatasetField(params, consumed, {Keys::FFTWindow});
  if (!key_fft_window.isEmpty()) {
    const auto candidate = params.value(key_fft_window).toInt();
    if (!PropertyHooks::isValidFftWindow(candidate))
      return QStringLiteral("Invalid fftWindow: must be 0-14 (5 = Blackman-Harris)");

    d.fftWindow = candidate;
  }

  const auto key_fft_sampling_rate = takeDatasetField(params, consumed, {Keys::FFTSamplingRate});
  if (!key_fft_sampling_rate.isEmpty())
    d.fftSamplingRate = params.value(key_fft_sampling_rate).toInt();

  const auto key_fft_min = takeDatasetField(params, consumed, {Keys::FFTMin});
  if (!key_fft_min.isEmpty())
    d.fftMin = SerialStudio::toDouble(params.value(key_fft_min));

  const auto key_fft_max = takeDatasetField(params, consumed, {Keys::FFTMax});
  if (!key_fft_max.isEmpty())
    d.fftMax = SerialStudio::toDouble(params.value(key_fft_max));

  const auto key_display_tick_count = takeDatasetField(params, consumed, {Keys::DisplayTickCount});
  if (!key_display_tick_count.isEmpty())
    d.displayTickCount = qMax(0, params.value(key_display_tick_count).toInt());

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared number fields onto d (part 3 of 3); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetNumberFields3(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  Q_UNUSED(rebuildTree);

  const auto key_decimal_points = takeDatasetField(params, consumed, {Keys::DecimalPoints});
  if (!key_decimal_points.isEmpty())
    d.decimalPoints = qBound(-1, params.value(key_decimal_points).toInt(), 15);

  const auto key_wgt_min = takeDatasetField(
    params, consumed,
    {QStringLiteral("wgtMin"), Keys::WgtMin});
  if (!key_wgt_min.isEmpty())
    d.wgtMin = SerialStudio::toDouble(params.value(key_wgt_min));

  const auto key_wgt_max = takeDatasetField(
    params, consumed,
    {QStringLiteral("wgtMax"), Keys::WgtMax});
  if (!key_wgt_max.isEmpty())
    d.wgtMax = SerialStudio::toDouble(params.value(key_wgt_max));

  const auto key_led_high = takeDatasetField(params, consumed, {Keys::LedHigh});
  if (!key_led_high.isEmpty())
    d.ledHigh = SerialStudio::toDouble(params.value(key_led_high));

  const auto key_transform_language = takeDatasetField(params, consumed, {Keys::TransformLanguage});
  if (!key_transform_language.isEmpty()) {
    const auto candidate = params.value(key_transform_language).toInt();
    if (!PropertyHooks::isValidTransformLanguage(candidate))
      return QStringLiteral("Invalid transformLanguage: must be -1 (inherit), 0 (JS), or 1 (Lua)");

    d.transformLanguage = candidate;
  }

  const auto key_source_id = takeDatasetField(
    params, consumed,
    {Keys::SourceId, Keys::DatasetSourceId});
  if (!key_source_id.isEmpty())
    d.sourceId = params.value(key_source_id).toInt();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared flag fields onto d (part 1 of 2); returns a non-empty error string when a
 *        value is rejected.
 */
static QString applyDatasetFlagFields1(DataModel::Dataset& d,
                                       const QJsonObject& params,
                                       bool& rebuildTree,
                                       QSet<QString>& consumed)
{
  const auto key_virtual = takeDatasetField(params, consumed, {Keys::Virtual});
  if (!key_virtual.isEmpty())
    d.virtual_ = params.value(key_virtual).toBool();

  const auto key_hide_on_dashboard = takeDatasetField(params, consumed, {Keys::HideOnDashboard});
  if (!key_hide_on_dashboard.isEmpty()) {
    d.hideOnDashboard = params.value(key_hide_on_dashboard).toBool();
    rebuildTree = true;
  }

  const auto key_plt = takeDatasetField(params, consumed, {Keys::Graph});
  if (!key_plt.isEmpty()) {
    d.plt = params.value(key_plt).toBool();
    rebuildTree = true;
  }

  const auto key_log = takeDatasetField(params, consumed, {Keys::Log});
  if (!key_log.isEmpty())
    d.log = params.value(key_log).toBool();

  const auto key_plt_log_x = takeDatasetField(params, consumed, {Keys::PltLogX});
  if (!key_plt_log_x.isEmpty())
    d.pltLogX = params.value(key_plt_log_x).toBool();

  const auto key_plt_log_y = takeDatasetField(params, consumed, {Keys::PltLogY});
  if (!key_plt_log_y.isEmpty())
    d.pltLogY = params.value(key_plt_log_y).toBool();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared flag fields onto d (part 2 of 2); returns a non-empty error string when a
 *        value is rejected.
 */
static QString applyDatasetFlagFields2(DataModel::Dataset& d,
                                       const QJsonObject& params,
                                       bool& rebuildTree,
                                       QSet<QString>& consumed)
{
  const auto key_fft = takeDatasetField(params, consumed, {Keys::FFT});
  if (!key_fft.isEmpty()) {
    d.fft = params.value(key_fft).toBool();
    rebuildTree = true;
  }

  const auto key_waterfall = takeDatasetField(params, consumed, {Keys::Waterfall});
  if (!key_waterfall.isEmpty()) {
    d.waterfall = params.value(key_waterfall).toBool();
    rebuildTree = true;
  }

  const auto key_fft_ballistics = takeDatasetField(params, consumed, {Keys::FFTBallistics});
  if (!key_fft_ballistics.isEmpty())
    d.fftBallistics = params.value(key_fft_ballistics).toBool();

  const auto key_fft_log_x = takeDatasetField(params, consumed, {Keys::FFTLogX});
  if (!key_fft_log_x.isEmpty())
    d.fftLogX = params.value(key_fft_log_x).toBool();

  const auto key_led = takeDatasetField(params, consumed, {Keys::LED});
  if (!key_led.isEmpty()) {
    d.led = params.value(key_led).toBool();
    rebuildTree = true;
  }

  const auto key_overview_display = takeDatasetField(params, consumed, {Keys::Overview});
  if (!key_overview_display.isEmpty()) {
    d.overviewDisplay = params.value(key_overview_display).toBool();
    rebuildTree = true;
  }

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces the nested alarm-band and frequency-marker collections from their declared field
 *        names, keeping the v3.3 alarmLow/alarmHigh/alarmEnabled inputs alive.
 */
static void applyDatasetSubEntityFields(DataModel::Dataset& d,
                                        const QJsonObject& params,
                                        QSet<QString>& consumed)
{
  const auto key_alarm_bands = takeDatasetField(params, consumed, {Keys::AlarmBands});
  const auto key_alarm_enabled = takeDatasetField(params, consumed, {Keys::AlarmEnabled});
  const auto key_alarm_low = takeDatasetField(params, consumed, {Keys::AlarmLow});
  const auto key_alarm_high = takeDatasetField(params, consumed, {Keys::AlarmHigh});
  if (!key_alarm_bands.isEmpty()) {
    d.alarmBands.clear();
    const auto entries = params.value(key_alarm_bands).toArray();
    d.alarmBands.reserve(entries.size());
    for (const auto& entry : entries) {
      DataModel::AlarmBand parsed;
      if (DataModel::read(parsed, entry.toObject()))
        d.alarmBands.push_back(std::move(parsed));
    }
  }
  else if (!key_alarm_enabled.isEmpty() || !key_alarm_low.isEmpty() || !key_alarm_high.isEmpty()) {
    std::optional<bool> enabled;
    if (!key_alarm_enabled.isEmpty())
      enabled = params.value(key_alarm_enabled).toBool();

    std::optional<double> low;
    if (!key_alarm_low.isEmpty())
      low = SerialStudio::toDouble(params.value(key_alarm_low));

    std::optional<double> high;
    if (!key_alarm_high.isEmpty())
      high = SerialStudio::toDouble(params.value(key_alarm_high));

    applySimpleAlarmFields(d, enabled, low, high);
  }

  const auto key_fft_markers = takeDatasetField(params, consumed, {Keys::FFTMarkers});
  if (!key_fft_markers.isEmpty()) {
    d.fftMarkers.clear();
    const auto entries = params.value(key_fft_markers).toArray();
    d.fftMarkers.reserve(entries.size());
    for (const auto& entry : entries) {
      DataModel::FrequencyMarker parsed;
      if (DataModel::read(parsed, entry.toObject()))
        d.fftMarkers.push_back(std::move(parsed));
    }
  }
}

}  // namespace API::Handlers

//--------------------------------------------------------------------------------------------------

/**
 * @brief Patches dataset fields from a generic params object; returns an error string on failure.
 */
QString API::Handlers::ProjectHandler::applyDatasetUpdateParams(DataModel::Dataset& d,
                                                                const QJsonObject& params,
                                                                bool& rebuildTree,
                                                                QSet<QString>& consumed)
{
  if (auto err = applyDatasetStringFields1(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetStringFields2(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetNumberFields1(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetNumberFields2(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetNumberFields3(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetFlagFields1(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetFlagFields2(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  applyDatasetSubEntityFields(d, params, consumed);
  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds one typed schema property entry.
 */
static QJsonObject datasetSchemaProperty(const char* type,
                                         const char* description,
                                         const QJsonArray& domain)
{
  QJsonObject prop;
  prop.insert(QStringLiteral("type"), QString::fromUtf8(type));
  prop.insert(QStringLiteral("description"), QString::fromUtf8(description));
  if (!domain.isEmpty())
    prop.insert(QStringLiteral("enum"), domain);

  return prop;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 1 of 4.
 */
static void datasetSchemaPart1(QJsonObject& props)
{
  props.insert(Keys::Title,
               datasetSchemaProperty("string",
                                     "Name of the dataset, used for labeling and identification",
                                     QJsonArray()));
  props.insert(Keys::Virtual,
               datasetSchemaProperty("boolean",
                                     "Virtual datasets compute their value from transforms and "
                                     "data tables, they do not require a frame index",
                                     QJsonArray()));
  props.insert(Keys::HideOnDashboard,
               datasetSchemaProperty("boolean",
                                     "Suppress this dataset's standalone dashboard tile; the "
                                     "painter widget can still read its values",
                                     QJsonArray()));
  props.insert(Keys::Index,
               datasetSchemaProperty("integer",
                                     "Frame position used for aligning datasets in time",
                                     QJsonArray()));
  props.insert(Keys::Units,
               datasetSchemaProperty("string",
                                     "Unit of measurement, such as volts or amps (optional)",
                                     QJsonArray()));
  props.insert(Keys::Alias,
               datasetSchemaProperty("string",
                                     "Stable name for getDataset-style script/API lookups; must "
                                     "be unique (optional)",
                                     QJsonArray()));
  props.insert(Keys::Color,
               datasetSchemaProperty("string",
                                     "Custom display color for this dataset; automatic uses the "
                                     "theme palette",
                                     QJsonArray()));
  props.insert(QStringLiteral("pltMin"),
               datasetSchemaProperty("number",
                                     "Lower bound of the dataset value range; widgets and FFT "
                                     "fall back to it when their own range is left unset",
                                     QJsonArray()));
  props.insert(QStringLiteral("pltMax"),
               datasetSchemaProperty("number",
                                     "Upper bound of the dataset value range; widgets and FFT "
                                     "fall back to it when their own range is left unset",
                                     QJsonArray()));
  props.insert(Keys::Graph,
               datasetSchemaProperty("boolean",
                                     "Plot data in real-time",
                                     QJsonArray()));
  props.insert(Keys::Log,
               datasetSchemaProperty("boolean",
                                     "Include this dataset in CSV/data logging",
                                     QJsonArray()));
  props.insert(QStringLiteral("xAxisId"),
               datasetSchemaProperty("integer",
                                     "Choose Time or a dataset to drive the X-Axis in plots",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 2 of 4.
 */
static void datasetSchemaPart2(QJsonObject& props)
{
  props.insert(Keys::PltLogX,
               datasetSchemaProperty("boolean",
                                     "Scale the X axis in decades; available when the X-Axis "
                                     "source is Samples or a dataset (not Time)",
                                     QJsonArray()));
  props.insert(Keys::PltLogY,
               datasetSchemaProperty("boolean",
                                     "Scale the Y axis in decades; values at or below zero are "
                                     "clamped",
                                     QJsonArray()));
  props.insert(Keys::FFT,
               datasetSchemaProperty("boolean",
                                     "Perform frequency-domain analysis of the dataset",
                                     QJsonArray()));
  props.insert(Keys::Waterfall,
               datasetSchemaProperty("boolean",
                                     "Show a scrolling spectrogram of frequency content over time "
                                     "(Pro)",
                                     QJsonArray()));
  props.insert(Keys::FFTBallistics,
               datasetSchemaProperty("boolean",
                                     "Analyzer-style display: peaks rise instantly and decay "
                                     "smoothly over the release time",
                                     QJsonArray()));
  props.insert(Keys::FFTBallisticsRelease,
               datasetSchemaProperty("integer",
                                     "Decay time for the ballistics display (50-5000 ms)",
                                     QJsonArray()));
  props.insert(Keys::WaterfallYAxis,
               datasetSchemaProperty("integer",
                                     "Choose Time (default) or any dataset whose value drives the "
                                     "Y axis -- produces a Campbell diagram when bound to e.g. "
                                     "RPM",
                                     QJsonArray()));
  props.insert(Keys::FFTSamples,
               datasetSchemaProperty("integer",
                                     "Number of samples used for each FFT calculation window",
                                     QJsonArray({8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
                                                 8192, 16384, 32768, 65536, 131072, 262144})));
  props.insert(Keys::FFTWindow,
               datasetSchemaProperty("integer",
                                     "Window applied before the transform to reduce spectral "
                                     "leakage; affects both the FFT plot and the waterfall",
                                     QJsonArray({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                                 14})));
  props.insert(Keys::FFTSamplingRate,
               datasetSchemaProperty("integer",
                                     "Sampling frequency used for FFT (in Hz)",
                                     QJsonArray()));
  props.insert(Keys::FFTLogX,
               datasetSchemaProperty("boolean",
                                     "Scale the frequency axis in decades so low octaves stay "
                                     "readable; applies to both the FFT plot and the waterfall",
                                     QJsonArray()));
  props.insert(Keys::FFTMin,
               datasetSchemaProperty("number",
                                     "Lower bound for data normalization; falls back to the "
                                     "dataset value range when left unset",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 3 of 4.
 */
static void datasetSchemaPart3(QJsonObject& props)
{
  props.insert(Keys::FFTMax,
               datasetSchemaProperty("number",
                                     "Upper bound for data normalization; falls back to the "
                                     "dataset value range when left unset",
                                     QJsonArray()));
  props.insert(Keys::Widget,
               datasetSchemaProperty("string",
                                     "Select the visual widget used to display this dataset",
                                     QJsonArray({"", "bar", "compass", "gauge", "meter"})));
  props.insert(Keys::DisplayTickCount,
               datasetSchemaProperty("integer",
                                     "Major-tick count on the dial scale (0 = auto-fit to widget "
                                     "size)",
                                     QJsonArray()));
  props.insert(Keys::DisplayFormat,
               datasetSchemaProperty("string",
                                     "Decimal places or notation used on tick labels and the "
                                     "value display",
                                     QJsonArray({"", "0d", "1d", "2d", "3d", "sci"})));
  props.insert(Keys::DecimalPoints,
               datasetSchemaProperty("integer",
                                     "Fixed decimal places for the value display; overrides the "
                                     "format (-1 = auto)",
                                     QJsonArray()));
  props.insert(QStringLiteral("wgtMin"),
               datasetSchemaProperty("number",
                                     "Lower bound of the gauge or bar range; falls back to the "
                                     "dataset value range when left unset",
                                     QJsonArray()));
  props.insert(QStringLiteral("wgtMax"),
               datasetSchemaProperty("number",
                                     "Upper bound of the gauge or bar range; falls back to the "
                                     "dataset value range when left unset",
                                     QJsonArray()));
  props.insert(Keys::LED,
               datasetSchemaProperty("boolean",
                                     "Enable visual status monitoring using an LED display",
                                     QJsonArray()));
  props.insert(Keys::LedHigh,
               datasetSchemaProperty("number",
                                     "LED lights up when value meets or exceeds this threshold; "
                                     "define alarm bands for multi-state colors",
                                     QJsonArray()));
  props.insert(Keys::Overview,
               datasetSchemaProperty("boolean",
                                     "Show this dataset in the project overview panel",
                                     QJsonArray()));
  props.insert(Keys::TransformCode,
               datasetSchemaProperty("string",
                                     "Per-dataset transform script; runs on every parsed value",
                                     QJsonArray()));
  props.insert(Keys::TransformLanguage,
               datasetSchemaProperty("integer",
                                     "Transform script language: -1 inherit, 0 JavaScript, 1 Lua",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 4 of 4.
 */
static void datasetSchemaPart4(QJsonObject& props)
{
  props.insert(Keys::SourceId,
               datasetSchemaProperty("integer",
                                     "Source (device) this dataset belongs to",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the typed schema properties for the dataset verbs, replacing the prose field
 *        enumeration the API used to publish.
 */
QJsonObject API::Handlers::datasetFieldSchema()
{
  QJsonObject props;
  datasetSchemaPart1(props);
  datasetSchemaPart2(props);
  datasetSchemaPart3(props);
  datasetSchemaPart4(props);
  return props;
}

// clang-format on
