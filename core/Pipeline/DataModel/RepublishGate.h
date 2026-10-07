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

class RepublishGate;

/**
 * @brief One synthetic-refresh lane's view of the gate. Its identity (dashboard-only or feeding
 *        the recording sinks) is fixed when the gate hands it out, so one pass can never ask with
 *        one lane and record with the other: the mix-up spec 0064 shipped is unrepresentable.
 */
class RepublishLane {
public:
  [[nodiscard]] bool feedsExports() const noexcept { return m_feedsExports; }

  [[nodiscard]] bool needed(int key, bool changed) const;
  void notePublished(int key);

private:
  friend class RepublishGate;

  RepublishLane(RepublishGate& gate, bool feedsExports) noexcept
    : m_gate(gate), m_feedsExports(feedsExports)
  {}

  RepublishGate& m_gate;
  bool m_feedsExports;
};

/**
 * @brief Per-source bookkeeping for the two synthetic-refresh lanes (spec 0064): the dashboard
 *        lane and the export lane must not share one "already republished" mark, or a masked
 *        refresh consuming the change-driven clock leaves every recording a publish behind.
 */
class RepublishGate {
public:
  /**
   * @brief The masked lane: republishes to the dashboard only, never discharging the sinks.
   */
  [[nodiscard]] RepublishLane dashboardLane() noexcept { return RepublishLane(*this, false); }

  /**
   * @brief The export lane: the only lane whose publish brings the recording sinks current.
   */
  [[nodiscard]] RepublishLane exportLane() noexcept { return RepublishLane(*this, true); }

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
   * @brief Whether the recording sinks are behind @p key's current values.
   */
  [[nodiscard]] bool sinkDirty(int key) const { return m_sinkDirty.contains(key); }

private:
  friend class RepublishLane;

  /**
   * @brief Whether a lane still owes @p key a publish. The export lane asks whether the SINKS are
   *        behind; the dashboard lane keeps the cheaper "changed, or never published" rule.
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

private:
  QSet<int> m_published;
  QSet<int> m_sinkDirty;
};

/**
 * @brief Whether this lane still owes @p key a publish.
 */
inline bool RepublishLane::needed(int key, bool changed) const
{
  return m_gate.needed(key, changed, m_feedsExports);
}

/**
 * @brief Records this lane's completed publish of @p key.
 */
inline void RepublishLane::notePublished(int key)
{
  m_gate.notePublished(key, m_feedsExports);
}

}  // namespace DataModel
