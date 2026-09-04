/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/ProviderJson.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QSet>

#include "Misc/JsonValidator.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// An error body is a diagnostic, never a payload: refuse to parse a large one
static constexpr qsizetype kMaxErrorBodyBytes = 256 * 1024;

//--------------------------------------------------------------------------------------------------
// Names and system text
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rewrites a tool name into the ^[a-zA-Z0-9_-]+ shape both Anthropic and OpenAI enforce.
 */
QString AI::ProviderJson::sanitizeToolName(const QString& original)
{
  QString out = original;
  out.replace(QChar('.'), QChar('_'));
  out.replace(QChar(':'), QChar('_'));
  return out;
}

/**
 * @brief Flattens structured system blocks into the single system string every
 *        Chat-Completions-shaped backend takes, blocks joined by a blank line.
 */
QString AI::ProviderJson::flattenSystemBlocks(const QJsonArray& blocks)
{
  QString systemText;
  for (const auto& v : blocks) {
    const auto block = v.toObject();
    const auto t     = block.value(QStringLiteral("text")).toString();
    if (t.isEmpty())
      continue;

    if (!systemText.isEmpty())
      systemText.append(QStringLiteral("\n\n"));

    systemText.append(t);
  }

  return systemText;
}

//--------------------------------------------------------------------------------------------------
// History and tool translation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Splits Anthropic content blocks into OpenAI text / tool_calls / tool messages.
 */
static void translateBlocks(const QJsonArray& blocks,
                            QString& textAccumulator,
                            QJsonArray& toolCalls,
                            QJsonArray& toolResultMessages)
{
  for (const auto& bv : blocks) {
    const auto block = bv.toObject();
    const auto type  = block.value(QStringLiteral("type")).toString();

    if (type == QStringLiteral("text")) {
      const auto t = block.value(QStringLiteral("text")).toString();
      if (t.isEmpty())
        continue;

      if (!textAccumulator.isEmpty())
        textAccumulator.append(QChar('\n'));

      textAccumulator.append(t);
      continue;
    }

    if (type == QStringLiteral("tool_use")) {
      QJsonObject fn;
      fn[QStringLiteral("name")] =
        AI::ProviderJson::sanitizeToolName(block.value(QStringLiteral("name")).toString());
      fn[QStringLiteral("arguments")] =
        QString::fromUtf8(QJsonDocument(block.value(QStringLiteral("input")).toObject())
                            .toJson(QJsonDocument::Compact));

      QJsonObject tc;
      tc[QStringLiteral("id")]       = block.value(QStringLiteral("id")).toString();
      tc[QStringLiteral("type")]     = QStringLiteral("function");
      tc[QStringLiteral("function")] = fn;
      toolCalls.append(tc);
      continue;
    }

    if (type != QStringLiteral("tool_result"))
      continue;

    QJsonObject toolMsg;
    toolMsg[QStringLiteral("role")]         = QStringLiteral("tool");
    toolMsg[QStringLiteral("tool_call_id")] = block.value(QStringLiteral("tool_use_id")).toString();
    toolMsg[QStringLiteral("content")]      = block.value(QStringLiteral("content")).toString();
    toolResultMessages.append(toolMsg);
  }
}

/**
 * @brief Inserts stub tool replies for any assistant tool_call that lacks a tool message.
 */
static QJsonArray backfillDanglingToolCalls(const QJsonArray& messages)
{
  QSet<QString> answered;
  for (const auto& value : messages) {
    const auto msg = value.toObject();
    if (msg.value(QStringLiteral("role")).toString() == QStringLiteral("tool"))
      answered.insert(msg.value(QStringLiteral("tool_call_id")).toString());
  }

  QJsonArray out;
  for (const auto& value : messages) {
    const auto msg = value.toObject();
    out.append(msg);

    const auto calls = msg.value(QStringLiteral("tool_calls")).toArray();
    for (const auto& callValue : calls) {
      const auto id = callValue.toObject().value(QStringLiteral("id")).toString();
      if (id.isEmpty() || answered.contains(id))
        continue;

      QJsonObject stub;
      stub[QStringLiteral("role")]         = QStringLiteral("tool");
      stub[QStringLiteral("tool_call_id")] = id;
      stub[QStringLiteral("content")] = QStringLiteral("{\"ok\":false,\"error\":\"no_result\"}");
      out.append(stub);
      answered.insert(id);
    }
  }

  return out;
}

/**
 * @brief Converts Anthropic-shaped history into the OpenAI Chat Completions shape, prepending
 *        the system text as a system (or developer) message and back-filling stub replies for
 *        assistant tool calls no tool message answers.
 */
QJsonArray AI::ProviderJson::translateHistory(const QJsonArray& history,
                                              const QString& systemText,
                                              bool useDeveloperRole)
{
  QJsonArray out;

  if (!systemText.isEmpty()) {
    QJsonObject sys;
    sys[QStringLiteral("role")] =
      useDeveloperRole ? QStringLiteral("developer") : QStringLiteral("system");
    sys[QStringLiteral("content")] = systemText;
    out.append(sys);
  }

  for (const auto& v : history) {
    const auto msg          = v.toObject();
    const auto role         = msg.value(QStringLiteral("role")).toString();
    const auto contentValue = msg.value(QStringLiteral("content"));

    if (contentValue.isString()) {
      QJsonObject m;
      m[QStringLiteral("role")]    = role;
      m[QStringLiteral("content")] = contentValue.toString();
      out.append(m);
      continue;
    }

    if (!contentValue.isArray())
      continue;

    QString textAccumulator;
    QJsonArray toolCalls;
    QJsonArray toolResultMessages;
    translateBlocks(contentValue.toArray(), textAccumulator, toolCalls, toolResultMessages);

    if (!textAccumulator.isEmpty() || !toolCalls.isEmpty()) {
      QJsonObject m;
      m[QStringLiteral("role")] = role;
      if (!textAccumulator.isEmpty())
        m[QStringLiteral("content")] = textAccumulator;

      if (!toolCalls.isEmpty())
        m[QStringLiteral("tool_calls")] = toolCalls;

      out.append(m);
    }

    for (const auto& tr : toolResultMessages)
      out.append(tr);
  }

  return backfillDanglingToolCalls(out);
}

/**
 * @brief Converts AI-tool definitions into the OpenAI tool-choice schema.
 */
QJsonArray AI::ProviderJson::translateTools(const QJsonArray& tools)
{
  QJsonArray out;
  for (const auto& v : tools) {
    const auto t    = v.toObject();
    const auto name = t.value(QStringLiteral("name")).toString();

    QJsonObject fn;
    fn[QStringLiteral("name")]        = sanitizeToolName(name);
    fn[QStringLiteral("description")] = t.value(QStringLiteral("description"));
    fn[QStringLiteral("parameters")]  = t.value(QStringLiteral("input_schema"));

    QJsonObject tool;
    tool[QStringLiteral("type")]     = QStringLiteral("function");
    tool[QStringLiteral("function")] = fn;
    out.append(tool);
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Request body
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the request body shared by every OpenAI-compatible endpoint. Vendor-specific
 *        fields (caching keys, reasoning effort, parallel tool calls) are added by the caller.
 */
QJsonObject AI::ProviderJson::chatCompletionsBody(const QString& model,
                                                  const QJsonArray& history,
                                                  const QString& systemText,
                                                  const QJsonArray& tools,
                                                  bool forbidToolUse,
                                                  bool useDeveloperRole)
{
  QJsonObject body;
  body[QStringLiteral("model")]    = model;
  body[QStringLiteral("stream")]   = true;
  body[QStringLiteral("messages")] = translateHistory(history, systemText, useDeveloperRole);

  if (!tools.isEmpty()) {
    body[QStringLiteral("tools")] = translateTools(tools);
    body[QStringLiteral("tool_choice")] =
      forbidToolUse ? QStringLiteral("none") : QStringLiteral("auto");
  }

  return body;
}

//--------------------------------------------------------------------------------------------------
// Error mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Extracts `error.message` from an HTTP error body, empty when the body carries none.
 *        The caller owns the user-facing wording so its tr() context stays with its class.
 */
QString AI::ProviderJson::errorMessageFromBody(const QByteArray& body)
{
  Misc::JsonValidator::Limits limits;
  limits.maxFileSize = kMaxErrorBodyBytes;

  const auto parsed = Misc::JsonValidator::parseAndValidate(body, limits);
  if (!parsed.valid || !parsed.document.isObject())
    return {};

  const auto err = parsed.document.object().value(QStringLiteral("error")).toObject();
  return err.value(QStringLiteral("message")).toString();
}

/**
 * @brief True when @p status is worth retrying: request timeout, rate limit, or a server fault.
 */
bool AI::ProviderJson::isTransientHttpStatus(int status)
{
  return status == 408 || status == 429 || status >= 500;
}
