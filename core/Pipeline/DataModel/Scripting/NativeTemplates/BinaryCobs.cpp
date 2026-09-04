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

#include "DataModel/Scripting/NativeTemplates/BinaryCobs.h"

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// COBS-encoded frames
//--------------------------------------------------------------------------------------------------

/**
 * @brief COBS decoder emitting one decimal channel per decoded byte.
 */
class CobsParser final : public INativeParser {
public:
  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseBinary(frame.toUtf8());
  }

  /**
   * @brief UTF-8 text frames already carry the raw bytes; skips the QString round-trip.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    return parseBinary(frame);
  }

  /**
   * @brief Decodes the COBS framing and emits the recovered bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    SS_ASSERT(!frame.isEmpty(), return {});

    QByteArray decoded;
    decoded.reserve(frame.size());

    qsizetype i          = 0;
    const qsizetype size = qMin<qsizetype>(frame.size(), kMaxBytesPerFrame);
    while (i < size) {
      const int code = u8At(frame, i++);
      if (code == 0)
        break;

      for (int j = 1; j < code && i < size; ++j)
        decoded.append(static_cast<char>(u8At(frame, i++)));

      if (code < 0xFF && i < size)
        decoded.append('\0');
    }

    return byteRowFrame(decoded);
  }
};

/**
 * @brief Descriptor for the COBS template.
 */
class CobsTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("cobs_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("COBS-encoded frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Removes Consistent Overhead Byte Stuffing and emits the decoded "
                            "bytes as channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override { return {}; }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(params)
    Q_UNUSED(error)
    return std::make_unique<CobsParser>();
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide COBS template descriptor.
 */
const DataModel::INativeTemplate& DataModel::cobsTemplate()
{
  static const CobsTemplate s_cobs;
  return s_cobs;
}
