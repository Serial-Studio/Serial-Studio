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
#include <QSettings>
#include <QString>

#include "API/Server/ConsentVerdict.h"

namespace API {

/**
 * @brief One tri-state user consent behind a yes/no prompt. authorize() never blocks: an unanswered
 *        consent posts the prompt queued and refuses with ConsentRequired, so no receive path,
 *        script worker or pipeline thread sits behind a modal (spec 0075 I1). A "yes" persists
 *        under the bound settings key (empty key: session-only); a "no" never persists.
 */
class ConsentGate : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void granted();

public:
  explicit ConsentGate(QSettings& settings,
                       const QString& settingsKey,
                       const QString& title,
                       const QString& question,
                       QObject* parent = nullptr);
  ConsentGate(ConsentGate&&)                 = delete;
  ConsentGate(const ConsentGate&)            = delete;
  ConsentGate& operator=(ConsentGate&&)      = delete;
  ConsentGate& operator=(const ConsentGate&) = delete;

public:
  [[nodiscard]] bool isGranted() const noexcept;
  [[nodiscard]] const QString& settingsKey() const noexcept;
  [[nodiscard]] ConsentVerdict authorize(const QString& detail = QString());

public slots:
  void grant();
  void rebind(const QString& settingsKey);
  void showPrompt();

private:
  /**
   * @brief The user's standing answer.
   */
  enum class State {
    Unset,
    Granted,
    Denied
  };

  QSettings& m_settings;
  QString m_key;
  QString m_title;
  QString m_detail;
  QString m_question;
  bool m_promptPosted;
  State m_state;
};

}  // namespace API
