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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Frame.h"

#include <QColor>

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

//--------------------------------------------------------------------------------------------------
// Table folder paths (single source of truth for the store key + script accessor)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Walks a table folder's parent chain to build its "/"-joined path (root -> leaf).
 */
QString DataModel::tableFolderPath(const std::vector<TableFolder>& folders, int parentFolderId)
{
  if (parentFolderId == -1)
    return QString();

  QString path;
  const int kMax = static_cast<int>(folders.size());
  int id         = parentFolderId;
  for (int i = 0; i <= kMax && id != -1; ++i) {
    const TableFolder* match = nullptr;
    for (const auto& f : folders)
      if (f.folderId == id) {
        match = &f;
        break;
      }

    if (!match)
      break;

    path = path.isEmpty() ? match->title : (match->title + QLatin1Char('/') + path);
    id   = match->parentFolderId;
  }

  return path;
}

/**
 * @brief Joins a table's folder path with its leaf name to form the full accessor/store path.
 */
QString DataModel::tableFullPath(const std::vector<TableFolder>& folders,
                                 int parentFolderId,
                                 const QString& name)
{
  const QString folderPath = tableFolderPath(folders, parentFolderId);
  if (folderPath.isEmpty())
    return name;

  return folderPath + QLatin1Char('/') + name;
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
  w.onLabel          = ss_jsr(obj, Keys::OutputOnLabel, "").toString().simplified();
  w.offLabel         = ss_jsr(obj, Keys::OutputOffLabel, "").toString().simplified();
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
 * @brief Deserializes a FrequencyMarker from a QJsonObject; rejects non-positive or absurd
 *        frequencies (ceiling = max possible Nyquist for an int sampling rate), demotes an
 *        inverted band to a point marker, and swaps reversed warn/alarm levels.
 */
bool DataModel::read(FrequencyMarker& m, const QJsonObject& obj)
{
  constexpr double nan        = std::numeric_limits<double>::quiet_NaN();
  constexpr double max_freqHz = 2147483648.0;
  if (obj.isEmpty())
    return false;

  m.frequency    = SerialStudio::toDouble(ss_jsr(obj, Keys::Frequency, 0));
  m.endFrequency = SerialStudio::toDouble(ss_jsr(obj, Keys::EndFrequency, 0));
  m.warningDb =
    obj.contains(Keys::WarningDb) ? SerialStudio::toDouble(obj.value(Keys::WarningDb)) : nan;
  m.alarmDb = obj.contains(Keys::AlarmDb) ? SerialStudio::toDouble(obj.value(Keys::AlarmDb)) : nan;
  m.color   = ss_jsr(obj, Keys::Color, "").toString().simplified();
  m.label   = ss_jsr(obj, Keys::Label, "").toString().simplified();

  if (!std::isfinite(m.frequency) || m.frequency <= 0.0 || m.frequency > max_freqHz)
    return false;

  if (!std::isfinite(m.endFrequency) || m.endFrequency <= m.frequency)
    m.endFrequency = 0.0;
  else
    m.endFrequency = qMin(m.endFrequency, max_freqHz);

  if (std::isfinite(m.warningDb) && std::isfinite(m.alarmDb) && m.warningDb > m.alarmDb)
    std::swap(m.warningDb, m.alarmDb);

  if (!m.color.isEmpty() && !QColor::fromString(m.color).isValid())
    m.color.clear();

  return true;
}

/**
 * @brief Populates @p d.fftMarkers from @p obj, dropping invalid entries.
 */
void DataModel::readDatasetFrequencyMarkers(Dataset& d, const QJsonObject& obj)
{
  d.fftMarkers.clear();
  if (!obj.contains(Keys::FFTMarkers))
    return;

  const auto arr = obj.value(Keys::FFTMarkers).toArray();
  d.fftMarkers.reserve(arr.size());
  for (const auto& v : arr) {
    FrequencyMarker m;
    if (read(m, v.toObject()))
      d.fftMarkers.push_back(std::move(m));
  }
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
