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

#include <functional>

#include "AI/Providers/Provider.h"

class QNetworkAccessManager;

namespace AI {

/**
 * @brief Mistral adapter on top of the OpenAI-compatible Chat Completions transport.
 */
class MistralProvider : public Provider {
public:
  MistralProvider(QNetworkAccessManager& nam, std::function<QString()> keyGetter);

  [[nodiscard]] QString displayName() const override;
  [[nodiscard]] QString keyVendorUrl() const override;
  [[nodiscard]] QStringList availableModels() const override;
  [[nodiscard]] QString defaultModel() const override;
  [[nodiscard]] QString modelDisplayName(const QString& modelId) const override;
  [[nodiscard]] ProviderCapabilities capabilities() const override;

  [[nodiscard]] Reply* sendMessage(const QJsonArray& history, const QJsonArray& tools) override;

private:
  QNetworkAccessManager& m_nam;
  std::function<QString()> m_keyGetter;
};

}  // namespace AI
