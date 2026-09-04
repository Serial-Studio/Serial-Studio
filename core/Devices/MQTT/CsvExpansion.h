/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <cstddef>
#  include <map>
#  include <QByteArray>
#  include <QMap>
#  include <QString>
#  include <vector>

#  include "DataModel/DataBlock.h"
#  include "DataModel/ExportSchema.h"
#  include "DataModel/Frame.h"

namespace MQTT {

/**
 * @brief Payload assembly for the publisher's dashboard modes: the RFC 4180 field escape, the
 *        column label both the CSV header and the Sparkplug registry publish a dataset under, and
 *        the block-to-frame materialisation the JSON and CSV batches share. Free functions over
 *        their arguments alone -- no broker, no client, no worker state -- so the whole transform
 *        is provable in the ctest tier without an MQTT connection.
 */
[[nodiscard]] QString escapeCsvField(const QString& field);
[[nodiscard]] QString csvColumnLabel(const DataModel::ExportColumn& column);
[[nodiscard]] QByteArray buildCsvHeader(const DataModel::ExportSchema& schema);

void appendCsvRow(QByteArray& out,
                  const DataModel::ExportSchema& schema,
                  const QMap<int, QString>& valuesByUniqueId);

void expandBlocks(const std::vector<DataModel::DataBlockPtr>& blocks,
                  std::map<int, DataModel::FrameTemplate>& templates,
                  std::size_t maxSamples,
                  std::vector<DataModel::Frame>& out);

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
