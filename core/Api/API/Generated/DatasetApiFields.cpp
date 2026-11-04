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

#include <optional>
#include <QJsonArray>

#include "API/Handlers/ProjectHandler.h"
#include "Core/SerialStudio.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/Project/PropertyHooks.h"
#include "DataModel/ProjectModel.h"

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
  auto& project = DataModel::pipelineModules().projectModel;

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

  const auto key_labels_default = takeDatasetField(params, consumed, {Keys::LabelsDefault});
  if (!key_labels_default.isEmpty())
    d.labelsDefault = params.value(key_labels_default).toString().simplified();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared number fields onto d (part 1 of 5); returns a non-empty error string when
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
 * @brief Applies declared number fields onto d (part 2 of 5); returns a non-empty error string when
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
 * @brief Applies declared number fields onto d (part 3 of 5); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetNumberFields3(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
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

  const auto key_alarm_deadband = takeDatasetField(params, consumed, {Keys::AlarmDeadband});
  if (!key_alarm_deadband.isEmpty()) {
    const auto candidate = SerialStudio::toDouble(params.value(key_alarm_deadband));
    if (!PropertyHooks::isValidAlarmDeadband(candidate))
      return QStringLiteral("Invalid alarmDeadband: must be >= 0 (dataset units, 0 = no "
                            "hysteresis)");

    d.alarmDeadband = candidate;
    rebuildTree = true;
  }

  const auto key_led_high = takeDatasetField(params, consumed, {Keys::LedHigh});
  if (!key_led_high.isEmpty())
    d.ledHigh = SerialStudio::toDouble(params.value(key_led_high));

  const auto key_transform_language = takeDatasetField(params, consumed, {Keys::TransformLanguage});
  if (!key_transform_language.isEmpty()) {
    const auto candidate = params.value(key_transform_language).toInt();
    if (!PropertyHooks::isValidTransformLanguage(candidate))
      return QStringLiteral("Invalid transformLanguage: must be -1 (inherit), 0 (JS), 1 (Lua) or "
                            "3 (Expression)");

    d.transformLanguage = candidate;
  }

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared number fields onto d (part 4 of 5); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetNumberFields4(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  Q_UNUSED(rebuildTree);

  const auto key_format = takeDatasetField(params, consumed, {Keys::Format});
  if (!key_format.isEmpty()) {
    const auto candidate = params.value(key_format).toInt();
    if (!PropertyHooks::isValidValueFormat(candidate))
      return QStringLiteral("Invalid format: must be 0 (none), 1 (fixed point), 2 (bit field) or "
                            "3 (scale)");

    d.format = candidate;
  }

  const auto key_format_width = takeDatasetField(params, consumed, {Keys::FormatWidth});
  if (!key_format_width.isEmpty())
    d.formatWidth = qBound(1, params.value(key_format_width).toInt(), 64);

  const auto key_format_fraction_bits = takeDatasetField(
    params, consumed,
    {Keys::FormatFractionBits});
  if (!key_format_fraction_bits.isEmpty())
    d.formatFractionBits = qBound(0, params.value(key_format_fraction_bits).toInt(), 63);

  const auto key_format_bit_offset = takeDatasetField(params, consumed, {Keys::FormatBitOffset});
  if (!key_format_bit_offset.isEmpty())
    d.formatBitOffset = qBound(0, params.value(key_format_bit_offset).toInt(), 63);

  const auto key_format_bit_width = takeDatasetField(params, consumed, {Keys::FormatBitWidth});
  if (!key_format_bit_width.isEmpty())
    d.formatBitWidth = qBound(1, params.value(key_format_bit_width).toInt(), 64);

  const auto key_format_scale = takeDatasetField(params, consumed, {Keys::FormatScale});
  if (!key_format_scale.isEmpty())
    d.formatScale = SerialStudio::toDouble(params.value(key_format_scale));

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared number fields onto d (part 5 of 5); returns a non-empty error string when
 *        a value is rejected.
 */
static QString applyDatasetNumberFields5(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  Q_UNUSED(rebuildTree);

  const auto key_format_offset = takeDatasetField(params, consumed, {Keys::FormatOffset});
  if (!key_format_offset.isEmpty())
    d.formatOffset = SerialStudio::toDouble(params.value(key_format_offset));

  const auto key_source_id = takeDatasetField(
    params, consumed,
    {Keys::SourceId, Keys::DatasetSourceId});
  if (!key_source_id.isEmpty())
    d.sourceId = params.value(key_source_id).toInt();

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared flag fields onto d (part 1 of 3); returns a non-empty error string when a
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
 * @brief Applies declared flag fields onto d (part 2 of 3); returns a non-empty error string when a
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

  const auto key_extreme_hold = takeDatasetField(params, consumed, {Keys::ExtremeHold});
  if (!key_extreme_hold.isEmpty())
    d.extremeHold = params.value(key_extreme_hold).toBool();

  const auto key_suppress_alarms = takeDatasetField(params, consumed, {Keys::SuppressAlarms});
  if (!key_suppress_alarms.isEmpty()) {
    d.suppressAlarms = params.value(key_suppress_alarms).toBool();
    rebuildTree = true;
  }

  return QString();
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies declared flag fields onto d (part 3 of 3); returns a non-empty error string when a
 *        value is rejected.
 */
static QString applyDatasetFlagFields3(DataModel::Dataset& d,
                                       const QJsonObject& params,
                                       bool& rebuildTree,
                                       QSet<QString>& consumed)
{
  const auto key_alarms_while_visible = takeDatasetField(
    params, consumed,
    {Keys::AlarmsWhileVisible});
  if (!key_alarms_while_visible.isEmpty()) {
    d.alarmsWhileVisible = params.value(key_alarms_while_visible).toBool();
    rebuildTree = true;
  }

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

  const auto key_format_signed = takeDatasetField(params, consumed, {Keys::FormatSigned});
  if (!key_format_signed.isEmpty())
    d.formatSigned = params.value(key_format_signed).toBool();

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

  const auto key_transform_params = takeDatasetField(params, consumed, {Keys::TransformParams});
  if (!key_transform_params.isEmpty()) {
    QJsonObject nested;
    nested.insert(Keys::TransformParams, params.value(key_transform_params));
    readDatasetTransformParams(d, nested);
  }

  const auto key_labels = takeDatasetField(params, consumed, {Keys::Labels});
  if (!key_labels.isEmpty()) {
    QJsonObject nested;
    nested.insert(Keys::Labels, params.value(key_labels));
    readDatasetLabels(d, nested);
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

  if (auto err = applyDatasetNumberFields4(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetNumberFields5(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetFlagFields1(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetFlagFields2(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetFlagFields3(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if ((consumed.contains(Keys::Format) || consumed.contains(Keys::FormatWidth)
       || consumed.contains(Keys::FormatFractionBits))
      && !PropertyHooks::isValidFormatFraction(d))
    return QStringLiteral("Invalid formatFractionBits: must be 0 to formatWidth - 1");

  if ((consumed.contains(Keys::Format) || consumed.contains(Keys::FormatWidth)
       || consumed.contains(Keys::FormatBitOffset) || consumed.contains(Keys::FormatBitWidth))
      && !PropertyHooks::isValidFormatBitField(d))
    return QStringLiteral("Invalid bit field: formatBitOffset + formatBitWidth must not exceed "
                          "formatWidth");

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
 * @brief Declares dataset schema properties, part 1 of 5.
 */
static void datasetSchemaPart1(QJsonObject& props)
{
  props.insert(Keys::Title,
               datasetSchemaProperty("string",
                                     "The name shown on the dashboard and in exports.",
                                     QJsonArray()));
  props.insert(Keys::Virtual,
               datasetSchemaProperty("boolean",
                                     "Calculates the value with a transform instead of reading it "
                                     "from the incoming data, so it needs no frame index.",
                                     QJsonArray()));
  props.insert(Keys::HideOnDashboard,
               datasetSchemaProperty("boolean",
                                     "Hides this dataset's own widget on the dashboard. Canvas "
                                     "widgets can still read its value.",
                                     QJsonArray()));
  props.insert(Keys::Index,
               datasetSchemaProperty("integer",
                                     "Which value of each frame this dataset reads, counting from "
                                     "1. Use 0 to leave it unassigned.",
                                     QJsonArray()));
  props.insert(Keys::Units,
               datasetSchemaProperty("string",
                                     "The unit shown next to the value, such as V or mA. Optional.",
                                     QJsonArray()));
  props.insert(Keys::Alias,
               datasetSchemaProperty("string",
                                     "A short, unique name that scripts and the API can use to "
                                     "find this dataset. Optional.",
                                     QJsonArray()));
  props.insert(Keys::Color,
               datasetSchemaProperty("string",
                                     "The color of this dataset's plots and widgets. Automatic "
                                     "uses the theme's colors.",
                                     QJsonArray()));
  props.insert(QStringLiteral("pltMin"),
               datasetSchemaProperty("number",
                                     "The lowest value this dataset is expected to reach. Widgets "
                                     "and the FFT plot use it when their own range is empty.",
                                     QJsonArray()));
  props.insert(QStringLiteral("pltMax"),
               datasetSchemaProperty("number",
                                     "The highest value this dataset is expected to reach. "
                                     "Widgets and the FFT plot use it when their own range is "
                                     "empty.",
                                     QJsonArray()));
  props.insert(Keys::Graph,
               datasetSchemaProperty("boolean",
                                     "Plots the value over time.",
                                     QJsonArray()));
  props.insert(Keys::Log,
               datasetSchemaProperty("boolean",
                                     "Include this dataset in CSV/data logging",
                                     QJsonArray()));
  props.insert(QStringLiteral("xAxisId"),
               datasetSchemaProperty("integer",
                                     "What the plot's horizontal axis shows: time, or another "
                                     "dataset.",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 2 of 5.
 */
static void datasetSchemaPart2(QJsonObject& props)
{
  props.insert(Keys::PltLogX,
               datasetSchemaProperty("boolean",
                                     "Uses a logarithmic scale for the horizontal axis. Available "
                                     "when the axis shows samples or a dataset, not time.",
                                     QJsonArray()));
  props.insert(Keys::PltLogY,
               datasetSchemaProperty("boolean",
                                     "Uses a logarithmic scale for the vertical axis. Values at "
                                     "or below zero are drawn at the bottom.",
                                     QJsonArray()));
  props.insert(Keys::FFT,
               datasetSchemaProperty("boolean",
                                     "Shows how strong each frequency is in the signal.",
                                     QJsonArray()));
  props.insert(Keys::Waterfall,
               datasetSchemaProperty("boolean",
                                     "Shows a scrolling picture of the signal's frequencies over "
                                     "time (Pro).",
                                     QJsonArray()));
  props.insert(Keys::FFTBallistics,
               datasetSchemaProperty("boolean",
                                     "Peaks jump up at once and fall back slowly over the release "
                                     "time, like a hardware audio analyzer.",
                                     QJsonArray()));
  props.insert(Keys::FFTBallisticsRelease,
               datasetSchemaProperty("integer",
                                     "How long a peak takes to fall back, from 50 to 5000 ms.",
                                     QJsonArray()));
  props.insert(Keys::WaterfallYAxis,
               datasetSchemaProperty("integer",
                                     "What the waterfall's vertical axis shows: time (the "
                                     "default) or another dataset. Choosing an RPM dataset, for "
                                     "example, turns the waterfall into a Campbell diagram.",
                                     QJsonArray()));
  props.insert(Keys::FFTSamples,
               datasetSchemaProperty("integer",
                                     "How many samples each frequency analysis uses. More samples "
                                     "show finer frequency detail but react more slowly.",
                                     QJsonArray({8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
                                                 8192, 16384, 32768, 65536, 131072, 262144})));
  props.insert(Keys::FFTWindow,
               datasetSchemaProperty("integer",
                                     "Shapes each block of samples before the analysis so that "
                                     "nearby frequencies don't blur together. Used by the FFT "
                                     "plot and the waterfall.",
                                     QJsonArray({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                                 14})));
  props.insert(Keys::FFTSamplingRate,
               datasetSchemaProperty("integer",
                                     "How many samples per second the device sends for this "
                                     "dataset. The frequency axis is only correct when this "
                                     "matches.",
                                     QJsonArray()));
  props.insert(Keys::FFTLogX,
               datasetSchemaProperty("boolean",
                                     "Uses a logarithmic frequency axis so low frequencies stay "
                                     "readable. Used by the FFT plot and the waterfall.",
                                     QJsonArray()));
  props.insert(Keys::FFTMin,
               datasetSchemaProperty("number",
                                     "The lowest value expected in the signal, used to scale the "
                                     "analysis. Leave empty to use the dataset's Minimum Value.",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 3 of 5.
 */
static void datasetSchemaPart3(QJsonObject& props)
{
  props.insert(Keys::FFTMax,
               datasetSchemaProperty("number",
                                     "The highest value expected in the signal, used to scale the "
                                     "analysis. Leave empty to use the dataset's Maximum Value.",
                                     QJsonArray()));
  props.insert(Keys::Widget,
               datasetSchemaProperty("string",
                                     "The widget that shows this dataset on the dashboard.",
                                     QJsonArray({"", "bar", "compass", "gauge", "meter"})));
  props.insert(Keys::DisplayTickCount,
               datasetSchemaProperty("integer",
                                     "How many major marks the dial shows. Use 0 to fit them to "
                                     "the widget's size.",
                                     QJsonArray()));
  props.insert(Keys::DisplayFormat,
               datasetSchemaProperty("string",
                                     "How numbers appear on gauge and bar marks and captions. "
                                     "Hexadecimal and Binary also apply wherever the value is "
                                     "shown as text.",
                                     QJsonArray({"", "0d", "1d", "2d", "3d", "sci", "hex",
                                                 "bin"})));
  props.insert(Keys::DecimalPoints,
               datasetSchemaProperty("integer",
                                     "How many decimal places the value shows. Overrides Label "
                                     "Format. Use -1 for automatic.",
                                     QJsonArray()));
  props.insert(QStringLiteral("wgtMin"),
               datasetSchemaProperty("number",
                                     "The lowest value on the gauge or bar. Leave empty to use "
                                     "the dataset's Minimum Value.",
                                     QJsonArray()));
  props.insert(QStringLiteral("wgtMax"),
               datasetSchemaProperty("number",
                                     "The highest value on the gauge or bar. Leave empty to use "
                                     "the dataset's Maximum Value.",
                                     QJsonArray()));
  props.insert(Keys::ExtremeHold,
               datasetSchemaProperty("boolean",
                                     "Marks the lowest and highest values seen since the data was "
                                     "last cleared.",
                                     QJsonArray()));
  props.insert(Keys::SuppressAlarms,
               datasetSchemaProperty("boolean",
                                     "Keeps the alarm bands on the widget, but never sends "
                                     "notifications, raises alarms, or plays sounds for them.",
                                     QJsonArray()));
  props.insert(Keys::AlarmsWhileVisible,
               datasetSchemaProperty("boolean",
                                     "Sends notifications, raises alarms, and plays sounds for "
                                     "the bands only while a widget showing this dataset is on "
                                     "screen. Suppress Alarms still turns them off entirely.",
                                     QJsonArray()));
  props.insert(Keys::AlarmDeadband,
               datasetSchemaProperty("number",
                                     "How far the value must move back out of a band, in the "
                                     "dataset's units, before that band can alarm again. Stops a "
                                     "value that hovers at a limit from alarming over and over. "
                                     "Use 0 for none.",
                                     QJsonArray()));
  props.insert(Keys::LED,
               datasetSchemaProperty("boolean",
                                     "Shows this dataset as a light in the LED panel.",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 4 of 5.
 */
static void datasetSchemaPart4(QJsonObject& props)
{
  props.insert(Keys::LedHigh,
               datasetSchemaProperty("number",
                                     "The LED turns on when the value reaches this level. Use "
                                     "alarm bands for more than two states.",
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
                                     "Transform script language: -1 inherit, 0 JavaScript, 1 Lua, "
                                     "3 Expression",
                                     QJsonArray()));
  props.insert(Keys::Format,
               datasetSchemaProperty("integer",
                                     "Decodes the raw number before any transform: as fixed "
                                     "point, as a group of bits, or with a scale and offset.",
                                     QJsonArray({0, 1, 2, 3})));
  props.insert(Keys::FormatWidth,
               datasetSchemaProperty("integer",
                                     "How many bits the raw number has, from 1 to 64. Numbers "
                                     "wider than 53 bits lose precision.",
                                     QJsonArray()));
  props.insert(Keys::FormatSigned,
               datasetSchemaProperty("boolean",
                                     "Reads the raw number as signed (two's complement) at this "
                                     "width.",
                                     QJsonArray()));
  props.insert(Keys::FormatFractionBits,
               datasetSchemaProperty("integer",
                                     "The raw number is divided by 2 to this power. For example, "
                                     "the Q15 format uses 15.",
                                     QJsonArray()));
  props.insert(Keys::FormatBitOffset,
               datasetSchemaProperty("integer",
                                     "Where the field starts, counted in bits from the lowest bit "
                                     "(0).",
                                     QJsonArray()));
  props.insert(Keys::FormatBitWidth,
               datasetSchemaProperty("integer",
                                     "How many bits the field has. Use 1 for a single on/off flag.",
                                     QJsonArray()));
  props.insert(Keys::FormatScale,
               datasetSchemaProperty("number",
                                     "The raw number is multiplied by this value, then the offset "
                                     "is added.",
                                     QJsonArray()));
  props.insert(Keys::FormatOffset,
               datasetSchemaProperty("number",
                                     "Added to the value after it is multiplied by the scale.",
                                     QJsonArray()));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Declares dataset schema properties, part 5 of 5.
 */
static void datasetSchemaPart5(QJsonObject& props)
{
  props.insert(Keys::LabelsDefault,
               datasetSchemaProperty("string",
                                     "Text shown for a value no label row matches; empty shows "
                                     "the number",
                                     QJsonArray()));
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
  datasetSchemaPart5(props);
  return props;
}

// clang-format on
