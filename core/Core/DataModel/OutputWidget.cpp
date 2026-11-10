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

#include "Core/DataModel/OutputWidget.h"

#include <algorithm>
#include <cmath>
#include <QDebug>
#include <utility>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Normalization and admission
//--------------------------------------------------------------------------------------------------

/**
 * @brief @p value when it is finite, else @p fallback. The project's numeric parser accepts the
 *        strings "nan" and "inf" and reports success, and a non-finite bound defeats every
 *        comparison downstream: an inversion swap never fires, qBound's own precondition does not
 *        trip, and the control publishes NaN to QML as its current value.
 */
[[nodiscard]] static double finiteOr(const double value, const double fallback)
{
  return std::isfinite(value) ? value : fallback;
}

/**
 * @brief Forces an output widget's numeric fields into the ranges the rest of the code assumes;
 *        see the header for why both doors into the model call it.
 */
void DataModel::normalize(OutputWidget& w)
{
  w.sourceId = qMax(0, w.sourceId);
  w.type     = static_cast<OutputWidgetType>(
    qBound(0, static_cast<int>(w.type), static_cast<int>(OutputWidgetType::Knob)));
  w.size = static_cast<OutputWidgetSize>(
    qBound(0, static_cast<int>(w.size), static_cast<int>(OutputWidgetSize::ExtraLarge)));
  w.style = static_cast<OutputWidgetStyle>(
    qBound(0, static_cast<int>(w.style), static_cast<int>(OutputWidgetStyle::Selector)));
  const bool knobStyle = w.style == OutputWidgetStyle::Selector;
  const bool buttonStyle =
    w.style == OutputWidgetStyle::Illuminated || w.style == OutputWidgetStyle::EmergencyStop;
  if ((buttonStyle && w.type != OutputWidgetType::Button)
      || (knobStyle && w.type != OutputWidgetType::Knob))
    w.style = OutputWidgetStyle::Standard;

  if (w.style == OutputWidgetStyle::EmergencyStop)
    w.checkable = true;

  if (w.style != OutputWidgetStyle::Selector)
    w.springReturn = false;

  const int encoding = w.txEncoding;
  w.txEncoding       = qBound(0, encoding, static_cast<int>(SerialStudio::EncEucKr));
  if (w.txEncoding != encoding)
    qWarning() << "[Project] output widget" << w.title << "has an unknown transmit encoding"
               << encoding << "; using" << w.txEncoding;

  w.minValue = finiteOr(w.minValue, 0);
  w.maxValue = finiteOr(w.maxValue, 100);
  if (w.minValue > w.maxValue) {
    qWarning() << "[Project] output widget" << w.title << "has min > max; swapping";
    std::swap(w.minValue, w.maxValue);
  }

  w.stepSize = finiteOr(w.stepSize, 1);
  if (w.stepSize <= 0) {
    qWarning() << "[Project] output widget" << w.title << "has a non-positive step; using 1";
    w.stepSize = 1;
  }

  w.initialValue = finiteOr(w.initialValue, 0);
  w.stateSource  = static_cast<OutputStateSource>(
    qBound(0, static_cast<int>(w.stateSource), static_cast<int>(OutputStateSource::Table)));
  w.stateConfirmMs        = qBound(0, w.stateConfirmMs, kMaxOutputStateConfirmMs);
  w.replyTokenDatasetId   = qMax(-1, w.replyTokenDatasetId);
  w.replyCodeDatasetId    = qMax(-1, w.replyCodeDatasetId);
  w.replyMessageDatasetId = qMax(-1, w.replyMessageDatasetId);
  w.inputKind             = static_cast<OutputInputKind>(
    qBound(0, static_cast<int>(w.inputKind), static_cast<int>(OutputInputKind::Number)));
  w.decimals = qBound(-1, w.decimals, kMaxOutputDecimals);
  w.units    = w.units.simplified();
}

/**
 * @brief Maps the style combo's @p index onto @p w; see the header. The Selector Switch entry
 *        sits in the button's list because that is where a user looks for a selector first.
 */
void DataModel::applyStyleChoice(OutputWidget& w, const int index)
{
  if (w.type == OutputWidgetType::Knob) {
    w.style = index == 1 ? OutputWidgetStyle::Selector : OutputWidgetStyle::Standard;
    return;
  }

  if (index == 3) {
    const auto off   = w.offLabel.isEmpty() ? QStringLiteral("OFF") : w.offLabel;
    const auto on    = w.onLabel.isEmpty() ? QStringLiteral("ON") : w.onLabel;
    w.type           = OutputWidgetType::Knob;
    w.style          = OutputWidgetStyle::Selector;
    w.checkable      = false;
    w.minValue       = 0;
    w.maxValue       = 1;
    w.stepSize       = 1;
    w.initialValue   = 0;
    w.positionLabels = off + QStringLiteral(", ") + on;
    return;
  }

  w.style = static_cast<OutputWidgetStyle>(
    qBound(0, index, static_cast<int>(OutputWidgetStyle::EmergencyStop)));
  if (w.style == OutputWidgetStyle::EmergencyStop)
    w.checkable = true;
}

/**
 * @brief True when token and result code are both bound (spec 0110); the message is optional.
 */
bool DataModel::outputExpectsReply(const OutputWidget& w)
{
  return w.replyTokenDatasetId >= 0 && w.replyCodeDatasetId >= 0;
}

/**
 * @brief True when the control carries a number bounded by Min to Max (spec 0109): a slider, a
 *        knob, or a text field whose input kind is Number.
 */
bool DataModel::outputIsNumeric(const OutputWidget& w)
{
  if (w.type == OutputWidgetType::Slider || w.type == OutputWidgetType::Knob)
    return true;

  return w.type == OutputWidgetType::TextField && w.inputKind == OutputInputKind::Number;
}

/**
 * @brief The one admission rule every numeric control's value passes before its transmit script
 *        (spec 0109 R3): a number is taken as is and text is parsed; a non-finite result is not a
 *        number; Quantize snaps to Step counted from Min; anything outside Min to Max is refused.
 */
DataModel::OutputAdmission DataModel::admitOutputValue(const OutputLimits& limits,
                                                       const QVariant& value,
                                                       double& out)
{
  SS_ASSERT_LOG(limits.minValue <= limits.maxValue);
  SS_ASSERT_LOG(std::isfinite(limits.minValue) && std::isfinite(limits.maxValue));

  bool ok           = false;
  const auto typeId = value.metaType().id();
  double v          = 0;
  if (typeId == QMetaType::QString)
    v = SerialStudio::toDouble(value.toString().trimmed(), &ok);
  else if (typeId != QMetaType::Bool)
    v = SerialStudio::toDouble(value, &ok);

  if (!ok || !std::isfinite(v))
    return OutputAdmission::NotANumber;

  if (limits.quantize && limits.stepSize > 0) {
    v = limits.minValue + std::round((v - limits.minValue) / limits.stepSize) * limits.stepSize;
    if (v > limits.maxValue && v - limits.maxValue <= limits.stepSize * 1e-9)
      v = limits.maxValue;
  }

  if (v < limits.minValue || v > limits.maxValue)
    return OutputAdmission::OutOfRange;

  out = v;
  return OutputAdmission::Accepted;
}

//--------------------------------------------------------------------------------------------------
// Selector and emergency-stop decisions (spec 0113)
//--------------------------------------------------------------------------------------------------

/**
 * @brief One detent per Step from Min to Max, held to 2 to 12 so a degenerate range still draws a
 *        switch; the same rule HardwareSelector.qml draws by.
 */
int DataModel::selectorPositionCount(const OutputLimits& limits)
{
  SS_ASSERT(std::isfinite(limits.minValue) && std::isfinite(limits.maxValue), return 2);
  SS_ASSERT(std::isfinite(limits.stepSize), return 2);

  const double step = std::max(limits.stepSize, 1e-9);
  const double span = std::round((limits.maxValue - limits.minValue) / step);
  const double n    = std::isfinite(span) ? span + 1 : 2;
  return static_cast<int>(std::clamp(n, 2.0, static_cast<double>(kMaxSelectorPositions)));
}

/**
 * @brief Matches a label first, so a device reporting "Auto" selects the Auto position; a text
 *        that is not a number and names no label matches nothing, rather than its parsed zero.
 *        A number matches only when it sits on a detent inside the drawn range.
 */
int DataModel::selectorPosition(const OutputLimits& limits,
                                const QString& labels,
                                const double value,
                                const QString& text)
{
  const int count = selectorPositionCount(limits);
  SS_ASSERT(count >= 2 && count <= kMaxSelectorPositions, return -1);

  const auto reported = text.trimmed();
  const auto names    = labels.split(QLatin1Char(','));
  const int named     = std::min(count, static_cast<int>(names.size()));
  for (int i = 0; i < named && !reported.isEmpty(); ++i)
    if (names[i].trimmed().compare(reported, Qt::CaseInsensitive) == 0)
      return i;

  bool numeric = true;
  if (!reported.isEmpty())
    (void)SerialStudio::toDouble(reported, &numeric);

  if (!numeric || !std::isfinite(value))
    return -1;

  const double step  = std::max(limits.stepSize, 1e-9);
  const double index = std::round((value - limits.minValue) / step);
  SS_ASSERT(std::isfinite(index), return -1);
  if (index < 0 || index >= count)
    return -1;

  const double detent = limits.minValue + index * step;
  if (std::abs(detent - value) > step * 1e-6)
    return -1;

  return static_cast<int>(index);
}

/**
 * @brief The rest position is Initial Value on its detent; letting go anywhere else returns there
 *        and sends it once, and letting go at rest sends nothing.
 */
bool DataModel::selectorReleaseValue(const OutputLimits& limits,
                                     const bool springReturn,
                                     const double initialValue,
                                     const double current,
                                     double& out)
{
  SS_ASSERT(std::isfinite(limits.minValue) && std::isfinite(limits.stepSize), return false);
  if (!springReturn)
    return false;

  const int count   = selectorPositionCount(limits);
  const double step = std::max(limits.stepSize, 1e-9);
  const double raw  = std::isfinite(initialValue) ? (initialValue - limits.minValue) / step : 0;
  const double rest = limits.minValue + std::clamp(std::round(raw), 0.0, count - 1.0) * step;
  SS_ASSERT(std::isfinite(rest), return false);
  if (std::abs(rest - current) <= step * 1e-6)
    return false;

  out = rest;
  return true;
}

/**
 * @brief An emergency stop never starts latched, whatever Initial Value says: a dashboard that
 *        opened already stopped would hide that nothing was sent.
 */
bool DataModel::outputStartsLatched(const OutputWidget& w)
{
  SS_ASSERT_LOG(std::isfinite(w.initialValue));
  SS_ASSERT_LOG(w.style != OutputWidgetStyle::EmergencyStop || w.type == OutputWidgetType::Button);
  if (w.style == OutputWidgetStyle::EmergencyStop)
    return false;

  return w.checkable && w.initialValue != 0;
}

/**
 * @brief A stop press must never wait behind the pacing window; everything else keeps it.
 */
bool DataModel::outputSendsUnpaced(const OutputWidgetStyle style, const bool latched)
{
  SS_ASSERT_LOG(static_cast<int>(style) >= 0);
  SS_ASSERT_LOG(style <= OutputWidgetStyle::Selector);
  return style == OutputWidgetStyle::EmergencyStop && latched;
}

//--------------------------------------------------------------------------------------------------
// JSON serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deserializes an OutputWidget from a QJsonObject.
 */
bool DataModel::read(OutputWidget& w, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  w.icon         = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  w.title        = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.sourceId     = ss_jsr(obj, Keys::SourceId, 0).toInt();
  w.type         = static_cast<OutputWidgetType>(ss_jsr(obj, Keys::OutputType, 0).toInt());
  w.minValue     = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputMinValue, 0));
  w.maxValue     = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputMaxValue, 100));
  w.stepSize     = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputStepSize, 1));
  w.initialValue = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputInitialValue, 0));
  w.monoIcon     = ss_jsr(obj, Keys::OutputMonoIcon, false).toBool();
  w.checkable    = ss_jsr(obj, Keys::OutputCheckable, false).toBool();
  w.hideTitle    = ss_jsr(obj, Keys::OutputHideTitle, false).toBool();
  w.color        = ss_jsr(obj, Keys::OutputColor, "").toString().simplified();
  w.size         = static_cast<OutputWidgetSize>(
    ss_jsr(obj, Keys::OutputSize, static_cast<int>(OutputWidgetSize::Normal)).toInt());
  w.style            = static_cast<OutputWidgetStyle>(ss_jsr(obj, Keys::OutputStyle, 0).toInt());
  w.onLabel          = ss_jsr(obj, Keys::OutputOnLabel, "").toString().simplified();
  w.offLabel         = ss_jsr(obj, Keys::OutputOffLabel, "").toString().simplified();
  w.transmitFunction = obj.value(Keys::TransmitFunction).toString();
  w.txEncoding       = ss_jsr(obj, Keys::OutputTxEncoding, 0).toInt();
  w.stateSource = static_cast<OutputStateSource>(ss_jsr(obj, Keys::OutputStateSource, 0).toInt());
  w.stateDatasetId        = ss_jsr(obj, Keys::OutputStateDatasetId, -1).toInt();
  w.stateTable            = ss_jsr(obj, Keys::OutputStateTable, "").toString().simplified();
  w.stateVariable         = ss_jsr(obj, Keys::OutputStateVariable, "").toString().simplified();
  w.stateOnValue          = ss_jsr(obj, Keys::OutputStateOnValue, "").toString();
  w.positionLabels        = ss_jsr(obj, Keys::OutputPositionLabels, "").toString().simplified();
  w.springReturn          = ss_jsr(obj, Keys::OutputSpringReturn, false).toBool();
  w.stateConfirmMs        = ss_jsr(obj, Keys::OutputStateConfirmMs, 3000).toInt();
  w.replyTokenDatasetId   = ss_jsr(obj, Keys::OutputReplyTokenId, -1).toInt();
  w.replyCodeDatasetId    = ss_jsr(obj, Keys::OutputReplyCodeId, -1).toInt();
  w.replyMessageDatasetId = ss_jsr(obj, Keys::OutputReplyMessageId, -1).toInt();
  w.inputKind = static_cast<OutputInputKind>(ss_jsr(obj, Keys::OutputInputKind, 0).toInt());
  w.decimals  = ss_jsr(obj, Keys::OutputDecimals, -1).toInt();
  w.units     = ss_jsr(obj, Keys::OutputUnits, "").toString();
  w.quantize  = ss_jsr(obj, Keys::OutputQuantize, false).toBool();

  normalize(w);
  return !w.title.isEmpty();
}

/**
 * @brief Serializes an OutputWidget to a QJsonObject.
 */
QJsonObject DataModel::serialize(const OutputWidget& w)
{
  QJsonObject obj;
  obj.insert(Keys::Icon, w.icon);
  obj.insert(Keys::Title, w.title);
  obj.insert(Keys::SourceId, w.sourceId);
  obj.insert(Keys::OutputType, static_cast<int>(w.type));
  obj.insert(Keys::OutputMinValue, w.minValue);
  obj.insert(Keys::OutputMaxValue, w.maxValue);
  obj.insert(Keys::OutputStepSize, w.stepSize);
  obj.insert(Keys::OutputInitialValue, w.initialValue);
  if (!w.onLabel.isEmpty())
    obj.insert(Keys::OutputOnLabel, w.onLabel);

  if (!w.offLabel.isEmpty())
    obj.insert(Keys::OutputOffLabel, w.offLabel);

  if (w.monoIcon)
    obj.insert(Keys::OutputMonoIcon, true);

  if (w.checkable)
    obj.insert(Keys::OutputCheckable, true);

  if (w.hideTitle)
    obj.insert(Keys::OutputHideTitle, true);

  if (!w.color.isEmpty())
    obj.insert(Keys::OutputColor, w.color);

  if (w.size != OutputWidgetSize::Normal)
    obj.insert(Keys::OutputSize, static_cast<int>(w.size));

  if (w.style != OutputWidgetStyle::Standard)
    obj.insert(Keys::OutputStyle, static_cast<int>(w.style));

  if (!w.positionLabels.isEmpty())
    obj.insert(Keys::OutputPositionLabels, w.positionLabels);

  if (w.springReturn)
    obj.insert(Keys::OutputSpringReturn, true);

  obj.insert(Keys::TransmitFunction, w.transmitFunction);
  obj.insert(Keys::OutputTxEncoding, w.txEncoding);

  const bool replyBound = outputExpectsReply(w);
  if (w.stateSource != OutputStateSource::None || replyBound)
    obj.insert(Keys::OutputStateConfirmMs, w.stateConfirmMs);

  if (replyBound) {
    obj.insert(Keys::OutputReplyTokenId, w.replyTokenDatasetId);
    obj.insert(Keys::OutputReplyCodeId, w.replyCodeDatasetId);
    if (w.replyMessageDatasetId >= 0)
      obj.insert(Keys::OutputReplyMessageId, w.replyMessageDatasetId);
  }

  if (w.stateSource != OutputStateSource::None) {
    obj.insert(Keys::OutputStateSource, static_cast<int>(w.stateSource));
    if (!w.stateOnValue.isEmpty())
      obj.insert(Keys::OutputStateOnValue, w.stateOnValue);

    if (w.stateSource == OutputStateSource::Dataset)
      obj.insert(Keys::OutputStateDatasetId, w.stateDatasetId);

    else {
      obj.insert(Keys::OutputStateTable, w.stateTable);
      obj.insert(Keys::OutputStateVariable, w.stateVariable);
    }
  }

  if (w.inputKind != OutputInputKind::Text)
    obj.insert(Keys::OutputInputKind, static_cast<int>(w.inputKind));

  if (w.decimals >= 0)
    obj.insert(Keys::OutputDecimals, w.decimals);

  if (!w.units.isEmpty())
    obj.insert(Keys::OutputUnits, w.units);

  if (w.quantize)
    obj.insert(Keys::OutputQuantize, true);

  return obj;
}
