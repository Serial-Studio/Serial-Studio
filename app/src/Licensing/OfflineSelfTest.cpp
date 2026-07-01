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
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Licensing/OfflineSelfTest.h"

#include <array>
#include <cstdint>
#include <QDateTime>
#include <QDebug>
#include <QSettings>

#include "Licensing/MonotonicClock.h"
#include "Licensing/OfflineCertificate.h"
#include "Licensing/SimpleCrypt.h"

//--------------------------------------------------------------------------------------------------
// Embedded TEST vectors (throwaway key, NOT the production kOfflinePublicKey)
//--------------------------------------------------------------------------------------------------

static constexpr std::array<std::uint8_t, 32> kTestPublicKey = {
  0x06, 0x63, 0x50, 0x76, 0xc4, 0x42, 0xff, 0xb7, 0x67, 0xa3, 0xd7, 0xb5, 0xcc, 0xbd, 0x70, 0x1e,
  0x9a, 0x7f, 0x2e, 0x19, 0x65, 0x17, 0x8f, 0x5b, 0x73, 0x39, 0xaa, 0x90, 0xd7, 0x25, 0x8e, 0xaf};

static constexpr auto kTestMachineId = "SS-SELFTEST-MACHINE";
static constexpr qint64 kTestNowSecs = 1751328000;

static constexpr auto kGoodCert =
  "pmF2AWNtaWRzU1MtU0VMRlRFU1QtTUFDSElORWN2YXJzRW50ZXJwcmlzZSAtIFllYXJseWNpaWRmaW5zdC0xY2lhdBpl"
  "U/EAY2V4cBr0hlcAgC7hZqsUSfIslsIYzFS2tRMPMKc6U1H4Sh82Oef/XF6ArdEeXsaEPsgxmCbbLm9q6WVH106logtw"
  "iD2KEF7KBw==";

static constexpr auto kExpiredCert =
  "pmF2AWNtaWRzU1MtU0VMRlRFU1QtTUFDSElORWN2YXJoTGlmZXRpbWVjaWlkZmluc3QtMmNpYXQaO5rKAGNleHAaO5xQ"
  "oGJbRbrWSuIqqun+45CmweSsUDA3uOJCtYyMGZbcLkfxfDhoWCotDuz7+giQKanZmQDsLJjIElE4HNsAy1wDzwI=";

static constexpr auto kWrongTierCert =
  "pmF2AWNtaWRzU1MtU0VMRlRFU1QtTUFDSElORWN2YXJgY2lpZGZpbnN0LTNjaWF0GmVT8QBjZXhwGvSGVwD6wismqbFz"
  "n4hktLFhpRhmxlXRT8Fe6PvPxgtqOTJFGuxxtJbvlxHP/Go3gNKcKAkrHnAqxFf4Jo0R//zxoqsG";

//--------------------------------------------------------------------------------------------------
// Assertion helper
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports a single check and returns 1 on failure, 0 on success.
 */
[[nodiscard]] static int expect(const char* name, bool condition)
{
  if (condition) {
    qInfo().noquote() << "  PASS" << name;
    return 0;
  }

  qCritical().noquote() << "  FAIL" << name;
  return 1;
}

/**
 * @brief Verifies a vector and returns whether it produced the expected status.
 */
[[nodiscard]] static bool statusIs(const QByteArray& framed,
                                   const QString& machine,
                                   qint64 nowSecs,
                                   Licensing::CertStatus expected)
{
  Licensing::CertificateFields fields;
  return Licensing::verifyCertificate(framed, kTestPublicKey, machine, nowSecs, fields) == expected;
}

//--------------------------------------------------------------------------------------------------
// Verifier vectors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Exercises signature, machine, expiry, tier, and malformed paths.
 */
[[nodiscard]] static int runVerifierChecks()
{
  const QByteArray good(kGoodCert);
  const QString machine(kTestMachineId);

  int fails  = 0;
  fails     += expect("good certificate accepted",
                  statusIs(good, machine, kTestNowSecs, Licensing::CertStatus::Valid));

  auto decoded  = QByteArray::fromBase64(good);
  decoded[3]    = static_cast<char>(decoded[3] ^ 0x01);
  fails        += expect(
    "payload bit-flip rejected (bad signature)",
    statusIs(decoded.toBase64(), machine, kTestNowSecs, Licensing::CertStatus::BadSignature));

  fails += expect(
    "wrong machine rejected",
    statusIs(
      good, QStringLiteral("SS-OTHER"), kTestNowSecs, Licensing::CertStatus::MachineMismatch));

  fails += expect(
    "expired certificate rejected",
    statusIs(QByteArray(kExpiredCert), machine, kTestNowSecs, Licensing::CertStatus::Expired));

  fails += expect(
    "empty-tier certificate rejected",
    statusIs(QByteArray(kWrongTierCert), machine, kTestNowSecs, Licensing::CertStatus::WrongTier));

  fails += expect("garbage input rejected (malformed)",
                  statusIs(QByteArray("!!not a certificate!!"),
                           machine,
                           kTestNowSecs,
                           Licensing::CertStatus::Malformed));

  fails += expect("truncated certificate rejected (malformed)",
                  statusIs(good.left(8), machine, kTestNowSecs, Licensing::CertStatus::Malformed));
  return fails;
}

//--------------------------------------------------------------------------------------------------
// Clock-rewind floor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Confirms the monotonic floor pins now to a previously observed future time.
 */
[[nodiscard]] static int runClockRewindCheck()
{
  QSettings settings(QStringLiteral("SerialStudioSelfTest"), QStringLiteral("OfflineClock"));
  settings.clear();

  Licensing::SimpleCrypt crypt(0x0123456789abcdefULL);
  crypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);

  const auto future = QDateTime::currentDateTime().addYears(5);
  settings.beginGroup("licensing");
  settings.setValue("lastSeen", crypt.encryptToString(future.toString(Qt::RFC2822Date)));
  settings.endGroup();

  const auto floored = Licensing::MonotonicClock::nowFloored(settings, crypt);
  settings.clear();

  return expect("clock floored to previously observed future (rewind blocked)",
                floored >= future.addSecs(-60));
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs headless offline-certificate verifier vectors; returns 0 on success.
 */
int Licensing::runOfflineSelfTest()
{
  qInfo().noquote() << "[offline-license] running self-test vectors";
  const int fails = runVerifierChecks() + runClockRewindCheck();

  if (fails == 0)
    qInfo().noquote() << "[offline-license] all checks passed";
  else
    qCritical().noquote() << "[offline-license]" << fails << "check(s) failed";

  return fails == 0 ? 0 : 1;
}
