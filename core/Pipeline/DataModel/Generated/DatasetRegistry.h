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

#pragma once

#include <QLatin1StringView>
#include <QVariant>

#include "DataModel/Frame.h"
#include "DataModel/Project/PropertyHooks.h"

// clang-format off

/**
 * @brief Form-field identifiers for the dataset view.
 */
typedef enum {
  kDatasetView_Title,
  kDatasetView_Index,
  kDatasetView_Units,
  kDatasetView_Widget,
  kDatasetView_FFT,
  kDatasetView_Waterfall,
  kDatasetView_WaterfallYAxis,
  kDatasetView_LED,
  kDatasetView_LED_High,
  kDatasetView_Plot,
  kDatasetView_FFTMin,
  kDatasetView_FFTMax,
  kDatasetView_PltMin,
  kDatasetView_PltMax,
  kDatasetView_WgtMin,
  kDatasetView_WgtMax,
  kDatasetView_FFT_Samples,
  kDatasetView_FFT_SamplingRate,
  kDatasetView_FFT_Window,
  kDatasetView_xAxis,
  kDatasetView_Overview,
  kDatasetView_HideOnDashboard,
  kDatasetView_TransformCode,
  kDatasetView_Virtual,
  kDatasetView_DisplayTickCount,
  kDatasetView_DisplayFormat,
  kDatasetView_DecimalPoints,
  kDatasetView_Color,
  kDatasetView_Alias,
  kDatasetView_Plt_LogX,
  kDatasetView_Plt_LogY,
  kDatasetView_FFT_LogX,
  kDatasetView_FFT_Ballistics,
  kDatasetView_FFT_BallisticsRelease,
  kDatasetView_ExtremeHold,
} DatasetItem;

namespace DataModel::Registry {

/**
 * @brief Value type of a declared dataset property.
 */
enum class PropertyType : quint8 { Int, Double, Bool, String };

/**
 * @brief Editor row kind of a declared property; mirrors ProjectEditor::EditorWidget.
 */
enum class PropertyWidget : quint8 {
  None, TextField, IntField, FloatField, AutoIntField, CheckBox, ComboBox, ColorPicker
};

/**
 * @brief Project-file write rule of a declared property.
 */
enum class PersistRule : quint8 {
  Always, Never, WhenTrue, WhenFalse, WhenNonEmpty, WhenNonZero, WhenPositive,
  WhenNonNegative, WhenNonDefault, WithProperty
};

/**
 * @brief One declared dataset property, joined across every derived surface.
 */
struct DatasetProperty {
  const char* id;
  const char* field;
  const char* apiName;
  const char* undoLabel;
  const char* coalesceKey;
  int formId;
  PropertyType type;
  PropertyWidget widget;
  PersistRule persist;
  bool hasFormRow;
  bool coalesce;
  bool rebuildTree;
  bool pro;
};

/**
 * @brief Every declared dataset property, in manifest order.
 */
inline constexpr DatasetProperty kDatasetProperties[] = {
  {"Title", "title", "title", "Rename Dataset", "dataset-title", kDatasetView_Title,
   PropertyType::String, PropertyWidget::TextField, PersistRule::Always, true, true, false,
   false},
  {"Virtual", "virtual_", "virtual", "Edit Dataset", "dataset", kDatasetView_Virtual,
   PropertyType::Bool, PropertyWidget::CheckBox, PersistRule::WhenTrue, true, false, false,
   false},
  {"HideOnDashboard", "hideOnDashboard", "hideOnDashboard", "Edit Dataset", "dataset",
   kDatasetView_HideOnDashboard, PropertyType::Bool, PropertyWidget::CheckBox,
   PersistRule::WhenTrue, true, false, false, false},
  {"Index", "index", "index", "Edit Dataset", "dataset", kDatasetView_Index, PropertyType::Int,
   PropertyWidget::IntField, PersistRule::Always, true, true, true, false},
  {"Units", "units", "units", "Edit Dataset", "dataset", kDatasetView_Units, PropertyType::String,
   PropertyWidget::TextField, PersistRule::Always, true, true, false, false},
  {"Alias", "alias", "alias", "Edit Dataset", "dataset", kDatasetView_Alias, PropertyType::String,
   PropertyWidget::TextField, PersistRule::WhenNonEmpty, true, true, false, false},
  {"Color", "color", "color", "Edit Dataset", "dataset", kDatasetView_Color, PropertyType::String,
   PropertyWidget::ColorPicker, PersistRule::WhenNonEmpty, true, true, false, false},
  {"PltMin", "pltMin", "pltMin", "Edit Dataset", "dataset", kDatasetView_PltMin,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"PltMax", "pltMax", "pltMax", "Edit Dataset", "dataset", kDatasetView_PltMax,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"Plt", "plt", "graph", "Edit Dataset", "dataset", kDatasetView_Plot, PropertyType::Bool,
   PropertyWidget::ComboBox, PersistRule::Always, true, false, false, false},
  {"Log", "log", "log", "Edit Dataset", "dataset", -1, PropertyType::Bool, PropertyWidget::None,
   PersistRule::Always, false, false, false, false},
  {"XAxis", "xAxisId", "xAxisId", "Edit Dataset", "dataset", kDatasetView_xAxis,
   PropertyType::Int, PropertyWidget::ComboBox, PersistRule::Always, true, false, false, false},
  {"PltLogX", "pltLogX", "plotLogX", "Edit Dataset", "dataset", kDatasetView_Plt_LogX,
   PropertyType::Bool, PropertyWidget::CheckBox, PersistRule::WhenTrue, true, false, false,
   false},
  {"PltLogY", "pltLogY", "plotLogY", "Edit Dataset", "dataset", kDatasetView_Plt_LogY,
   PropertyType::Bool, PropertyWidget::CheckBox, PersistRule::WhenTrue, true, false, false,
   false},
  {"Fft", "fft", "fft", "Edit Dataset", "dataset", kDatasetView_FFT, PropertyType::Bool,
   PropertyWidget::CheckBox, PersistRule::Always, true, false, false, false},
  {"Waterfall", "waterfall", "waterfall", "Edit Dataset", "dataset", kDatasetView_Waterfall,
   PropertyType::Bool, PropertyWidget::CheckBox, PersistRule::WhenTrue, true, false, false, true},
  {"FftBallistics", "fftBallistics", "fftBallistics", "Edit Dataset", "dataset",
   kDatasetView_FFT_Ballistics, PropertyType::Bool, PropertyWidget::CheckBox,
   PersistRule::WhenTrue, true, false, false, false},
  {"FftBallisticsRelease", "fftBallisticsRelease", "fftBallisticsRelease", "Edit Dataset",
   "dataset", kDatasetView_FFT_BallisticsRelease, PropertyType::Int, PropertyWidget::IntField,
   PersistRule::WithProperty, true, true, false, false},
  {"WaterfallYAxis", "waterfallYAxis", "waterfallYAxis", "Edit Dataset", "dataset",
   kDatasetView_WaterfallYAxis, PropertyType::Int, PropertyWidget::ComboBox,
   PersistRule::WhenNonZero, true, false, false, true},
  {"FftSamples", "fftSamples", "fftSamples", "Edit Dataset", "dataset", kDatasetView_FFT_Samples,
   PropertyType::Int, PropertyWidget::ComboBox, PersistRule::Always, true, false, false, false},
  {"FftWindow", "fftWindow", "fftWindow", "Edit Dataset", "dataset", kDatasetView_FFT_Window,
   PropertyType::Int, PropertyWidget::ComboBox, PersistRule::Always, true, false, false, false},
  {"FftSamplingRate", "fftSamplingRate", "fftSamplingRate", "Edit Dataset", "dataset",
   kDatasetView_FFT_SamplingRate, PropertyType::Int, PropertyWidget::IntField,
   PersistRule::Always, true, true, false, false},
  {"FftLogX", "fftLogX", "fftLogX", "Edit Dataset", "dataset", kDatasetView_FFT_LogX,
   PropertyType::Bool, PropertyWidget::CheckBox, PersistRule::WhenTrue, true, false, false,
   false},
  {"FftMin", "fftMin", "fftMin", "Edit Dataset", "dataset", kDatasetView_FFTMin,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"FftMax", "fftMax", "fftMax", "Edit Dataset", "dataset", kDatasetView_FFTMax,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"Widget", "widget", "widget", "Edit Dataset", "dataset", kDatasetView_Widget,
   PropertyType::String, PropertyWidget::ComboBox, PersistRule::Always, true, false, false,
   false},
  {"DisplayTickCount", "displayTickCount", "displayTickCount", "Edit Dataset", "dataset",
   kDatasetView_DisplayTickCount, PropertyType::Int, PropertyWidget::AutoIntField,
   PersistRule::WhenPositive, true, true, false, false},
  {"DisplayFormat", "displayFormat", "displayFormat", "Edit Dataset", "dataset",
   kDatasetView_DisplayFormat, PropertyType::String, PropertyWidget::ComboBox,
   PersistRule::WhenNonEmpty, true, false, false, false},
  {"DecimalPoints", "decimalPoints", "decimalPoints", "Edit Dataset", "dataset",
   kDatasetView_DecimalPoints, PropertyType::Int, PropertyWidget::AutoIntField,
   PersistRule::WhenNonNegative, true, true, false, false},
  {"WgtMin", "wgtMin", "wgtMin", "Edit Dataset", "dataset", kDatasetView_WgtMin,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"WgtMax", "wgtMax", "wgtMax", "Edit Dataset", "dataset", kDatasetView_WgtMax,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"ExtremeHold", "extremeHold", "extremeHold", "Edit Dataset", "dataset",
   kDatasetView_ExtremeHold, PropertyType::Bool, PropertyWidget::CheckBox, PersistRule::WhenTrue,
   true, false, false, false},
  {"Led", "led", "led", "Edit Dataset", "dataset", kDatasetView_LED, PropertyType::Bool,
   PropertyWidget::CheckBox, PersistRule::Always, true, false, false, false},
  {"LedHigh", "ledHigh", "ledHigh", "Edit Dataset", "dataset", kDatasetView_LED_High,
   PropertyType::Double, PropertyWidget::FloatField, PersistRule::Always, true, true, false,
   false},
  {"OverviewDisplay", "overviewDisplay", "overviewDisplay", "Edit Dataset", "dataset",
   kDatasetView_Overview, PropertyType::Bool, PropertyWidget::None, PersistRule::WhenTrue, false,
   false, false, false},
  {"TransformCode", "transformCode", "transformCode", "Edit Dataset", "dataset",
   kDatasetView_TransformCode, PropertyType::String, PropertyWidget::None,
   PersistRule::WhenNonEmpty, true, true, false, false},
  {"TransformLanguage", "transformLanguage", "transformLanguage", "Edit Dataset", "dataset", -1,
   PropertyType::Int, PropertyWidget::None, PersistRule::WithProperty, false, false, false,
   false},
  {"GroupId", "groupId", "", "", "dataset", -1, PropertyType::Int, PropertyWidget::None,
   PersistRule::Always, false, false, false, false},
  {"DatasetId", "datasetId", "", "", "dataset", -1, PropertyType::Int, PropertyWidget::None,
   PersistRule::Always, false, false, false, false},
  {"UniqueId", "uniqueId", "", "", "dataset", -1, PropertyType::Int, PropertyWidget::None,
   PersistRule::WhenNonNegative, false, false, false, false},
  {"SourceId", "sourceId", "sourceId", "", "dataset", -1, PropertyType::Int, PropertyWidget::None,
   PersistRule::Never, false, false, false, false},
  {"Enabled", "enabled", "", "", "dataset", -1, PropertyType::Bool, PropertyWidget::None,
   PersistRule::WhenFalse, false, false, false, false},
};

/**
 * @brief Number of declared dataset properties.
 */
inline constexpr int kDatasetPropertyCount = 42;

/**
 * @brief Returns the property bound to a form-field id, or null when none is.
 */
[[nodiscard]] inline const DatasetProperty* datasetPropertyForFormId(int formId)
{
  for (int i = 0; i < kDatasetPropertyCount; ++i)
    if (kDatasetProperties[i].hasFormRow && kDatasetProperties[i].formId == formId)
      return &kDatasetProperties[i];

  return nullptr;
}

/**
 * @brief Returns the property with a registry id, or null when undeclared.
 */
[[nodiscard]] inline const DatasetProperty* datasetPropertyById(QLatin1StringView id)
{
  for (int i = 0; i < kDatasetPropertyCount; ++i)
    if (id == QLatin1StringView(kDatasetProperties[i].id))
      return &kDatasetProperties[i];

  return nullptr;
}

//--------------------------------------------------------------------------------------------------
// Choice domains (defined in DatasetForm.cpp)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the shared option source for the datasetWidgets domain.
 */
[[nodiscard]] const PropertyHooks::ExtensibleMapOptions& datasetWidgetsOptions();

/**
 * @brief Returns the shared option source for the displayFormats domain.
 */
[[nodiscard]] const PropertyHooks::StaticMapOptions& displayFormatsOptions();

/**
 * @brief Returns the shared option source for the plotOptions domain.
 */
[[nodiscard]] const PropertyHooks::TupleOptions& plotOptions();

/**
 * @brief Returns the shared option source for the fftSampleCounts domain.
 */
[[nodiscard]] const PropertyHooks::ParallelValueOptions& fftSampleCountsOptions();

/**
 * @brief Returns the shared option source for the fftWindows domain.
 */
[[nodiscard]] const PropertyHooks::ParallelValueOptions& fftWindowsOptions();

/**
 * @brief Returns the shared option source for the xAxisSources domain.
 */
[[nodiscard]] const PropertyHooks::LiveProviderOptions& xAxisSourcesOptions();

/**
 * @brief Returns the shared option source for the waterfallYSources domain.
 */
[[nodiscard]] const PropertyHooks::LiveProviderOptions& waterfallYSourcesOptions();

/**
 * @brief Applies one dataset form edit onto d and reports the rebuild the caller must run; never
 *        touches ProjectModel and never rebuilds a form itself.
 */
[[nodiscard]] PropertyHooks::RebuildHint applyDatasetFormEdit(int formId,
                                                            const QVariant& value,
                                                            Dataset& d,
                                                            const ProjectModel& pm);

/**
 * @brief Returns the value a dataset form row carries, or an invalid QVariant when the row is not
 *        built for this dataset.
 */
[[nodiscard]] QVariant datasetFormValue(int formId,
                                        const Dataset& d,
                                        const ProjectModel& pm);

}  // namespace DataModel::Registry

// clang-format on
