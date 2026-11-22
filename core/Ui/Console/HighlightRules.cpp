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

#include "Console/HighlightRules.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kRulesKey   = QStringLiteral("Console/HighlightRules");
static const QString kKeywordKey = QStringLiteral("keyword");
static const QString kCaseKey    = QStringLiteral("caseSensitive");
static const QString kFgKey      = QStringLiteral("foreground");
static const QString kBgKey      = QStringLiteral("background");
static const QString kEnabledKey = QStringLiteral("enabled");
static const QString kDefaultFg  = QStringLiteral("#ffffff");
static const QString kDefaultBg  = QStringLiteral("#c0392b");

//--------------------------------------------------------------------------------------------------
// Construction and reading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the stored rules and compiles the enabled ones.
 */
Console::HighlightRules::HighlightRules(QObject* parent) : QObject(parent)
{
  loadRules();
  recompile();
}

/**
 * @brief Most rules the table holds.
 */
int Console::HighlightRules::maxRules() const noexcept
{
  return kMaxHighlightRules;
}

/**
 * @brief The rule table, each entry a map of keyword, caseSensitive, foreground, background and
 *        enabled; colours are "#rrggbb" names.
 */
const QVariantList& Console::HighlightRules::rules() const noexcept
{
  return m_rules;
}

/**
 * @brief The enabled rules in table order, which is also their priority where two overlap.
 */
const std::vector<Console::KeywordRule>& Console::HighlightRules::compiled() const noexcept
{
  return m_compiled;
}

//--------------------------------------------------------------------------------------------------
// Rule table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Brings a rule into its stored shape: a keyword within the length cap, two valid
 *        colours (the defaults where one is missing or unreadable). False for an empty keyword.
 */
bool Console::HighlightRules::normalize(const QVariantMap& rule, QVariantMap& out)
{
  const auto keyword = rule.value(kKeywordKey).toString();
  if (keyword.isEmpty() || keyword.size() > kMaxHighlightKeyword)
    return false;

  const QColor fg(rule.value(kFgKey, kDefaultFg).toString());
  const QColor bg(rule.value(kBgKey, kDefaultBg).toString());

  out.clear();
  out.insert(kKeywordKey, keyword);
  out.insert(kCaseKey, rule.value(kCaseKey, true).toBool());
  out.insert(kFgKey, fg.isValid() ? fg.name() : kDefaultFg);
  out.insert(kBgKey, bg.isValid() ? bg.name() : kDefaultBg);
  out.insert(kEnabledKey, rule.value(kEnabledKey, true).toBool());
  return true;
}

/**
 * @brief Appends a rule; one without a keyword, or one past the cap, is ignored.
 */
void Console::HighlightRules::addRule(const QVariantMap& rule)
{
  QVariantMap stored;
  if (m_rules.count() >= kMaxHighlightRules || !normalize(rule, stored))
    return;

  m_rules.append(stored);
  saveRules();
}

/**
 * @brief Replaces the rule at @a index.
 */
void Console::HighlightRules::updateRule(int index, const QVariantMap& rule)
{
  SS_ASSERT(index >= 0 && index < m_rules.count(), return);

  QVariantMap stored;
  if (!normalize(rule, stored))
    return;

  m_rules.replace(index, stored);
  saveRules();
}

/**
 * @brief Removes the rule at @a index.
 */
void Console::HighlightRules::removeRule(int index)
{
  SS_ASSERT(index >= 0 && index < m_rules.count(), return);

  m_rules.removeAt(index);
  saveRules();
}

/**
 * @brief Moves the rule at @a from to position @a to; an earlier rule wins an overlap.
 */
void Console::HighlightRules::moveRule(int from, int to)
{
  SS_ASSERT(from >= 0 && from < m_rules.count(), return);
  SS_ASSERT(to >= 0 && to < m_rules.count(), return);
  if (from == to)
    return;

  m_rules.move(from, to);
  saveRules();
}

/**
 * @brief Turns the rule at @a index on or off without losing its colours.
 */
void Console::HighlightRules::setRuleEnabled(int index, bool enabled)
{
  SS_ASSERT(index >= 0 && index < m_rules.count(), return);

  auto rule = m_rules.at(index).toMap();
  rule.insert(kEnabledKey, enabled);
  m_rules.replace(index, rule);
  saveRules();
}

/**
 * @brief Loads the rules from app settings. Settings are user-editable, so a corrupt blob yields
 *        an empty table and a malformed entry is skipped; the loop is bounded.
 */
void Console::HighlightRules::loadRules()
{
  const auto doc = QJsonDocument::fromJson(m_settings.value(kRulesKey).toByteArray());
  if (!doc.isArray())
    return;

  const auto array = doc.array();
  for (const auto& entry : array) {
    if (m_rules.count() >= kMaxHighlightRules)
      break;

    QVariantMap stored;
    if (entry.isObject() && normalize(entry.toObject().toVariantMap(), stored))
      m_rules.append(stored);
  }
}

/**
 * @brief Stores the table, recompiles and notifies the editor and every console view.
 */
void Console::HighlightRules::saveRules()
{
  const auto doc = QJsonDocument(QJsonArray::fromVariantList(m_rules));
  m_settings.setValue(kRulesKey, doc.toJson(QJsonDocument::Compact));
  recompile();
  Q_EMIT rulesChanged();
}

/**
 * @brief Rebuilds the list the paint pass reads from the enabled rules.
 */
void Console::HighlightRules::recompile()
{
  m_compiled.clear();
  for (const auto& entry : std::as_const(m_rules)) {
    const auto rule = entry.toMap();
    if (!rule.value(kEnabledKey, true).toBool())
      continue;

    KeywordRule compiled;
    compiled.keyword    = rule.value(kKeywordKey).toString();
    compiled.foreground = QColor(rule.value(kFgKey).toString());
    compiled.background = QColor(rule.value(kBgKey).toString());
    compiled.sensitivity =
      rule.value(kCaseKey, true).toBool() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    m_compiled.push_back(compiled);
  }

  SS_ASSERT_LOG(m_compiled.size() <= static_cast<size_t>(kMaxHighlightRules));
}
