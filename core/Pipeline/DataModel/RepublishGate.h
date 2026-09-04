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

#include <QSet>

namespace DataModel {

/**
 * @brief Per-source bookkeeping for the two synthetic-refresh lanes (spec 0064): the dashboard
 *        lane and the export lane must not share one "already republished" mark, or a masked
 *        refresh consuming the change-driven clock leaves every recording a publish behind.
 */
class RepublishGate {
public:
  /**
   * @brief Drops every mark; a new session owes both lanes a first publish again.
   */
  void clear() noexcept
  {
    m_published.clear();
    m_sinkDirty.clear();
  }

  /**
   * @brief Marks @p key's values newer than anything the recording sinks hold. Every lane calls
   *        this, masked included: a masked pass is precisely what leaves the sinks stale.
   */
  void noteChanged(int key) { m_sinkDirty.insert(key); }

  /**
   * @brief Suppresses the first synthetic publish after a template went out on its own.
   */
  void notePublishedTemplate(int key) { m_published.insert(key); }

  /**
   * @brief Whether this lane still owes @p key a publish. The export lane asks whether the SINKS
   *        are behind; the dashboard lane keeps the cheaper "changed, or never published" rule.
   */
  [[nodiscard]] bool needed(int key, bool changed, bool feedExports) const
  {
    if (feedExports)
      return m_sinkDirty.contains(key) || !m_published.contains(key);

    return changed || !m_published.contains(key);
  }

  /**
   * @brief Records a completed publish. Only an export publish clears the sink-dirty mark.
   */
  void notePublished(int key, bool feedExports)
  {
    m_published.insert(key);
    if (feedExports)
      m_sinkDirty.remove(key);
  }

  /**
   * @brief Whether the recording sinks are behind @p key's current values.
   */
  [[nodiscard]] bool sinkDirty(int key) const { return m_sinkDirty.contains(key); }

private:
  QSet<int> m_published;
  QSet<int> m_sinkDirty;
};

}  // namespace DataModel
