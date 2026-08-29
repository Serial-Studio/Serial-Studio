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
#include <QTemporaryDir>
#include <QTest>

#include "Misc/ThemeCatalog.h"

// Every test function here is self-contained: each builds its own extension folder in a fresh
// QTemporaryDir, so Qt Test's declaration-order execution is never load-bearing.

/**
 * @brief Covers the user-theme scan and the localized-name projection of Misc::ThemeCatalog: the
 *        half of the old ThemeManager that decides which themes exist and what they are called.
 */
class TstThemeCatalog : public QObject {
  Q_OBJECT

private slots:
  void userThemeIsRegisteredAndSystemStaysLast();
  void rescanDropsThemesThatWentAway();
  void duplicateTitleIsIgnored();
  void malformedThemeIsRejected();
  void codeEditorThemeResolvesToAnAbsolutePath();
  void localizedNamesFollowTheLanguage();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p theme as <root>/<folder>/theme.json, creating the folder.
 */
static bool writeTheme(const QString& root, const QString& folder, const QJsonObject& theme)
{
  const auto dir = root + QStringLiteral("/") + folder;
  if (!QDir().mkpath(dir))
    return false;

  QFile file(dir + QStringLiteral("/theme.json"));
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return false;

  file.write(QJsonDocument(theme).toJson(QJsonDocument::Compact));
  file.close();
  return true;
}

/**
 * @brief Builds the smallest object the catalog accepts as a theme.
 */
static QJsonObject minimalTheme(const QString& title)
{
  QJsonObject colors;
  colors.insert(QStringLiteral("base"), QStringLiteral("#000000"));

  QJsonObject theme;
  theme.insert(QStringLiteral("title"), title);
  theme.insert(QStringLiteral("colors"), colors);
  return theme;
}

//--------------------------------------------------------------------------------------------------
// Scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief A theme folder becomes one catalog entry, and the "System" row stays at the end where
 *        the selector expects it.
 */
void TstThemeCatalog::userThemeIsRegisteredAndSystemStaysLast()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeTheme(root.path(), QStringLiteral("neon"), minimalTheme(QStringLiteral("Neon"))));

  Misc::ThemeCatalog catalog;
  catalog.reloadUserThemes(root.path());

  QVERIFY(catalog.contains(QStringLiteral("Neon")));
  QVERIFY(catalog.userThemes().contains(QStringLiteral("Neon")));
  QVERIFY(catalog.indexOf(QStringLiteral("Neon")) >= 0);
  QCOMPARE(catalog.titleAt(catalog.count() - 1), Misc::ThemeCatalog::systemTitle());
  QCOMPARE(catalog.titleAt(catalog.count()), QString());
  QCOMPARE(catalog.indexOf(QStringLiteral("Nope")), -1);
}

/**
 * @brief An uninstalled theme leaves the catalog on the next scan, and "System" is not duplicated.
 */
void TstThemeCatalog::rescanDropsThemesThatWentAway()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeTheme(root.path(), QStringLiteral("neon"), minimalTheme(QStringLiteral("Neon"))));

  Misc::ThemeCatalog catalog;
  catalog.reloadUserThemes(root.path());
  QCOMPARE(static_cast<int>(catalog.userThemes().count()), 1);

  QVERIFY(QDir(root.path() + QStringLiteral("/neon")).removeRecursively());
  catalog.reloadUserThemes(root.path());

  QVERIFY(!catalog.contains(QStringLiteral("Neon")));
  QVERIFY(catalog.userThemes().isEmpty());
  QCOMPARE(static_cast<int>(catalog.titles().count(Misc::ThemeCatalog::systemTitle())), 1);
}

/**
 * @brief Two extensions publishing the same title must not both claim it; the first scan wins so
 *        an extension can never shadow a bundled theme.
 */
void TstThemeCatalog::duplicateTitleIsIgnored()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());
  QVERIFY(writeTheme(root.path(), QStringLiteral("a"), minimalTheme(QStringLiteral("Twin"))));
  QVERIFY(writeTheme(root.path(), QStringLiteral("b"), minimalTheme(QStringLiteral("Twin"))));

  Misc::ThemeCatalog catalog;
  catalog.reloadUserThemes(root.path());

  QCOMPARE(static_cast<int>(catalog.titles().count(QStringLiteral("Twin"))), 1);
  QCOMPARE(static_cast<int>(catalog.userThemes().count()), 1);
}

/**
 * @brief A theme with no title, no colors, or unparseable JSON is skipped instead of registered.
 */
void TstThemeCatalog::malformedThemeIsRejected()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());

  QJsonObject titleless;
  titleless.insert(QStringLiteral("colors"), QJsonObject());
  QVERIFY(writeTheme(root.path(), QStringLiteral("no-title"), titleless));

  QJsonObject colorless;
  colorless.insert(QStringLiteral("title"), QStringLiteral("Colorless"));
  QVERIFY(writeTheme(root.path(), QStringLiteral("no-colors"), colorless));

  const auto brokenDir = root.path() + QStringLiteral("/broken");
  QVERIFY(QDir().mkpath(brokenDir));
  QFile broken(brokenDir + QStringLiteral("/theme.json"));
  QVERIFY(broken.open(QIODevice::WriteOnly));
  broken.write("{ not json");
  broken.close();

  Misc::ThemeCatalog catalog;
  catalog.reloadUserThemes(root.path());

  QVERIFY(catalog.userThemes().isEmpty());
  QVERIFY(!catalog.contains(QStringLiteral("Colorless")));
}

/**
 * @brief A code-editor theme is referenced by key; the catalog rewrites it to the file next to the
 *        theme, because the editor is handed a path and cannot resolve the key itself.
 */
void TstThemeCatalog::codeEditorThemeResolvesToAnAbsolutePath()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());

  auto theme = minimalTheme(QStringLiteral("Editorish"));
  QJsonObject parameters;
  parameters.insert(QStringLiteral("code-editor-theme"), QStringLiteral("midnight"));
  theme.insert(QStringLiteral("parameters"), parameters);
  QVERIFY(writeTheme(root.path(), QStringLiteral("editorish"), theme));

  const auto editorDir = root.path() + QStringLiteral("/editorish/code-editor");
  QVERIFY(QDir().mkpath(editorDir));
  QFile xml(editorDir + QStringLiteral("/midnight.xml"));
  QVERIFY(xml.open(QIODevice::WriteOnly));
  xml.write("<theme/>");
  xml.close();

  Misc::ThemeCatalog catalog;
  catalog.reloadUserThemes(root.path());

  const auto stored = catalog.theme(QStringLiteral("Editorish"))
                        .value(QStringLiteral("parameters"))
                        .toObject()
                        .value(QStringLiteral("code-editor-theme"))
                        .toString();
  QCOMPARE(stored, editorDir + QStringLiteral("/midnight.xml"));
}

//--------------------------------------------------------------------------------------------------
// Localization
//--------------------------------------------------------------------------------------------------

/**
 * @brief The display name comes from the theme's own translations, keyed exactly like the .qm
 *        files; English and a missing translation both fall back to the theme title.
 */
void TstThemeCatalog::localizedNamesFollowTheLanguage()
{
  QTemporaryDir root;
  QVERIFY(root.isValid());

  auto translated = minimalTheme(QStringLiteral("Midnight"));
  QJsonObject translations;
  translations.insert(QStringLiteral("de_DE"), QStringLiteral("Mitternacht"));
  translated.insert(QStringLiteral("translations"), translations);
  QVERIFY(writeTheme(root.path(), QStringLiteral("midnight"), translated));
  QVERIFY(writeTheme(root.path(), QStringLiteral("plain"), minimalTheme(QStringLiteral("Plain"))));

  Misc::ThemeCatalog catalog;
  catalog.reloadUserThemes(root.path());

  catalog.updateLocalizedNames(Misc::Translator::German);
  QCOMPARE(static_cast<int>(catalog.localizedNames().count()),
           static_cast<int>(catalog.titles().count()));
  QCOMPARE(catalog.localizedNames().at(catalog.indexOf(QStringLiteral("Midnight"))),
           QStringLiteral("Mitternacht"));
  QCOMPARE(catalog.localizedNames().at(catalog.indexOf(QStringLiteral("Plain"))),
           QStringLiteral("Plain"));

  catalog.updateLocalizedNames(Misc::Translator::English);
  QCOMPARE(catalog.localizedNames().at(catalog.indexOf(QStringLiteral("Midnight"))),
           QStringLiteral("Midnight"));

  const int systemIndex = catalog.indexOf(Misc::ThemeCatalog::systemTitle());
  QVERIFY(systemIndex >= 0);
  QVERIFY(!catalog.localizedNames().at(systemIndex).isEmpty());
}

QTEST_APPLESS_MAIN(TstThemeCatalog)

#include "tst_theme_catalog.moc"
