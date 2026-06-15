/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Frame.h"

#include "AppInfo.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Project version stamp
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the running application version used for project writer stamps.
 */
QString DataModel::current_writer_version()
{
  return QString::fromUtf8(APP_VERSION);
}

//--------------------------------------------------------------------------------------------------
// Frame processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Finalises a frame: commercial flag and stable uniqueId per dataset.
 */
void DataModel::finalize_frame(DataModel::Frame& frame)
{
  frame.containsCommercialFeatures = SerialStudio::commercialCfg(frame.groups);
  for (auto& group : frame.groups) {
    for (auto& dataset : group.datasets) {
      dataset.sourceId = group.sourceId;
      if (dataset.uniqueId < 0)
        dataset.uniqueId = dataset_unique_id(group.sourceId, dataset.groupId, dataset.datasetId);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Configuration reading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads frame delimiters and checksum algorithm from a source JSON object, resolving
 *        hex/escape encodings and accepting the legacy "checksum" key as a fallback.
 */
void DataModel::read_io_settings(QByteArray& frameStart,
                                 QByteArray& frameEnd,
                                 QString& checksum,
                                 const QJsonObject& obj)
{
  auto fEndStr   = ss_jsr(obj, Keys::FrameEnd, "").toString();
  auto fStartStr = ss_jsr(obj, Keys::FrameStart, "").toString();
  auto isHex     = ss_jsr(obj, Keys::HexadecimalDelimiters, false).toBool();

  if (obj.contains(Keys::ChecksumAlgorithm))
    checksum = obj.value(Keys::ChecksumAlgorithm).toString();
  else
    checksum = ss_jsr(obj, Keys::Checksum, "").toString();

  if (isHex) {
    QString resolvedEnd   = SerialStudio::resolveEscapeSequences(fEndStr);
    QString resolvedStart = SerialStudio::resolveEscapeSequences(fStartStr);
    frameStart            = QByteArray::fromHex(resolvedStart.remove(' ').toUtf8());
    frameEnd              = QByteArray::fromHex(resolvedEnd.remove(' ').toUtf8());
  }

  else {
    frameEnd   = SerialStudio::resolveEscapeSequences(fEndStr).toUtf8();
    frameStart = SerialStudio::resolveEscapeSequences(fStartStr).toUtf8();
  }
}

//--------------------------------------------------------------------------------------------------
// JSON helpers that parse numbers (out-of-line so they can reach SerialStudio::toDouble)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes a RegisterDef to a QJsonObject.
 */
QJsonObject DataModel::serialize(const RegisterDef& r)
{
  QJsonObject obj;
  obj.insert(Keys::Name, r.name);

  obj.insert(Keys::RegisterTypeName,
             r.type == RegisterType::Computed ? QStringLiteral("computed")
                                              : QStringLiteral("constant"));

  if (r.defaultValue.typeId() == QMetaType::Double)
    obj.insert(Keys::Value, SerialStudio::toDouble(r.defaultValue));
  else if (!r.defaultValue.toString().isEmpty())
    obj.insert(Keys::Value, r.defaultValue.toString());

  return obj;
}

/**
 * @brief Deserializes a RegisterDef from a QJsonObject.
 */
bool DataModel::read(RegisterDef& r, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  r.name = ss_jsr(obj, Keys::Name, "").toString().simplified();
  if (r.name.isEmpty())
    return false;

  const auto typeStr = ss_jsr(obj, Keys::RegisterTypeName, "constant").toString();
  r.type = (typeStr == QLatin1String("computed")) ? RegisterType::Computed : RegisterType::Constant;

  const auto val = obj.value(Keys::Value);
  if (val.isDouble())
    r.defaultValue = SerialStudio::toDouble(val);
  else if (val.isString())
    r.defaultValue = val.toString();
  else
    r.defaultValue = QVariant(0.0);

  return true;
}

/**
 * @brief Deserializes an OutputWidget from a QJsonObject.
 */
bool DataModel::read(OutputWidget& w, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  w.icon     = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  w.title    = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.sourceId = ss_jsr(obj, Keys::SourceId, 0).toInt();
  w.type     = static_cast<OutputWidgetType>(
    qBound(0, ss_jsr(obj, Keys::OutputType, 0).toInt(), static_cast<int>(OutputWidgetType::Knob)));
  w.minValue         = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputMinValue, 0));
  w.maxValue         = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputMaxValue, 100));
  w.stepSize         = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputStepSize, 1));
  w.initialValue     = SerialStudio::toDouble(ss_jsr(obj, Keys::OutputInitialValue, 0));
  w.monoIcon         = ss_jsr(obj, Keys::OutputMonoIcon, false).toBool();
  w.transmitFunction = obj.value(Keys::TransmitFunction).toString();
  w.txEncoding       = ss_jsr(obj, Keys::OutputTxEncoding, 0).toInt();

  return !w.title.isEmpty();
}

/**
 * @brief Deserializes an AlarmBand from a QJsonObject.
 */
bool DataModel::read(AlarmBand& b, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  b.min = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
  b.max = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  if (b.min > b.max)
    std::swap(b.min, b.max);

  const int sev = ss_jsr(obj, Keys::Severity, static_cast<int>(AlarmSeverity::Warning)).toInt();
  b.severity    = static_cast<AlarmSeverity>(qBound(0, sev, 3));
  b.blink       = ss_jsr(obj, Keys::Blink, false).toBool();
  b.color       = ss_jsr(obj, Keys::Color, "").toString().simplified();
  b.label       = ss_jsr(obj, Keys::Label, "").toString().simplified();
  return b.max > b.min;
}

/**
 * @brief Populates @p d.alarmBands from @p obj, accepting both canonical and v3.3 legacy fields.
 */
void DataModel::readDatasetAlarmBands(Dataset& d, const QJsonObject& obj)
{
  d.alarmBands.clear();
  if (obj.contains(Keys::AlarmBands)) {
    const auto arr = obj.value(Keys::AlarmBands).toArray();
    d.alarmBands.reserve(arr.size());
    for (const auto& v : arr) {
      AlarmBand b;
      if (read(b, v.toObject()))
        d.alarmBands.push_back(std::move(b));
    }
    return;
  }

  if (!ss_jsr(obj, Keys::AlarmEnabled, false).toBool())
    return;

  double lo =
    SerialStudio::toDouble(ss_jsr(obj, Keys::AlarmLow, std::numeric_limits<double>::quiet_NaN()));
  double hi =
    SerialStudio::toDouble(ss_jsr(obj, Keys::AlarmHigh, std::numeric_limits<double>::quiet_NaN()));
  if (std::isnan(hi) && obj.contains(Keys::Alarm))
    hi = SerialStudio::toDouble(ss_jsr(obj, Keys::Alarm, 0));

  const double rangeMin = qMin(d.wgtMin, d.wgtMax);
  const double rangeMax = qMax(d.wgtMin, d.wgtMax);
  if (!std::isnan(lo) && lo > rangeMin && lo < rangeMax) {
    AlarmBand low;
    low.min      = rangeMin;
    low.max      = lo;
    low.severity = AlarmSeverity::Warning;
    d.alarmBands.push_back(std::move(low));
  }

  if (!std::isnan(hi) && hi > rangeMin && hi < rangeMax) {
    AlarmBand high;
    high.min      = hi;
    high.max      = rangeMax;
    high.severity = AlarmSeverity::Warning;
    d.alarmBands.push_back(std::move(high));
  }
}

//--------------------------------------------------------------------------------------------------
// Dataset deserialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deserializes a Dataset from a QJsonObject.
 *        Lives here (not inline in Frame.h) so it can reach SerialStudio::toDouble.
 */
bool DataModel::read(Dataset& d, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  d.index          = ss_jsr(obj, Keys::Index, -1).toInt();
  d.fft            = ss_jsr(obj, Keys::FFT, false).toBool();
  d.led            = ss_jsr(obj, Keys::LED, false).toBool();
  d.log            = ss_jsr(obj, Keys::Log, false).toBool();
  d.plt            = ss_jsr(obj, Keys::Graph, false).toBool();
  d.waterfall      = ss_jsr(obj, Keys::Waterfall, false).toBool();
  d.waterfallYAxis = ss_jsr(obj, Keys::WaterfallYAxis, 0).toInt();
  d.xAxisId        = ss_jsr(obj, Keys::XAxis, kXAxisTime).toInt();

  d.fftMin            = SerialStudio::toDouble(ss_jsr(obj, Keys::FFTMin, 0));
  d.fftMax            = SerialStudio::toDouble(ss_jsr(obj, Keys::FFTMax, 0));
  d.pltMin            = SerialStudio::toDouble(ss_jsr(obj, Keys::PltMin, 0));
  d.pltMax            = SerialStudio::toDouble(ss_jsr(obj, Keys::PltMax, 0));
  d.wgtMin            = SerialStudio::toDouble(ss_jsr(obj, Keys::WgtMin, 0));
  d.wgtMax            = SerialStudio::toDouble(ss_jsr(obj, Keys::WgtMax, 0));
  d.fftSamples        = ss_jsr(obj, Keys::FFTSamples, -1).toInt();
  d.title             = ss_jsr(obj, Keys::Title, "").toString().simplified();
  d.value             = ss_jsr(obj, Keys::Value, "").toString().simplified();
  d.units             = ss_jsr(obj, Keys::Units, "").toString().simplified();
  d.overviewDisplay   = ss_jsr(obj, Keys::Overview, false).toBool();
  d.ledHigh           = SerialStudio::toDouble(ss_jsr(obj, Keys::LedHigh, 0));
  d.widget            = ss_jsr(obj, Keys::Widget, "").toString().simplified();
  d.fftSamplingRate   = ss_jsr(obj, Keys::FFTSamplingRate, -1).toInt();
  d.displayTickCount  = ss_jsr(obj, Keys::DisplayTickCount, 5).toInt();
  d.displayFormat     = ss_jsr(obj, Keys::DisplayFormat, "0d").toString();
  d.decimalPoints     = ss_jsr(obj, Keys::DecimalPoints, -1).toInt();
  d.sourceId          = ss_jsr(obj, Keys::DatasetSourceId, 0).toInt();
  d.transformCode     = obj.value(Keys::TransformCode).toString();
  d.transformLanguage = ss_jsr(obj, Keys::TransformLanguage, -1).toInt();
  d.virtual_          = ss_jsr(obj, Keys::Virtual, false).toBool();
  d.hideOnDashboard   = ss_jsr(obj, Keys::HideOnDashboard, false).toBool();
  d.uniqueId          = ss_jsr(obj, Keys::UniqueId, -1).toInt();

  if (!d.value.isEmpty())
    d.numericValue = SerialStudio::toDouble(d.value, &d.isNumeric);

  if (!obj.contains(Keys::FFTMin) || !obj.contains(Keys::FFTMax)) {
    d.fftMin = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
    d.fftMax = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  }

  if (!obj.contains(Keys::PltMin) || !obj.contains(Keys::PltMax)) {
    d.pltMin = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
    d.pltMax = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  }

  if (!obj.contains(Keys::WgtMin) || !obj.contains(Keys::WgtMax)) {
    d.wgtMin = SerialStudio::toDouble(ss_jsr(obj, Keys::Min, 0));
    d.wgtMax = SerialStudio::toDouble(ss_jsr(obj, Keys::Max, 0));
  }

  readDatasetAlarmBands(d, obj);
  normalizeDatasetRanges(d);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Data conversion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Encodes an Action's TX payload (text/hex with optional EOL) to the on-wire byte array.
 */
QByteArray DataModel::get_tx_bytes(const Action& action)
{
  QByteArray b;
  const auto enc = static_cast<SerialStudio::TextEncoding>(action.txEncoding);
  if (action.binaryData)
    b = SerialStudio::hexToBytes(action.txData);
  else
    b = SerialStudio::encodeText(SerialStudio::resolveEscapeSequences(action.txData), enc);

  if (!action.eolSequence.isEmpty()) {
    const auto eol = SerialStudio::resolveEscapeSequences(action.eolSequence);
    b.append(action.binaryData ? eol.toUtf8() : SerialStudio::encodeText(eol, enc));
  }

  return b;
}
