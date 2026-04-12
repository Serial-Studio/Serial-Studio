/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include "DataModel/ScriptTemplates.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Misc/Translator.h"

namespace {

QString localeKeyForLanguage(const Misc::Translator::Language language)
{
  switch (language) {
    case Misc::Translator::Spanish:
      return QStringLiteral("es_MX");
    case Misc::Translator::Chinese:
      return QStringLiteral("zh_CN");
    case Misc::Translator::German:
      return QStringLiteral("de_DE");
    case Misc::Translator::Russian:
      return QStringLiteral("ru_RU");
    case Misc::Translator::French:
      return QStringLiteral("fr_FR");
    case Misc::Translator::Japanese:
      return QStringLiteral("ja_JP");
    case Misc::Translator::Korean:
      return QStringLiteral("ko_KR");
    case Misc::Translator::Portuguese:
      return QStringLiteral("pt_BR");
    case Misc::Translator::Italian:
      return QStringLiteral("it_IT");
    case Misc::Translator::Polish:
      return QStringLiteral("pl_PL");
    case Misc::Translator::Turkish:
      return QStringLiteral("tr_TR");
    case Misc::Translator::Ukrainian:
      return QStringLiteral("uk_UA");
    case Misc::Translator::Czech:
      return QStringLiteral("cs_CZ");
    case Misc::Translator::Hindi:
      return QStringLiteral("hi_IN");
    case Misc::Translator::Dutch:
      return QStringLiteral("nl_NL");
    case Misc::Translator::Romanian:
      return QStringLiteral("ro_RO");
    case Misc::Translator::Swedish:
      return QStringLiteral("sv_SE");
    case Misc::Translator::English:
    default:
      return QStringLiteral("en_US");
  }
}

}  // namespace

QString DataModel::readTextResource(const QString& path)
{
  QFile file(path);
  if (file.open(QFile::ReadOnly))
    return QString::fromUtf8(file.readAll());

  return {};
}

QString DataModel::localizedTemplateName(const QJsonObject& object, const char* translationContext)
{
  const QString fallback  = object.value(QStringLiteral("name")).toString();
  const auto translations = object.value(QStringLiteral("translations")).toObject();
  const QString localeKey = localeKeyForLanguage(Misc::Translator::instance().language());

  if (!localeKey.isEmpty()) {
    const QString localized = translations.value(localeKey).toString();
    if (!localized.isEmpty())
      return localized;
  }

  if (translationContext && *translationContext && !fallback.isEmpty()) {
    const QByteArray utf8    = fallback.toUtf8();
    const QString translated = QCoreApplication::translate(translationContext, utf8.constData());
    if (!translated.isEmpty())
      return translated;
  }

  return fallback;
}

QList<DataModel::ScriptTemplateDefinition> DataModel::loadScriptTemplateManifest(
  const QString& manifestPath, const char* translationContext)
{
  QList<ScriptTemplateDefinition> templates;
  const auto json = readTextResource(manifestPath);
  if (json.isEmpty())
    return templates;

  QJsonParseError parseError;
  const auto document = QJsonDocument::fromJson(json.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isArray())
    return templates;

  const auto array = document.array();
  templates.reserve(array.size());

  for (const auto& entry : array) {
    if (!entry.isObject())
      continue;

    const auto object = entry.toObject();
    const auto file   = object.value(QStringLiteral("file")).toString();
    if (file.isEmpty())
      continue;

    ScriptTemplateDefinition definition;
    definition.file      = file;
    definition.name      = localizedTemplateName(object, translationContext);
    definition.isDefault = object.value(QStringLiteral("default")).toBool(false);
    templates.append(definition);
  }

  return templates;
}

QString DataModel::templateResourcePath(const QString& directory,
                                        const QString& file,
                                        const QString& suffix)
{
  return QStringLiteral("%1/%2%3").arg(directory, file, suffix);
}
