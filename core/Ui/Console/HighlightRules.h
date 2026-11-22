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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <vector>

namespace Console {

/**
 * @brief Bounds of the console colour-rule table (spec 0127 R8).
 */
inline constexpr int kMaxHighlightRules   = 32;
inline constexpr int kMaxHighlightKeyword = 64;

/**
 * @brief One enabled colour rule, ready for the terminal's paint pass.
 */
struct KeywordRule {
  QString keyword;
  QColor foreground;
  QColor background;
  Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
};

/**
 * @brief The console's keyword colour rules (spec 0127 R8/R9): app-wide settings that only
 *        change how console text is drawn. Every console view reads the same compiled list, and
 *        nothing here touches the text the console stores, copies or logs.
 */
class HighlightRules : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QVariantList rules
             READ rules
             NOTIFY rulesChanged)
  Q_PROPERTY(int maxRules
             READ maxRules
             CONSTANT)
  // clang-format on

signals:
  void rulesChanged();

public:
  explicit HighlightRules(QObject* parent = nullptr);

  [[nodiscard]] int maxRules() const noexcept;
  [[nodiscard]] const QVariantList& rules() const noexcept;
  [[nodiscard]] const std::vector<KeywordRule>& compiled() const noexcept;

public slots:
  void removeRule(int index);
  void moveRule(int from, int to);
  void addRule(const QVariantMap& rule);
  void setRuleEnabled(int index, bool enabled);
  void updateRule(int index, const QVariantMap& rule);

private:
  void loadRules();
  void saveRules();
  void recompile();
  [[nodiscard]] static bool normalize(const QVariantMap& rule, QVariantMap& out);

private:
  QSettings m_settings;
  QVariantList m_rules;
  std::vector<KeywordRule> m_compiled;
};

}  // namespace Console
