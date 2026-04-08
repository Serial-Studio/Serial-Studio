/*
 * Copyright (c) 2015-2025 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QSimpleUpdater.h>

#include <QtTest>

class Test_Versioning : public QObject {
  Q_OBJECT

private slots:

  void TestPrefixSameVersion()
  {
    bool needsUpgrade;
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.1", "0.0.1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.1", "v0.0.1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.1", "0.0.1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.1", "v0.0.1");
    QVERIFY(!needsUpgrade);
  }

  void MajorMinorRelease()
  {
    bool needsUpgrade;

    // Patch version comparisons with optional 'v' prefix
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.2", "0.0.1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.2", "v0.0.1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.2", "0.0.1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.2", "v0.0.1");
    QVERIFY(needsUpgrade);

    // Revisions are treated as numbers, not text
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.8", "0.0.2");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.8", "v0.0.2");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.9", "0.0.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.8", "v0.0.2");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.8", "v0.0.2");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.12", "v0.0.2");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.112", "v0.0.2");
    QVERIFY(needsUpgrade);

    // Minor version comparisons
    needsUpgrade = QSimpleUpdater::compareVersions("0.1.2", "0.0.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("0.1.2", "v0.0.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.1.2", "0.0.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.1.2", "v0.0.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.8", "0.1.2");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.8", "0.1.2");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("0.0.8", "v0.1.2");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v0.0.8", "v0.1.2");
    QVERIFY(!needsUpgrade);

    // Major version comparisons
    needsUpgrade = QSimpleUpdater::compareVersions("1.0.2", "0.9.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("1.1.2", "v0.7.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v2.0.2", "1.9.8");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v100.2$.2", "v100.1.8");
    QVERIFY(needsUpgrade);
  }

  void AlphaVersions()
  {
    bool needsUpgrade;

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha2", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha10", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha1", "v1.0.0-alpha2");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha1", "v1.0.0-alpha19");
    QVERIFY(!needsUpgrade);

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-alpha10");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-alpha18");
    QVERIFY(needsUpgrade);

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha1", "v1.0.0");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha1000", "v1.0.0");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha88", "v1.0.0");
    QVERIFY(!needsUpgrade);
  }

  void BetaVersions()
  {
    bool needsUpgrade;

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta2", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta0", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha1", "v1.0.0-beta2");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha9999", "v1.0.0-beta19");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta2", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta19", "v1.0.0-alpha9999");
    QVERIFY(needsUpgrade);

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-beta0");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-beta1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v0.1.0-beta9999");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v2.0.0-beta9999");
    QVERIFY(!needsUpgrade);
  }

  void RemoteCandidateVersions()
  {
    bool needsUpgrade;

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-rc");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0", "v1.0.0-rc1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-rc1", "v1.0.0-rc1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-rc2", "v1.0.0-rc1");
    QVERIFY(needsUpgrade);

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-rc1", "v1.0.0-alpha1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-rc1", "v1.0.0-alpha200");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-rc1", "v1.0.0-beta1");
    QVERIFY(needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-rc1", "v1.0.0-beta2000");
    QVERIFY(needsUpgrade);

    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha1", "v1.0.0-rc1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-alpha200", "v1.0.0-rc1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta1", "v1.0.0-rc1");
    QVERIFY(!needsUpgrade);
    needsUpgrade = QSimpleUpdater::compareVersions("v1.0.0-beta2000", "v1.0.0-rc1");
    QVERIFY(!needsUpgrade);
  }
};
