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

#include "Misc/ThemeCatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

#include "Core/SSAssert.h"
#include "Misc/LanguageTable.h"

//--------------------------------------------------------------------------------------------------
// Catalog constants
//--------------------------------------------------------------------------------------------------

static const QString kSystemTheme   = QStringLiteral("System");
static const QString kFallbackTheme = QStringLiteral("Fallback");

/**
 * @brief The themes compiled into the binary, in the order the selector lists them.
 */
static QStringList builtInThemeFiles()
{
  return {QStringLiteral("default"), QStringLiteral("fluent-light"), QStringLiteral("fluent-dark")};
}

//--------------------------------------------------------------------------------------------------
// Loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the bundled themes out of the resource tree. A theme that fails to open, parse, or
 *        carry a title is skipped rather than fatal, and a build where every one of them fails
 *        still offers a single named entry so the selector is never empty.
 */
void Misc::ThemeCatalog::loadBuiltInThemes()
{
  const auto themes = builtInThemeFiles();
  for (const auto& theme : themes) {
    QFile file(QStringLiteral(":/themes/%1.json").arg(theme));
    if (!file.open(QFile::ReadOnly)) {
      qWarning() << "Failed to open theme resource:" << theme;
      continue;
    }

    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || document.isNull()) {
      qWarning() << "Failed to parse theme" << theme << ":" << parseError.errorString();
      continue;
    }

    const auto title = document.object().value(QStringLiteral("title")).toString();
    if (title.isEmpty()) {
      qWarning() << "Theme" << theme << "has no title, skipping";
      continue;
    }

    m_themes.insert(title, document.object());
    m_titles.append(title);
  }

  if (m_titles.isEmpty()) {
    qCritical() << "No themes loaded! Adding fallback";
    m_titles.append(kFallbackTheme);
  }
}

/**
 * @brief Rescans the extension folder for user themes, replacing the ones registered by the
 *        previous scan and keeping the "System" entry last.
 */
void Misc::ThemeCatalog::reloadUserThemes(const QString& themesDir)
{
  for (const auto& name : std::as_const(m_userThemes)) {
    m_themes.remove(name);
    m_titles.removeAll(name);
  }

  m_userThemes.clear();
  m_titles.removeAll(kSystemTheme);

  scanUserThemes(themesDir);

  m_titles.append(kSystemTheme);
  SS_ASSERT_LOG(m_titles.last() == kSystemTheme);
}

/**
 * @brief Walks one extension folder per subdirectory and registers every theme file it holds. A
 *        missing folder is normal (no extension installed yet), not an error.
 */
void Misc::ThemeCatalog::scanUserThemes(const QString& themesDir)
{
  QDir dir(themesDir);
  if (!dir.exists())
    return;

  const auto subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const auto& subdir : subdirs) {
    const auto subdirPath = themesDir + QStringLiteral("/") + subdir;
    QDir addonDir(subdirPath);
    const auto jsonFiles = addonDir.entryList({QStringLiteral("*.json")}, QDir::Files);
    for (const auto& jsonFile : jsonFiles)
      tryLoadUserThemeFile(subdirPath, jsonFile);
  }
}

/**
 * @brief Registers one user-theme JSON file. A theme whose title collides with an already known
 *        one is ignored, so an extension can never shadow a bundled theme; a code-editor theme
 *        reference is rewritten to the absolute path the editor can actually open.
 */
void Misc::ThemeCatalog::tryLoadUserThemeFile(const QString& subdirPath, const QString& jsonFile)
{
  QFile file(subdirPath + QStringLiteral("/") + jsonFile);
  if (!file.open(QFile::ReadOnly))
    return;

  QJsonParseError parseError;
  const auto doc = QJsonDocument::fromJson(file.readAll(), &parseError);
  if (parseError.error != QJsonParseError::NoError || doc.isNull())
    return;

  auto obj         = doc.object();
  const auto title = obj.value(QStringLiteral("title")).toString();
  if (title.isEmpty() || !obj.contains(QStringLiteral("colors")))
    return;

  if (m_themes.contains(title))
    return;

  auto params          = obj.value(QStringLiteral("parameters")).toObject();
  const auto editorKey = params.value(QStringLiteral("code-editor-theme")).toString();
  if (!editorKey.isEmpty()) {
    const auto xmlPath =
      subdirPath + QStringLiteral("/code-editor/") + editorKey + QStringLiteral(".xml");
    if (QFile::exists(xmlPath))
      params.insert(QStringLiteral("code-editor-theme"), xmlPath);

    obj.insert(QStringLiteral("parameters"), params);
  }

  m_themes.insert(title, obj);
  m_titles.append(title);
  m_userThemes.append(title);
}

//--------------------------------------------------------------------------------------------------
// Localization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the display names for @p language. A theme publishes its translations under the
 *        same keys the application uses for its own ".qm" files; English, an untranslated theme,
 *        and the "System" entry all fall back to a name that is always present.
 */
void Misc::ThemeCatalog::updateLocalizedNames(const Translator::Language language)
{
  m_localizedNames.clear();
  m_localizedNames.reserve(m_titles.count());

  const auto key = LanguageTable::qmName(language);

  for (const auto& title : std::as_const(m_titles)) {
    if (title == kSystemTheme) {
      m_localizedNames.append(QCoreApplication::translate("Misc::ThemeManager", "System"));
      continue;
    }

    const auto themeObj     = m_themes.value(title);
    const auto translations = themeObj.value(QStringLiteral("translations")).toObject();

    QString localized;
    if (language != Translator::English)
      localized = translations.value(key).toString();

    if (localized.isEmpty())
      localized = themeObj.value(QStringLiteral("title")).toString();

    if (localized.isEmpty())
      localized = title;

    m_localizedNames.append(localized);
  }

  SS_ASSERT_LOG(m_localizedNames.count() == m_titles.count());
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the title of the "follow the operating system" entry, which is not a theme file
 *        and always sorts last.
 */
QString Misc::ThemeCatalog::systemTitle()
{
  return kSystemTheme;
}

/**
 * @brief Returns how many entries the selector shows.
 */
int Misc::ThemeCatalog::count() const
{
  return static_cast<int>(m_titles.count());
}

/**
 * @brief Returns the position of @p title, or -1 when the catalog does not know it.
 */
int Misc::ThemeCatalog::indexOf(const QString& title) const
{
  return static_cast<int>(m_titles.indexOf(title));
}

/**
 * @brief Returns the title at @p index, or an empty string when the index is out of range.
 */
QString Misc::ThemeCatalog::titleAt(const int index) const
{
  if (index < 0 || index >= m_titles.count())
    return {};

  return m_titles.at(index);
}

/**
 * @brief Returns the raw JSON of a theme; an unknown title yields an empty object, which the
 *        caller reads as a theme with no colors rather than a crash.
 */
QJsonObject Misc::ThemeCatalog::theme(const QString& title) const
{
  return m_themes.value(title);
}

/**
 * @brief Returns true when a theme file is registered under @p title.
 */
bool Misc::ThemeCatalog::contains(const QString& title) const
{
  return m_themes.contains(title);
}

/**
 * @brief Returns the catalog titles, in selector order.
 */
const QStringList& Misc::ThemeCatalog::titles() const noexcept
{
  return m_titles;
}

/**
 * @brief Returns the titles contributed by installed extensions on the last scan.
 */
const QStringList& Misc::ThemeCatalog::userThemes() const noexcept
{
  return m_userThemes;
}

/**
 * @brief Returns the display names, parallel to titles().
 */
const QStringList& Misc::ThemeCatalog::localizedNames() const noexcept
{
  return m_localizedNames;
}
