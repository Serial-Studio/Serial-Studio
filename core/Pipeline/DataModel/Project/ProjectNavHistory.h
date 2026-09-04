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

#include <functional>
#include <QString>
#include <vector>

namespace DataModel {

/**
 * @brief Back/forward cursor over the Project Editor's visited tree nodes; plain value type (no
 *        QObject, no singleton access, no dependency on the editor's item maps) so the ring
 *        arithmetic can be exercised without a tree. Entry identity is opaque, and resolving an
 *        entry back to a live item stays the editor's job.
 */
class ProjectNavHistory {
public:
  /**
   * @brief Stable logical identity of a visited tree node.
   */
  struct Entry {
    bool valid     = false;
    bool container = false;
    int kind       = 0;
    int view       = 0;
    int id         = -1;
    int parentId   = -1;
    QString key;
  };

  using Resolver = std::function<bool(const Entry&)>;

  ProjectNavHistory();

  [[nodiscard]] static bool sameTarget(const Entry& a, const Entry& b) noexcept;

  [[nodiscard]] bool canGoBack() const noexcept;
  [[nodiscard]] bool canGoForward() const noexcept;

  [[nodiscard]] int cursor() const noexcept;
  [[nodiscard]] int size() const noexcept;
  [[nodiscard]] const Entry& entryAt(int index) const;

  [[nodiscard]] int direction() const noexcept;
  [[nodiscard]] bool navigating() const noexcept;

  [[nodiscard]] bool push(const Entry& entry);
  [[nodiscard]] bool clear();

  [[nodiscard]] int previousResolvable(const Resolver& resolvable) const;
  [[nodiscard]] int nextResolvable(const Resolver& resolvable) const;

  void setCursor(int index);
  void setDirection(int direction);
  void setNavigating(bool navigating);

private:
  static constexpr int kMaxEntries = 128;

  std::vector<Entry> m_entries;
  int m_cursor;
  int m_direction;
  bool m_navigating;
};

}  // namespace DataModel
