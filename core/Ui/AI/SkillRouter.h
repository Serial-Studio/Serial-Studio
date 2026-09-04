/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QHash>
#include <QJsonArray>
#include <QSet>
#include <QString>
#include <QStringList>

namespace AI {

/**
 * @brief Deterministic skill routing: matches user text against the bundled trigger
 *        vocabulary and builds the synthetic meta.loadSkill pair a compliant model
 *        would have produced itself.
 */
class SkillRouter {
public:
  SkillRouter();

  [[nodiscard]] QString match(const QString& userText, const QSet<QString>& alreadyLoaded) const;
  [[nodiscard]] static QJsonArray buildInjectionPair(const QString& skillId, int byteBudget);

private:
  void loadTriggers();

private:
  QHash<QString, QStringList> m_triggers;
};

}  // namespace AI
