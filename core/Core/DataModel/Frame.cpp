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

#include "Core/DataModel/Frame.h"

#include <algorithm>
#include <cmath>
#include <QDebug>
#include <utility>

#include "Core/AppInfo.h"
#include "Core/DataModel/FrameSupport.h"
#include "Core/DataModel/PropertyValidators.h"
#include "Core/IO/FrameConfig.h"
#include "Core/SerialStudio.h"

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

/**
 * @brief Returns the Fixed Length framing settings of a source, clamping each byte and masking
 *        each value.
 */
DataModel::FixedFrameSettings DataModel::fixed_frame_settings(const Source& source)
{
  auto byteOf = [](int value) {
    return static_cast<quint8>(qBound(0, value, 255));
  };

  FixedFrameSettings out;
  out.length      = source.frameLength;
  out.headerMask  = byteOf(source.frameHeaderMask);
  out.headerValue = byteOf(source.frameHeaderValue) & out.headerMask;
  out.bodyMask    = byteOf(source.frameBodyMask);
  out.bodyValue   = byteOf(source.frameBodyValue) & out.bodyMask;
  return out;
}

/**
 * @brief Reads the Fixed Length framing settings from JSON through fixed_frame_settings().
 */
void DataModel::read_fixed_frame_settings(FixedFrameSettings& out, const QJsonObject& obj)
{
  Source source;
  source.frameLength      = ss_jsr(obj, Keys::FrameLength, 0).toInt();
  source.frameHeaderMask  = ss_jsr(obj, Keys::FrameHeaderMask, 0).toInt();
  source.frameHeaderValue = ss_jsr(obj, Keys::FrameHeaderValue, 0).toInt();
  source.frameBodyMask    = ss_jsr(obj, Keys::FrameBodyMask, 0).toInt();
  source.frameBodyValue   = ss_jsr(obj, Keys::FrameBodyValue, 0).toInt();
  out                     = fixed_frame_settings(source);
}

/**
 * @brief Validates and applies the Fixed Length keys of an API params object to a Source.
 */
bool DataModel::patch_fixed_frame_fields(Source& source,
                                         const QJsonObject& params,
                                         QString& error,
                                         bool& touched)
{
  struct Field {
    Keys::KeyView key;
    int* target;
    int max;
  };

  const Field fields[] = {
    {     Keys::FrameLength,      &source.frameLength, IO::kMaxFixedFrameLength},
    { Keys::FrameHeaderMask,  &source.frameHeaderMask,                      255},
    {Keys::FrameHeaderValue, &source.frameHeaderValue,                      255},
    {   Keys::FrameBodyMask,    &source.frameBodyMask,                      255},
    {  Keys::FrameBodyValue,   &source.frameBodyValue,                      255},
  };

  touched = false;
  for (const auto& field : fields) {
    if (!params.contains(field.key))
      continue;

    const int min   = (field.key == Keys::FrameLength) ? 1 : 0;
    const auto json = params.value(field.key);
    if (!json.isDouble() || json.toInt(-1) < min || json.toInt(-1) > field.max) {
      error = QStringLiteral("%1 must be an integer from %2 to %3")
                .arg(QString(field.key))
                .arg(min)
                .arg(field.max);
      return false;
    }

    *field.target = json.toInt();
    touched       = true;
  }

  source.frameHeaderValue &= source.frameHeaderMask;
  source.frameBodyValue   &= source.frameBodyMask;
  return true;
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
 * @brief Serializes an AlarmBand to a QJsonObject; optional fields are omitted when empty.
 */
QJsonObject DataModel::serialize(const AlarmBand& b)
{
  QJsonObject obj;
  obj.insert(Keys::Min, qMin(b.min, b.max));
  obj.insert(Keys::Max, qMax(b.min, b.max));
  obj.insert(Keys::Severity, static_cast<int>(b.severity));
  if (b.blink)
    obj.insert(Keys::Blink, b.blink);

  if (!b.color.isEmpty())
    obj.insert(Keys::Color, b.color);

  if (!b.label.isEmpty())
    obj.insert(Keys::Label, b.label);

  if (!b.sound.isEmpty())
    obj.insert(Keys::Sound, b.sound);

  return obj;
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
  b.sound       = ss_jsr(obj, Keys::Sound, "").toString().trimmed();
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

  if (!PropertyHooks::isValidColor(m.color))
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

/**
 * @brief Populates @p d.transformParams from @p obj. Only numbers, strings and booleans have a
 *        script-side shape, so a nested object or array is dropped with one warning rather than
 *        reaching a transform as an opaque value.
 */
void DataModel::readDatasetTransformParams(Dataset& d, const QJsonObject& obj)
{
  d.transformParams.clear();
  if (!obj.contains(Keys::TransformParams))
    return;

  const auto params = obj.value(Keys::TransformParams).toObject();
  for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    const auto& value = it.value();
    SS_ASSERT_LOG(!it.key().isEmpty());
    if (value.isDouble() || value.isString() || value.isBool())
      d.transformParams.insert(it.key(), value.toVariant());
    else
      qWarning() << "[Frame] Dataset" << d.uniqueId << "transform parameter" << it.key()
                 << "dropped: only numbers, strings and booleans are supported";
  }
}

/**
 * @brief Writes @p d.transformParams into @p obj as one JSON object when the dataset has any.
 */
void DataModel::writeDatasetTransformParams(QJsonObject& obj, const Dataset& d)
{
  if (d.transformParams.isEmpty())
    return;

  obj.insert(Keys::TransformParams, QJsonObject::fromVariantMap(d.transformParams));
}

/**
 * @brief Populates @p d.labels from @p obj (spec 0108). A row whose value is not an integer an
 *        IEEE double holds exactly, or whose text is empty, is dropped; the table is sorted by
 *        value for the presentation lookup's binary search, and the first row of a key wins.
 */
void DataModel::readDatasetLabels(Dataset& d, const QJsonObject& obj)
{
  d.labels.clear();
  if (!obj.contains(Keys::Labels))
    return;

  constexpr double kExactIntegerLimit = 9007199254740992.0;
  const auto arr                      = obj.value(Keys::Labels).toArray();
  d.labels.reserve(arr.size());
  for (const auto& v : arr) {
    const auto row   = v.toObject();
    bool ok          = false;
    const double key = SerialStudio::toDouble(row.value(Keys::Value), &ok);
    const auto text  = row.value(Keys::LabelText).toString().simplified();
    if (!ok || text.isEmpty() || std::abs(key) > kExactIntegerLimit || key != std::trunc(key))
      continue;

    d.labels.push_back(ValueLabel{static_cast<qint64>(key), text});
  }

  const auto byValue = [](const ValueLabel& a, const ValueLabel& b) {
    return a.value < b.value;
  };
  const auto sameKey = [](const ValueLabel& a, const ValueLabel& b) {
    return a.value == b.value;
  };
  std::stable_sort(d.labels.begin(), d.labels.end(), byValue);
  d.labels.erase(std::unique(d.labels.begin(), d.labels.end(), sameKey), d.labels.end());
  SS_ASSERT_LOG(std::is_sorted(d.labels.begin(), d.labels.end(), byValue));
}

/**
 * @brief Writes @p d.labels into @p obj as one JSON array when the dataset has any.
 */
void DataModel::writeDatasetLabels(QJsonObject& obj, const Dataset& d)
{
  if (d.labels.empty())
    return;

  QJsonArray rows;
  for (const auto& label : d.labels) {
    QJsonObject row;
    row.insert(Keys::Value, label.value);
    row.insert(Keys::LabelText, label.text);
    rows.append(row);
  }

  obj.insert(Keys::Labels, rows);
}
