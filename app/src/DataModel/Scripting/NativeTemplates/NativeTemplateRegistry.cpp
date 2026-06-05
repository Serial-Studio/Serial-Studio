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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

//--------------------------------------------------------------------------------------------------
// Registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns every native parser template in display order (default template first).
 */
const QList<const DataModel::INativeTemplate*>& DataModel::nativeTemplates()
{
  static const QList<const INativeTemplate*> s_templates = [] {
    QList<const INativeTemplate*> list;
    list += textNativeTemplates();
    list += binaryNativeTemplates();
    list += mapNativeTemplates();
    list += multiFrameNativeTemplates();
    return list;
  }();

  Q_ASSERT(!s_templates.isEmpty());
  Q_ASSERT(s_templates.constFirst() != nullptr);
  return s_templates;
}

/**
 * @brief Returns the template descriptor with the given stable id, or nullptr.
 */
const DataModel::INativeTemplate* DataModel::nativeTemplateById(const QString& id)
{
  if (id.isEmpty())
    return nullptr;

  const auto& templates = nativeTemplates();
  for (const auto* tmpl : templates) {
    Q_ASSERT(tmpl != nullptr);
    if (tmpl->id() == id)
      return tmpl;
  }

  return nullptr;
}

/**
 * @brief Returns the id of the default native template (first registry entry).
 */
QString DataModel::defaultNativeTemplateId()
{
  const auto& templates = nativeTemplates();
  Q_ASSERT(!templates.isEmpty());

  return templates.constFirst()->id();
}
