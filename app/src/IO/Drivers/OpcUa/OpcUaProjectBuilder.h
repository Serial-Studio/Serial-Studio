/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>

#include "DataModel/Frame.h"
#include "IO/Drivers/OpcUa/OpcUaTag.h"

namespace IO {
namespace Drivers {

/**
 * @brief Builds a Serial Studio project from a set of OPC UA tags (spec 0066 R9). A pure
 *        translation from tag list to project JSON: it reads the tags it was constructed with and
 *        nothing else, so the connection settings are handed in by the driver. Holding no driver
 *        reference is what makes the wire schema and the group layout assertable from a test.
 */
class OpcUaProjectBuilder {
public:
  explicit OpcUaProjectBuilder(const QList<OpcUaTag>& tags);

  [[nodiscard]] QJsonArray wireSchema() const;
  [[nodiscard]] QJsonObject buildProject(const QJsonObject& connectionSettings) const;

  [[nodiscard]] static DataModel::Dataset datasetFor(const OpcUaTag& tag, int element, int index);

private:
  QList<OpcUaTag> m_tags;
};

}  // namespace Drivers
}  // namespace IO
