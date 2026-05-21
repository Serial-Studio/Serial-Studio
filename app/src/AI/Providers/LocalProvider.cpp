/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/LocalProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QUrl>

#include "AI/Assistant.h"
#include "AI/ContextBuilder.h"
#include "AI/KeyVault.h"
#include "AI/Logging.h"
#include "AI/Providers/OpenAIProvider.h"
#include "AI/Providers/OpenAIReply.h"
#include "Misc/JsonValidator.h"

namespace detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 */
class ImmediateErrorReplyLP : public AI::Reply {
public:
  /**
   * @brief Schedules errorOccurred + finished on the next event-loop tick.
   */
  ImmediateErrorReplyLP(const QString& message, QObject* parent = nullptr)
    : AI::Reply(parent), m_message(message)
  {
    QTimer::singleShot(0, this, [this]() {
      Q_EMIT errorOccurred(m_message);
      Q_EMIT finished();
    });
  }

  /**
   * @brief No-op since the error fires immediately.
   */
  void abort() override {}

private:
  QString m_message;
};

}  // namespace detail

//--------------------------------------------------------------------------------------------------
// Static fallbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the default base URL (Ollama on localhost:11434).
 */
QString AI::LocalProvider::defaultBaseUrl()
{
  return QStringLiteral("http://localhost:11434/v1");
}

/**
 * @brief Static fallback model list shown when the live query has not (yet) succeeded.
 */
static QStringList fallbackModels()
{
  return {
    QStringLiteral("llama3.2"),
    QStringLiteral("llama3.1"),
    QStringLiteral("qwen2.5"),
    QStringLiteral("mistral"),
    QStringLiteral("phi4"),
    QStringLiteral("deepseek-r1"),
  };
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the persisted base URL + cached model list and kicks off a refresh.
 */
AI::LocalProvider::LocalProvider(QNetworkAccessManager& nam) : QObject(nullptr), m_nam(nam)
{
  m_settings.beginGroup(QStringLiteral("ai/local"));
  m_baseUrl = m_settings.value(QStringLiteral("baseUrl"), defaultBaseUrl()).toString();
  m_settings.endGroup();

  loadCachedModels();

  // Query live models on next tick so QNAM and providers are fully wired
  QTimer::singleShot(0, this, &LocalProvider::refreshModels);
}

//--------------------------------------------------------------------------------------------------
// Provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::LocalProvider::displayName() const
{
  return QStringLiteral("Local model");
}

/**
 * @brief Returns a documentation link rather than a paid-key portal.
 */
QString AI::LocalProvider::keyVendorUrl() const
{
  return QStringLiteral("https://ollama.com/download");
}

/**
 * @brief Returns the cached model list, falling back to a small static set.
 */
QStringList AI::LocalProvider::availableModels() const
{
  if (!m_models.isEmpty())
    return m_models;

  return fallbackModels();
}

/**
 * @brief Returns the first available model as the default.
 */
QString AI::LocalProvider::defaultModel() const
{
  const auto list = availableModels();
  return list.isEmpty() ? QStringLiteral("llama3.2") : list.first();
}

/**
 * @brief Local model ids are passed through verbatim.
 */
QString AI::LocalProvider::modelDisplayName(const QString& modelId) const
{
  return modelId;
}

/**
 * @brief Returns conservative capabilities for local OpenAI-compatible servers.
 */
AI::ProviderCapabilities AI::LocalProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.needsSmallToolSurface = true;
  caps.toolResultByteBudget  = 3072;
  return caps;
}

/**
 * @brief Accepts any non-empty model id since the live list may not yet be loaded.
 */
void AI::LocalProvider::setCurrentModel(const QString& model)
{
  if (model.isEmpty())
    return;

  m_currentModel = model;
}

//--------------------------------------------------------------------------------------------------
// Base URL
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the configured base URL (e.g. http://localhost:11434/v1).
 */
QString AI::LocalProvider::baseUrl() const
{
  return m_baseUrl;
}

/**
 * @brief Sets and persists the base URL, then re-queries the model list.
 */
void AI::LocalProvider::setBaseUrl(const QString& url)
{
  auto trimmed = url.trimmed();
  if (trimmed.endsWith(QChar('/')))
    trimmed.chop(1);

  if (trimmed.isEmpty())
    trimmed = defaultBaseUrl();

  if (trimmed == m_baseUrl)
    return;

  m_baseUrl = trimmed;
  m_settings.beginGroup(QStringLiteral("ai/local"));
  m_settings.setValue(QStringLiteral("baseUrl"), m_baseUrl);
  m_settings.endGroup();

  qCDebug(serialStudioAI) << "Local base URL set to" << m_baseUrl;
  refreshModels();
}

//--------------------------------------------------------------------------------------------------
// Live model query
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the configured chat completions endpoint URL.
 */
QString AI::LocalProvider::chatEndpoint() const
{
  return m_baseUrl + QStringLiteral("/chat/completions");
}

/**
 * @brief Returns the OpenAI-compatible models listing URL.
 */
QString AI::LocalProvider::modelsEndpoint() const
{
  return m_baseUrl + QStringLiteral("/models");
}

/**
 * @brief Loads the previously-cached model list from QSettings.
 */
void AI::LocalProvider::loadCachedModels()
{
  m_settings.beginGroup(QStringLiteral("ai/local"));
  m_models = m_settings.value(QStringLiteral("cachedModels")).toStringList();
  m_settings.endGroup();
}

/**
 * @brief Persists the current model list to QSettings for next launch.
 */
void AI::LocalProvider::persistCachedModels() const
{
  m_settings.beginGroup(QStringLiteral("ai/local"));
  m_settings.setValue(QStringLiteral("cachedModels"), m_models);
  m_settings.endGroup();
}

/**
 * @brief Issues a GET to /v1/models and refreshes the cached model list.
 */
void AI::LocalProvider::refreshModels()
{
  QNetworkRequest req((QUrl(modelsEndpoint())));
  req.setTransferTimeout(5 * 1000);
  req.setRawHeader("accept", "application/json");

  qCDebug(serialStudioAI) << "GET" << modelsEndpoint();

  auto* reply = m_nam.get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
      // Suppress background refresh noise when Local is not the active provider
      const bool localActive =
        static_cast<ProviderId>(AI::Assistant::instance().currentProvider()) == ProviderId::Local;
      if (reply->error() == QNetworkReply::ConnectionRefusedError || !localActive)
        qCDebug(serialStudioAI) << "Local model list query failed:" << reply->errorString();
      else
        qCInfo(serialStudioAI) << "Local model list query failed:" << reply->errorString();

      return;
    }

    Misc::JsonValidator::Limits limits;
    limits.maxFileSize = 256 * 1024;
    const auto parsed  = Misc::JsonValidator::parseAndValidate(reply->readAll(), limits);
    if (!parsed.valid || !parsed.document.isObject()) {
      qCInfo(serialStudioAI) << "Local model list: invalid JSON --" << parsed.errorMessage;
      return;
    }

    QStringList ids;
    const auto data = parsed.document.object().value(QStringLiteral("data")).toArray();
    for (const auto& v : data) {
      const auto id = v.toObject().value(QStringLiteral("id")).toString();
      if (!id.isEmpty())
        ids.append(id);
    }

    if (ids.isEmpty()) {
      qCInfo(serialStudioAI) << "Local model list returned 0 entries; keeping cache";
      return;
    }

    if (ids == m_models)
      return;

    m_models = ids;
    persistCachedModels();
    qCInfo(serialStudioAI) << "Local model list refreshed:" << ids.size() << "models";
    Q_EMIT modelsChanged();
  });
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::LocalProvider::sendMessage(const QJsonArray& history, const QJsonArray& tools)
{
  if (m_baseUrl.isEmpty())
    return new detail::ImmediateErrorReplyLP(QObject::tr("No local model server URL configured. "
                                                         "Open Manage Keys to set one."));

  // Flatten Anthropic-shaped system blocks into a single string
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
  body[QStringLiteral("model")]  = currentModel();
  body[QStringLiteral("stream")] = true;
  body[QStringLiteral("messages")] =
    OpenAIProvider::translateHistory(history, systemText, /*useDeveloperRole=*/false);
  if (!tools.isEmpty()) {
    body[QStringLiteral("tools")]       = OpenAIProvider::translateTools(tools);
    body[QStringLiteral("tool_choice")] = QStringLiteral("auto");
  }

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "Local request:" << chatEndpoint() << "tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(
    m_nam, chatEndpoint(), QString(), QString(), bytes, QStringLiteral("Local"));
}
