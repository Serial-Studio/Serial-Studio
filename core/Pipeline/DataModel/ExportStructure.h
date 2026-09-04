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

#pragma once

#include <QDir>
#include <QString>

#include "DataModel/Frame.h"

namespace DataModel {

/**
 * @brief The schema half of an export worker, shared by the CSV, MDF4 and Sessions lanes (spec
 *        0075, B11): the template frame a file's columns are created from, the two ways it is
 *        adopted, and the path rules that decide where the session's files land. Owned by each
 *        worker and touched only on that worker's thread.
 */
class ExportStructure {
public:
  ExportStructure() = default;

  void clear();
  void setTemplateFrame(const Frame& frame);
  void applyPublishedStructure(const Frame& frame, bool resourceOpen);

  [[nodiscard]] bool hasStructure() const noexcept;
  [[nodiscard]] const Frame& templateFrame() const noexcept;

  [[nodiscard]] static QString sanitizeTitle(const QString& title, const QString& fallback);
  [[nodiscard]] static QDir sessionDir(const QString& workspaceKey,
                                       const QString& title,
                                       const QString& fallback);

private:
  Frame m_templateFrame;
};

}  // namespace DataModel
