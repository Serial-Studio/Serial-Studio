/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QString>
#include <QTest>

#include "Licensing/CommercialToken.h"

// The suite compiles CommercialToken.cpp with its own build salt, so the tier stays independent
// of how the application was configured. Every case builds its own token; the process-wide
// current() slot is restored by the case that touches it.

/**
 * @brief Pins the capability token every Pro gate reads: a token is valid only when it was sealed
 *        over its own fields, and any field edited after the seal invalidates it. This is an
 *        integrity property, not a feature gate (spec 0075, M11).
 */
class TstCommercialToken : public QObject {
  Q_OBJECT

private slots:
  void defaultTokenIsInvalid();
  void sealedTokenIsValid();
  void unsealedTokenIsInvalid();
  void editingAFieldAfterSealingInvalidatesIt();
  void tierNoneIsAlwaysInvalid();
  void currentSlotSetsAndClears();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the token a successful validation installs.
 */
static Licensing::CommercialToken sealedToken()
{
  Licensing::CommercialToken token;
  token.setVariantName(QStringLiteral("Pro - Yearly"));
  token.setInstanceName(QStringLiteral("machine-1234"));
  token.setGraceDaysRemaining(30);
  token.setFeatureTier(Licensing::FeatureTier::Pro);
  token.seal();
  return token;
}

//--------------------------------------------------------------------------------------------------
// Validity
//--------------------------------------------------------------------------------------------------

/**
 * @brief A default-constructed token entitles nothing.
 */
void TstCommercialToken::defaultTokenIsInvalid()
{
  const Licensing::CommercialToken token;
  QVERIFY(!token.isValid());
  QCOMPARE(token.featureTier(), Licensing::FeatureTier::None);
}

/**
 * @brief A sealed token reports its fields and validates.
 */
void TstCommercialToken::sealedTokenIsValid()
{
  const auto token = sealedToken();
  QVERIFY(token.isValid());
  QCOMPARE(token.featureTier(), Licensing::FeatureTier::Pro);
  QCOMPARE(token.graceDaysRemaining(), 30);
  QCOMPARE(token.variantName(), QStringLiteral("Pro - Yearly"));
  QCOMPARE(token.instanceName(), QStringLiteral("machine-1234"));
}

/**
 * @brief Filling the fields without sealing is not enough.
 */
void TstCommercialToken::unsealedTokenIsInvalid()
{
  Licensing::CommercialToken token;
  token.setVariantName(QStringLiteral("Pro - Yearly"));
  token.setInstanceName(QStringLiteral("machine-1234"));
  token.setFeatureTier(Licensing::FeatureTier::Pro);

  QVERIFY(!token.isValid());
}

/**
 * @brief A field edited after the seal breaks the token, so a copied token cannot be re-pointed.
 */
void TstCommercialToken::editingAFieldAfterSealingInvalidatesIt()
{
  auto token = sealedToken();
  QVERIFY(token.isValid());

  token.setInstanceName(QStringLiteral("another-machine"));
  QVERIFY(!token.isValid());

  auto tierEdited = sealedToken();
  tierEdited.setFeatureTier(Licensing::FeatureTier::Enterprise);
  QVERIFY(!tierEdited.isValid());
}

/**
 * @brief The None tier never validates, even sealed.
 */
void TstCommercialToken::tierNoneIsAlwaysInvalid()
{
  Licensing::CommercialToken token;
  token.setVariantName(QStringLiteral("Free"));
  token.setInstanceName(QStringLiteral("machine-1234"));
  token.setFeatureTier(Licensing::FeatureTier::None);
  token.seal();

  QVERIFY(!token.isValid());
}

//--------------------------------------------------------------------------------------------------
// Current slot
//--------------------------------------------------------------------------------------------------

/**
 * @brief The process-wide slot holds what was installed and clears back to invalid, which is the
 *        transition every entitlement notification is latched on.
 */
void TstCommercialToken::currentSlotSetsAndClears()
{
  Licensing::CommercialToken::clearCurrent();
  QVERIFY(!Licensing::CommercialToken::current().isValid());

  Licensing::CommercialToken::setCurrent(sealedToken());
  QVERIFY(Licensing::CommercialToken::current().isValid());
  QCOMPARE(Licensing::CommercialToken::current().featureTier(), Licensing::FeatureTier::Pro);

  Licensing::CommercialToken::clearCurrent();
  QVERIFY(!Licensing::CommercialToken::current().isValid());
}

QTEST_APPLESS_MAIN(TstCommercialToken)

#include "tst_commercial_token.moc"
