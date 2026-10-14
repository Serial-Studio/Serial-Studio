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

#include <QtGlobal>

namespace Widgets {
/**
 * @brief Where a hidden terminal stopped formatting console output (spec 0097 R12/R13): the
 *        console's display mark at the first chunk it skipped. On show the facade asks the
 *        console for everything since that mark and either continues from it or rebuilds.
 */
class TerminalCatchUp {
public:
  TerminalCatchUp();

  [[nodiscard]] bool pending() const noexcept;
  [[nodiscard]] qint64 take() noexcept;

  void reset() noexcept;
  void skip(qint64 markAfter, qsizetype size) noexcept;

private:
  qint64 m_mark;
  bool m_pending;
};
}  // namespace Widgets
