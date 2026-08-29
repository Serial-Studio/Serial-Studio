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

#include "DataModel/Scripting/ParserTemplateCatalog.h"

#include <QJsonDocument>

#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptTemplates.h"
#include "SerialStudio.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Resource layout
//--------------------------------------------------------------------------------------------------

static const QString kManifestPath  = QStringLiteral(":/scripts/parser/templates.json");
static const char* const kTrContext = "DataModel::FrameParser";

/**
 * @brief Returns the resource directory holding the shipped templates of @p language.
 */
[[nodiscard]] static QString scriptDirectory(int language)
{
  return (language == SerialStudio::Lua) ? QStringLiteral(":/scripts/parser/lua")
                                         : QStringLiteral(":/scripts/parser/js");
}

/**
 * @brief Returns the file suffix of the shipped templates of @p language.
 */
[[nodiscard]] static QString scriptSuffix(int language)
{
  return (language == SerialStudio::Lua) ? QStringLiteral(".lua") : QStringLiteral(".js");
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty catalog; reload() fills it from the resource manifest and the registry.
 */
DataModel::ParserTemplateCatalog::ParserTemplateCatalog()
  : m_defaultFile(), m_files(), m_names(), m_nativeNames()
{}

/**
 * @brief Rebuilds the template file list, the localized display names and the native name list.
 */
void DataModel::ParserTemplateCatalog::reload()
{
  m_defaultFile.clear();
  m_files.clear();
  m_names.clear();
  m_nativeNames.clear();

  const auto templates = loadScriptTemplateManifest(kManifestPath, kTrContext);
  for (const auto& tmpl : templates) {
    m_files.append(tmpl.file);
    m_names.append(tmpl.name);
    if (m_defaultFile.isEmpty() && tmpl.isDefault)
      m_defaultFile = tmpl.file;
  }

  if (m_defaultFile.isEmpty() && !m_files.isEmpty())
    m_defaultFile = m_files.constFirst();

  const auto& native = nativeTemplates();
  for (const auto* tmpl : native) {
    SS_ASSERT_LOG(tmpl != nullptr);
    m_nativeNames.append(tmpl->name());
  }
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of shipped JS/Lua templates.
 */
int DataModel::ParserTemplateCatalog::fileCount() const noexcept
{
  return static_cast<int>(m_files.size());
}

/**
 * @brief Returns the index of the default template file, or -1 when the manifest is empty.
 */
int DataModel::ParserTemplateCatalog::defaultFileIndex() const
{
  return static_cast<int>(m_files.indexOf(m_defaultFile));
}

/**
 * @brief Returns the resource file basenames of every available template.
 */
const QStringList& DataModel::ParserTemplateCatalog::files() const noexcept
{
  return m_files;
}

/**
 * @brief Returns the localized display names of the shipped JS/Lua templates.
 */
const QStringList& DataModel::ParserTemplateCatalog::names() const noexcept
{
  return m_names;
}

/**
 * @brief Returns the display names of the compiled-in native templates, registry-ordered.
 */
const QStringList& DataModel::ParserTemplateCatalog::nativeNames() const noexcept
{
  return m_nativeNames;
}

//--------------------------------------------------------------------------------------------------
// Template code
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the shipped template @p file for @p language out of the resource tree.
 */
QString DataModel::ParserTemplateCatalog::scriptResource(const QString& file, int language)
{
  SS_ASSERT(!file.isEmpty(), return {});

  const auto path = templateResourcePath(scriptDirectory(language), file, scriptSuffix(language));
  SS_ASSERT_LOG(!path.isEmpty());
  return readTextResource(path);
}

/**
 * @brief Returns the code (or native descriptor) of the template at @p index for @p language.
 */
QString DataModel::ParserTemplateCatalog::codeForIndex(int index, int language) const
{
  if (language == SerialStudio::Native) {
    const auto& templates = nativeTemplates();
    if (index < 0 || index >= templates.size())
      return {};

    const auto* tmpl = templates.at(index);
    SS_ASSERT(tmpl != nullptr, return {});
    return CFrameParser::buildDescriptor(tmpl->id(), nativeTemplateDefaults(*tmpl));
  }

  if (index < 0 || index >= m_files.count())
    return {};

  return scriptResource(m_files.at(index), language);
}

/**
 * @brief Returns the default frame parser template code for @p language.
 */
QString DataModel::ParserTemplateCatalog::defaultCode(int language)
{
  if (language == SerialStudio::Native) {
    const auto* tmpl = nativeTemplateById(defaultNativeTemplateId());
    SS_ASSERT(tmpl != nullptr, return {});
    return CFrameParser::buildDescriptor(tmpl->id(), nativeTemplateDefaults(*tmpl));
  }

  const auto templates = loadScriptTemplateManifest(kManifestPath, kTrContext);

  QString defaultFile;
  for (const auto& tmpl : templates) {
    if (tmpl.isDefault) {
      defaultFile = tmpl.file;
      break;
    }
  }

  if (defaultFile.isEmpty() && !templates.isEmpty())
    defaultFile = templates.constFirst().file;

  if (defaultFile.isEmpty())
    return {};

  return scriptResource(defaultFile, language);
}

//--------------------------------------------------------------------------------------------------
// Detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the native registry index matching a JSON descriptor, or -1.
 */
int DataModel::ParserTemplateCatalog::detectNativeTemplate(const QString& descriptor)
{
  SS_ASSERT(!descriptor.isEmpty(), return -1);

  const auto doc = QJsonDocument::fromJson(descriptor.toUtf8());
  if (!doc.isObject())
    return -1;

  const QString id = doc.object().value(QStringLiteral("template")).toString();
  if (id.isEmpty())
    return -1;

  const auto& templates = nativeTemplates();
  for (int i = 0; i < templates.size(); ++i)
    if (templates.at(i)->id() == id)
      return i;

  return -1;
}

/**
 * @brief Returns the template index whose shipped code matches @p code in either language, or -1.
 */
int DataModel::ParserTemplateCatalog::detect(const QString& code) const
{
  const QString trimmed = code.trimmed();
  if (trimmed.isEmpty())
    return -1;

  if (trimmed.startsWith(QLatin1Char('{')))
    return detectNativeTemplate(trimmed);

  for (int i = 0; i < m_files.size(); ++i) {
    const auto& file = m_files[i];

    const QString luaCode = scriptResource(file, SerialStudio::Lua).trimmed();
    if (!luaCode.isEmpty() && luaCode == trimmed)
      return i;

    const QString jsCode = scriptResource(file, SerialStudio::JavaScript).trimmed();
    if (!jsCode.isEmpty() && jsCode == trimmed)
      return i;
  }

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Script <-> native template mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a JS/Lua template file basename to its native template id and default params.
 */
bool DataModel::ParserTemplateCatalog::nativeEquivalentForFile(const QString& file,
                                                               QString& templateId,
                                                               QJsonObject& params)
{
  struct DelimitedVariant {
    QLatin1StringView file;
    QLatin1StringView separator;
  };

  static constexpr DelimitedVariant kDelimited[] = {
    {    QLatin1StringView("comma_separated"),   QLatin1StringView(",")},
    {      QLatin1StringView("tab_separated"), QLatin1StringView("\\t")},
    {     QLatin1StringView("pipe_delimited"),   QLatin1StringView("|")},
    {QLatin1StringView("semicolon_separated"),   QLatin1StringView(";")},
  };

  for (const auto& variant : kDelimited) {
    if (file != variant.file)
      continue;

    const auto* tmpl = nativeTemplateById(QStringLiteral("delimited"));
    SS_ASSERT(tmpl != nullptr, return false);

    templateId = tmpl->id();
    params     = nativeTemplateDefaults(*tmpl);
    params.insert(QStringLiteral("separator"), QString(variant.separator));
    return true;
  }

  QString id = file;
  if (file == QStringLiteral("fixed_width_fields"))
    id = QStringLiteral("fixed_width");
  else if (file == QStringLiteral("key_value_pairs"))
    id = QStringLiteral("key_value");

  const auto* tmpl = nativeTemplateById(id);
  if (!tmpl)
    return false;

  templateId = tmpl->id();
  params     = nativeTemplateDefaults(*tmpl);
  return true;
}

/**
 * @brief Maps a native template id (+ params) to the equivalent JS/Lua template file.
 */
QString DataModel::ParserTemplateCatalog::fileForNativeTemplate(const QString& templateId,
                                                                const QJsonObject& params)
{
  SS_ASSERT(!nativeTemplates().isEmpty(), return {});

  if (templateId == QStringLiteral("delimited")) {
    const QString separator = SerialStudio::resolveEscapeSequences(
      params.value(QStringLiteral("separator")).toString(QStringLiteral(",")));

    if (separator == QStringLiteral("\t"))
      return QStringLiteral("tab_separated");

    if (separator == QStringLiteral("|"))
      return QStringLiteral("pipe_delimited");

    if (separator == QStringLiteral(";"))
      return QStringLiteral("semicolon_separated");

    return QStringLiteral("comma_separated");
  }

  if (templateId == QStringLiteral("fixed_width"))
    return QStringLiteral("fixed_width_fields");

  if (templateId == QStringLiteral("key_value"))
    return QStringLiteral("key_value_pairs");

  return templateId;
}
