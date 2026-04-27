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
  // Detect commercial features and assign stable unique IDs to each dataset
  frame.containsCommercialFeatures = SerialStudio::commercialCfg(frame.groups);

  for (auto& group : frame.groups) {
    for (auto& dataset : group.datasets) {
      dataset.sourceId = group.sourceId;
      dataset.uniqueId = group.sourceId * 1000000 + dataset.groupId * 10000 + dataset.datasetId;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Configuration reading
//--------------------------------------------------------------------------------------------------

void DataModel::read_io_settings(QByteArray& frameStart,
                                 QByteArray& frameEnd,
                                 QString& checksum,
                                 const QJsonObject& obj)
{
  // Obtain frame delimiters
  auto fEndStr   = ss_jsr(obj, Keys::FrameEnd, "").toString();
  auto fStartStr = ss_jsr(obj, Keys::FrameStart, "").toString();
  auto isHex     = ss_jsr(obj, Keys::HexadecimalDelimiters, false).toBool();

  // Read checksum method, preferring the canonical "checksumAlgorithm" key and
  // falling back to legacy "checksum" written by older Serial Studio versions
  if (obj.contains(Keys::ChecksumAlgorithm))
    checksum = obj.value(Keys::ChecksumAlgorithm).toString();
  else
    checksum = ss_jsr(obj, Keys::Checksum, "").toString();

  // Convert hex + escape strings (e.g. "0A 0D", or "\\n0D") to raw bytes
  if (isHex) {
    QString resolvedEnd   = SerialStudio::resolveEscapeSequences(fEndStr);
    QString resolvedStart = SerialStudio::resolveEscapeSequences(fStartStr);
    frameStart            = QByteArray::fromHex(resolvedStart.remove(' ').toUtf8());
    frameEnd              = QByteArray::fromHex(resolvedEnd.remove(' ').toUtf8());
  }

  // Resolve escape sequences (e.g. "\\n") and encode to UTF-8 bytes
  else {
    frameEnd   = SerialStudio::resolveEscapeSequences(fEndStr).toUtf8();
    frameStart = SerialStudio::resolveEscapeSequences(fStartStr).toUtf8();
  }
}

//--------------------------------------------------------------------------------------------------
// Data conversion
//--------------------------------------------------------------------------------------------------

QByteArray DataModel::get_tx_bytes(const Action& action)
{
  // Convert action payload to bytes: binary hex or escaped text in the
  // user-selected encoding (defaults to UTF-8 for back-compat)
  const auto enc = static_cast<SerialStudio::TextEncoding>(action.txEncoding);

  QByteArray b;
  if (action.binaryData)
    b = SerialStudio::hexToBytes(action.txData);
  else
    b = SerialStudio::encodeText(SerialStudio::resolveEscapeSequences(action.txData), enc);

  // Append the EOL sequence using the same encoding so multi-byte CR/LF
  // representations line up with the rest of the payload
  if (!action.eolSequence.isEmpty()) {
    const auto eol = SerialStudio::resolveEscapeSequences(action.eolSequence);
    b.append(action.binaryData ? eol.toUtf8() : SerialStudio::encodeText(eol, enc));
  }

  return b;
}
