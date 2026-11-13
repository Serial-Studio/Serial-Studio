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

#include "Core/JsonValidator.h"

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
 * @brief Text stand-in for an attachment block that reached a translator without its bytes.
 */
static QString missingAttachmentText(const QJsonObject& block)
{
  return QStringLiteral("[Attachment \"%1\" was not sent.]")
    .arg(block.value(QStringLiteral("name")).toString());
}

/**
 * @brief Translates a resolved attachment block into Anthropic's document or image block.
 */
QJsonObject AI::ProviderJson::anthropicAttachment(const QJsonObject& block)
{
  const auto data = block.value(QStringLiteral("_data")).toString();
  QJsonObject out;
  if (data.isEmpty()) {
    out[QStringLiteral("type")] = QStringLiteral("text");
    out[QStringLiteral("text")] = missingAttachmentText(block);
    return out;
  }

  QJsonObject source;
  source[QStringLiteral("type")]       = QStringLiteral("base64");
  source[QStringLiteral("media_type")] = block.value(QStringLiteral("media_type"));
  source[QStringLiteral("data")]       = data;

  const bool is_pdf = block.value(QStringLiteral("kind")).toString() == QStringLiteral("pdf");
  out[QStringLiteral("type")]   = is_pdf ? QStringLiteral("document") : QStringLiteral("image");
  out[QStringLiteral("source")] = source;
  if (is_pdf)
    out[QStringLiteral("title")] = block.value(QStringLiteral("name"));

  return out;
}

/**
 * @brief Translates a resolved attachment block into a Gemini inlineData part.
 */
QJsonObject AI::ProviderJson::geminiAttachment(const QJsonObject& block)
{
  const auto data = block.value(QStringLiteral("_data")).toString();
  QJsonObject out;
  if (data.isEmpty()) {
    out[QStringLiteral("text")] = missingAttachmentText(block);
    return out;
  }

  QJsonObject inline_data;
  inline_data[QStringLiteral("mimeType")] = block.value(QStringLiteral("media_type"));
  inline_data[QStringLiteral("data")]     = data;
  out[QStringLiteral("inlineData")]       = inline_data;
  return out;
}

/**
 * @brief Translates a resolved attachment block into a Chat Completions content part.
 */
QJsonObject AI::ProviderJson::chatCompletionsAttachment(const QJsonObject& block)
{
  const auto data = block.value(QStringLiteral("_data")).toString();
  QJsonObject out;
  if (data.isEmpty()) {
    out[QStringLiteral("type")] = QStringLiteral("text");
    out[QStringLiteral("text")] = missingAttachmentText(block);
    return out;
  }

  const auto url = QStringLiteral("data:%1;base64,%2")
                     .arg(block.value(QStringLiteral("media_type")).toString(), data);
  if (block.value(QStringLiteral("kind")).toString() == QStringLiteral("pdf")) {
    QJsonObject file;
    file[QStringLiteral("filename")]  = block.value(QStringLiteral("name"));
    file[QStringLiteral("file_data")] = url;
    out[QStringLiteral("type")]       = QStringLiteral("file");
    out[QStringLiteral("file")]       = file;
    return out;
  }

  QJsonObject image;
  image[QStringLiteral("url")]     = url;
  out[QStringLiteral("type")]      = QStringLiteral("image_url");
  out[QStringLiteral("image_url")] = image;
  return out;
}

/**
 * @brief Content of a translated message: the plain string every backend takes, or, when the
 *        message carries attachments, a parts array with the file parts first and its text last.
 */
static QJsonValue messageContent(QJsonArray parts, const QString& text)
{
  if (parts.isEmpty())
    return text;

  if (text.isEmpty())
    return parts;

  QJsonObject part;
  part[QStringLiteral("type")] = QStringLiteral("text");
  part[QStringLiteral("text")] = text;
  parts.append(part);
  return parts;
}

/**
 * @brief Splits Anthropic content blocks into OpenAI text / tool_calls / tool messages, plus
 *        the content parts of any resolved attachment.
 */
static void translateBlocks(const QJsonArray& blocks,
                            QString& textAccumulator,
                            QJsonArray& toolCalls,
                            QJsonArray& toolResultMessages,
                            QJsonArray& attachmentParts)
{
  for (const auto& bv : blocks) {
    const auto block = bv.toObject();
    const auto type  = block.value(QStringLiteral("type")).toString();

    if (type == QStringLiteral("attachment")) {
      attachmentParts.append(AI::ProviderJson::chatCompletionsAttachment(block));
      continue;
    }

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
    QJsonArray attachmentParts;
    translateBlocks(
      contentValue.toArray(), textAccumulator, toolCalls, toolResultMessages, attachmentParts);

    if (!textAccumulator.isEmpty() || !toolCalls.isEmpty() || !attachmentParts.isEmpty()) {
      QJsonObject m;
      m[QStringLiteral("role")] = role;
      if (!textAccumulator.isEmpty() || !attachmentParts.isEmpty())
        m[QStringLiteral("content")] = messageContent(attachmentParts, textAccumulator);

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
