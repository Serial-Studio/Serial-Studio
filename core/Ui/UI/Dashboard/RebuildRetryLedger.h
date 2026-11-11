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

#include <QHash>
#include <QtGlobal>

#include "Core/SSAssert.h"

namespace UI {

/**
 * @brief What the dashboard does about a block that does not match its widget model.
 */
enum class RebuildVerdict : quint8 {
  Rebuild,
  Quarantine,
  Drop,
};

/**
 * @brief Per-source rebuild bookkeeping (spec 0119): one rebuild per (source, structure
 * generation), then that pair is quarantined. Tracked per source, so a healthy source's blocks can
 * never re-arm another's retry and turn one bad layout into an endless rebuild loop.
 */
class RebuildRetryLedger {
public:
  /**
   * @brief Records a mismatch for @p sourceId and answers what to do. A generation other than the
   *        recorded one earns a fresh rebuild.
   */
  [[nodiscard]] RebuildVerdict onMismatch(int sourceId, quint64 generation)
  {
    SS_ASSERT(sourceId >= 0, return RebuildVerdict::Drop);

    const auto it = m_entries.find(sourceId);
    if (it == m_entries.end() || it->generation != generation) {
      m_entries.insert(sourceId, Entry{generation, false});
      return RebuildVerdict::Rebuild;
    }

    if (it->quarantined)
      return RebuildVerdict::Drop;

    it->quarantined = true;
    return RebuildVerdict::Quarantine;
  }

  /**
   * @brief Forgets every retry and quarantine. Hard resets only: a rebuild must never call this, or
   *        another source's layout arriving re-arms a quarantined source.
   */
  void clear() noexcept { m_entries.clear(); }

private:
  struct Entry {
    quint64 generation;
    bool quarantined;
  };

private:
  QHash<int, Entry> m_entries;
};

}  // namespace UI
