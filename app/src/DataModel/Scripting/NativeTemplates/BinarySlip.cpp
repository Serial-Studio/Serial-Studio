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

#include "DataModel/Scripting/NativeTemplates/BinarySlip.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// SLIP-encoded frames
//--------------------------------------------------------------------------------------------------

/**
 * @brief SLIP (RFC 1055) decoder emitting one decimal channel per decoded byte.
 */
class SlipParser final : public INativeParser {
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
   * @brief Resolves SLIP escape sequences and emits the recovered bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    SS_ASSERT(!frame.isEmpty(), return {});

    QByteArray decoded;
    decoded.reserve(frame.size());

    qsizetype i          = 0;
    const qsizetype size = qMin<qsizetype>(frame.size(), kMaxBytesPerFrame);
    while (i < size) {
      const int byte = u8At(frame, i++);
      if (byte == kEnd)
        continue;

      if (byte != kEsc) {
        decoded.append(static_cast<char>(byte));
        continue;
      }

      if (i >= size) {
        decoded.append(static_cast<char>(kEsc));
        continue;
      }

      const int next = u8At(frame, i++);
      if (next == kEscEnd)
        decoded.append(static_cast<char>(kEnd));
      else if (next == kEscEsc)
        decoded.append(static_cast<char>(kEsc));
      else {
        decoded.append(static_cast<char>(kEsc));
        decoded.append(static_cast<char>(next));
      }
    }

    return byteRowFrame(decoded);
  }

private:
  static constexpr int kEnd    = 0xC0;
  static constexpr int kEsc    = 0xDB;
  static constexpr int kEscEnd = 0xDC;
  static constexpr int kEscEsc = 0xDD;
};

/**
 * @brief Descriptor for the SLIP template.
 */
class SlipTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("slip_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("SLIP-encoded frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Resolves SLIP escape sequences (RFC 1055) and emits the decoded "
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
    return std::make_unique<SlipParser>();
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide SLIP template descriptor.
 */
const DataModel::INativeTemplate& DataModel::slipTemplate()
{
  static const SlipTemplate s_slip;
  return s_slip;
}
