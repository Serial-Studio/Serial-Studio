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
#include <QRegularExpression>
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
  void monoSurfaceIgnoresTheActivePack();
  void unknownIdAndUnknownPackStayLoud();
  void rasterTiersResolveBesideVectorOnes();
  void hiddenPackStaysOutOfTheSelector();
  void unavailablePackIsNotSelectableAndStaysQuiet();
  void basicPackStandsInForAnAbsentHouseSet();
  void widgetToolbarsFollowTheirOwnChoice();

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
  QVERIFY(writePack(Misc::IconRegistry::monoPackId(), true));
  QVERIFY(writeSvg(
    QStringLiteral("iconpacks/%1/commands/48/settings.svg").arg(Misc::IconRegistry::monoPackId())));
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

void TstIconPacks::houseSetResolvesWithoutPack()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  QVERIFY(registry.pack().isEmpty());
  QCOMPARE(registry.packs().size(), 3);
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

/**
 * @brief The widget-toolbar resolver draws from the fixed mono pack under the house set and
 *        under a colour pack alike, in the text tint, and serves the house file for a glyph the
 *        mono pack lacks.
 */
void TstIconPacks::monoSurfaceIgnoresTheActivePack()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  registry.setTints(QStringLiteral("#232629"), QStringLiteral("#FFFFFF"));
  const auto expected = QStringLiteral("image://iconpack/%1/commands/48/settings.svg?tint=232629")
                          .arg(Misc::IconRegistry::monoPackId());
  QCOMPARE(registry.monoIconById(QStringLiteral("commands/settings"), 18), expected);
  registry.setPack(QStringLiteral("colour"));
  QCOMPARE(registry.monoIconById(QStringLiteral("commands/settings"), 18), expected);
  QCOMPARE(registry.icon(QStringLiteral("commands"), QStringLiteral("settings"), 18),
           QStringLiteral("qrc:/icons/commands/24/settings.svg"));
  QCOMPARE(registry.monoIconById(QStringLiteral("widgets/gauge"), 18),
           QStringLiteral("qrc:/icons/widgets/32/gauge.svg"));
  QCOMPARE(registry.monoIconById(QStringLiteral("malformed"), 18),
           QStringLiteral("qrc:/icons/system/16/missing.svg"));
}

/**
 * @brief The widget toolbars take the pack chosen for them, tinted only when it is monochrome, and
 *        a glyph that pack lacks falls back to the fixed mono pack before the house file.
 */
void TstIconPacks::widgetToolbarsFollowTheirOwnChoice()
{
  Misc::IconRegistry registry(m_root.path() + QStringLiteral("/icons"),
                              m_root.path() + QStringLiteral("/iconpacks"));
  registry.setTints(QStringLiteral("#232629"), QStringLiteral("#FFFFFF"));
  const auto settings = QStringLiteral("commands/settings");
  const auto gauge    = QStringLiteral("widgets/gauge");
  const auto fixed    = QStringLiteral("image://iconpack/%1/commands/48/settings.svg?tint=232629")
                          .arg(Misc::IconRegistry::monoPackId());

  registry.setWidgetPack(QStringLiteral("mono"));
  QCOMPARE(registry.monoIconById(settings, 18),
           QStringLiteral("image://iconpack/mono/commands/48/settings.svg?tint=232629"));

  registry.setWidgetPack(QStringLiteral("colour"));
  QCOMPARE(registry.monoIconById(gauge, 18),
           QStringLiteral("qrc:/iconpacks/colour/widgets/48/gauge.svg"));
  QCOMPARE(registry.monoIconById(settings, 18), fixed);

  registry.setWidgetPack(Misc::IconRegistry::houseId());
  QCOMPARE(registry.monoIconById(settings, 18),
           QStringLiteral("qrc:/icons/commands/24/settings.svg"));

  registry.setWidgetPack(QString());
  QCOMPARE(registry.monoIconById(settings, 18), fixed);

  registry.setWidgetPack(QStringLiteral("absent"));
  QCOMPARE(registry.widgetPack(), Misc::IconRegistry::houseId());
  QCOMPARE(registry.monoIconById(settings, 18),
           QStringLiteral("qrc:/icons/commands/24/settings.svg"));
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

/**
 * @brief Writes @p text at @p relative under @p root, creating its folders.
 */
[[nodiscard]] static bool writeText(const QString& root,
                                    const QString& relative,
                                    const QString& text)
{
  const auto path = root + QChar('/') + relative;
  if (!QDir().mkpath(QFileInfo(path).absolutePath()))
    return false;

  QFile file(path);
  return file.open(QFile::WriteOnly) && file.write(text.toUtf8()) > 0;
}

/**
 * @brief A public build mixes PNG and SVG tiers (spec 0116): each tier keeps its own suffix, and an
 *        @2x file never counts as an icon, even with no @1x file beside it.
 */
void TstIconPacks::rasterTiersResolveBesideVectorOnes()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeText(root.path(), QStringLiteral("icons/system/16/missing.svg"), kSvg));
  QVERIFY(writeText(root.path(), QStringLiteral("icons/widgets/16/gauge.png"), kSvg));
  QVERIFY(writeText(root.path(), QStringLiteral("icons/widgets/16/gauge@2x.png"), kSvg));
  QVERIFY(writeText(root.path(), QStringLiteral("icons/widgets/32/gauge.svg"), kSvg));
  QVERIFY(writeText(root.path(), QStringLiteral("icons/commands/48/orphan@2x.png"), kSvg));

  Misc::IconRegistry registry(root.path() + QStringLiteral("/icons"),
                              root.path() + QStringLiteral("/iconpacks"));
  QCOMPARE(registry.icon(QStringLiteral("widgets"), QStringLiteral("gauge"), 16),
           QStringLiteral("qrc:/icons/widgets/16/gauge.png"));
  QCOMPARE(registry.icon(QStringLiteral("widgets"), QStringLiteral("gauge"), 24),
           QStringLiteral("qrc:/icons/widgets/32/gauge.svg"));
  QCOMPARE(registry.icon(QStringLiteral("commands"), QStringLiteral("orphan@2x"), 48),
           QStringLiteral("qrc:/icons/system/16/missing.svg"));
}

/**
 * @brief A hidden pack is bundled and serves glyphs, but a user can never pick it (spec 0116 R11).
 */
void TstIconPacks::hiddenPackStaysOutOfTheSelector()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeText(root.path(), QStringLiteral("icons/system/16/missing.svg"), kSvg));
  QVERIFY(writeText(root.path(),
                    QStringLiteral("iconpacks/glyphs/pack.json"),
                    QStringLiteral("{\"id\":\"glyphs\",\"mono\":true,\"hidden\":true}")));
  QVERIFY(writeText(root.path(),
                    QStringLiteral("iconpacks/open/pack.json"),
                    QStringLiteral("{\"id\":\"open\",\"mono\":false}")));

  Misc::IconRegistry registry(root.path() + QStringLiteral("/icons"),
                              root.path() + QStringLiteral("/iconpacks"));
  QCOMPARE(registry.packs().size(), 2);
  QCOMPARE(registry.visiblePacks().size(), 1);
  QCOMPARE(registry.visiblePacks().first().id, QStringLiteral("open"));
}

/**
 * @brief A theme or saved setting naming a hidden or absent pack resolves to the house set with
 *        nothing logged (spec 0116 AC9).
 */
void TstIconPacks::unavailablePackIsNotSelectableAndStaysQuiet()
{
  QTest::failOnWarning(QRegularExpression(QStringLiteral(".*")));

  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeText(root.path(), QStringLiteral("icons/system/16/missing.svg"), kSvg));
  QVERIFY(writeText(root.path(),
                    QStringLiteral("iconpacks/glyphs/pack.json"),
                    QStringLiteral("{\"id\":\"glyphs\",\"mono\":true,\"hidden\":true}")));
  QVERIFY(writeText(root.path(),
                    QStringLiteral("iconpacks/open/pack.json"),
                    QStringLiteral("{\"id\":\"open\",\"mono\":false}")));

  Misc::IconRegistry registry(root.path() + QStringLiteral("/icons"),
                              root.path() + QStringLiteral("/iconpacks"));
  QCOMPARE(registry.selectablePack(QStringLiteral("open")), QStringLiteral("open"));
  QVERIFY(registry.selectablePack(QStringLiteral("glyphs")).isEmpty());
  QVERIFY(registry.selectablePack(QStringLiteral("absent")).isEmpty());
  QVERIFY(registry.selectablePack(QString()).isEmpty());
}

/**
 * @brief Without the licensed set the Basic pack is the default and is not offered twice; with it,
 *        Basic is an ordinary choice that still fills in for any icon the set lacks (spec 0116).
 */
void TstIconPacks::basicPackStandsInForAnAbsentHouseSet()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeText(root.path(), QStringLiteral("icons/system/16/missing.svg"), kSvg));
  QVERIFY(writeText(root.path(),
                    QStringLiteral("iconpacks/basic/pack.json"),
                    QStringLiteral("{\"id\":\"basic\",\"title\":\"Basic\"}")));
  QVERIFY(writeText(root.path(), QStringLiteral("iconpacks/basic/commands/48/settings.svg"), kSvg));
  QVERIFY(writeText(root.path(), QStringLiteral("iconpacks/basic/commands/48/other.svg"), kSvg));

  const auto house    = root.path() + QStringLiteral("/icons");
  const auto packs    = root.path() + QStringLiteral("/iconpacks");
  const auto commands = QStringLiteral("commands");
  const auto basic    = QStringLiteral("basic");

  Misc::IconRegistry unlicensed(house, packs);
  QCOMPARE(unlicensed.houseTitle(), QStringLiteral("Basic"));
  QVERIFY(unlicensed.visiblePacks().isEmpty());
  QVERIFY(unlicensed.selectablePack(basic).isEmpty());
  QCOMPARE(unlicensed.icon(commands, QStringLiteral("settings"), 18),
           QStringLiteral("qrc:/iconpacks/basic/commands/48/settings.svg"));

  QVERIFY(writeText(root.path(), QStringLiteral("icons/commands/24/settings.svg"), kSvg));
  Misc::IconRegistry licensed(house, packs);
  QVERIFY(licensed.houseTitle().isEmpty());
  QCOMPARE(licensed.selectablePack(basic), basic);
  QCOMPARE(licensed.icon(commands, QStringLiteral("settings"), 18),
           QStringLiteral("qrc:/icons/commands/24/settings.svg"));
  QCOMPARE(licensed.icon(commands, QStringLiteral("other"), 18),
           QStringLiteral("qrc:/iconpacks/basic/commands/48/other.svg"));
}

QTEST_GUILESS_MAIN(TstIconPacks)
#include "tst_icon_packs.moc"
