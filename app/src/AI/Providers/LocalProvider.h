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

#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "AI/Providers/Provider.h"

class QNetworkAccessManager;

namespace AI {

/**
 * @brief Local OpenAI-compatible provider (Ollama, llama.cpp, LM Studio, vLLM).
 */
class LocalProvider
  : public QObject
  , public Provider {
  Q_OBJECT

public:
  explicit LocalProvider(QNetworkAccessManager& nam);

  [[nodiscard]] QString displayName() const override;
  [[nodiscard]] QString keyVendorUrl() const override;
  [[nodiscard]] QStringList availableModels() const override;
  [[nodiscard]] QString defaultModel() const override;
  [[nodiscard]] QString modelDisplayName(const QString& modelId) const override;
  [[nodiscard]] ProviderCapabilities capabilities() const override;

  void setCurrentModel(const QString& model) override;

  [[nodiscard]] Reply* sendMessage(const QJsonArray& history,
                                   const QJsonArray& tools,
                                   bool forbidToolUse = false) override;

  [[nodiscard]] QString baseUrl() const;
  void setBaseUrl(const QString& url);
  void refreshModels();

  static QString defaultBaseUrl();

signals:
  void modelsChanged();

private:
  [[nodiscard]] QString chatEndpoint() const;
  [[nodiscard]] QString modelsEndpoint() const;
  void loadCachedModels();
  void persistCachedModels() const;

private:
  QNetworkAccessManager& m_nam;
  mutable QSettings m_settings;
  QString m_baseUrl;
  QStringList m_models;
};

}  // namespace AI
