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

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QTest>

#include "AI/Redactor.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Pins the scrubber that runs over every tool result before it enters the model context:
 *        a secret-shaped substring is replaced, ordinary telemetry is not (spec 0075, M10).
 */
class TstRedactor : public QObject {
  Q_OBJECT

private slots:
  void providerKeyShapesAreReplaced();
  void bearerHeaderIsReplaced();
  void privateKeyBlockIsReplaced();
  void ordinaryTextIsUntouched();
  void nestedJsonLeavesAreScrubbed();
};

//--------------------------------------------------------------------------------------------------
// String scrubbing
//--------------------------------------------------------------------------------------------------

/**
 * @brief An OpenAI-shaped key never survives the scrubber.
 */
void TstRedactor::providerKeyShapesAreReplaced()
{
  QString text = QStringLiteral("key is sk-abcdefghijklmnopqrstuvwxyz012345 ok");
  QVERIFY(AI::Redactor::scrub(text));
  QVERIFY(!text.contains(QStringLiteral("abcdefghijklmnopqrstuvwxyz")));
  QVERIFY(text.contains(QStringLiteral("[REDACTED:api_key]")));
}

/**
 * @brief An Authorization header value is redacted with its own reason.
 */
void TstRedactor::bearerHeaderIsReplaced()
{
  QString text = QStringLiteral("Authorization: Bearer abcdefghijklmnopqrstuvwxyz0123");
  QVERIFY(AI::Redactor::scrub(text));
  QVERIFY(text.contains(QStringLiteral("[REDACTED:bearer_token]")));
}

/**
 * @brief A PEM block is removed whole, not line by line.
 */
void TstRedactor::privateKeyBlockIsReplaced()
{
  QString text = QStringLiteral("-----BEGIN RSA PRIVATE KEY-----\nAAAA\nBBBB\n"
                                "-----END RSA PRIVATE KEY-----");
  QVERIFY(AI::Redactor::scrub(text));
  QCOMPARE(text, QStringLiteral("[REDACTED:private_key]"));
}

/**
 * @brief Ordinary telemetry text is passed through unchanged, so the scrubber cannot corrupt a
 *        tool result that carries no secret.
 */
void TstRedactor::ordinaryTextIsUntouched()
{
  QString text      = QStringLiteral("temperature=21.5 humidity=48 status=ok");
  const auto before = text;
  QVERIFY(!AI::Redactor::scrub(text));
  QCOMPARE(text, before);
}

//--------------------------------------------------------------------------------------------------
// JSON walking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every string leaf of a nested tool result is scrubbed, arrays included.
 */
void TstRedactor::nestedJsonLeavesAreScrubbed()
{
  QJsonObject inner;
  inner.insert(QStringLiteral("token"),
               QStringLiteral("ghp_abcdefghijklmnopqrstuvwxyz0123456789AB"));

  QJsonArray list;
  list.append(QStringLiteral("sk-abcdefghijklmnopqrstuvwxyz012345"));
  list.append(42);

  QJsonObject root;
  root.insert(QStringLiteral("inner"), inner);
  root.insert(QStringLiteral("list"), list);

  const auto out           = AI::Redactor::scrubObject(root);
  const auto scrubbedInner = out.value(QStringLiteral("inner")).toObject();
  const auto scrubbedList  = out.value(QStringLiteral("list")).toArray();

  QCOMPARE(scrubbedInner.value(QStringLiteral("token")).toString(),
           QStringLiteral("[REDACTED:github_token]"));
  QCOMPARE(scrubbedList.at(0).toString(), QStringLiteral("[REDACTED:api_key]"));
  QCOMPARE(scrubbedList.at(1).toInt(), 42);
}

QTEST_APPLESS_MAIN(TstRedactor)

#include "tst_redactor.moc"
