/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <functional>

#include "AI/Providers/Provider.h"

class QNetworkAccessManager;

namespace AI {

/**
 * @brief Everything that distinguishes one OpenAI-compatible vendor from another: the endpoint,
 *        the model roster (@ref modelIds row-aligned with @ref modelLabels, default first) and
 *        the two model-id substrings that decide the capability hints.
 */
struct OpenAICompatibleVendor {
  QString name;
  QString endpoint;
  QString keyUrl;
  QStringList modelIds;
  QStringList modelLabels;
  QString thinkingMarker;
  QString smallSurfaceMarker;
  bool alwaysSmallSurface  = false;
  int toolResultBytes      = 8192;
  int smallToolResultBytes = 4096;
};

/**
 * @brief One adapter for every vendor that speaks OpenAI's Chat Completions transport with a
 *        bearer key (spec 0075, J5): DeepSeek, Groq, Mistral and OpenRouter were four verbatim
 *        copies of this file, and each new vendor added a fifth. The vendor table is the only
 *        thing that varies, so it is data.
 */
class OpenAICompatibleProvider : public Provider {
public:
  OpenAICompatibleProvider(QNetworkAccessManager& nam,
                           std::function<QString()> keyGetter,
                           OpenAICompatibleVendor vendor);

  [[nodiscard]] static OpenAICompatibleVendor deepSeek();
  [[nodiscard]] static OpenAICompatibleVendor groq();
  [[nodiscard]] static OpenAICompatibleVendor mistral();
  [[nodiscard]] static OpenAICompatibleVendor openRouter();

  [[nodiscard]] QString displayName() const override;
  [[nodiscard]] QString keyVendorUrl() const override;
  [[nodiscard]] QStringList availableModels() const override;
  [[nodiscard]] QString defaultModel() const override;
  [[nodiscard]] QString modelDisplayName(const QString& modelId) const override;
  [[nodiscard]] ProviderCapabilities capabilities() const override;

  [[nodiscard]] Reply* sendMessage(const QJsonArray& history,
                                   const QJsonArray& tools,
                                   bool forbidToolUse = false) override;

private:
  QNetworkAccessManager& m_nam;
  std::function<QString()> m_keyGetter;
  OpenAICompatibleVendor m_vendor;
};

}  // namespace AI
