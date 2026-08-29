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

#include "DataModel/Editors/ScriptTemplateCatalog.h"

#include <utility>

#include "DataModel/Scripting/ScriptTemplates.h"

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the manifest entry flagged as default, falling back to the first entry.
 */
static QString defaultFileOf(const QList<DataModel::ScriptTemplateDefinition>& templates)
{
  for (const auto& tmpl : templates)
    if (tmpl.isDefault)
      return tmpl.file;

  return templates.isEmpty() ? QString() : templates.constFirst().file;
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the catalog to a manifest without reading it; call reload() to populate.
 */
DataModel::ScriptTemplateCatalog::ScriptTemplateCatalog(QString manifestPath,
                                                        QString resourceRoot,
                                                        QString suffix,
                                                        const char* translationContext)
  : m_context(translationContext)
  , m_suffix(std::move(suffix))
  , m_manifestPath(std::move(manifestPath))
  , m_resourceRoot(std::move(resourceRoot))
{}

//--------------------------------------------------------------------------------------------------
// Catalog
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the cached names and resource paths from the manifest.
 */
void DataModel::ScriptTemplateCatalog::reload()
{
  m_names.clear();
  m_files.clear();
  m_defaultFile.clear();

  const auto templates = loadScriptTemplateManifest(m_manifestPath, m_context);
  for (const auto& tmpl : templates) {
    m_names.append(tmpl.name);
    m_files.append(templateResourcePath(m_resourceRoot, tmpl.file, m_suffix));
  }

  m_defaultFile = defaultFileOf(templates);
}

/**
 * @brief Returns true when the manifest yielded no templates.
 */
bool DataModel::ScriptTemplateCatalog::isEmpty() const noexcept
{
  return m_names.isEmpty();
}

/**
 * @brief Returns the manifest file name of the default template.
 */
QString DataModel::ScriptTemplateCatalog::defaultFile() const
{
  return m_defaultFile;
}

/**
 * @brief Returns the localized template display names.
 */
const QStringList& DataModel::ScriptTemplateCatalog::names() const noexcept
{
  return m_names;
}

/**
 * @brief Returns the resolved resource path of every template, parallel to names().
 */
const QStringList& DataModel::ScriptTemplateCatalog::files() const noexcept
{
  return m_files;
}

//--------------------------------------------------------------------------------------------------
// Standalone default lookup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the default template's source straight from the manifest, for callers that have no
 *        catalog instance (project creation seeds a new widget with it).
 */
QString DataModel::defaultScriptTemplateCode(const QString& manifestPath,
                                             const QString& resourceRoot,
                                             const QString& suffix,
                                             const char* translationContext)
{
  const auto templates  = loadScriptTemplateManifest(manifestPath, translationContext);
  const QString defFile = defaultFileOf(templates);
  if (defFile.isEmpty())
    return {};

  return readTextResource(templateResourcePath(resourceRoot, defFile, suffix));
}
