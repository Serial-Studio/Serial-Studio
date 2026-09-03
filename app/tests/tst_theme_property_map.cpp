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

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QQmlPropertyMap>
#include <QSignalSpy>
#include <QTest>

#include "Misc/ThemeManager.h"

// The theme colors QML reads (spec 0075, G2). `colors` used to be a QVariantMap property: every
// one of the ~2700 bindings that read a color converted all 118 entries, so a theme switch or a
// dashboard rebuild paid bindings x 118 conversions. It is a QQmlPropertyMap now, which makes a
// read one lookup and a notify per key -- but only if the republish honours three rules: every
// key of the shipped theme is present (a missing property is a QML reference error, not an empty
// color), only the keys that actually moved notify, and a key a later theme drops is emptied
// rather than removed. The bracket syntax the QML uses is unchanged either way.

class ThemePropertyMapTest : public QObject {
  Q_OBJECT

private slots:
  void seedsEveryKeyOfTheShippedTheme();
  void unchangedKeysDoNotNotify();
  void changedKeysNotifyOncePerKey();
  void droppedKeysStayPresentButEmpty();

private:
  [[nodiscard]] static QVariantMap shippedColors();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the checked-in default theme's color table straight off disk, so the key set the
 *        map has to carry is the one the application ships rather than one the test invented.
 */
QVariantMap ThemePropertyMapTest::shippedColors()
{
  QFile file(QStringLiteral(SS_THEME_DEFAULT_JSON));
  if (!file.open(QIODevice::ReadOnly))
    return {};

  const auto document = QJsonDocument::fromJson(file.readAll());
  const auto colors   = document.object().value(QStringLiteral("colors")).toObject();

  QVariantMap map;
  for (auto it = colors.constBegin(); it != colors.constEnd(); ++it)
    map.insert(it.key(), it.value().toVariant());

  return map;
}

namespace {
/**
 * @brief The notify signal a QML binding on @p key listens to. QQmlPropertyMap::valueChanged
 *        reports only writes arriving from QML, so a republish made in C++ is observable through
 *        the property's own notify signal and nowhere else.
 */
[[nodiscard]] QMetaMethod notifySignal(const QQmlPropertyMap& map, const QString& key)
{
  const auto* metaObject = map.metaObject();
  const int index        = metaObject->indexOfProperty(key.toUtf8().constData());
  return index < 0 ? QMetaMethod() : metaObject->property(index).notifySignal();
}
}  // namespace

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every color the shipped theme declares resolves through the map after one republish.
 */
void ThemePropertyMapTest::seedsEveryKeyOfTheShippedTheme()
{
  const auto colors = shippedColors();
  QVERIFY(colors.size() > 100);

  QObject owner;
  QQmlPropertyMap& map = *QQmlPropertyMap::create(&owner);
  Misc::syncColorMap(map, colors);

  for (auto it = colors.cbegin(); it != colors.cend(); ++it) {
    QVERIFY2(map.contains(it.key()), qPrintable(it.key()));
    QCOMPARE(map.value(it.key()), it.value());
  }
}

/**
 * @brief Republishing the same theme notifies nothing: a repaint must not re-evaluate every
 *        binding that reads a color.
 */
void ThemePropertyMapTest::unchangedKeysDoNotNotify()
{
  QVariantMap colors;
  colors.insert(QStringLiteral("text"), QStringLiteral("#101010"));
  colors.insert(QStringLiteral("base"), QStringLiteral("#f0f0f0"));

  QObject owner;
  QQmlPropertyMap& map = *QQmlPropertyMap::create(&owner);
  Misc::syncColorMap(map, colors);

  const auto textSignal = notifySignal(map, QStringLiteral("text"));
  const auto baseSignal = notifySignal(map, QStringLiteral("base"));
  QVERIFY(textSignal.isValid());
  QVERIFY(baseSignal.isValid());

  QSignalSpy textSpy(&map, textSignal);
  QSignalSpy baseSpy(&map, baseSignal);
  Misc::syncColorMap(map, colors);

  QCOMPARE(textSpy.count(), 0);
  QCOMPARE(baseSpy.count(), 0);
}

/**
 * @brief A theme switch notifies the keys that moved, and only those.
 */
void ThemePropertyMapTest::changedKeysNotifyOncePerKey()
{
  QVariantMap light;
  light.insert(QStringLiteral("text"), QStringLiteral("#101010"));
  light.insert(QStringLiteral("base"), QStringLiteral("#f0f0f0"));

  QObject owner;
  QQmlPropertyMap& map = *QQmlPropertyMap::create(&owner);
  Misc::syncColorMap(map, light);

  QVariantMap dark = light;
  dark.insert(QStringLiteral("text"), QStringLiteral("#ededed"));

  const auto textSignal = notifySignal(map, QStringLiteral("text"));
  const auto baseSignal = notifySignal(map, QStringLiteral("base"));
  QVERIFY(textSignal.isValid());
  QVERIFY(baseSignal.isValid());

  QSignalSpy textSpy(&map, textSignal);
  QSignalSpy baseSpy(&map, baseSignal);
  Misc::syncColorMap(map, dark);

  QCOMPARE(textSpy.count(), 1);
  QCOMPARE(baseSpy.count(), 0);
  QCOMPARE(map.value(QStringLiteral("text")).toString(), QStringLiteral("#ededed"));
}

/**
 * @brief A key the next theme omits keeps its property and empties out, so a binding reading it
 *        resolves to an empty color instead of failing to resolve at all.
 */
void ThemePropertyMapTest::droppedKeysStayPresentButEmpty()
{
  QVariantMap full;
  full.insert(QStringLiteral("text"), QStringLiteral("#101010"));
  full.insert(QStringLiteral("accent"), QStringLiteral("#2277ff"));

  QObject owner;
  QQmlPropertyMap& map = *QQmlPropertyMap::create(&owner);
  Misc::syncColorMap(map, full);

  QVariantMap partial;
  partial.insert(QStringLiteral("text"), QStringLiteral("#101010"));
  Misc::syncColorMap(map, partial);

  QVERIFY(map.contains(QStringLiteral("accent")));
  QVERIFY(!map.value(QStringLiteral("accent")).isValid());
}

QTEST_APPLESS_MAIN(ThemePropertyMapTest)

#include "tst_theme_property_map.moc"
