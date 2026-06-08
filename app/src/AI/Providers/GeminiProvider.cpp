/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/GeminiProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"
#include "AI/Providers/GeminiReply.h"

namespace AI::detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 */
class ImmediateErrorReply : public AI::Reply {
public:
  /**
   * @brief Schedules an error+finished pair via QTimer::singleShot(0, ...).
   */
  ImmediateErrorReply(const QString& message, QObject* parent = nullptr)
    : AI::Reply(parent), m_message(message)
  {
    QTimer::singleShot(0, this, [this]() {
      Q_EMIT errorOccurred(m_message);
      Q_EMIT finished();
    });
  }

  /**
   * @brief No-op: the timer-driven completion cannot be cancelled.
   */
  void abort() override {}

private:
  QString m_message;
};

}  // namespace AI::detail

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter.
 */
AI::GeminiProvider::GeminiProvider(QNetworkAccessManager& nam, KeyGetter keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::GeminiProvider::displayName() const
{
  return QStringLiteral("Gemini");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::GeminiProvider::keyVendorUrl() const
{
  return QStringLiteral("https://aistudio.google.com/app/apikey");
}

/**
 * @brief Returns the list of selectable Gemini models, default first.
 */
QStringList AI::GeminiProvider::availableModels() const
{
  return {
    QStringLiteral("gemini-2.5-flash"),
    QStringLiteral("gemini-2.5-pro"),
    QStringLiteral("gemini-2.0-flash"),
  };
}

/**
 * @brief Returns the default Gemini model for new sessions.
 */
QString AI::GeminiProvider::defaultModel() const
{
  return QStringLiteral("gemini-2.5-flash");
}

/**
 * @brief Returns a human-friendly label for a known Gemini model id.
 */
QString AI::GeminiProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("gemini-2.5-flash"))
    return QStringLiteral("2.5 Flash (Free)");

  if (modelId == QStringLiteral("gemini-2.5-pro"))
    return QStringLiteral("2.5 Pro");

  if (modelId == QStringLiteral("gemini-2.0-flash"))
    return QStringLiteral("2.0 Flash (Free)");

  return modelId;
}

/**
 * @brief Returns Gemini-specific tool/context shaping hints.
 */
AI::ProviderCapabilities AI::GeminiProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.structuredToolResults = true;
  caps.needsSmallToolSurface = currentModel().contains(QStringLiteral("flash"));
  caps.toolResultByteBudget  = caps.needsSmallToolSurface ? 4096 : 8192;
  return caps;
}

//--------------------------------------------------------------------------------------------------
// Translators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Translates a single Anthropic-shaped content block into a Gemini part.
 */
QJsonObject AI::GeminiProvider::translateBlock(const QJsonObject& block)
{
  const auto type = block.value(QStringLiteral("type")).toString();

  if (type == QStringLiteral("text")) {
    const auto t = block.value(QStringLiteral("text")).toString();
    if (t.isEmpty())
      return {};

    QJsonObject textPart;
    textPart[QStringLiteral("text")] = t;
    return textPart;
  }

  if (type == QStringLiteral("tool_use")) {
    QJsonObject fc;
    fc[QStringLiteral("name")] = block.value(QStringLiteral("name")).toString();
    fc[QStringLiteral("args")] = block.value(QStringLiteral("input")).toObject();

    QJsonObject fcPart;
    fcPart[QStringLiteral("functionCall")] = fc;
    return fcPart;
  }

  if (type == QStringLiteral("tool_result")) {
    QJsonObject responseObj;
    const auto preParsed = block.value(QStringLiteral("_gemini_response")).toObject();
    if (!preParsed.isEmpty()) {
      responseObj = preParsed;
    } else {
      const auto contentStr = block.value(QStringLiteral("content")).toString();
      QJsonParseError parseError;
      const auto doc = QJsonDocument::fromJson(contentStr.toUtf8(), &parseError);
      if (parseError.error == QJsonParseError::NoError && doc.isObject())
        responseObj = doc.object();
      else
        responseObj[QStringLiteral("output")] = contentStr;
    }

    QJsonObject fr;
    fr[QStringLiteral("name")]     = block.value(QStringLiteral("_tool_name")).toString();
    fr[QStringLiteral("response")] = responseObj;

    QJsonObject frPart;
    frPart[QStringLiteral("functionResponse")] = fr;
    return frPart;
  }

  return {};
}

/**
 * @brief Collects translated Gemini parts from a single message's content value.
 */
QJsonArray AI::GeminiProvider::collectMessageParts(const QJsonValue& contentValue)
{
  QJsonArray parts;
  if (contentValue.isString()) {
    QJsonObject textPart;
    textPart[QStringLiteral("text")] = contentValue.toString();
    parts.append(textPart);
    return parts;
  }

  if (!contentValue.isArray())
    return parts;

  for (const auto& bv : contentValue.toArray()) {
    const auto part = translateBlock(bv.toObject());
    if (!part.isEmpty())
      parts.append(part);
  }
  return parts;
}

/**
 * @brief Converts Anthropic-shaped history into Gemini's contents array.
 */
QJsonArray AI::GeminiProvider::translateHistory(const QJsonArray& history)
{
  QJsonArray out;

  for (const auto& v : history) {
    const auto msg          = v.toObject();
    const auto role         = msg.value(QStringLiteral("role")).toString();
    const auto contentValue = msg.value(QStringLiteral("content"));

    const auto parts = collectMessageParts(contentValue);
    if (parts.isEmpty())
      continue;

    QJsonObject content;
    content[QStringLiteral("role")] =
      (role == QStringLiteral("assistant")) ? QStringLiteral("model") : role;
    content[QStringLiteral("parts")] = parts;
    out.append(content);
  }

  return out;
}

/**
 * @brief Converts AI-tool definitions into Gemini's functionDeclarations.
 */
QJsonArray AI::GeminiProvider::translateTools(const QJsonArray& tools)
{
  if (tools.isEmpty())
    return {};

  QJsonArray declarations;
  for (const auto& v : tools) {
    const auto t = v.toObject();
    QJsonObject decl;
    decl[QStringLiteral("name")]        = t.value(QStringLiteral("name"));
    decl[QStringLiteral("description")] = t.value(QStringLiteral("description"));
    decl[QStringLiteral("parameters")]  = t.value(QStringLiteral("input_schema"));
    declarations.append(decl);
  }

  QJsonObject toolWrapper;
  toolWrapper[QStringLiteral("functionDeclarations")] = declarations;

  QJsonArray out;
  out.append(toolWrapper);
  return out;
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the streamGenerateContent request and returns a streaming Reply.
 */
AI::Reply* AI::GeminiProvider::sendMessage(const QJsonArray& history,
                                           const QJsonArray& tools,
                                           bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new AI::detail::ImmediateErrorReply(
      QObject::tr("No Gemini API key set. Open Manage Keys to add one."));

  const auto systemBlocks = ContextBuilder::buildSystemArray(false);
  QString systemText;
  for (const auto& v : systemBlocks) {
    const auto block = v.toObject();
    const auto t     = block.value(QStringLiteral("text")).toString();
    if (!t.isEmpty()) {
      if (!systemText.isEmpty())
        systemText.append(QStringLiteral("\n\n"));

      systemText.append(t);
    }
  }

  QJsonObject body;
  body[QStringLiteral("contents")] = translateHistory(history);

  if (!systemText.isEmpty()) {
    QJsonObject sysPart;
    sysPart[QStringLiteral("text")] = systemText;
    QJsonArray sysParts;
    sysParts.append(sysPart);

    QJsonObject sysInstruction;
    sysInstruction[QStringLiteral("parts")]    = sysParts;
    body[QStringLiteral("system_instruction")] = sysInstruction;
  }

  const auto translatedTools = translateTools(tools);
  if (!translatedTools.isEmpty()) {
    body[QStringLiteral("tools")] = translatedTools;

    if (forbidToolUse) {
      QJsonObject fnConfig;
      fnConfig[QStringLiteral("mode")] = QStringLiteral("NONE");
      QJsonObject toolConfig;
      toolConfig[QStringLiteral("function_calling_config")] = fnConfig;
      body[QStringLiteral("tool_config")]                   = toolConfig;
    }
  }

  QUrl url(QStringLiteral(
             "https://generativelanguage.googleapis.com/v1beta/models/%1:streamGenerateContent")
             .arg(currentModel()));
  QUrlQuery q;
  q.addQueryItem(QStringLiteral("key"), key);
  q.addQueryItem(QStringLiteral("alt"), QStringLiteral("sse"));
  url.setQuery(q);

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "Gemini request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new GeminiReply(m_nam, url, key, bytes);
}
