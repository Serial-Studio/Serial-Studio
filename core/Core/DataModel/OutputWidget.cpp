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
  w.color        = ss_jsr(obj, Keys::OutputColor, "").toString().simplified();
  w.size         = static_cast<OutputWidgetSize>(
    ss_jsr(obj, Keys::OutputSize, static_cast<int>(OutputWidgetSize::Normal)).toInt());
  w.onLabel          = ss_jsr(obj, Keys::OutputOnLabel, "").toString().simplified();
  w.offLabel         = ss_jsr(obj, Keys::OutputOffLabel, "").toString().simplified();
  w.transmitFunction = obj.value(Keys::TransmitFunction).toString();
  w.txEncoding       = ss_jsr(obj, Keys::OutputTxEncoding, 0).toInt();
  w.stateSource = static_cast<OutputStateSource>(ss_jsr(obj, Keys::OutputStateSource, 0).toInt());
  w.stateDatasetId        = ss_jsr(obj, Keys::OutputStateDatasetId, -1).toInt();
  w.stateTable            = ss_jsr(obj, Keys::OutputStateTable, "").toString().simplified();
  w.stateVariable         = ss_jsr(obj, Keys::OutputStateVariable, "").toString().simplified();
  w.stateOnValue          = ss_jsr(obj, Keys::OutputStateOnValue, "").toString();
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

  if (!w.color.isEmpty())
    obj.insert(Keys::OutputColor, w.color);

  if (w.size != OutputWidgetSize::Normal)
    obj.insert(Keys::OutputSize, static_cast<int>(w.size));

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
