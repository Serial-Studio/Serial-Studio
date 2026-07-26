/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QStringConverter>
#include <QtCore5Compat/QTextCodec>

#include "SerialStudio.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// The SerialStudio statics that DataModel/Frame.cpp reaches, kept apart from SerialStudio.cpp so
// serialization links without the icon registry, theme manager and player singletons (spec 0032).
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Commercial feature detection, appreciate your respect for this project
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if a transform script references the notify() API family.
 */
static bool transformUsesNotifications(const QString& code)
{
  return code.contains(QStringLiteral("notify(")) || code.contains(QStringLiteral("notifyInfo("))
      || code.contains(QStringLiteral("notifyWarning("))
      || code.contains(QStringLiteral("notifyCritical("))
      || code.contains(QStringLiteral("notifyClear("));
}

/**
 * @brief Checks if a project configuration (QVector form) requires commercial features.
 */
bool SerialStudio::commercialCfg(const QVector<DataModel::Group>& g)
{
  for (const auto& group : std::as_const(g)) {
    if (group.groupType == DataModel::GroupType::Output)
      return true;

    if (group.widget == QStringLiteral("plot3d"))
      return true;

    if (group.widget == QStringLiteral("image"))
      return true;

    if (group.widget == QStringLiteral("painter"))
      return true;

    for (const auto& dataset : std::as_const((group.datasets))) {
      if (dataset.waterfall)
        return true;

      if (!dataset.transformCode.isEmpty() && transformUsesNotifications(dataset.transformCode))
        return true;
    }
  }

  return false;
}

/**
 * @brief Checks if a project configuration requires commercial features.
 */
bool SerialStudio::commercialCfg(const std::vector<DataModel::Group>& g)
{
  for (const auto& group : std::as_const(g)) {
    if (group.groupType == DataModel::GroupType::Output)
      return true;

    if (group.widget == QStringLiteral("plot3d"))
      return true;

    if (group.widget == QStringLiteral("image"))
      return true;

    if (group.widget == QStringLiteral("painter"))
      return true;

    for (const auto& dataset : std::as_const((group.datasets))) {
      if (dataset.waterfall)
        return true;

      if (!dataset.transformCode.isEmpty() && transformUsesNotifications(dataset.transformCode))
        return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// String processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a hexadecimal string into a raw QByteArray.
 */
QByteArray SerialStudio::hexToBytes(const QString& data)
{
  QString withoutSpaces = data;
  withoutSpaces.replace(QStringLiteral(" "), "");
  if (withoutSpaces.length() % 2 != 0) {
    qWarning() << data << "is not a valid hexadecimal array";
    return QByteArray();
  }

  bool ok;
  QByteArray array;
  for (int i = 0; i < withoutSpaces.length(); i += 2) {
    auto chr1       = withoutSpaces.at(i);
    auto chr2       = withoutSpaces.at(i + 1);
    QString byteStr = QStringLiteral("%1%2").arg(chr1, chr2);

    int byte = byteStr.toInt(&ok, 16);
    if (!ok) {
      qWarning() << data << "is not a valid hexadecimal array";
      return QByteArray();
    }

    array.append(static_cast<char>(byte));
  }

  return array;
}

/**
 * @brief Resolves C-style escape sequences in a string into their corresponding control characters.
 */
QString SerialStudio::resolveEscapeSequences(const QString& str)
{
  QString escapedStr;
  escapedStr.reserve(str.size());

  for (int i = 0; i < str.size(); ++i) {
    const QChar current = str.at(i);
    if (current != u'\\' || i + 1 >= str.size()) {
      escapedStr.append(current);
      continue;
    }

    const QChar next = str.at(i + 1);
    ++i;
    switch (next.unicode()) {
      case u'a':
        escapedStr.append(QChar(u'\a'));
        break;
      case u'b':
        escapedStr.append(QChar(u'\b'));
        break;
      case u'f':
        escapedStr.append(QChar(u'\f'));
        break;
      case u'n':
        escapedStr.append(QChar(u'\n'));
        break;
      case u'r':
        escapedStr.append(QChar(u'\r'));
        break;
      case u't':
        escapedStr.append(QChar(u'\t'));
        break;
      case u'v':
        escapedStr.append(QChar(u'\v'));
        break;
      case u'\\':
        escapedStr.append(QChar(u'\\'));
        break;
      default:
        escapedStr.append(current).append(next);
        break;
    }
  }

  return escapedStr;
}

//--------------------------------------------------------------------------------------------------
// Text encoding helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the QStringConverter encoding for natively-supported entries.
 */
static std::optional<QStringConverter::Encoding> nativeEncoding(SerialStudio::TextEncoding enc)
{
  switch (enc) {
    case SerialStudio::EncUtf8:
      return QStringConverter::Utf8;
    case SerialStudio::EncUtf16LE:
      return QStringConverter::Utf16LE;
    case SerialStudio::EncUtf16BE:
      return QStringConverter::Utf16BE;
    case SerialStudio::EncLatin1:
      return QStringConverter::Latin1;
    case SerialStudio::EncSystem:
      return QStringConverter::System;
    default:
      return std::nullopt;
  }
}

/**
 * @brief Returns the `QTextCodec` for multi-byte East-Asian encodings.
 */
static QTextCodec* legacyCodec(SerialStudio::TextEncoding enc)
{
  const char* name = nullptr;
  switch (enc) {
    case SerialStudio::EncGbk:
      name = "GBK";
      break;
    case SerialStudio::EncGb18030:
      name = "GB18030";
      break;
    case SerialStudio::EncBig5:
      name = "Big5";
      break;
    case SerialStudio::EncShiftJis:
      name = "Shift_JIS";
      break;
    case SerialStudio::EncEucJp:
      name = "EUC-JP";
      break;
    case SerialStudio::EncEucKr:
      name = "EUC-KR";
      break;
    default:
      break;
  }

  QTextCodec* codec = name ? QTextCodec::codecForName(name) : nullptr;
  if (!codec)
    codec = QTextCodec::codecForName("UTF-8");

  SS_ASSERT_LOG(codec != nullptr);
  return codec;
}

/**
 * @brief Encodes a QString to raw bytes using the given text encoding.
 */
QByteArray SerialStudio::encodeText(const QString& text, SerialStudio::TextEncoding enc)
{
  if (text.isEmpty())
    return {};

  if (const auto native = nativeEncoding(enc); native.has_value()) {
    QStringEncoder encoder(*native);
    return QByteArray(encoder.encode(text));
  }

  auto* codec = legacyCodec(enc);
  SS_ASSERT(codec != nullptr, return {});
  return codec->fromUnicode(text);
}

/**
 * @brief Decodes raw bytes to a QString using the given text encoding.
 */
QString SerialStudio::decodeText(QByteArrayView bytes, SerialStudio::TextEncoding enc)
{
  if (bytes.isEmpty())
    return {};

  if (const auto native = nativeEncoding(enc); native.has_value()) {
    QStringDecoder decoder(*native);
    return decoder.decode(bytes);
  }

  auto* codec = legacyCodec(enc);
  SS_ASSERT(codec != nullptr, return {});
  return codec->toUnicode(bytes.constData(), static_cast<int>(bytes.size()));
}
