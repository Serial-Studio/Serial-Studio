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

#include "Core/IconRegistry.h"
#include "Core/Services.h"
#include "DataModel/Generated/DatasetRegistry.h"
#include "DataModel/ProjectModel.h"
#include "ProjectEditor/ProjectEditor.h"

// clang-format off

using DataModel::PropertyHooks::RebuildHint;
namespace PropertyHooks = DataModel::PropertyHooks;
namespace Registry      = DataModel::Registry;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief Entry table backing the datasetWidgets choice domain.
 */
static const PropertyHooks::StaticOptionEntry kDatasetWidgetsEntries[] = {
  {"", "None"},
  {"bar", "Bar"},
  {"compass", "Compass"},
  {"gauge", "Gauge"},
  {"meter", "Meter"},
};

/**
 * @brief Entry table backing the displayFormats choice domain.
 */
static const PropertyHooks::StaticOptionEntry kDisplayFormatsEntries[] = {
  {"", "Auto"},
  {"0d", "Integer (0 decimals)"},
  {"1d", "1 decimal"},
  {"2d", "2 decimals"},
  {"3d", "3 decimals"},
  {"sci", "Scientific"},
  {"hex", "Hexadecimal"},
  {"bin", "Binary"},
};

/**
 * @brief Entry table backing the valueFormats choice domain.
 */
static const PropertyHooks::IntOptionEntry kValueFormatsEntries[] = {
  {0, "None"},
  {1, "Fixed point"},
  {2, "Bit field"},
  {3, "Scale"},
};

/**
 * @brief Entry table backing the plotOptions choice domain.
 */
static const PropertyHooks::TupleOptionEntry kPlotOptionsEntries[] = {
  {false, false, "No"},
  {true, false, "Yes"},
};

/**
 * @brief Entry table backing the fftSampleCounts choice domain.
 */
static const PropertyHooks::IntOptionEntry kFftSampleCountsEntries[] = {
  {8, "8"},
  {16, "16"},
  {32, "32"},
  {64, "64"},
  {128, "128"},
  {256, "256"},
  {512, "512"},
  {1024, "1024"},
  {2048, "2048"},
  {4096, "4096"},
  {8192, "8192"},
  {16384, "16384"},
  {32768, "32768"},
  {65536, "65536"},
  {131072, "131072"},
  {262144, "262144"},
};

/**
 * @brief Entry table backing the fftWindows choice domain.
 */
static const PropertyHooks::IntOptionEntry kFftWindowsEntries[] = {
  {0, "Rectangular (None)"},
  {1, "Bartlett (Triangular)"},
  {2, "Hann"},
  {3, "Hamming"},
  {4, "Blackman"},
  {5, "Blackman-Harris"},
  {6, "Nuttall"},
  {7, "Blackman-Nuttall"},
  {8, "Flat Top"},
  {9, "Welch"},
  {10, "Bartlett-Hann"},
  {11, "Bohman"},
  {12, "Cosine (Sine)"},
  {13, "Lanczos"},
  {14, "Parzen"},
};

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the datasetWidgets domain, whose declared rows the
 *        widgetExtensionOptions provider extends at runtime.
 */
const PropertyHooks::ExtensibleMapOptions& Registry::datasetWidgetsOptions()
{
  static const PropertyHooks::ExtensibleMapOptions source(&kDatasetWidgetsEntries[0], 5,
                                                          "ProjectEditor",
                                                          &PropertyHooks::widgetExtensionOptions);
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the displayFormats domain.
 */
const PropertyHooks::StaticMapOptions& Registry::displayFormatsOptions()
{
  static const PropertyHooks::StaticMapOptions source(&kDisplayFormatsEntries[0], 8,
                                                      "ProjectEditor");
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the valueFormats domain.
 */
const PropertyHooks::ParallelValueOptions& Registry::valueFormatsOptions()
{
  static const PropertyHooks::ParallelValueOptions source(&kValueFormatsEntries[0], 4,
                                                          "ProjectEditor", 0);
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the plotOptions domain.
 */
const PropertyHooks::TupleOptions& Registry::plotOptions()
{
  static const PropertyHooks::TupleOptions source(&kPlotOptionsEntries[0], 2, "ProjectEditor");
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the fftSampleCounts domain.
 */
const PropertyHooks::ParallelValueOptions& Registry::fftSampleCountsOptions()
{
  static const PropertyHooks::ParallelValueOptions source(&kFftSampleCountsEntries[0], 16, nullptr,
                                                          7);
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the fftWindows domain.
 */
const PropertyHooks::ParallelValueOptions& Registry::fftWindowsOptions()
{
  static const PropertyHooks::ParallelValueOptions source(&kFftWindowsEntries[0], 15,
                                                          "ProjectEditor", 5);
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the xAxisSources domain.
 */
const PropertyHooks::LiveProviderOptions& Registry::xAxisSourcesOptions()
{
  static const PropertyHooks::LiveProviderOptions source(
    &DataModel::ProjectModel::xDataSources,
    &DataModel::ProjectModel::xDataSourceUniqueIds,
    -2);
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the waterfallYSources domain.
 */
const PropertyHooks::LiveProviderOptions& Registry::waterfallYSourcesOptions()
{
  static const PropertyHooks::LiveProviderOptions source(
    &DataModel::ProjectModel::yWaterfallSources,
    &DataModel::ProjectModel::yWaterfallSourceUniqueIds,
    0);
  return source;
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the General Information section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addGeneralSection(CustomModel* model, const Dataset& dataset)
{
  auto& registry = Core::services().iconRegistry;
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("General Information"), PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("editor"),
                                QStringLiteral("dataset"), 16),
                  ParameterIcon);
  model->appendRow(header);

  auto* item_title = new QStandardItem();
  item_title->setEditable(true);
  item_title->setData(true, Active);
  item_title->setData(TextField, WidgetType);
  item_title->setData(dataset.title, EditableValue);
  item_title->setData(kDatasetView_Title, ParameterType);
  item_title->setData(tr("Untitled Dataset"), PlaceholderValue);
  item_title->setData(tr("Dataset Title"), ParameterName);
  item_title->setData(tr("The name shown on the dashboard and in exports."), ParameterDescription);
  model->appendRow(item_title);

  auto* item_virtual = new QStandardItem();
  item_virtual->setEditable(true);
  item_virtual->setData(true, Active);
  item_virtual->setData(CheckBox, WidgetType);
  item_virtual->setData(dataset.virtual_, EditableValue);
  item_virtual->setData(kDatasetView_Virtual, ParameterType);
  item_virtual->setData(tr("Computed Dataset"), ParameterName);
  item_virtual->setData(tr("Calculates the value with a transform instead of reading it from the "
                           "incoming data, so it needs no frame index."), ParameterDescription);
  model->appendRow(item_virtual);

  if (PropertyHooks::insidePainterGroup(dataset, m_projectModelRef)) {
    auto* item_hide_on_dashboard = new QStandardItem();
    item_hide_on_dashboard->setEditable(true);
    item_hide_on_dashboard->setData(true, Active);
    item_hide_on_dashboard->setData(CheckBox, WidgetType);
    item_hide_on_dashboard->setData(dataset.hideOnDashboard, EditableValue);
    item_hide_on_dashboard->setData(kDatasetView_HideOnDashboard, ParameterType);
    item_hide_on_dashboard->setData(tr("Hide on Dashboard"), ParameterName);
    item_hide_on_dashboard->setData(tr("Hides this dataset's own widget on the dashboard. Canvas "
                                       "widgets can still read its value."), ParameterDescription);
    model->appendRow(item_hide_on_dashboard);
  }

  const bool on_index = PropertyHooks::notVirtual(dataset, m_projectModelRef);
  auto* item_index = new QStandardItem();
  item_index->setEditable(on_index);
  item_index->setData(on_index, Active);
  item_index->setData(IntField, WidgetType);
  item_index->setData(dataset.index, EditableValue);
  item_index->setData(kDatasetView_Index, ParameterType);
  const auto ph_index = PropertyHooks::datasetIndexPlaceholder(dataset, m_projectModelRef);
  item_index->setData(ph_index, PlaceholderValue);
  item_index->setData(tr("Frame Index"), ParameterName);
  item_index->setData(tr("Which value of each frame this dataset reads, counting from 1. Use 0 to "
                         "leave it unassigned."), ParameterDescription);
  model->appendRow(item_index);

  auto* item_units = new QStandardItem();
  item_units->setEditable(true);
  item_units->setData(true, Active);
  item_units->setData(TextField, WidgetType);
  item_units->setData(dataset.units, EditableValue);
  item_units->setData(kDatasetView_Units, ParameterType);
  item_units->setData(tr("Volts, Amps, etc."), PlaceholderValue);
  item_units->setData(tr("Measurement Unit"), ParameterName);
  item_units->setData(tr("The unit shown next to the value, such as V or mA. Optional."),
                      ParameterDescription);
  model->appendRow(item_units);

  addDatasetAliasRow(model, dataset);

  addGeneralColorRow(model, dataset);

  addDatasetRangeRows(model, dataset);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the addDatasetAliasRow rows of the dataset form model.
 */
void DataModel::ProjectEditor::addDatasetAliasRow(CustomModel* model, const Dataset& dataset)
{
  auto* item_alias = new QStandardItem();
  item_alias->setEditable(true);
  item_alias->setData(true, Active);
  item_alias->setData(TextField, WidgetType);
  item_alias->setData(dataset.alias, EditableValue);
  item_alias->setData(kDatasetView_Alias, ParameterType);
  item_alias->setData(tr("Stable name"), PlaceholderValue);
  item_alias->setData(tr("Script Alias"), ParameterName);
  item_alias->setData(tr("A short, unique name that scripts and the API can use to find this "
                         "dataset. Optional."), ParameterDescription);
  model->appendRow(item_alias);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the addGeneralColorRow rows of the dataset form model.
 */
void DataModel::ProjectEditor::addGeneralColorRow(CustomModel* model, const Dataset& dataset)
{
  auto* item_color = new QStandardItem();
  item_color->setEditable(true);
  item_color->setData(true, Active);
  item_color->setData(ColorPicker, WidgetType);
  item_color->setData(dataset.color, EditableValue);
  item_color->setData(kDatasetView_Color, ParameterType);
  item_color->setData(tr("Automatic"), PlaceholderValue);
  item_color->setData(tr("Widget Color"), ParameterName);
  item_color->setData(tr("The color of this dataset's plots and widgets. Automatic uses the "
                         "theme's colors."), ParameterDescription);
  model->appendRow(item_color);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the addDatasetRangeRows rows of the dataset form model.
 */
void DataModel::ProjectEditor::addDatasetRangeRows(CustomModel* model, const Dataset& dataset)
{
  auto* item_plt_min = new QStandardItem();
  item_plt_min->setEditable(true);
  item_plt_min->setData(true, Active);
  item_plt_min->setData(FloatField, WidgetType);
  item_plt_min->setData(dataset.pltMin, EditableValue);
  item_plt_min->setData(kDatasetView_PltMin, ParameterType);
  item_plt_min->setData(0, PlaceholderValue);
  item_plt_min->setData(tr("Minimum Value"), ParameterName);
  item_plt_min->setData(tr("The lowest value this dataset is expected to reach. Widgets and the "
                           "FFT plot use it when their own range is empty."), ParameterDescription);
  model->appendRow(item_plt_min);

  auto* item_plt_max = new QStandardItem();
  item_plt_max->setEditable(true);
  item_plt_max->setData(true, Active);
  item_plt_max->setData(FloatField, WidgetType);
  item_plt_max->setData(dataset.pltMax, EditableValue);
  item_plt_max->setData(kDatasetView_PltMax, ParameterType);
  item_plt_max->setData(0, PlaceholderValue);
  item_plt_max->setData(tr("Maximum Value"), ParameterName);
  item_plt_max->setData(tr("The highest value this dataset is expected to reach. Widgets and the "
                           "FFT plot use it when their own range is empty."), ParameterDescription);
  model->appendRow(item_plt_max);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Value Format section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addFormatSection(CustomModel* model, const Dataset& dataset)
{
  auto& registry = Core::services().iconRegistry;
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("Value Format"), PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("editor"),
                                QStringLiteral("data-conversion"), 16),
                  ParameterIcon);
  model->appendRow(header);

  const auto val_format =
    Registry::valueFormatsOptions().indexForValue(m_projectModelRef, dataset.format);
  auto* item_format = new QStandardItem();
  item_format->setEditable(true);
  item_format->setData(true, Active);
  item_format->setData(ComboBox, WidgetType);
  item_format->setData(val_format, EditableValue);
  item_format->setData(kDatasetView_Format, ParameterType);
  item_format->setData(Registry::valueFormatsOptions().labels(m_projectModelRef), ComboBoxData);
  item_format->setData(tr("Format"), ParameterName);
  item_format->setData(tr("Decodes the raw number before any transform: as fixed point, as a "
                          "group of bits, or with a scale and offset."), ParameterDescription);
  model->appendRow(item_format);

  if (PropertyHooks::formatHasWidth(dataset, m_projectModelRef)) {
    auto* item_format_width = new QStandardItem();
    item_format_width->setEditable(true);
    item_format_width->setData(true, Active);
    item_format_width->setData(IntField, WidgetType);
    item_format_width->setData(dataset.formatWidth, EditableValue);
    item_format_width->setData(kDatasetView_FormatWidth, ParameterType);
    item_format_width->setData(16, PlaceholderValue);
    item_format_width->setData(1, MinValue);
    item_format_width->setData(64, MaxValue);
    item_format_width->setData(tr("Integer Width (bits)"), ParameterName);
    item_format_width->setData(tr("How many bits the raw number has, from 1 to 64. Numbers wider "
                                  "than 53 bits lose precision."), ParameterDescription);
    model->appendRow(item_format_width);
  }

  if (PropertyHooks::formatHasWidth(dataset, m_projectModelRef)) {
    auto* item_format_signed = new QStandardItem();
    item_format_signed->setEditable(true);
    item_format_signed->setData(true, Active);
    item_format_signed->setData(CheckBox, WidgetType);
    item_format_signed->setData(dataset.formatSigned, EditableValue);
    item_format_signed->setData(kDatasetView_FormatSigned, ParameterType);
    item_format_signed->setData(tr("Signed"), ParameterName);
    item_format_signed->setData(tr("Reads the raw number as signed (two's complement) at this "
                                   "width."), ParameterDescription);
    model->appendRow(item_format_signed);
  }

  if (PropertyHooks::formatIsFixed(dataset, m_projectModelRef)) {
    auto* item_format_fraction_bits = new QStandardItem();
    item_format_fraction_bits->setEditable(true);
    item_format_fraction_bits->setData(true, Active);
    item_format_fraction_bits->setData(IntField, WidgetType);
    item_format_fraction_bits->setData(dataset.formatFractionBits, EditableValue);
    item_format_fraction_bits->setData(kDatasetView_FormatFractionBits, ParameterType);
    item_format_fraction_bits->setData(0, PlaceholderValue);
    item_format_fraction_bits->setData(0, MinValue);
    item_format_fraction_bits->setData(63, MaxValue);
    item_format_fraction_bits->setData(tr("Fraction Bits"), ParameterName);
    item_format_fraction_bits->setData(tr("The raw number is divided by 2 to this power. For "
                                          "example, the Q15 format uses 15."),
                                       ParameterDescription);
    model->appendRow(item_format_fraction_bits);
  }

  buildFormatFieldRows(model, dataset);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the buildFormatFieldRows rows of the dataset form model.
 */
void DataModel::ProjectEditor::buildFormatFieldRows(CustomModel* model, const Dataset& dataset)
{
  if (PropertyHooks::formatIsBits(dataset, m_projectModelRef)) {
    auto* item_format_bit_offset = new QStandardItem();
    item_format_bit_offset->setEditable(true);
    item_format_bit_offset->setData(true, Active);
    item_format_bit_offset->setData(IntField, WidgetType);
    item_format_bit_offset->setData(dataset.formatBitOffset, EditableValue);
    item_format_bit_offset->setData(kDatasetView_FormatBitOffset, ParameterType);
    item_format_bit_offset->setData(0, PlaceholderValue);
    item_format_bit_offset->setData(0, MinValue);
    item_format_bit_offset->setData(63, MaxValue);
    item_format_bit_offset->setData(tr("Bit Offset"), ParameterName);
    item_format_bit_offset->setData(tr("Where the field starts, counted in bits from the lowest "
                                       "bit (0)."), ParameterDescription);
    model->appendRow(item_format_bit_offset);
  }

  if (PropertyHooks::formatIsBits(dataset, m_projectModelRef)) {
    auto* item_format_bit_width = new QStandardItem();
    item_format_bit_width->setEditable(true);
    item_format_bit_width->setData(true, Active);
    item_format_bit_width->setData(IntField, WidgetType);
    item_format_bit_width->setData(dataset.formatBitWidth, EditableValue);
    item_format_bit_width->setData(kDatasetView_FormatBitWidth, ParameterType);
    item_format_bit_width->setData(1, PlaceholderValue);
    item_format_bit_width->setData(1, MinValue);
    item_format_bit_width->setData(64, MaxValue);
    item_format_bit_width->setData(tr("Bit Count"), ParameterName);
    item_format_bit_width->setData(tr("How many bits the field has. Use 1 for a single on/off "
                                      "flag."), ParameterDescription);
    model->appendRow(item_format_bit_width);
  }

  if (PropertyHooks::formatIsScale(dataset, m_projectModelRef)) {
    auto* item_format_scale = new QStandardItem();
    item_format_scale->setEditable(true);
    item_format_scale->setData(true, Active);
    item_format_scale->setData(FloatField, WidgetType);
    item_format_scale->setData(dataset.formatScale, EditableValue);
    item_format_scale->setData(kDatasetView_FormatScale, ParameterType);
    item_format_scale->setData(1, PlaceholderValue);
    item_format_scale->setData(tr("Scale"), ParameterName);
    item_format_scale->setData(tr("The raw number is multiplied by this value, then the offset is "
                                  "added."), ParameterDescription);
    model->appendRow(item_format_scale);
  }

  if (PropertyHooks::formatIsScale(dataset, m_projectModelRef)) {
    auto* item_format_offset = new QStandardItem();
    item_format_offset->setEditable(true);
    item_format_offset->setData(true, Active);
    item_format_offset->setData(FloatField, WidgetType);
    item_format_offset->setData(dataset.formatOffset, EditableValue);
    item_format_offset->setData(kDatasetView_FormatOffset, ParameterType);
    item_format_offset->setData(0, PlaceholderValue);
    item_format_offset->setData(tr("Offset"), ParameterName);
    item_format_offset->setData(tr("Added to the value after it is multiplied by the scale."),
                                ParameterDescription);
    model->appendRow(item_format_offset);
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Plot Settings section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addPlotSection(CustomModel* model, const Dataset& dataset)
{
  auto& registry = Core::services().iconRegistry;
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("Plot Settings"), PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("widgets"),
                                QStringLiteral("plot"), 16),
                  ParameterIcon);
  model->appendRow(header);

  const auto val_plt = Registry::plotOptions().indexForPair(dataset.plt, dataset.log);
  auto* item_plt = new QStandardItem();
  item_plt->setEditable(true);
  item_plt->setData(true, Active);
  item_plt->setData(ComboBox, WidgetType);
  item_plt->setData(val_plt, EditableValue);
  item_plt->setData(kDatasetView_Plot, ParameterType);
  item_plt->setData(Registry::plotOptions().labels(m_projectModelRef), ComboBoxData);
  item_plt->setData(tr("Enable Plot Widget"), ParameterName);
  item_plt->setData(tr("Plots the value over time."), ParameterDescription);
  model->appendRow(item_plt);

  const bool on_x_axis_id = PropertyHooks::plotEnabled(dataset, m_projectModelRef);
  const auto val_x_axis_id =
    Registry::xAxisSourcesOptions().indexForValue(m_projectModelRef, dataset.xAxisId);
  auto* item_x_axis_id = new QStandardItem();
  item_x_axis_id->setEditable(on_x_axis_id);
  item_x_axis_id->setData(on_x_axis_id, Active);
  item_x_axis_id->setData(ComboBox, WidgetType);
  item_x_axis_id->setData(val_x_axis_id, EditableValue);
  item_x_axis_id->setData(kDatasetView_xAxis, ParameterType);
  item_x_axis_id->setData(Registry::xAxisSourcesOptions().labels(m_projectModelRef), ComboBoxData);
  item_x_axis_id->setData(tr("X-Axis Source"), ParameterName);
  item_x_axis_id->setData(tr("What the plot's horizontal axis shows: time, or another dataset."),
                          ParameterDescription);
  model->appendRow(item_x_axis_id);

  const bool on_plt_log_x = PropertyHooks::plotEnabledNonTimeX(dataset, m_projectModelRef);
  auto* item_plt_log_x = new QStandardItem();
  item_plt_log_x->setEditable(on_plt_log_x);
  item_plt_log_x->setData(on_plt_log_x, Active);
  item_plt_log_x->setData(CheckBox, WidgetType);
  item_plt_log_x->setData(dataset.pltLogX, EditableValue);
  item_plt_log_x->setData(kDatasetView_Plt_LogX, ParameterType);
  item_plt_log_x->setData(0, PlaceholderValue);
  item_plt_log_x->setData(tr("Logarithmic X Axis"), ParameterName);
  item_plt_log_x->setData(tr("Uses a logarithmic scale for the horizontal axis. Available when "
                             "the axis shows samples or a dataset, not time."),
                          ParameterDescription);
  model->appendRow(item_plt_log_x);

  const bool on_plt_log_y = PropertyHooks::plotEnabled(dataset, m_projectModelRef);
  auto* item_plt_log_y = new QStandardItem();
  item_plt_log_y->setEditable(on_plt_log_y);
  item_plt_log_y->setData(on_plt_log_y, Active);
  item_plt_log_y->setData(CheckBox, WidgetType);
  item_plt_log_y->setData(dataset.pltLogY, EditableValue);
  item_plt_log_y->setData(kDatasetView_Plt_LogY, ParameterType);
  item_plt_log_y->setData(0, PlaceholderValue);
  item_plt_log_y->setData(tr("Logarithmic Y Axis"), ParameterName);
  item_plt_log_y->setData(tr("Uses a logarithmic scale for the vertical axis. Values at or below "
                             "zero are drawn at the bottom."), ParameterDescription);
  model->appendRow(item_plt_log_y);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Frequency Analysis section rows to the dataset form model.
 */
void DataModel::ProjectEditor::buildFftGeneralRows(CustomModel* model, const Dataset& dataset)
{
  auto& registry = Core::services().iconRegistry;
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("Frequency Analysis"), PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("widgets"),
                                QStringLiteral("fft"), 16),
                  ParameterIcon);
  model->appendRow(header);

  auto* item_fft = new QStandardItem();
  item_fft->setEditable(true);
  item_fft->setData(true, Active);
  item_fft->setData(CheckBox, WidgetType);
  item_fft->setData(dataset.fft, EditableValue);
  item_fft->setData(kDatasetView_FFT, ParameterType);
  item_fft->setData(0, PlaceholderValue);
  item_fft->setData(tr("Enable FFT Analysis"), ParameterName);
  item_fft->setData(tr("Shows how strong each frequency is in the signal."), ParameterDescription);
  model->appendRow(item_fft);

  auto* item_waterfall = new QStandardItem();
  item_waterfall->setEditable(true);
  item_waterfall->setData(true, Active);
  item_waterfall->setData(CheckBox, WidgetType);
  item_waterfall->setData(dataset.waterfall, EditableValue);
  item_waterfall->setData(kDatasetView_Waterfall, ParameterType);
  item_waterfall->setData(0, PlaceholderValue);
  item_waterfall->setData(tr("Enable Waterfall Plot"), ParameterName);
  item_waterfall->setData(tr("Shows a scrolling picture of the signal's frequencies over time "
                             "(Pro)."), ParameterDescription);
  model->appendRow(item_waterfall);

  const bool on_fft_ballistics = PropertyHooks::fftEnabled(dataset, m_projectModelRef);
  auto* item_fft_ballistics = new QStandardItem();
  item_fft_ballistics->setEditable(on_fft_ballistics);
  item_fft_ballistics->setData(on_fft_ballistics, Active);
  item_fft_ballistics->setData(CheckBox, WidgetType);
  item_fft_ballistics->setData(dataset.fftBallistics, EditableValue);
  item_fft_ballistics->setData(kDatasetView_FFT_Ballistics, ParameterType);
  item_fft_ballistics->setData(0, PlaceholderValue);
  item_fft_ballistics->setData(tr("Peak Ballistics"), ParameterName);
  item_fft_ballistics->setData(tr("Peaks jump up at once and fall back slowly over the release "
                                  "time, like a hardware audio analyzer."), ParameterDescription);
  model->appendRow(item_fft_ballistics);

  const bool on_fft_ballistics_release = PropertyHooks::fftEnabled(dataset, m_projectModelRef);
  auto* item_fft_ballistics_release = new QStandardItem();
  item_fft_ballistics_release->setEditable(on_fft_ballistics_release);
  item_fft_ballistics_release->setData(on_fft_ballistics_release, Active);
  item_fft_ballistics_release->setData(IntField, WidgetType);
  item_fft_ballistics_release->setData(dataset.fftBallisticsRelease, EditableValue);
  item_fft_ballistics_release->setData(kDatasetView_FFT_BallisticsRelease, ParameterType);
  item_fft_ballistics_release->setData(300, PlaceholderValue);
  item_fft_ballistics_release->setData(tr("Ballistics Release (ms)"), ParameterName);
  item_fft_ballistics_release->setData(tr("How long a peak takes to fall back, from 50 to 5000 "
                                          "ms."), ParameterDescription);
  model->appendRow(item_fft_ballistics_release);

  if (PropertyHooks::waterfallEnabled(dataset, m_projectModelRef)) {
    const auto val_waterfall_y_axis =
      Registry::waterfallYSourcesOptions().indexForValue(m_projectModelRef, dataset.waterfallYAxis);
    auto* item_waterfall_y_axis = new QStandardItem();
    item_waterfall_y_axis->setEditable(true);
    item_waterfall_y_axis->setData(true, Active);
    item_waterfall_y_axis->setData(ComboBox, WidgetType);
    item_waterfall_y_axis->setData(val_waterfall_y_axis, EditableValue);
    item_waterfall_y_axis->setData(kDatasetView_WaterfallYAxis, ParameterType);
    item_waterfall_y_axis->setData(Registry::waterfallYSourcesOptions().labels(m_projectModelRef),
                                   ComboBoxData);
    item_waterfall_y_axis->setData(tr("Waterfall Y Axis"), ParameterName);
    item_waterfall_y_axis->setData(tr("What the waterfall's vertical axis shows: time (the "
                                      "default) or another dataset. Choosing an RPM dataset, for "
                                      "example, turns the waterfall into a Campbell diagram."),
                                   ParameterDescription);
    model->appendRow(item_waterfall_y_axis);
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the buildFftRangeRows rows of the dataset form model.
 */
void DataModel::ProjectEditor::buildFftRangeRows(CustomModel* model, const Dataset& dataset)
{
  const bool on_fft_samples = PropertyHooks::fftOrWaterfallEnabled(dataset, m_projectModelRef);
  const auto val_fft_samples =
    Registry::fftSampleCountsOptions().indexForValue(m_projectModelRef, dataset.fftSamples);
  auto* item_fft_samples = new QStandardItem();
  item_fft_samples->setEditable(on_fft_samples);
  item_fft_samples->setData(on_fft_samples, Active);
  item_fft_samples->setData(ComboBox, WidgetType);
  item_fft_samples->setData(val_fft_samples, EditableValue);
  item_fft_samples->setData(kDatasetView_FFT_Samples, ParameterType);
  item_fft_samples->setData(Registry::fftSampleCountsOptions().labels(m_projectModelRef),
                            ComboBoxData);
  item_fft_samples->setData(tr("FFT Window Size"), ParameterName);
  item_fft_samples->setData(tr("How many samples each frequency analysis uses. More samples show "
                               "finer frequency detail but react more slowly."),
                            ParameterDescription);
  model->appendRow(item_fft_samples);

  const bool on_fft_window = PropertyHooks::fftOrWaterfallEnabled(dataset, m_projectModelRef);
  const auto val_fft_window =
    Registry::fftWindowsOptions().indexForValue(m_projectModelRef, dataset.fftWindow);
  auto* item_fft_window = new QStandardItem();
  item_fft_window->setEditable(on_fft_window);
  item_fft_window->setData(on_fft_window, Active);
  item_fft_window->setData(ComboBox, WidgetType);
  item_fft_window->setData(val_fft_window, EditableValue);
  item_fft_window->setData(kDatasetView_FFT_Window, ParameterType);
  item_fft_window->setData(Registry::fftWindowsOptions().labels(m_projectModelRef), ComboBoxData);
  item_fft_window->setData(tr("FFT Window Function"), ParameterName);
  item_fft_window->setData(tr("Shapes each block of samples before the analysis so that nearby "
                              "frequencies don't blur together. Used by the FFT plot and the "
                              "waterfall."), ParameterDescription);
  model->appendRow(item_fft_window);

  const bool on_fft_sampling_rate =
    PropertyHooks::fftOrWaterfallEnabled(dataset, m_projectModelRef);
  auto* item_fft_sampling_rate = new QStandardItem();
  item_fft_sampling_rate->setEditable(on_fft_sampling_rate);
  item_fft_sampling_rate->setData(on_fft_sampling_rate, Active);
  item_fft_sampling_rate->setData(IntField, WidgetType);
  item_fft_sampling_rate->setData(dataset.fftSamplingRate, EditableValue);
  item_fft_sampling_rate->setData(kDatasetView_FFT_SamplingRate, ParameterType);
  item_fft_sampling_rate->setData(100, PlaceholderValue);
  item_fft_sampling_rate->setData(tr("FFT Sampling Rate (Hz, required)"), ParameterName);
  item_fft_sampling_rate->setData(tr("How many samples per second the device sends for this "
                                     "dataset. The frequency axis is only correct when this "
                                     "matches."), ParameterDescription);
  model->appendRow(item_fft_sampling_rate);

  const bool on_fft_log_x = PropertyHooks::fftOrWaterfallEnabled(dataset, m_projectModelRef);
  auto* item_fft_log_x = new QStandardItem();
  item_fft_log_x->setEditable(on_fft_log_x);
  item_fft_log_x->setData(on_fft_log_x, Active);
  item_fft_log_x->setData(CheckBox, WidgetType);
  item_fft_log_x->setData(dataset.fftLogX, EditableValue);
  item_fft_log_x->setData(kDatasetView_FFT_LogX, ParameterType);
  item_fft_log_x->setData(0, PlaceholderValue);
  item_fft_log_x->setData(tr("Logarithmic Frequency Axis"), ParameterName);
  item_fft_log_x->setData(tr("Uses a logarithmic frequency axis so low frequencies stay readable. "
                             "Used by the FFT plot and the waterfall."), ParameterDescription);
  model->appendRow(item_fft_log_x);

  const bool on_fft_min = PropertyHooks::fftOrWaterfallEnabled(dataset, m_projectModelRef);
  auto* item_fft_min = new QStandardItem();
  item_fft_min->setEditable(on_fft_min);
  item_fft_min->setData(on_fft_min, Active);
  item_fft_min->setData(FloatField, WidgetType);
  item_fft_min->setData(dataset.fftMin, EditableValue);
  item_fft_min->setData(kDatasetView_FFTMin, ParameterType);
  item_fft_min->setData(0, PlaceholderValue);
  item_fft_min->setData(tr("Minimum Value (optional)"), ParameterName);
  item_fft_min->setData(tr("The lowest value expected in the signal, used to scale the analysis. "
                           "Leave empty to use the dataset's Minimum Value."),
                        ParameterDescription);
  model->appendRow(item_fft_min);

  const bool on_fft_max = PropertyHooks::fftOrWaterfallEnabled(dataset, m_projectModelRef);
  auto* item_fft_max = new QStandardItem();
  item_fft_max->setEditable(on_fft_max);
  item_fft_max->setData(on_fft_max, Active);
  item_fft_max->setData(FloatField, WidgetType);
  item_fft_max->setData(dataset.fftMax, EditableValue);
  item_fft_max->setData(kDatasetView_FFTMax, ParameterType);
  item_fft_max->setData(0, PlaceholderValue);
  item_fft_max->setData(tr("Maximum Value (optional)"), ParameterName);
  item_fft_max->setData(tr("The highest value expected in the signal, used to scale the analysis. "
                           "Leave empty to use the dataset's Maximum Value."),
                        ParameterDescription);
  model->appendRow(item_fft_max);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Widget Settings section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addWidgetSection(CustomModel* model, const Dataset& dataset)
{
  auto& registry = Core::services().iconRegistry;
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("Widget Settings"), PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("editor"),
                                QStringLiteral("widget"), 16),
                  ParameterIcon);
  model->appendRow(header);

  const bool on_widget = PropertyHooks::widgetSelectable(dataset, m_projectModelRef);
  const auto val_widget =
    Registry::datasetWidgetsOptions().indexForValue(m_projectModelRef, dataset.widget);
  auto* item_widget = new QStandardItem();
  item_widget->setEditable(on_widget);
  item_widget->setData(on_widget, Active);
  item_widget->setData(ComboBox, WidgetType);
  item_widget->setData(val_widget, EditableValue);
  item_widget->setData(kDatasetView_Widget, ParameterType);
  item_widget->setData(Registry::datasetWidgetsOptions().labels(m_projectModelRef), ComboBoxData);
  item_widget->setData(tr("Widget"), ParameterName);
  item_widget->setData(tr("The widget that shows this dataset on the dashboard."),
                       ParameterDescription);
  model->appendRow(item_widget);

  buildWidgetFormatRows(model, dataset,
                        PropertyHooks::widgetRangeApplicable(dataset, m_projectModelRef));

  buildWidgetRangeRows(model, dataset,
                       PropertyHooks::widgetRangeApplicable(dataset, m_projectModelRef));
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the buildWidgetFormatRows rows of the dataset form model.
 */
void DataModel::ProjectEditor::buildWidgetFormatRows(CustomModel* model,
                                                     const Dataset& dataset,
                                                     bool rangeEnabled)
{
  auto* item_display_tick_count = new QStandardItem();
  item_display_tick_count->setEditable(rangeEnabled);
  item_display_tick_count->setData(rangeEnabled, Active);
  item_display_tick_count->setData(AutoIntField, WidgetType);
  item_display_tick_count->setData(dataset.displayTickCount, EditableValue);
  item_display_tick_count->setData(kDatasetView_DisplayTickCount, ParameterType);
  item_display_tick_count->setData(tr("Auto"), PlaceholderValue);
  item_display_tick_count->setData(tr("Tick Count"), ParameterName);
  item_display_tick_count->setData(tr("How many major marks the dial shows. Use 0 to fit them to "
                                      "the widget's size."), ParameterDescription);
  model->appendRow(item_display_tick_count);

  const auto val_display_format =
    Registry::displayFormatsOptions().indexForValue(m_projectModelRef, dataset.displayFormat);
  auto* item_display_format = new QStandardItem();
  item_display_format->setEditable(true);
  item_display_format->setData(true, Active);
  item_display_format->setData(ComboBox, WidgetType);
  item_display_format->setData(val_display_format, EditableValue);
  item_display_format->setData(kDatasetView_DisplayFormat, ParameterType);
  item_display_format->setData(Registry::displayFormatsOptions().labels(m_projectModelRef),
                               ComboBoxData);
  item_display_format->setData(tr("Label Format"), ParameterName);
  item_display_format->setData(tr("How numbers appear on gauge and bar marks and captions. "
                                  "Hexadecimal and Binary also apply wherever the value is shown "
                                  "as text."), ParameterDescription);
  model->appendRow(item_display_format);

  auto* item_decimal_points = new QStandardItem();
  item_decimal_points->setEditable(true);
  item_decimal_points->setData(true, Active);
  item_decimal_points->setData(AutoIntField, WidgetType);
  item_decimal_points->setData(dataset.decimalPoints, EditableValue);
  item_decimal_points->setData(kDatasetView_DecimalPoints, ParameterType);
  item_decimal_points->setData(tr("Auto"), PlaceholderValue);
  item_decimal_points->setData(-1, MinValue);
  item_decimal_points->setData(15, MaxValue);
  item_decimal_points->setData(tr("Decimal Points"), ParameterName);
  item_decimal_points->setData(tr("How many decimal places the value shows. Overrides Label "
                                  "Format. Use -1 for automatic."), ParameterDescription);
  model->appendRow(item_decimal_points);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the buildWidgetRangeRows rows of the dataset form model.
 */
void DataModel::ProjectEditor::buildWidgetRangeRows(CustomModel* model,
                                                    const Dataset& dataset,
                                                    bool rangeEnabled)
{
  auto* item_wgt_min = new QStandardItem();
  item_wgt_min->setEditable(rangeEnabled);
  item_wgt_min->setData(rangeEnabled, Active);
  item_wgt_min->setData(FloatField, WidgetType);
  item_wgt_min->setData(dataset.wgtMin, EditableValue);
  item_wgt_min->setData(kDatasetView_WgtMin, ParameterType);
  item_wgt_min->setData(0, PlaceholderValue);
  item_wgt_min->setData(tr("Minimum Value (optional)"), ParameterName);
  item_wgt_min->setData(tr("The lowest value on the gauge or bar. Leave empty to use the "
                           "dataset's Minimum Value."), ParameterDescription);
  model->appendRow(item_wgt_min);

  auto* item_wgt_max = new QStandardItem();
  item_wgt_max->setEditable(rangeEnabled);
  item_wgt_max->setData(rangeEnabled, Active);
  item_wgt_max->setData(FloatField, WidgetType);
  item_wgt_max->setData(dataset.wgtMax, EditableValue);
  item_wgt_max->setData(kDatasetView_WgtMax, ParameterType);
  item_wgt_max->setData(0, PlaceholderValue);
  item_wgt_max->setData(tr("Maximum Value (optional)"), ParameterName);
  item_wgt_max->setData(tr("The highest value on the gauge or bar. Leave empty to use the "
                           "dataset's Maximum Value."), ParameterDescription);
  model->appendRow(item_wgt_max);

  const bool on_extreme_hold = PropertyHooks::extremeHoldApplicable(dataset, m_projectModelRef);
  auto* item_extreme_hold = new QStandardItem();
  item_extreme_hold->setEditable(on_extreme_hold);
  item_extreme_hold->setData(on_extreme_hold, Active);
  item_extreme_hold->setData(CheckBox, WidgetType);
  item_extreme_hold->setData(dataset.extremeHold, EditableValue);
  item_extreme_hold->setData(kDatasetView_ExtremeHold, ParameterType);
  item_extreme_hold->setData(0, PlaceholderValue);
  item_extreme_hold->setData(tr("Hold Min/Max Markers"), ParameterName);
  item_extreme_hold->setData(tr("Marks the lowest and highest values seen since the data was last "
                                "cleared."), ParameterDescription);
  model->appendRow(item_extreme_hold);

  if (PropertyHooks::alarmBandsDrawn(dataset, m_projectModelRef)) {
    auto* item_suppress_alarms = new QStandardItem();
    item_suppress_alarms->setEditable(true);
    item_suppress_alarms->setData(true, Active);
    item_suppress_alarms->setData(CheckBox, WidgetType);
    item_suppress_alarms->setData(dataset.suppressAlarms, EditableValue);
    item_suppress_alarms->setData(kDatasetView_SuppressAlarms, ParameterType);
    item_suppress_alarms->setData(tr("Suppress Alarms"), ParameterName);
    item_suppress_alarms->setData(tr("Keeps the alarm bands on the widget, but never sends "
                                     "notifications, raises alarms, or plays sounds for them."),
                                  ParameterDescription);
    model->appendRow(item_suppress_alarms);
  }

  if (PropertyHooks::alarmBandsDrawn(dataset, m_projectModelRef)) {
    auto* item_alarms_while_visible = new QStandardItem();
    item_alarms_while_visible->setEditable(true);
    item_alarms_while_visible->setData(true, Active);
    item_alarms_while_visible->setData(CheckBox, WidgetType);
    item_alarms_while_visible->setData(dataset.alarmsWhileVisible, EditableValue);
    item_alarms_while_visible->setData(kDatasetView_AlarmsWhileVisible, ParameterType);
    item_alarms_while_visible->setData(tr("Alarms only while visible"), ParameterName);
    item_alarms_while_visible->setData(tr("Sends notifications, raises alarms, and plays sounds "
                                          "for the bands only while a widget showing this dataset "
                                          "is on screen. Suppress Alarms still turns them off "
                                          "entirely."), ParameterDescription);
    model->appendRow(item_alarms_while_visible);
  }

  if (PropertyHooks::alarmBandsDrawn(dataset, m_projectModelRef)) {
    auto* item_alarm_deadband = new QStandardItem();
    item_alarm_deadband->setEditable(true);
    item_alarm_deadband->setData(true, Active);
    item_alarm_deadband->setData(FloatField, WidgetType);
    item_alarm_deadband->setData(dataset.alarmDeadband, EditableValue);
    item_alarm_deadband->setData(kDatasetView_AlarmDeadband, ParameterType);
    item_alarm_deadband->setData(0, PlaceholderValue);
    item_alarm_deadband->setData(tr("Alarm Deadband"), ParameterName);
    item_alarm_deadband->setData(tr("How far the value must move back out of a band, in the "
                                    "dataset's units, before that band can alarm again. Stops a "
                                    "value that hovers at a limit from alarming over and over. "
                                    "Use 0 for none."), ParameterDescription);
    model->appendRow(item_alarm_deadband);
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the LED Display Settings section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addLEDSection(CustomModel* model, const Dataset& dataset)
{
  auto& registry = Core::services().iconRegistry;
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("LED Display Settings"), PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("editor"),
                                QStringLiteral("led"), 16),
                  ParameterIcon);
  model->appendRow(header);

  auto* item_led = new QStandardItem();
  item_led->setEditable(true);
  item_led->setData(true, Active);
  item_led->setData(CheckBox, WidgetType);
  item_led->setData(dataset.led, EditableValue);
  item_led->setData(kDatasetView_LED, ParameterType);
  item_led->setData(0, PlaceholderValue);
  item_led->setData(tr("Show in LED Panel"), ParameterName);
  item_led->setData(tr("Shows this dataset as a light in the LED panel."), ParameterDescription);
  model->appendRow(item_led);

  if (PropertyHooks::ledBandsAbsent(dataset, m_projectModelRef)) {
    const bool on_led_high = PropertyHooks::ledEnabled(dataset, m_projectModelRef);
    auto* item_led_high = new QStandardItem();
    item_led_high->setEditable(on_led_high);
    item_led_high->setData(on_led_high, Active);
    item_led_high->setData(FloatField, WidgetType);
    item_led_high->setData(dataset.ledHigh, EditableValue);
    item_led_high->setData(kDatasetView_LED_High, ParameterType);
    item_led_high->setData(0, PlaceholderValue);
    item_led_high->setData(tr("LED On Threshold (required)"), ParameterName);
    item_led_high->setData(tr("The LED turns on when the value reaches this level. Use alarm "
                              "bands for more than two states."), ParameterDescription);
    model->appendRow(item_led_high);
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a string-typed dataset form edit; returns false when the id is not a string row.
 */
static bool applyDatasetStringEdit(int formId, const QVariant& value, DataModel::Dataset& d)
{
  switch (formId) {
    case kDatasetView_Title:
      d.title = value.toString();
      return true;
    case kDatasetView_Units:
      d.units = value.toString();
      return true;
    case kDatasetView_Alias:
      d.alias = value.toString().simplified();
      return true;
    case kDatasetView_Color:
      d.color = value.toString().simplified();
      return true;
    case kDatasetView_TransformCode:
      d.transformCode = value.toString();
      return true;
    default:
      return false;
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a choice-typed dataset form edit; returns false when the id is not a choice row.
 */
static bool applyDatasetChoiceEdit(int formId,
                                   const QVariant& value,
                                   DataModel::Dataset& d,
                                   const DataModel::ProjectModel& pm)
{
  switch (formId) {
    case kDatasetView_Plot:
      d.plt = Registry::plotOptions().firstForIndex(value.toInt());
      d.log = Registry::plotOptions().secondForIndex(value.toInt());
      return true;
    case kDatasetView_xAxis:
      d.xAxisId = Registry::xAxisSourcesOptions().valueForIndex(pm, value.toInt()).toInt();
      return true;
    case kDatasetView_WaterfallYAxis:
      d.waterfallYAxis =
        Registry::waterfallYSourcesOptions().valueForIndex(pm, value.toInt()).toInt();
      return true;
    case kDatasetView_FFT_Samples:
      d.fftSamples = Registry::fftSampleCountsOptions().valueForIndex(pm, value.toInt()).toInt();
      return true;
    case kDatasetView_FFT_Window:
      d.fftWindow = Registry::fftWindowsOptions().valueForIndex(pm, value.toInt()).toInt();
      return true;
    case kDatasetView_Widget:
      d.widget = Registry::datasetWidgetsOptions().valueForIndex(pm, value.toInt()).toString();
      return true;
    case kDatasetView_DisplayFormat:
      d.displayFormat =
        Registry::displayFormatsOptions().valueForIndex(pm, value.toInt()).toString();
      return true;
    case kDatasetView_Format:
      d.format = Registry::valueFormatsOptions().valueForIndex(pm, value.toInt()).toInt();
      return true;
    default:
      return false;
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a number-typed dataset form edit; returns false when the id is not a number row.
 */
static bool applyDatasetNumberEdit(int formId, const QVariant& value, DataModel::Dataset& d)
{
  switch (formId) {
    case kDatasetView_Index:
      d.index = value.toInt();
      return true;
    case kDatasetView_PltMin:
      d.pltMin = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_PltMax:
      d.pltMax = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_FFT_BallisticsRelease:
      d.fftBallisticsRelease = qBound(50, value.toInt(), 5000);
      return true;
    case kDatasetView_FFT_SamplingRate:
      d.fftSamplingRate = value.toInt();
      return true;
    case kDatasetView_FFTMin:
      d.fftMin = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_FFTMax:
      d.fftMax = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_DisplayTickCount:
      d.displayTickCount = qMax(0, value.toInt());
      return true;
    case kDatasetView_DecimalPoints:
      d.decimalPoints = qMax(-1, value.toInt());
      return true;
    case kDatasetView_WgtMin:
      d.wgtMin = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_WgtMax:
      d.wgtMax = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_AlarmDeadband:
      d.alarmDeadband = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_LED_High:
      d.ledHigh = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_FormatWidth:
      d.formatWidth = qBound(1, value.toInt(), 64);
      return true;
    case kDatasetView_FormatFractionBits:
      d.formatFractionBits = qBound(0, value.toInt(), 63);
      return true;
    case kDatasetView_FormatBitOffset:
      d.formatBitOffset = qBound(0, value.toInt(), 63);
      return true;
    case kDatasetView_FormatBitWidth:
      d.formatBitWidth = qBound(1, value.toInt(), 64);
      return true;
    case kDatasetView_FormatScale:
      d.formatScale = SerialStudio::toDouble(value);
      return true;
    case kDatasetView_FormatOffset:
      d.formatOffset = SerialStudio::toDouble(value);
      return true;
    default:
      return false;
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a flag-typed dataset form edit; returns false when the id is not a flag row.
 */
static bool applyDatasetFlagEdit(int formId, const QVariant& value, DataModel::Dataset& d)
{
  switch (formId) {
    case kDatasetView_Virtual:
      d.virtual_ = value.toBool();
      return true;
    case kDatasetView_HideOnDashboard:
      d.hideOnDashboard = value.toBool();
      return true;
    case kDatasetView_Plt_LogX:
      d.pltLogX = value.toBool();
      return true;
    case kDatasetView_Plt_LogY:
      d.pltLogY = value.toBool();
      return true;
    case kDatasetView_FFT:
      d.fft = value.toBool();
      return true;
    case kDatasetView_Waterfall:
      d.waterfall = value.toBool();
      return true;
    case kDatasetView_FFT_Ballistics:
      d.fftBallistics = value.toBool();
      return true;
    case kDatasetView_FFT_LogX:
      d.fftLogX = value.toBool();
      return true;
    case kDatasetView_ExtremeHold:
      d.extremeHold = value.toBool();
      return true;
    case kDatasetView_SuppressAlarms:
      d.suppressAlarms = value.toBool();
      return true;
    case kDatasetView_AlarmsWhileVisible:
      d.alarmsWhileVisible = value.toBool();
      return true;
    case kDatasetView_LED:
      d.led = value.toBool();
      return true;
    case kDatasetView_FormatSigned:
      d.formatSigned = value.toBool();
      return true;
    default:
      return false;
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the declared commit-side-effect hook for a form id, if it declares one.
 */
static RebuildHint datasetCommitHook(int formId, DataModel::Dataset& d)
{
  switch (formId) {
    case kDatasetView_Title:
      return PropertyHooks::onTitleChanged(d);
    case kDatasetView_Virtual:
      return PropertyHooks::onVirtualChanged(d);
    case kDatasetView_Plot:
      return PropertyHooks::onReshape(d);
    case kDatasetView_xAxis:
      return PropertyHooks::onXAxisChanged(d);
    case kDatasetView_FFT:
      return PropertyHooks::onReshape(d);
    case kDatasetView_Waterfall:
      return PropertyHooks::onReshape(d);
    case kDatasetView_Widget:
      return PropertyHooks::onWidgetChanged(d);
    case kDatasetView_ExtremeHold:
      return PropertyHooks::onReshape(d);
    case kDatasetView_LED:
      return PropertyHooks::onReshape(d);
    case kDatasetView_Format:
      return PropertyHooks::onReshape(d);
    default:
      return RebuildHint::None;
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies one dataset form edit onto d and reports the rebuild the caller must run.
 */
RebuildHint Registry::applyDatasetFormEdit(int formId,
                                          const QVariant& value,
                                          DataModel::Dataset& d,
                                          const DataModel::ProjectModel& pm)
{
  const bool handled = applyDatasetStringEdit(formId, value, d)
                       || applyDatasetChoiceEdit(formId, value, d, pm)
                       || applyDatasetNumberEdit(formId, value, d)
                       || applyDatasetFlagEdit(formId, value, d);
  if (!handled)
    return RebuildHint::None;

  return datasetCommitHook(formId, d);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports whether a dataset form row is built at all, mirroring the row emitters'
 *        visibleWhen guard.
 */
static bool datasetRowVisible(int formId,
                              const DataModel::Dataset& d,
                              const DataModel::ProjectModel& pm)
{
  switch (formId) {
    case kDatasetView_HideOnDashboard:
      return PropertyHooks::insidePainterGroup(d, pm);
    case kDatasetView_FormatWidth:
      return PropertyHooks::formatHasWidth(d, pm);
    case kDatasetView_FormatSigned:
      return PropertyHooks::formatHasWidth(d, pm);
    case kDatasetView_FormatFractionBits:
      return PropertyHooks::formatIsFixed(d, pm);
    case kDatasetView_FormatBitOffset:
      return PropertyHooks::formatIsBits(d, pm);
    case kDatasetView_FormatBitWidth:
      return PropertyHooks::formatIsBits(d, pm);
    case kDatasetView_FormatScale:
      return PropertyHooks::formatIsScale(d, pm);
    case kDatasetView_FormatOffset:
      return PropertyHooks::formatIsScale(d, pm);
    case kDatasetView_WaterfallYAxis:
      return PropertyHooks::waterfallEnabled(d, pm);
    case kDatasetView_SuppressAlarms:
      return PropertyHooks::alarmBandsDrawn(d, pm);
    case kDatasetView_AlarmsWhileVisible:
      return PropertyHooks::alarmBandsDrawn(d, pm);
    case kDatasetView_AlarmDeadband:
      return PropertyHooks::alarmBandsDrawn(d, pm);
    case kDatasetView_LED_High:
      return PropertyHooks::ledBandsAbsent(d, pm);
    default:
      return true;
  }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the value a dataset form row would carry, or an invalid QVariant when the row is
 *        not built for this dataset.
 */
QVariant Registry::datasetFormValue(int formId,
                                    const DataModel::Dataset& d,
                                    const DataModel::ProjectModel& pm)
{
  if (!datasetRowVisible(formId, d, pm))
    return {};

  switch (formId) {
    case kDatasetView_Title:
      return d.title;
    case kDatasetView_Virtual:
      return d.virtual_;
    case kDatasetView_HideOnDashboard:
      return d.hideOnDashboard;
    case kDatasetView_Index:
      return d.index;
    case kDatasetView_Units:
      return d.units;
    case kDatasetView_Alias:
      return d.alias;
    case kDatasetView_Color:
      return d.color;
    case kDatasetView_PltMin:
      return d.pltMin;
    case kDatasetView_PltMax:
      return d.pltMax;
    case kDatasetView_Format:
      return Registry::valueFormatsOptions().indexForValue(pm, d.format);
    case kDatasetView_FormatWidth:
      return d.formatWidth;
    case kDatasetView_FormatSigned:
      return d.formatSigned;
    case kDatasetView_FormatFractionBits:
      return d.formatFractionBits;
    case kDatasetView_FormatBitOffset:
      return d.formatBitOffset;
    case kDatasetView_FormatBitWidth:
      return d.formatBitWidth;
    case kDatasetView_FormatScale:
      return d.formatScale;
    case kDatasetView_FormatOffset:
      return d.formatOffset;
    case kDatasetView_Plot:
      return Registry::plotOptions().indexForPair(d.plt, d.log);
    case kDatasetView_xAxis:
      return Registry::xAxisSourcesOptions().indexForValue(pm, d.xAxisId);
    case kDatasetView_Plt_LogX:
      return d.pltLogX;
    case kDatasetView_Plt_LogY:
      return d.pltLogY;
    case kDatasetView_FFT:
      return d.fft;
    case kDatasetView_Waterfall:
      return d.waterfall;
    case kDatasetView_FFT_Ballistics:
      return d.fftBallistics;
    case kDatasetView_FFT_BallisticsRelease:
      return d.fftBallisticsRelease;
    case kDatasetView_WaterfallYAxis:
      return Registry::waterfallYSourcesOptions().indexForValue(pm, d.waterfallYAxis);
    case kDatasetView_FFT_Samples:
      return Registry::fftSampleCountsOptions().indexForValue(pm, d.fftSamples);
    case kDatasetView_FFT_Window:
      return Registry::fftWindowsOptions().indexForValue(pm, d.fftWindow);
    case kDatasetView_FFT_SamplingRate:
      return d.fftSamplingRate;
    case kDatasetView_FFT_LogX:
      return d.fftLogX;
    case kDatasetView_FFTMin:
      return d.fftMin;
    case kDatasetView_FFTMax:
      return d.fftMax;
    case kDatasetView_Widget:
      return Registry::datasetWidgetsOptions().indexForValue(pm, d.widget);
    case kDatasetView_DisplayTickCount:
      return d.displayTickCount;
    case kDatasetView_DisplayFormat:
      return Registry::displayFormatsOptions().indexForValue(pm, d.displayFormat);
    case kDatasetView_DecimalPoints:
      return d.decimalPoints;
    case kDatasetView_WgtMin:
      return d.wgtMin;
    case kDatasetView_WgtMax:
      return d.wgtMax;
    case kDatasetView_ExtremeHold:
      return d.extremeHold;
    case kDatasetView_SuppressAlarms:
      return d.suppressAlarms;
    case kDatasetView_AlarmsWhileVisible:
      return d.alarmsWhileVisible;
    case kDatasetView_AlarmDeadband:
      return d.alarmDeadband;
    case kDatasetView_LED:
      return d.led;
    case kDatasetView_LED_High:
      return d.ledHigh;
    default:
      return {};
  }
}

// clang-format on
