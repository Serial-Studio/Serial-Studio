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

#include "Core/IO/FrameConfigBuilder.h"

#include <QDebug>

#include "Core/Bus/Messages.h"
#include "Core/DataModel/Frame.h"
#include "Core/SSAssert.h"

/**
 * @brief Repairs a frame-detection mode whose settings the source does not carry, so a reader
 *        never waits for a delimiter or a frame length that cannot arrive.
 */
SerialStudio::FrameDetection IO::FrameConfigBuilder::repairedDetection(const FrameConfig& cfg)
{
  auto detection = cfg.frameDetection;
  if (detection == SerialStudio::FixedLength
      && (cfg.fixedFrameLength < 1 || cfg.fixedFrameLength > kMaxFixedFrameLength)) [[unlikely]] {
    qWarning() << "Fixed Length framing needs a length from 1 to" << kMaxFixedFrameLength
               << "bytes, got" << cfg.fixedFrameLength << "- reading the raw stream instead";
    return SerialStudio::NoDelimiters;
  }

  if ((detection == SerialStudio::StartDelimiterOnly
       || detection == SerialStudio::StartAndEndDelimiter)
      && cfg.startSequences.isEmpty()) [[unlikely]]
    detection =
      cfg.finishSequences.isEmpty() ? SerialStudio::NoDelimiters : SerialStudio::EndDelimiterOnly;

  if (detection == SerialStudio::EndDelimiterOnly && cfg.finishSequences.isEmpty()) [[unlikely]]
    detection = SerialStudio::NoDelimiters;

  return detection;
}

/**
 * @brief Builds the reader configuration for @p deviceId from the retained project structure and
 *        operation mode (spec 0077): Quick Plot and Console Only sessions use source 0's framing
 *        as published, a project source its own framing settings, and a source the project does
 *        not list the project-wide defaults.
 */
IO::FrameConfig IO::FrameConfigBuilder::build(const Core::Bus::ProjectStructureSnapshot& project,
                                              const SerialStudio::OperationMode mode,
                                              const FrameConfig& source0Config,
                                              const int deviceId)
{
  SS_ASSERT_LOG(deviceId >= 0);

  if (mode == SerialStudio::QuickPlot || mode == SerialStudio::ConsoleOnly)
    return source0Config;

  FrameConfig cfg;
  cfg.operationMode = mode;

  for (const auto& src : project.sources) {
    if (src.sourceId != deviceId)
      continue;

    QByteArray start, end;
    QString checksum;
    DataModel::read_io_settings(start, end, checksum, DataModel::serialize(src));

    cfg.startSequences    = start.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{start};
    cfg.finishSequences   = end.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{end};
    cfg.checksumAlgorithm = checksum;
    cfg.frameDetection    = static_cast<SerialStudio::FrameDetection>(src.frameDetection);

    const auto fixed     = DataModel::fixed_frame_settings(src);
    cfg.fixedFrameLength = fixed.length;
    cfg.headerMask       = fixed.headerMask;
    cfg.headerValue      = fixed.headerValue;
    cfg.bodyMask         = fixed.bodyMask;
    cfg.bodyValue        = fixed.bodyValue;

    cfg.frameDetection = repairedDetection(cfg);
    return cfg;
  }

  SS_ASSERT_LOG(cfg.operationMode == mode);

  cfg.startSequences    = {QByteArray("/*")};
  cfg.finishSequences   = {QByteArray("*/")};
  cfg.checksumAlgorithm = QString();
  cfg.frameDetection    = static_cast<SerialStudio::FrameDetection>(project.frameDetection);
  return cfg;
}
