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

#include "Core/SSAssert.h"
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
    list += multiFrameNativeTemplates();
    return list;
  }();

  SS_ASSERT_LOG(!s_templates.isEmpty());
  SS_ASSERT_LOG(!s_templates.isEmpty() && s_templates.constFirst() != nullptr);
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
    SS_ASSERT_LOG(tmpl != nullptr);
    if (!tmpl)
      continue;

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
  SS_ASSERT(!templates.isEmpty(), return {});

  return templates.constFirst()->id();
}
