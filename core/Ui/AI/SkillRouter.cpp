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

#include "AI/SkillRouter.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"

/**
 * @brief Escapes forged untrusted delimiters inside a payload (mirror of the
 *        Conversation/ContextBuilder envelope rule).
 */
static QString neutralizeDelimiter(const QString& payload)
{
  QString out = payload;
  out.replace(QStringLiteral("</untrusted"), QStringLiteral("< /untrusted"), Qt::CaseInsensitive);
  out.replace(QStringLiteral("<untrusted"), QStringLiteral("< untrusted"), Qt::CaseInsensitive);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Construction & trigger vocabulary
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the bundled trigger vocabulary.
 */
AI::SkillRouter::SkillRouter()
{
  loadTriggers();
}

/**
 * @brief Reads :/ai/skill_triggers.json, keeping only ids that are real skills.
 */
void AI::SkillRouter::loadTriggers()
{
  QFile file(QStringLiteral(":/ai/skill_triggers.json"));
  if (!file.open(QIODevice::ReadOnly)) {
    qCWarning(serialStudioAI) << "SkillRouter: trigger resource not readable";
    return;
  }

  QJsonParseError parse_error;
  const auto doc = QJsonDocument::fromJson(file.readAll(), &parse_error);
  file.close();
  if (!doc.isObject()) {
    qCWarning(serialStudioAI) << "SkillRouter: invalid trigger resource:"
                              << parse_error.errorString();
    return;
  }

  const auto known  = ContextBuilder::skillIds();
  const auto skills = doc.object().value(QStringLiteral("skills")).toObject();
  for (auto it = skills.constBegin(); it != skills.constEnd(); ++it) {
    if (!known.contains(it.key())) {
      qCWarning(serialStudioAI) << "SkillRouter: unknown skill id in triggers:" << it.key();
      continue;
    }

    QStringList phrases;
    for (const auto& v : it.value().toArray()) {
      const auto phrase = v.toString().toLower().trimmed();
      if (!phrase.isEmpty())
        phrases.append(phrase);
    }

    if (!phrases.isEmpty())
      m_triggers.insert(it.key(), phrases);
  }
}

//--------------------------------------------------------------------------------------------------
// Matching & pair construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the single best-matching skill id for a user message (longest matched
 *        phrase wins; equal lengths break on the lexicographically smaller id so routing
 *        is deterministic despite QHash iteration order), or empty. Already-loaded skills
 *        never re-match, so a chat pays for each skill body once.
 */
QString AI::SkillRouter::match(const QString& userText, const QSet<QString>& alreadyLoaded) const
{
  const auto haystack = userText.toLower();

  QString best;
  int bestLen = 0;
  for (auto it = m_triggers.constBegin(); it != m_triggers.constEnd(); ++it) {
    if (alreadyLoaded.contains(it.key()))
      continue;

    for (const auto& phrase : it.value()) {
      if (!haystack.contains(phrase))
        continue;

      const int len   = static_cast<int>(phrase.size());
      const bool wins = len > bestLen || (len == bestLen && it.key() < best);
      if (wins) {
        best    = it.key();
        bestLen = len;
      }
    }
  }

  if (!best.isEmpty())
    qCDebug(serialStudioAI) << "SkillRouter: matched" << best;

  return best;
}

/**
 * @brief Builds the assistant tool_use + user tool_result message pair for one skill,
 *        shaped exactly like a model-initiated meta.loadSkill so the history reconciler
 *        and result-aging machinery treat it as any other pair; the body honors the same
 *        provider byte budget a real tool result would.
 */
QJsonArray AI::SkillRouter::buildInjectionPair(const QString& skillId, int byteBudget)
{
  auto body = ContextBuilder::skillBody(skillId);
  if (body.isEmpty())
    return {};

  const int budget = qBound(2048, byteBudget, 16 * 1024);
  if (body.toUtf8().size() > budget) {
    while (body.toUtf8().size() > budget && !body.isEmpty())
      body.chop(body.size() / 8 + 1);

    body += QStringLiteral("\n... [skill truncated to fit the model context]");
  }

  const auto callId = QStringLiteral("synthetic-skill-") + skillId;

  QJsonObject toolUse;
  toolUse[QStringLiteral("type")]  = QStringLiteral("tool_use");
  toolUse[QStringLiteral("id")]    = callId;
  toolUse[QStringLiteral("name")]  = QStringLiteral("meta.loadSkill");
  toolUse[QStringLiteral("input")] = QJsonObject{
    {QStringLiteral("name"), skillId}
  };
  toolUse[QStringLiteral("_synthetic")] = true;

  QJsonObject assistant;
  assistant[QStringLiteral("role")]    = QStringLiteral("assistant");
  assistant[QStringLiteral("content")] = QJsonArray{toolUse};

  QJsonObject payload;
  payload[QStringLiteral("ok")]         = true;
  payload[QStringLiteral("skill")]      = skillId;
  payload[QStringLiteral("body")]       = body;
  payload[QStringLiteral("autoLoaded")] = true;

  const auto payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
  QString wrapped;
  wrapped += QStringLiteral("<untrusted source=\"meta.loadSkill\">\n");
  wrapped += neutralizeDelimiter(QString::fromUtf8(payloadBytes));
  wrapped += QStringLiteral("\n</untrusted>");

  QJsonObject toolResult;
  toolResult[QStringLiteral("type")]                  = QStringLiteral("tool_result");
  toolResult[QStringLiteral("tool_use_id")]           = callId;
  toolResult[QStringLiteral("content")]               = wrapped;
  QJsonObject geminiPayload                           = payload;
  geminiPayload[QStringLiteral("__untrusted_source")] = QStringLiteral("meta.loadSkill");
  toolResult[QStringLiteral("_gemini_response")]      = geminiPayload;
  toolResult[QStringLiteral("_tool_name")]            = QStringLiteral("meta.loadSkill");

  QJsonObject user;
  user[QStringLiteral("role")]    = QStringLiteral("user");
  user[QStringLiteral("content")] = QJsonArray{toolResult};

  return QJsonArray{assistant, user};
}
