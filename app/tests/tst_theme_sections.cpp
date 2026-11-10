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

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "Misc/ThemeSectionCheck.h"

#ifndef SS_THEME_DIR
#  error "SS_THEME_DIR must point at app/rcc/themes"
#endif

/**
 * @brief Pins the unknown-name check of a theme (spec 0113 R2, R3, R5): bundled themes are clean,
 *        a colour-only theme is silent, and every unknown name yields exactly one message.
 */
class TstThemeSections : public QObject {
  Q_OBJECT

private slots:
  void bundledThemesAreClean();
  void colourOnlyThemeIsSilent();
  void eachUnknownNameYieldsOneMessage();
  void pathLikeParameterIsAnUnknownParameter();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a JSON object from a literal, failing the caller's test on a typo.
 */
[[nodiscard]] static QJsonObject parseObject(const char* json)
{
  return QJsonDocument::fromJson(QByteArray(json)).object();
}

/**
 * @brief Counts the messages that mention @p name.
 */
[[nodiscard]] static int mentions(const QStringList& messages, const QString& name)
{
  int n = 0;
  for (const auto& message : messages)
    if (message.contains(name))
      ++n;

  return n;
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief The files the build ships must not trip their own check: a warning on a bundled theme is
 *        a defect in the theme or in the vocabulary, never something to ship.
 */
void TstThemeSections::bundledThemesAreClean()
{
  const QDir dir(QStringLiteral(SS_THEME_DIR));
  const auto files = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
  QVERIFY2(files.size() >= 6, "expected the six bundled themes");

  for (const auto& name : files) {
    QFile file(dir.filePath(name));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(name));

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    QVERIFY2(error.error == QJsonParseError::NoError, qPrintable(name));

    const auto messages = Misc::unknownThemeNames(doc.object());
    QVERIFY2(messages.isEmpty(), qPrintable(name + QStringLiteral(": ") + messages.join('|')));
  }
}

/**
 * @brief A theme written before materials existed carries only a title and colours; it must load
 *        without a single message (R2), or every older extension theme would start warning.
 */
void TstThemeSections::colourOnlyThemeIsSilent()
{
  const auto theme = parseObject(R"({"title": "Old", "colors": {"base": "#000000"}})");
  QVERIFY(!theme.isEmpty());
  QVERIFY(Misc::unknownThemeNames(theme).isEmpty());
}

/**
 * @brief One typo must produce one message that names it, so the log points at the fix. A
 *        malformed value ("lots") is deliberately not a warning: only names are checked.
 */
void TstThemeSections::eachUnknownNameYieldsOneMessage()
{
  const auto theme = parseObject(R"({
    "title": "Odd",
    "colors": {"base": "#000000"},
    "sparkle": {},
    "materials": {
      "steel": {"recipe": "chrome"},
      "wood": {"recipe": "flat", "grain": "lots"}
    },
    "surfaces": {"window": "wood", "toolbar": "marble"},
    "widgets": {"lamp1": "neon", "lamp2": "lamp"}
  })");
  QVERIFY(!theme.isEmpty());

  const auto messages = Misc::unknownThemeNames(theme);
  QCOMPARE(static_cast<int>(messages.size()), 4);
  QCOMPARE(mentions(messages, QStringLiteral("sparkle")), 1);
  QCOMPARE(mentions(messages, QStringLiteral("chrome")), 1);
  QCOMPARE(mentions(messages, QStringLiteral("marble")), 1);
  QCOMPARE(mentions(messages, QStringLiteral("neon")), 1);
  QCOMPARE(mentions(messages, QStringLiteral("lots")), 0);
}

/**
 * @brief No material parameter may name a file (R5): "image" and "file" are reported like any other
 *        unknown key and nothing else in the theme is disturbed, so a hostile theme cannot make
 *        the renderer reach outside the bundle.
 */
void TstThemeSections::pathLikeParameterIsAnUnknownParameter()
{
  const auto theme = parseObject(R"({
    "title": "Sneaky",
    "colors": {"base": "#000000"},
    "materials": {
      "a": {"recipe": "flat", "image": "../../../outside.png"},
      "b": {"recipe": "glass", "file": "C:/outside.png"}
    },
    "surfaces": {"window": "a"}
  })");
  QVERIFY(!theme.isEmpty());

  const auto messages = Misc::unknownThemeNames(theme);
  QCOMPARE(static_cast<int>(messages.size()), 2);
  QCOMPARE(mentions(messages, QStringLiteral("image")), 1);
  QCOMPARE(mentions(messages, QStringLiteral("file")), 1);
  QCOMPARE(mentions(messages, QStringLiteral("unknown parameter")), 2);
}

QTEST_APPLESS_MAIN(TstThemeSections)

#include "tst_theme_sections.moc"
