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

#include <QObject>
#include <QSet>
#include <QSettings>
#include <QString>

#include "API/Server/ConsentGate.h"

namespace API {

/**
 * @brief The per-project consent to launch programs (spec 0098): one gate for a script's
 *        system.exec and a Process source's start, persisted per project file. A remote change
 *        clears it once per project per session (R13), and a headless operator's command-line
 *        project starts granted for the run. AUTO_CONSENT grants it for good.
 */
class LaunchConsent : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void granted();

public:
  explicit LaunchConsent(QSettings& settings, QObject* parent = nullptr);
  LaunchConsent(LaunchConsent&&)                 = delete;
  LaunchConsent(const LaunchConsent&)            = delete;
  LaunchConsent& operator=(LaunchConsent&&)      = delete;
  LaunchConsent& operator=(const LaunchConsent&) = delete;

public:
  [[nodiscard]] bool isGranted() const noexcept;
  [[nodiscard]] ConsentVerdict authorize(const QString& detail);
  [[nodiscard]] static QString keyFor(const QString& projectPath);

public slots:
  void noteRemoteChange();
  void bindProject(const QString& projectPath);
  void setOperatorConsentedPath(const QString& projectPath);

private:
  void clearOnce(const QString& key);
  [[nodiscard]] bool isOperatorPath(const QString& projectPath) const;

  bool m_tainted;
  ConsentGate m_gate;
  bool m_autoConsent;
  QString m_projectPath;
  QString m_operatorPath;
  QSet<QString> m_cleared;
};

}  // namespace API
