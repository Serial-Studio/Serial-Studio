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

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

#include <QCoreApplication>

#include "SerialStudio.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Translation context
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated UI string for the shared native-template context.
 */
QString DataModel::TemplateSupport::trNativeTemplate(const char* text)
{
  return QCoreApplication::translate("NativeTemplates", text);
}

//--------------------------------------------------------------------------------------------------
// Byte-frame helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts every byte into one decimal channel value.
 */
QList<QStringList> DataModel::TemplateSupport::byteRowFrame(const QByteArray& bytes)
{
  SS_ASSERT_LOG(bytes.size() <= kMaxBytesPerFrame);

  QStringList row;
  row.reserve(bytes.size());
  const qsizetype count = qMin<qsizetype>(bytes.size(), kMaxBytesPerFrame);
  for (qsizetype i = 0; i < count; ++i)
    row.append(QString::number(u8At(bytes, i)));

  QList<QStringList> out;
  out.append(std::move(row));
  return out;
}

/**
 * @brief Combines one byte group at the offset into a decimal value string.
 */
QString DataModel::TemplateSupport::byteGroupValue(
  const QByteArray& bytes, qsizetype offset, int bytesPerValue, bool bigEndian, bool signedValues)
{
  SS_ASSERT(bytesPerValue >= 1, bytesPerValue = 1);
  SS_ASSERT(bytesPerValue <= 8, bytesPerValue = 8);
  SS_ASSERT(offset + bytesPerValue <= bytes.size(), return QStringLiteral("0"));

  quint64 value = 0;
  for (int b = 0; b < bytesPerValue; ++b) {
    const qsizetype idx = bigEndian ? (offset + b) : (offset + bytesPerValue - 1 - b);
    value               = (value << 8) | u8At(bytes, idx);
  }

  if (!signedValues)
    return QString::number(value);

  const int bits         = bytesPerValue * 8;
  const quint64 sign_bit = quint64(1) << (bits - 1);
  if (bits < 64 && (value & sign_bit))
    return QString::number(static_cast<qint64>(value) - (qint64(1) << bits));

  return QString::number(static_cast<qint64>(value));
}

/**
 * @brief Groups frame bytes into one channel per group; trailing partial groups are dropped.
 */
QList<QStringList> DataModel::TemplateSupport::groupedByteFrame(const QByteArray& bytes,
                                                                int bytesPerValue,
                                                                bool bigEndian,
                                                                bool signedValues)
{
  if (bytesPerValue < 1 || bytesPerValue > 8)
    return {};

  QStringList row;
  row.reserve(bytes.size() / bytesPerValue + 1);

  const qsizetype count = qMin<qsizetype>(bytes.size(), kMaxBytesPerFrame);
  for (qsizetype i = 0; i + bytesPerValue <= count; i += bytesPerValue)
    row.append(byteGroupValue(bytes, i, bytesPerValue, bigEndian, signedValues));

  QList<QStringList> out;
  out.append(std::move(row));
  return out;
}

//--------------------------------------------------------------------------------------------------
// Text-frame helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps a single channel row as the one-frame multi-frame result.
 */
QList<QStringList> DataModel::TemplateSupport::singleFrame(QStringList&& row)
{
  QList<QStringList> out;
  out.append(std::move(row));
  return out;
}

/**
 * @brief Builds the key -> channel-index lookup shared by the latching key/value templates.
 */
QHash<QString, int> DataModel::TemplateSupport::buildKeyIndex(const QStringList& keys)
{
  QHash<QString, int> index;
  index.reserve(keys.size());
  for (qsizetype i = 0; i < keys.size(); ++i)
    index.insert(keys.at(i), static_cast<int>(i));

  return index;
}

/**
 * @brief Converts a scalar JSON value to its channel string ("0" when absent or non-scalar).
 */
QString DataModel::TemplateSupport::jsonScalar(const QJsonValue& value)
{
  if (value.isDouble())
    return QString::number(SerialStudio::toDouble(value));

  if (value.isString())
    return value.toString();

  if (value.isBool())
    return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");

  return QStringLiteral("0");
}
