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

#pragma once

#include <QByteArray>
#include <QList>
#include <QString>

#include "SerialStudio.h"

namespace IO {

/**
 * @brief Plain aggregate carrying all FrameReader configuration parameters.
 *
 * DeviceManager constructs one of these from the project/source settings and
 * passes it to the FrameReader before moving it to its worker thread. All
 * members have sensible defaults so callers only need to override what they
 * care about.
 *
 * Start and finish sequences are lists so a single config can express
 * multiple valid delimiters (e.g., QuickPlot accepts CR, LF, or CRLF).
 * For single-delimiter projects the list contains exactly one element.
 */
struct FrameConfig {
  QList<QByteArray> startSequences{QByteArray("/*")};
  QList<QByteArray> finishSequences{QByteArray("*/")};
  QString checksumAlgorithm;
  SerialStudio::OperationMode operationMode   = SerialStudio::QuickPlot;
  SerialStudio::FrameDetection frameDetection = SerialStudio::EndDelimiterOnly;
};

}  // namespace IO
