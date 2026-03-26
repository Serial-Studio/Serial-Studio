/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Licensing/CommercialToken.h"

#include <QCryptographicHash>

//--------------------------------------------------------------------------------------------------
// Singleton token storage
//--------------------------------------------------------------------------------------------------

static Licensing::CommercialToken s_currentToken;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty (invalid) commercial token.
 */
Licensing::CommercialToken::CommercialToken() : m_tier(FeatureTier::None), m_graceDays(0), m_hmac(0)
{}

//--------------------------------------------------------------------------------------------------
// Static accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the currently active commercial token.
 *
 * This is the single point of access for all Pro feature code.
 * The token is set by LemonSqueezy or Trial after successful validation.
 */
const Licensing::CommercialToken& Licensing::CommercialToken::current()
{
  return s_currentToken;
}

/**
 * @brief Replaces the current token with a newly validated one.
 * @param token A sealed token produced by LemonSqueezy or Trial.
 */
void Licensing::CommercialToken::setCurrent(const CommercialToken& token)
{
  s_currentToken = token;
}

/**
 * @brief Resets the current token to an invalid empty state.
 */
void Licensing::CommercialToken::clearCurrent()
{
  s_currentToken = CommercialToken();
}

//--------------------------------------------------------------------------------------------------
// Validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether this token was produced by a legitimate validation.
 *
 * Recomputes the HMAC from the token's data fields and the compile-time
 * build salt, then compares it against the stored HMAC.  Patching this
 * function to return true doesn't help — downstream code reads the actual
 * fields (featureTier, variantName, etc.) which remain empty/zero without
 * a real token.
 *
 * @return true if the HMAC matches and the token carries valid data.
 */
bool Licensing::CommercialToken::isValid() const
{
  if (m_tier == FeatureTier::None)
    return false;

  if (m_variantName.isEmpty())
    return false;

  if (m_instanceName.isEmpty())
    return false;

  if (m_hmac == 0)
    return false;

  return m_hmac == computeHmac();
}

//--------------------------------------------------------------------------------------------------
// Field accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the feature tier derived from the license variant.
 */
Licensing::FeatureTier Licensing::CommercialToken::featureTier() const noexcept
{
  return m_tier;
}

/**
 * @brief Returns the number of days the token remains valid offline.
 *
 * For trial tokens this is the trial days remaining.  For full licenses
 * this is the 30-day grace period countdown (30 = just validated online).
 */
int Licensing::CommercialToken::graceDaysRemaining() const noexcept
{
  return m_graceDays;
}

/**
 * @brief Returns the variant name from the license (e.g. "Pro - Monthly").
 *
 * Pro features may use this for display or feature-tiering decisions.
 */
const QString& Licensing::CommercialToken::variantName() const noexcept
{
  return m_variantName;
}

/**
 * @brief Returns the machine instance name the license is bound to.
 */
const QString& Licensing::CommercialToken::instanceName() const noexcept
{
  return m_instanceName;
}

//--------------------------------------------------------------------------------------------------
// Token construction (used by LemonSqueezy / Trial)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the feature tier for this token.
 */
void Licensing::CommercialToken::setFeatureTier(FeatureTier tier)
{
  m_tier = tier;
}

/**
 * @brief Sets the offline grace period remaining in days.
 */
void Licensing::CommercialToken::setGraceDaysRemaining(int days)
{
  m_graceDays = days;
}

/**
 * @brief Sets the variant name from the license response.
 */
void Licensing::CommercialToken::setVariantName(const QString& name)
{
  m_variantName = name;
}

/**
 * @brief Sets the machine instance name.
 */
void Licensing::CommercialToken::setInstanceName(const QString& name)
{
  m_instanceName = name;
}

/**
 * @brief Computes and stores the HMAC, finalizing the token.
 *
 * Must be called after all fields are set.  After sealing, isValid()
 * will return true.
 */
void Licensing::CommercialToken::seal()
{
  m_hmac = computeHmac();
}

//--------------------------------------------------------------------------------------------------
// HMAC computation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes a keyed hash from the token fields and build salt.
 *
 * The salt is a compile-time constant that only exists in binaries built
 * after successful CMake-time license validation.  Without it, the
 * static_assert in the header prevents compilation entirely.
 *
 * @return 64-bit truncated HMAC.
 */
quint64 Licensing::CommercialToken::computeHmac() const
{
  // Mix build salt with token data
  QByteArray message;
  message.append(QByteArray::number(static_cast<quint64>(COMMERCIAL_BUILD_SALT)));
  message.append(m_variantName.toUtf8());
  message.append(m_instanceName.toUtf8());
  message.append(QByteArray::number(static_cast<quint8>(m_tier)));
  message.append(QByteArray::number(m_graceDays));

  // Use SHA-256 and truncate to 64 bits
  const auto hash = QCryptographicHash::hash(message, QCryptographicHash::Sha256);
  quint64 result  = 0;
  if (hash.size() >= 8)
    memcpy(&result, hash.constData(), sizeof(result));

  return result;
}
