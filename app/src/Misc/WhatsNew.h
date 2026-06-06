/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QObject>
#include <QSettings>
#include <QString>

namespace Misc {
/**
 * @brief Tracks the last app version a user has seen the "What's New" dialog for,
 *        and the opt-out flags for the What's New and Tips dialogs.
 */
class WhatsNew : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString currentVersion
             READ currentVersion
             CONSTANT)
  Q_PROPERTY(bool shouldShowWhatsNew
             READ shouldShowWhatsNew
             CONSTANT)
  Q_PROPERTY(bool showWhatsNewOnStartup
             READ showWhatsNewOnStartup
             WRITE setShowWhatsNewOnStartup
             NOTIFY showWhatsNewOnStartupChanged)
  Q_PROPERTY(bool showTipsOnStartup
             READ showTipsOnStartup
             WRITE setShowTipsOnStartup
             NOTIFY showTipsOnStartupChanged)
  // clang-format on

signals:
  void showTipsOnStartupChanged();
  void showWhatsNewOnStartupChanged();

private:
  explicit WhatsNew();
  WhatsNew(WhatsNew&&)                 = delete;
  WhatsNew(const WhatsNew&)            = delete;
  WhatsNew& operator=(WhatsNew&&)      = delete;
  WhatsNew& operator=(const WhatsNew&) = delete;

public:
  [[nodiscard]] static WhatsNew& instance();

  [[nodiscard]] QString currentVersion() const;
  [[nodiscard]] bool shouldShowWhatsNew() const;
  [[nodiscard]] bool showTipsOnStartup() const;
  [[nodiscard]] bool showWhatsNewOnStartup() const;

public slots:
  void syncVersion();
  void setShowTipsOnStartup(const bool show);
  void setShowWhatsNewOnStartup(const bool show);

private:
  [[nodiscard]] static int compareVersions(const QString& a, const QString& b);

private:
  static constexpr const char* kKeyLastSeen = "WhatsNew/LastSeenVersion";
  static constexpr const char* kKeyTips     = "WhatsNew/ShowTipsOnStartup";
  static constexpr const char* kKeyStartup  = "WhatsNew/ShowOnStartup";

  QSettings m_settings;
};
}  // namespace Misc
