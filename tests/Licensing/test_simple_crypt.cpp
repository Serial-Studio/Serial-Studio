/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include <QtTest>
#include "Licensing/SimpleCrypt.h"

class TestSimpleCrypt : public QObject
{
  Q_OBJECT

private slots:
  void testEncryptDecrypt();
};

void TestSimpleCrypt::testEncryptDecrypt()
{
  SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));

  const QString original = "Hello, World!";
  const QString encrypted = crypto.encryptToString(original);
  const QString decrypted = crypto.decryptToString(encrypted);

  QCOMPARE(decrypted, original);
}

QTEST_MAIN(TestSimpleCrypt)
#include "test_simple_crypt.moc"
