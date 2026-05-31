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

#include <QSettings>
#include <QString>

#include "Licensing/SimpleCrypt.h"

namespace AI {

/**
 * @brief AI provider identifiers, index-aligned with the QML combobox model.
 */
enum class ProviderId : int {
  Anthropic  = 0,
  OpenAI     = 1,
  Gemini     = 2,
  DeepSeek   = 3,
  OpenRouter = 4,
  Groq       = 5,
  Mistral    = 6,
  Local      = 7,
};

/**
 * @brief Total number of AI providers wired into the Assistant.
 */
inline constexpr int kProviderCount = 8;

/**
 * @brief RAII helper that best-effort scrubs a QString on destruction.
 */
class ZeroOnDestroy {
public:
  explicit ZeroOnDestroy(QString& target) noexcept;
  ~ZeroOnDestroy();
  ZeroOnDestroy(ZeroOnDestroy&&)                 = delete;
  ZeroOnDestroy(const ZeroOnDestroy&)            = delete;
  ZeroOnDestroy& operator=(ZeroOnDestroy&&)      = delete;
  ZeroOnDestroy& operator=(const ZeroOnDestroy&) = delete;

private:
  QString& m_ref;
};

/**
 * @brief Per-machine encrypted storage for BYOK API keys.
 */
class KeyVault {
public:
  KeyVault();

  [[nodiscard]] bool hasKey(ProviderId provider) const;
  [[nodiscard]] bool hasAnyKey() const;
  [[nodiscard]] QString key(ProviderId provider) const;

  void setKey(ProviderId provider, const QString& plaintext);
  void clearKey(ProviderId provider);
  void clearAllKeys();

  [[nodiscard]] static QString redact(const QString& key);

private:
  [[nodiscard]] static QString settingsKey(ProviderId provider);

private:
  mutable QSettings m_settings;
  mutable Licensing::SimpleCrypt m_simpleCrypt;
};

}  // namespace AI
