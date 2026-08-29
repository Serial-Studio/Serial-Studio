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
#include <QSettings>
#include <QString>
#include <QStringList>

namespace Misc {
/**
 * @brief The extension update policy and the queue it drives: whether installed extensions are
 *        checked at all, whether an available update installs without asking, which updates the
 *        user has already refused this session, and which ones are still to be installed. The
 *        prompt itself stays with ExtensionManager, so no user-facing string lives here.
 */
class ExtensionAutoUpdater {
public:
  explicit ExtensionAutoUpdater(QSettings& settings);
  ExtensionAutoUpdater(ExtensionAutoUpdater&&)                 = delete;
  ExtensionAutoUpdater(const ExtensionAutoUpdater&)            = delete;
  ExtensionAutoUpdater& operator=(ExtensionAutoUpdater&&)      = delete;
  ExtensionAutoUpdater& operator=(const ExtensionAutoUpdater&) = delete;

  [[nodiscard]] bool hasPending() const noexcept;
  [[nodiscard]] bool checkEnabled() const noexcept;
  [[nodiscard]] bool automaticUpdates() const noexcept;
  [[nodiscard]] bool declined(const QString& id) const;
  [[nodiscard]] QString takeNext();

  [[nodiscard]] bool setCheckEnabled(const bool enabled);
  [[nodiscard]] bool setAutomaticUpdates(const bool enabled);

  void rememberAlways();
  void enqueue(const QStringList& ids);
  void decline(const QStringList& ids);

private:
  void storePolicy(const int policy);

private:
  int m_policy;
  QStringList m_queue;
  QSet<QString> m_declined;
  QSettings& m_settings;
};
}  // namespace Misc
