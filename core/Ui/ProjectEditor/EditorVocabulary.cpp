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

#include "ProjectEditor/EditorVocabulary.h"

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "ProjectEditor/ProjectEditor.h"
#include "UI/WidgetExtensions.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade whose combo vocabulary this sub-object fills.
 */
EditorVocabulary::EditorVocabulary(ProjectEditor& editor) : m_editor(editor) {}

//--------------------------------------------------------------------------------------------------
// Vocabulary generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates the combobox string lists used by the form models; runs at construction and
 *        again on every language or extension-catalog change.
 */
void EditorVocabulary::generateComboBoxModels()
{
  generateFftLists();
  generateSourceLists();
  generateWidgetMaps();
  generateFormatMaps();
}

/**
 * @brief Adds one widget-picker entry per installed extension package, in the picker matching the
 *        scope it declared. A bundled package that ships as a built-in implementation is skipped:
 *        its built-in entry already exists and keeps its own label.
 */
void EditorVocabulary::appendExtensionWidgets()
{
  static auto& catalog = UI::WidgetExtensions::instance();

  const auto packages = catalog.ids();
  for (const auto& id : packages) {
    const auto& package = catalog.descriptor(id);
    if (!package.replaces.isEmpty() || package.title.isEmpty())
      continue;

    if (package.scope == UI::WidgetExtensions::GroupScope)
      m_editor.m_groupWidgets.insert(id, package.title);

    else
      m_editor.m_datasetWidgets.insert(id, package.title);
  }
}

//--------------------------------------------------------------------------------------------------
// Vocabulary sections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills the FFT sample-count list and the window-function labels with their enum values.
 */
void EditorVocabulary::generateFftLists()
{
  m_editor.m_fftSamples.clear();
  m_editor.m_fftSamples << "8" << "16" << "32" << "64" << "128" << "256" << "512" << "1024"
                        << "2048" << "4096" << "8192" << "16384" << "32768" << "65536" << "131072"
                        << "262144";

  m_editor.m_fftWindows.clear();
  m_editor.m_fftWindowValues.clear();
  m_editor.m_fftWindows << tr("Rectangular (None)") << tr("Bartlett (Triangular)") << tr("Hann")
                        << tr("Hamming") << tr("Blackman") << tr("Blackman-Harris") << tr("Nuttall")
                        << tr("Blackman-Nuttall") << tr("Flat Top") << tr("Welch")
                        << tr("Bartlett-Hann") << tr("Bohman") << tr("Cosine (Sine)")
                        << tr("Lanczos") << tr("Parzen");
  m_editor.m_fftWindowValues << SerialStudio::FFTWindowRectangular
                             << SerialStudio::FFTWindowBartlett << SerialStudio::FFTWindowHann
                             << SerialStudio::FFTWindowHamming << SerialStudio::FFTWindowBlackman
                             << SerialStudio::FFTWindowBlackmanHarris
                             << SerialStudio::FFTWindowNuttall
                             << SerialStudio::FFTWindowBlackmanNuttall
                             << SerialStudio::FFTWindowFlatTop << SerialStudio::FFTWindowWelch
                             << SerialStudio::FFTWindowBartlettHann << SerialStudio::FFTWindowBohman
                             << SerialStudio::FFTWindowCosine << SerialStudio::FFTWindowLanczos
                             << SerialStudio::FFTWindowParzen;
}

/**
 * @brief Fills the action timer modes, the source decoder, checksum and frame-detection lists,
 *        and (commercial builds) the image-detection and output-widget type lists.
 */
void EditorVocabulary::generateSourceLists()
{
  m_editor.m_timerModes.clear();
  m_editor.m_timerModes << tr("Off") << tr("Auto Start") << tr("Start on Trigger")
                        << tr("Toggle on Trigger") << tr("Repeat N Times");

  m_editor.m_decoderOptions.clear();
  m_editor.m_decoderOptions << tr("Plain Text (UTF8)") << tr("Hexadecimal") << tr("Base64")
                            << tr("Binary (Direct)");

  m_editor.m_checksumMethods = IO::availableChecksums();
  const int noChecksumIndex  = m_editor.m_checksumMethods.indexOf(QLatin1String(""));
  if (noChecksumIndex >= 0)
    m_editor.m_checksumMethods[noChecksumIndex] = tr("No Checksum");

  m_editor.m_frameDetectionMethods.clear();
  m_editor.m_frameDetectionMethodsValues.clear();
  m_editor.m_frameDetectionMethods << tr("End Delimiter Only") << tr("Start Delimiter Only")
                                   << tr("Start + End Delimiter") << tr("No Delimiters")
                                   << tr("Fixed Length");
  m_editor.m_frameDetectionMethodsValues
    << SerialStudio::EndDelimiterOnly << SerialStudio::StartDelimiterOnly
    << SerialStudio::StartAndEndDelimiter << SerialStudio::NoDelimiters
    << SerialStudio::FixedLength;

#ifdef BUILD_COMMERCIAL
  m_editor.m_imgDetectionModes.clear();
  m_editor.m_imgDetectionModes << tr("Auto-detect") << tr("Manual Delimiters");

  m_editor.m_outputWidgetTypes.clear();
  m_editor.m_outputWidgetTypes << tr("Button") << tr("Slider") << tr("Toggle") << tr("Text Field")
                               << tr("Knob");
#endif
}

/**
 * @brief Fills the group and dataset widget pickers, then appends the installed extensions.
 */
void EditorVocabulary::generateWidgetMaps()
{
  m_editor.m_groupWidgets.clear();
  m_editor.m_groupWidgets.insert(QStringLiteral("barpanel"), tr("Bar Panel"));
  m_editor.m_groupWidgets.insert(QStringLiteral("ledpanel"), tr("LED Panel"));
  m_editor.m_groupWidgets.insert(QStringLiteral("datagrid"), tr("Data Grid"));
  m_editor.m_groupWidgets.insert(QStringLiteral("map"), tr("GPS Map"));
  m_editor.m_groupWidgets.insert(QStringLiteral("gyro"), tr("Gyroscope"));
  m_editor.m_groupWidgets.insert(QStringLiteral("multiplot"), tr("Multi-Plot"));
  m_editor.m_groupWidgets.insert(QStringLiteral("accelerometer"), tr("Accelerometer"));
  m_editor.m_groupWidgets.insert(QStringLiteral("plot3d"), tr("3D Plot"));
  m_editor.m_groupWidgets.insert(QStringLiteral("image"), tr("Image View"));
  m_editor.m_groupWidgets.insert(QStringLiteral("painter"), tr("Canvas Widget"));
  m_editor.m_groupWidgets.insert(QStringLiteral("webview"), tr("Web View"));
  m_editor.m_groupWidgets.insert(QLatin1String(""), tr("None"));

  m_editor.m_datasetWidgets.clear();
  m_editor.m_datasetWidgets.insert(QLatin1String(""), tr("None"));
  m_editor.m_datasetWidgets.insert(QStringLiteral("bar"), tr("Bar"));
  m_editor.m_datasetWidgets.insert(QStringLiteral("gauge"), tr("Gauge"));
  m_editor.m_datasetWidgets.insert(QStringLiteral("compass"), tr("Compass"));
  m_editor.m_datasetWidgets.insert(QStringLiteral("meter"), tr("Meter"));

  appendExtensionWidgets();
}

/**
 * @brief Fills the display-format, EOL-sequence and plot-option label maps.
 */
void EditorVocabulary::generateFormatMaps()
{
  m_editor.m_displayFormats.clear();
  m_editor.m_displayFormats.insert(QLatin1String(""), tr("Auto"));
  m_editor.m_displayFormats.insert(QStringLiteral("0d"), tr("Integer (0 decimals)"));
  m_editor.m_displayFormats.insert(QStringLiteral("1d"), tr("1 decimal"));
  m_editor.m_displayFormats.insert(QStringLiteral("2d"), tr("2 decimals"));
  m_editor.m_displayFormats.insert(QStringLiteral("3d"), tr("3 decimals"));
  m_editor.m_displayFormats.insert(QStringLiteral("sci"), tr("Scientific"));
  m_editor.m_displayFormats.insert(QStringLiteral("hex"), tr("Hexadecimal"));
  m_editor.m_displayFormats.insert(QStringLiteral("bin"), tr("Binary"));

  m_editor.m_eolSequences.clear();
  m_editor.m_eolSequences.insert(QLatin1String(""), tr("None"));
  m_editor.m_eolSequences.insert(QStringLiteral("\n"), tr("New Line (\\n)"));
  m_editor.m_eolSequences.insert(QStringLiteral("\r"), tr("Carriage Return (\\r)"));
  m_editor.m_eolSequences.insert(QStringLiteral("\r\n"), tr("CRLF (\\r\\n)"));

  m_editor.m_plotOptions.clear();
  m_editor.m_plotOptions.insert(qMakePair(false, false), tr("No"));
  m_editor.m_plotOptions.insert(qMakePair(true, false), tr("Yes"));
}

}  // namespace DataModel
