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

#pragma once

#include <QJsonObject>
#include <QString>
#include <QtGlobal>
#include <QVariant>

/**
 * @file OutputWidget.h
 * @brief The output-widget vocabulary of the project model: the control struct, its enums and
 *        ceilings, and the free functions that serialize, normalize and admit its values.
 *        Frame.h includes this header, so every Frame.h consumer sees it unchanged.
 */

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// OutputWidget structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief Type of interactive output widget for bidirectional communication.
 */
enum class OutputWidgetType : quint8 {
  Button,
  Slider,
  Toggle,
  TextField,
  Knob,
};

/**
 * @brief Presentation size of an output widget, relative to the normal metrics.
 */
enum class OutputWidgetSize : quint8 {
  Small,
  Normal,
  Large,
  ExtraLarge,
};

/**
 * @brief What a control is, chosen per control in the project and independent of the theme (spec
 *        0113 R17). Standard is every control written before styles existed. Illuminated and
 *        EmergencyStop are button styles (an emergency stop always latches); Selector is a knob
 *        style with one labelled detent per Step from Min to Max.
 */
enum class OutputWidgetStyle : quint8 {
  Standard      = 0,
  Illuminated   = 1,
  EmergencyStop = 2,
  Selector      = 3,
};

/**
 * @brief Ceiling for an output control's confirm-within window. A window longer than this
 *        describes no real equipment, and every millisecond past it is one where the control
 *        latches "request outstanding" and refuses to adopt what the plant reports.
 */
inline constexpr int kMaxOutputStateConfirmMs = 120000;

/**
 * @brief Ceiling for an output control's displayed decimals; -1 is the floor (pick from Step).
 */
inline constexpr int kMaxOutputDecimals = 9;

/**
 * @brief Returns the metric multiplier of a size class; the ONE definition shared by the panel
 *        packing engine and the QML controls, so both agree on how large a control really is.
 */
[[nodiscard]] inline constexpr double outputSizeScale(OutputWidgetSize size)
{
  switch (size) {
    case OutputWidgetSize::Small:
      return 0.8;
    case OutputWidgetSize::Large:
      return 1.35;
    case OutputWidgetSize::ExtraLarge:
      return 1.75;
    default:
      return 1.0;
  }
}

/**
 * @brief Where a control reads the state it displays (spec 0080). None is the default, so a
 *        project written before state feedback existed reads back unbound and behaves exactly
 *        as it always did.
 */
enum class OutputStateSource : quint8 {
  None    = 0,
  Dataset = 1,
  Table   = 2,
};

/**
 * @brief How a text field reads what the operator typed (spec 0109). Text hands the string to the
 *        transmit script untouched; Number parses it and enforces Min to Max first.
 */
enum class OutputInputKind : quint8 {
  Text   = 0,
  Number = 1,
};

/**
 * @brief Represents an interactive output widget that sends data to the device.
 */
struct alignas(8) OutputWidget {
  int widgetId            = -1;                        ///< Unique widget ID within the group
  int groupId             = -1;                        ///< Owning group ID
  int sourceId            = 0;                         ///< Target source/device ID
  int txEncoding          = 0;                         ///< Encoding used for transmit payload
  OutputWidgetType type   = OutputWidgetType::Button;  ///< Widget presentation/behavior type
  bool monoIcon           = false;                     ///< Use monochrome icon styling
  bool checkable          = false;                     ///< Button latches on/off instead of pulsing
  bool hideTitle          = false;                     ///< Draw without title and frame
  OutputWidgetSize size   = OutputWidgetSize::Normal;  ///< Presentation size class
  OutputWidgetStyle style = OutputWidgetStyle::Standard;  ///< What the control is (spec 0113)
  double minValue         = 0;                            ///< Minimum allowed value
  double maxValue         = 100;                          ///< Maximum allowed value
  double stepSize         = 1;                            ///< Value increment step
  double initialValue     = 0;                            ///< Initial widget value
  QString icon;
  QString color;  ///< Optional hex fill override; empty -> group accent colour
  QString title;
  QString onLabel;
  QString offLabel;
  QString transmitFunction;

  OutputStateSource stateSource = OutputStateSource::None;  ///< Where displayed state comes from
  int stateDatasetId            = -1;    ///< Dataset uniqueId driving the display, -1 = none
  int stateConfirmMs            = 3000;  ///< How long a request stays outstanding, milliseconds
  QString stateTable;                    ///< Data table holding the state variable
  QString stateVariable;                 ///< Variable within stateTable
  QString stateOnValue;                  ///< Empty -> any non-zero value means on
  QString positionLabels;                ///< Selector: comma-separated labels, one per position

  int replyTokenDatasetId   = -1;  ///< Dataset uniqueId echoing the command token, -1 = none
  int replyCodeDatasetId    = -1;  ///< Dataset uniqueId carrying the result code, -1 = none
  int replyMessageDatasetId = -1;  ///< Dataset uniqueId carrying the reply text, -1 = none

  OutputInputKind inputKind = OutputInputKind::Text;  ///< Text field: Text or Number (spec 0109)
  int decimals              = -1;  ///< Displayed decimals, -1 = automatic from Step
  QString units;                   ///< Suffix shown beside the value, e.g. "%"
  bool quantize     = false;       ///< Number field: snap a typed value to Step from Min
  bool springReturn = false;       ///< Selector: returns to Initial Value on release
};

static_assert(sizeof(OutputWidget) % alignof(OutputWidget) == 0, "Unaligned OutputWidget struct");

/**
 * @brief Serializes an OutputWidget to a QJsonObject.
 */
[[nodiscard]] QJsonObject serialize(const OutputWidget& w);

/**
 * @brief Deserializes an OutputWidget from a QJsonObject (OutputWidget.cpp: uses
 *        SerialStudio::toDouble).
 */
[[nodiscard]] bool read(OutputWidget& w, const QJsonObject& obj);

/**
 * @brief Forces an output widget's numeric fields into the ranges the rest of the code assumes.
 *        Every door into the model calls this: read() for a project file and the API's patch
 *        handler for a live edit. Clamping in only one of them is how an out-of-range encoding,
 *        a negative source id or an inverted min/max reached the widgets through the other.
 */
void normalize(OutputWidget& w);

/**
 * @brief Applies the Project Editor's style choice @p index to @p w: a knob offers Standard and
 *        Selector; a button Standard, Illuminated, EmergencyStop and Selector Switch, the last
 *        of which turns it into a two-position selector knob labelled from its captions.
 */
void applyStyleChoice(OutputWidget& w, int index);

/**
 * @brief True when the control waits for a device reply (spec 0110): token and code both bound.
 */
[[nodiscard]] bool outputExpectsReply(const OutputWidget& w);

/**
 * @brief Verdict of admitOutputValue (spec 0109).
 */
enum class OutputAdmission : quint8 {
  Accepted   = 0,
  NotANumber = 1,
  OutOfRange = 2,
};

/**
 * @brief The four control fields admitOutputValue reads, so a caller hands over those rather than
 *        a copy of the whole control.
 */
struct OutputLimits {
  double minValue = 0;      ///< Minimum allowed value
  double maxValue = 100;    ///< Maximum allowed value
  double stepSize = 1;      ///< Quantize step, counted from minValue
  bool quantize   = false;  ///< Snap a value to stepSize before the range check
};

/**
 * @brief True when the control carries a numeric value bounded by Min to Max: a slider, a knob or
 *        a text field whose input kind is Number.
 */
[[nodiscard]] bool outputIsNumeric(const OutputWidget& w);

/**
 * @brief Pure admission rule for a numeric control: parses @p value, rejects a non-finite one,
 *        quantizes to Step from Min when enabled, then checks Min to Max. @p out receives the
 *        admitted double on Accepted.
 */
[[nodiscard]] OutputAdmission admitOutputValue(const OutputLimits& limits,
                                               const QVariant& value,
                                               double& out);

/**
 * @brief Most detents a selector draws; HardwareSelector.qml applies the same 2 to 12 rule.
 */
inline constexpr int kMaxSelectorPositions = 12;

/**
 * @brief Number of detents a selector shows: one per Step from Min to Max, 2 to 12.
 */
[[nodiscard]] int selectorPositionCount(const OutputLimits& limits);

/**
 * @brief The detent a reported state names (spec 0113 R23): text equal to a position label
 *        (case-insensitive), or a number on a detent; -1 when no position matches.
 */
[[nodiscard]] int selectorPosition(const OutputLimits& limits,
                                   const QString& labels,
                                   double value,
                                   const QString& text);

/**
 * @brief The value a spring-return selector sends when let go (spec 0113 R22): true with Initial
 *        Value on its detent in @p out when the knob is elsewhere, false when it already rests.
 */
[[nodiscard]] bool selectorReleaseValue(
  const OutputLimits& limits, bool springReturn, double initialValue, double current, double& out);

/**
 * @brief The latch a button starts with (spec 0113 R26): an emergency stop is always released, a
 *        latching button follows Initial Value, a momentary one holds nothing.
 */
[[nodiscard]] bool outputStartsLatched(const OutputWidget& w);

/**
 * @brief True when a latch change skips the pacing window (spec 0113 R26): only an emergency
 *        stop being pressed; its release is paced like any other change.
 */
[[nodiscard]] bool outputSendsUnpaced(OutputWidgetStyle style, bool latched);

}  // namespace DataModel
