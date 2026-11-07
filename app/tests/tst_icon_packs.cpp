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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "Core/IconRegistry.h"

/**
 * @brief Covers the pack side of Misc::IconRegistry (spec 0115 AC2): every house id resolves off
 *        the placeholder under every pack, the fixed categories ignore the pack, a mono pack's URL
 *        carries the tint, and an unknown id still serves the placeholder.
 */
class TstIconPacks : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void houseSetResolvesWithoutPack();
  void packArtworkWinsAndFallsBackPerIcon();
  void fixedCategoriesIgnoreThePack();
  void monoPackCarriesTheTintPerTone();
  void unknownIdAndUnknownPackStayLoud();

private:
  [[nodiscard]] bool writeSvg(const QString& relative) const;
  [[nodiscard]] bool writePack(const QString& id, bool mono) const;

private:
  QTemporaryDir m_root;
};

static const QString kSvg =
  QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 48 48\"/>");

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

bool TstIconPacks::writeSvg(const QString& relative) const
{
  const auto path = m_root.path() + QStringLiteral("/") + relative;
  if (!QDir().mkpath(QFileInfo(path).absolutePath()))
    return false;

  QFile file(path);
  return file.open(QFile::WriteOnly) && file.write(kSvg.toUtf8()) > 0;
}

bool TstIconPacks::writePack(const QString& id, const bool mono) const
{
  const auto path =
    m_root.path() + QStringLiteral("/iconpacks/") + id + QStringLiteral("/pack.json");
  if (!QDir().mkpath(QFileInfo(path).absolutePath()))
    return false;

  QFile file(path);
  const auto json = QStringLiteral("{\"id\":\"%1\",\"title\":\"%1\",\"style\":\"x\",\"mono\":%2}")
                      .arg(id, mono ? QStringLiteral("true") : QStringLiteral("false"));
  return file.open(QFile::WriteOnly) && file.write(json.toUtf8()) > 0;
}

/**
 * @brief A house tree of four icons plus a colour pack that covers two of them and a mono pack
 *        that covers one; a pack file in a fixed category is planted to prove it is ignored.
 */
void TstIconPacks::initTestCase()
{
  QVERIFY(m_root.isValid());
  QVERIFY(writeSvg(QStringLiteral("icons/system/16/missing.svg")));
  QVERIFY(writeSvg(QStringLiteral("icons/window/32/close.svg")));
  QVERIFY(writeSvg(QStringLiteral("icons/widgets/16/gauge.svg")));
  QVERIFY(writeSvg(QStringLiteral("icons/widgets/32/gauge.svg")));
  QVERIFY(writeSvg(QStringLiteral("icons/commands/24/settings.svg")));
  QVERIFY(writePack(QStringLiteral("colour"), false));
  QVERIFY(writeSvg(QStringLiteral("iconpacks/colour/widgets/48/gauge.svg")));
  QVERIFY(writeSvg(QStringLiteral("iconpacks/colour/window/32/close.svg")));
  QVERIFY(writePack(QStringLiteral("mono"), true));
  QVERIFY(writeSvg(QStringLiteral("iconpacks/mono/commands/48/settings.svg")));
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

void TstIconPacks::houseSetResolvesWithoutPack()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  QVERIFY(registry.pack().isEmpty());
  QCOMPARE(registry.packs().size(), 2);
  QCOMPARE(registry.icon(QStringLiteral("widgets"), QStringLiteral("gauge"), 24),
           QStringLiteral("qrc:/icons/widgets/32/gauge.svg"));
  QCOMPARE(registry.iconById(QStringLiteral("commands/settings"), 16),
           QStringLiteral("qrc:/icons/commands/24/settings.svg"));
}

void TstIconPacks::packArtworkWinsAndFallsBackPerIcon()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  registry.setPack(QStringLiteral("colour"));
  QCOMPARE(registry.pack(), QStringLiteral("colour"));
  QVERIFY(!registry.isMono());
  QCOMPARE(registry.icon(QStringLiteral("widgets"), QStringLiteral("gauge"), 16),
           QStringLiteral("qrc:/iconpacks/colour/widgets/48/gauge.svg"));
  QCOMPARE(registry.icon(QStringLiteral("commands"), QStringLiteral("settings"), 24),
           QStringLiteral("qrc:/icons/commands/24/settings.svg"));
  QCOMPARE(registry.iconPath(QStringLiteral("widgets"), QStringLiteral("gauge"), 32),
           QStringLiteral(":/iconpacks/colour/widgets/48/gauge.svg"));
}

void TstIconPacks::fixedCategoriesIgnoreThePack()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  registry.setPack(QStringLiteral("colour"));
  QCOMPARE(registry.icon(QStringLiteral("window"), QStringLiteral("close"), 32),
           QStringLiteral("qrc:/icons/window/32/close.svg"));
  QCOMPARE(registry.icon(QStringLiteral("system"), QStringLiteral("missing"), 16),
           QStringLiteral("qrc:/icons/system/16/missing.svg"));
}

void TstIconPacks::monoPackCarriesTheTintPerTone()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  registry.setPack(QStringLiteral("mono"));
  registry.setTints(QStringLiteral("#232629"), QStringLiteral("#FFFFFF"));
  QVERIFY(registry.isMono());
  QCOMPARE(registry.icon(QStringLiteral("commands"), QStringLiteral("settings"), 24),
           QStringLiteral("image://iconpack/mono/commands/48/settings.svg?tint=232629"));
  QCOMPARE(
    registry.iconOn(
      Misc::IconRegistry::Tone::Strip, QStringLiteral("commands"), QStringLiteral("settings"), 24),
    QStringLiteral("image://iconpack/mono/commands/48/settings.svg?tint=ffffff"));
  QCOMPARE(registry.icon(QStringLiteral("widgets"), QStringLiteral("gauge"), 32),
           QStringLiteral("qrc:/icons/widgets/32/gauge.svg"));
  QCOMPARE(registry.iconPath(QStringLiteral("commands"), QStringLiteral("settings"), 24),
           QStringLiteral(":/iconpacks/mono/commands/48/settings.svg"));
}

void TstIconPacks::unknownIdAndUnknownPackStayLoud()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  registry.setPack(QStringLiteral("nope"));
  QVERIFY(registry.pack().isEmpty());
  registry.setPack(QStringLiteral("mono"));
  QCOMPARE(registry.icon(QStringLiteral("widgets"), QStringLiteral("nothing"), 16),
           QStringLiteral("qrc:/icons/system/16/missing.svg"));
  QCOMPARE(registry.iconById(QStringLiteral("malformed"), 16),
           QStringLiteral("qrc:/icons/system/16/missing.svg"));
}

QTEST_GUILESS_MAIN(TstIconPacks)
#include "tst_icon_packs.moc"
