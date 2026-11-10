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

#include "Misc/ThemeSectionCheck.h"

#include <QVariantMap>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Vocabulary
//--------------------------------------------------------------------------------------------------

/**
 * @brief The names this build understands in a material theme (spec 0113 R1, R7): sections,
 *        recipes, the parameters MaterialSurface.qml reads, and the widget looks.
 */
static const QStringList& knownSections()
{
  static const QStringList kNames = {QStringLiteral("title"),
                                     QStringLiteral("parameters"),
                                     QStringLiteral("translations"),
                                     QStringLiteral("colors"),
                                     QStringLiteral("fonts"),
                                     QStringLiteral("materials"),
                                     QStringLiteral("surfaces"),
                                     QStringLiteral("styles"),
                                     QStringLiteral("widgets")};
  return kNames;
}

static const QStringList& knownRecipes()
{
  static const QStringList kNames = {QStringLiteral("flat"),
                                     QStringLiteral("gradient"),
                                     QStringLiteral("brushed-metal"),
                                     QStringLiteral("glass")};
  return kNames;
}

static const QStringList& knownParameters()
{
  static const QStringList kNames = {QStringLiteral("recipe"),
                                     QStringLiteral("color"),
                                     QStringLiteral("top"),
                                     QStringLiteral("bottom"),
                                     QStringLiteral("grain"),
                                     QStringLiteral("shade"),
                                     QStringLiteral("bevel")};
  return kNames;
}

static const QStringList& knownLooks()
{
  static const QStringList kNames = {
    QStringLiteral("lamp"), QStringLiteral("hardware"), QStringLiteral("plain")};
  return kNames;
}

//--------------------------------------------------------------------------------------------------
// Check
//--------------------------------------------------------------------------------------------------

/**
 * @brief Names every material's unknown recipe and unknown parameters: a theme is data and no
 *        parameter names a file, so a path-like key is ignored like any other (spec 0113 R5).
 */
static void checkMaterials(const QVariantMap& materials, QStringList& out)
{
  SS_ASSERT(knownRecipes().size() == 4, return);
  SS_ASSERT(knownParameters().contains(QStringLiteral("recipe")), return);
  for (auto it = materials.cbegin(); it != materials.cend(); ++it) {
    const auto material = it.value().toMap();
    const auto recipe   = material.value(QStringLiteral("recipe")).toString();
    if (!knownRecipes().contains(recipe))
      out << QStringLiteral("material %1 names an unknown recipe %2").arg(it.key(), recipe);

    for (auto key = material.cbegin(); key != material.cend(); ++key)
      if (!knownParameters().contains(key.key()))
        out << QStringLiteral("material %1 has an unknown parameter %2").arg(it.key(), key.key());
  }
}

/**
 * @brief Returns one message per name a material theme uses and this build does not know (spec
 *        0113 R3): a section, a recipe, a material parameter, a surface pointing at a material the
 *        theme never defines, a widget look. Empty for a colour-only theme. The theme still
 *        applies; every consumer ignores what it cannot resolve.
 */
QStringList Misc::unknownThemeNames(const QJsonObject& data)
{
  SS_ASSERT(knownSections().contains(QStringLiteral("colors")), return {});
  SS_ASSERT(knownLooks().contains(QStringLiteral("plain")), return {});

  QStringList out;
  const auto sections = data.keys();
  for (const auto& section : sections)
    if (!knownSections().contains(section))
      out << QStringLiteral("unknown section %1").arg(section);

  const auto materials = data.value(QStringLiteral("materials")).toObject().toVariantMap();
  const auto surfaces  = data.value(QStringLiteral("surfaces")).toObject().toVariantMap();
  const auto looks     = data.value(QStringLiteral("widgets")).toObject().toVariantMap();
  checkMaterials(materials, out);

  for (auto it = surfaces.cbegin(); it != surfaces.cend(); ++it)
    if (!materials.contains(it.value().toString()))
      out << QStringLiteral("surface %1 names an unknown material %2")
               .arg(it.key(), it.value().toString());

  for (auto it = looks.cbegin(); it != looks.cend(); ++it)
    if (!knownLooks().contains(it.value().toString()))
      out << QStringLiteral("widget look %1 is unknown: %2").arg(it.key(), it.value().toString());

  return out;
}
