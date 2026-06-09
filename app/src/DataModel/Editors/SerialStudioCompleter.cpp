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

#include "DataModel/Editors/SerialStudioCompleter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLanguage>
#include <QStringListModel>

//--------------------------------------------------------------------------------------------------
// Symbol sources
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the base language keywords for JavaScript or Lua from the editor resources.
 */
static QStringList languageKeywords(bool lua)
{
  QStringList list;
  QFile file(lua ? QStringLiteral(":/languages/lua.xml")
                 : QStringLiteral(":/languages/javascript.xml"));
  if (!file.open(QIODevice::ReadOnly))
    return list;

  QLanguage language(&file);
  if (!language.isLoaded())
    return list;

  const auto keys = language.keys();
  for (const auto& key : keys)
    list.append(language.names(key));

  return list;
}

/**
 * @brief Returns every SerialStudio SDK symbol from the generated symbol list.
 */
static QStringList sdkSymbols()
{
  QFile file(QStringLiteral(":/api/sdk-symbols.json"));
  if (!file.open(QIODevice::ReadOnly))
    return {};

  const auto array = QJsonDocument::fromJson(file.readAll()).array();

  QStringList list;
  list.reserve(array.size());
  for (const auto& value : array)
    list.append(value.toString());

  return list;
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a completer merging the language keywords with all SDK symbols.
 */
DataModel::SerialStudioCompleter::SerialStudioCompleter(bool lua, QObject* parent)
  : QCompleter(parent)
{
  QStringList list = languageKeywords(lua);
  list.append(sdkSymbols());
  list.removeDuplicates();

  setModel(new QStringListModel(list, this));
  setCompletionColumn(0);
  setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  setCaseSensitivity(Qt::CaseSensitive);
  setWrapAround(true);
}
