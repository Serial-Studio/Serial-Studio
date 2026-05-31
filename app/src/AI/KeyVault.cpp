/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/KeyVault.h"

#include <QChar>

#include "AI/Logging.h"
#include "Licensing/MachineID.h"

//--------------------------------------------------------------------------------------------------
// ZeroOnDestroy
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores a reference to the target QString to be scrubbed on destruction.
 */
AI::ZeroOnDestroy::ZeroOnDestroy(QString& target) noexcept : m_ref(target) {}

/**
 * @brief Best-effort overwrite and clear of the referenced QString.
 */
AI::ZeroOnDestroy::~ZeroOnDestroy()
{
  m_ref.fill(QChar(QChar::Null));
  m_ref.clear();
}

//--------------------------------------------------------------------------------------------------
// KeyVault construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures SimpleCrypt with a per-machine key and HMAC integrity.
 */
AI::KeyVault::KeyVault()
{
  m_simpleCrypt.setKey(Licensing::MachineID::instance().machineSpecificKey());
  m_simpleCrypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when a non-empty key is stored for the given provider.
 */
bool AI::KeyVault::hasKey(ProviderId provider) const
{
  return !key(provider).isEmpty();
}

/**
 * @brief Returns true when at least one provider has a key stored.
 */
bool AI::KeyVault::hasAnyKey() const
{
  for (int i = 0; i < kProviderCount; ++i)
    if (hasKey(static_cast<ProviderId>(i)))
      return true;

  return false;
}

/**
 * @brief Decrypts and returns the plaintext key for the given provider.
 */
QString AI::KeyVault::key(ProviderId provider) const
{
  m_settings.beginGroup(QStringLiteral("ai/keys"));
  const auto cipher = m_settings.value(settingsKey(provider)).toString();
  m_settings.endGroup();

  if (cipher.isEmpty())
    return {};

  auto plaintext = m_simpleCrypt.decryptToString(cipher);
  if (m_simpleCrypt.lastError() != Licensing::SimpleCrypt::ErrorNoError) {
    qCWarning(serialStudioAI) << "Key decrypt failed for" << settingsKey(provider);
    return {};
  }

  return plaintext;
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Encrypts and writes the given plaintext key to QSettings.
 */
void AI::KeyVault::setKey(ProviderId provider, const QString& plaintext)
{
  auto trimmed = plaintext.trimmed();
  ZeroOnDestroy scrub(trimmed);

  if (trimmed.isEmpty()) {
    clearKey(provider);
    return;
  }

  const auto cipher = m_simpleCrypt.encryptToString(trimmed);
  m_settings.beginGroup(QStringLiteral("ai/keys"));
  m_settings.setValue(settingsKey(provider), cipher);
  m_settings.endGroup();

  qCDebug(serialStudioAI) << "Key set for" << settingsKey(provider) << "value" << redact(trimmed);
}

/**
 * @brief Removes the stored key for the given provider.
 */
void AI::KeyVault::clearKey(ProviderId provider)
{
  m_settings.beginGroup(QStringLiteral("ai/keys"));
  m_settings.setValue(settingsKey(provider), QString());
  m_settings.endGroup();

  qCDebug(serialStudioAI) << "Key cleared for" << settingsKey(provider);
}

/**
 * @brief Removes the stored key for every provider.
 */
void AI::KeyVault::clearAllKeys()
{
  for (int i = 0; i < kProviderCount; ++i)
    clearKey(static_cast<ProviderId>(i));
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a redacted form of the key, safe to log.
 */
QString AI::KeyVault::redact(const QString& key)
{
  if (key.size() < 8)
    return QStringLiteral("***");

  return key.left(4) + QStringLiteral("...") + key.right(4);
}

/**
 * @brief Returns the QSettings sub-key name for the given provider.
 */
QString AI::KeyVault::settingsKey(ProviderId provider)
{
  switch (provider) {
    case ProviderId::Anthropic:
      return QStringLiteral("anthropic");
    case ProviderId::OpenAI:
      return QStringLiteral("openai");
    case ProviderId::Gemini:
      return QStringLiteral("gemini");
    case ProviderId::DeepSeek:
      return QStringLiteral("deepseek");
    case ProviderId::OpenRouter:
      return QStringLiteral("openrouter");
    case ProviderId::Groq:
      return QStringLiteral("groq");
    case ProviderId::Mistral:
      return QStringLiteral("mistral");
    case ProviderId::Local:
      return QStringLiteral("local");
  }

  return QStringLiteral("unknown");
}
