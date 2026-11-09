/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "Misc/IconPacks.h"

#include <QColor>
#include <QCoreApplication>
#include <QJSEngine>
#include <QtLogging>

#include "Core/LanguageTable.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/Translator.h"
#include "Misc/ThemeManager.h"

static const QString kSettingsKey = QStringLiteral("IconPack");
static const QString kThemeParam  = QStringLiteral("icon-pack");
static const QString kHouseId     = QStringLiteral("office");

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the five resolver functions in @p engine, restores the persisted selection and
 *        follows @p theme for the effective pack and the mono tints. The dependencies arrive as
 *        references from the composition root: nothing here reaches a singleton.
 */
Misc::IconPacks::IconPacks(QJSEngine& engine,
                           IconRegistry& registry,
                           ThemeManager& theme,
                           QObject* parent)
  : QObject(parent), m_registry(registry), m_theme(theme), m_persistSettings(true)
{
  const auto bind = [&engine](const char* body) {
    const auto fn = engine.evaluate(QString::fromLatin1(body));
    SS_ASSERT_LOG(fn.isCallable());
    return fn;
  };

  m_icon         = bind("(function(c, n, px) { return this.resolve(c, n, px) })");
  m_iconById     = bind("(function(id, px) { return this.resolveById(id, px) })");
  m_iconOn       = bind("(function(t, c, n, px) { return this.resolveOn(t, c, n, px) })");
  m_iconByIdOn   = bind("(function(t, id, px) { return this.resolveByIdOn(t, id, px) })");
  m_monoIconById = bind("(function(id, px) { return this.resolveMonoById(id, px) })");

  m_selected = m_settings.value(kSettingsKey).toString();
  if (m_selected != kHouseId && m_registry.selectablePack(m_selected).isEmpty())
    m_selected.clear();

  applyEffectivePack();
  updateLocalizedNames();

  connect(&m_theme, &ThemeManager::themeChanged, this, &IconPacks::applyEffectivePack);
  connect(&Core::services().translator,
          &Translator::languageChanged,
          this,
          &IconPacks::updateLocalizedNames);
}

//--------------------------------------------------------------------------------------------------
// Properties
//--------------------------------------------------------------------------------------------------

/**
 * @brief The selector index: 0 is "Follow theme", 1 the house set ("Office"), then the bundled
 *        packs in catalog order.
 */
int Misc::IconPacks::pack() const
{
  if (m_selected.isEmpty())
    return 0;

  if (m_selected == kHouseId)
    return 1;

  const auto packs = m_registry.visiblePacks();
  for (int i = 0; i < packs.size(); ++i)
    if (packs[i].id == m_selected)
      return i + 2;

  return 0;
}

/**
 * @brief Whether the pack in effect is monochrome and drawn through the tint provider.
 */
bool Misc::IconPacks::mono() const
{
  return m_registry.isMono();
}

/**
 * @brief The pack in effect (empty for the house set), after the theme's parameter is applied.
 */
QString Misc::IconPacks::activePackId() const
{
  return m_registry.pack();
}

/**
 * @brief The selector entries in the interface language.
 */
const QStringList& Misc::IconPacks::availablePacks() const noexcept
{
  return m_names;
}

/**
 * @brief The (category, name, px) resolver as a JS function; read through a notifying property.
 */
QJSValue Misc::IconPacks::iconFunction() const
{
  return m_icon;
}

/**
 * @brief The ("category/name", px) resolver as a JS function.
 */
QJSValue Misc::IconPacks::iconByIdFunction() const
{
  return m_iconById;
}

/**
 * @brief The tone-aware (tone, category, name, px) resolver as a JS function.
 */
QJSValue Misc::IconPacks::iconOnFunction() const
{
  return m_iconOn;
}

/**
 * @brief The tone-aware (tone, "category/name", px) resolver as a JS function.
 */
QJSValue Misc::IconPacks::iconByIdOnFunction() const
{
  return m_iconByIdOn;
}

/**
 * @brief The fixed-mono-pack ("category/name", px) resolver as a JS function.
 */
QJSValue Misc::IconPacks::monoIconByIdFunction() const
{
  return m_monoIconById;
}

//--------------------------------------------------------------------------------------------------
// Resolvers behind the JS functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Text-tone resolution, the target of the `icon` function.
 */
QString Misc::IconPacks::resolve(const QString& category, const QString& name, const int px) const
{
  return m_registry.icon(category, name, px);
}

/**
 * @brief Text-tone resolution by combined id, the target of the `iconById` function.
 */
QString Misc::IconPacks::resolveById(const QString& id, const int px) const
{
  return m_registry.iconById(id, px);
}

/**
 * @brief Resolution in the named tone ("text" or "strip"), the target of the `iconOn` function.
 */
QString Misc::IconPacks::resolveOn(const QString& tone,
                                   const QString& category,
                                   const QString& name,
                                   const int px) const
{
  return m_registry.iconOn(toneFrom(tone), category, name, px);
}

/**
 * @brief Tone-aware resolution by combined id, the target of the `iconByIdOn` function.
 */
QString Misc::IconPacks::resolveByIdOn(const QString& tone, const QString& id, const int px) const
{
  return m_registry.iconByIdOn(toneFrom(tone), id, px);
}

/**
 * @brief Fixed-mono-pack resolution by combined id, the target of the `monoIconById` function.
 */
QString Misc::IconPacks::resolveMonoById(const QString& id, const int px) const
{
  return m_registry.monoIconById(id, px);
}

/**
 * @brief Re-tints a mono pack URL to @p color for a surface whose foreground is neither the text
 *        nor the strip tone (a pane caption, for one); a qrc URL comes back unchanged.
 */
QString Misc::IconPacks::retint(const QString& url, const QColor& color) const
{
  const auto marker = QStringLiteral("?tint=");
  const auto at     = url.indexOf(marker);
  if (!url.startsWith(QStringLiteral("image://iconpack")) || at < 0 || !color.isValid())
    return url;

  return url.left(at + marker.length()) + color.name().mid(1).toLower();
}

//--------------------------------------------------------------------------------------------------
// Selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a selector entry (0 follows the theme, 1 pins the house set) and persists it.
 */
void Misc::IconPacks::setPack(const int index)
{
  const auto packs = m_registry.visiblePacks();
  QString next;
  if (index == 1)
    next = kHouseId;
  else if (index > 1 && index <= packs.size() + 1)
    next = packs[index - 2].id;

  if (next == m_selected)
    return;

  m_selected = next;
  if (m_persistSettings)
    m_settings.setValue(kSettingsKey, m_selected);

  applyEffectivePack();
  Q_EMIT packChanged();
}

/**
 * @brief Enables or disables writing the selection to the settings store (self-test runs).
 */
void Misc::IconPacks::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * @brief Pushes the pack in effect (the explicit choice, else the theme's icon-pack parameter)
 *        and the current tints to the registry, then re-evaluates every icon binding when either
 *        moved; a theme change that keeps both costs no rebind.
 */
void Misc::IconPacks::applyEffectivePack()
{
  auto effective = m_selected;
  if (effective.isEmpty())
    effective = m_theme.parameters().value(kThemeParam).toString();

  effective = effective == kHouseId ? QString() : m_registry.selectablePack(effective);

  const auto text  = m_theme.getColor(QStringLiteral("text")).name();
  const auto strip = m_theme.getColor(QStringLiteral("toolbar_text")).name();
  const auto state = effective + QChar('|') + text + QChar('|') + strip;
  if (state == m_applied)
    return;

  m_applied = state;
  m_registry.setPack(effective);
  m_registry.setTints(text, strip);
  Q_EMIT revisionChanged();
}

/**
 * @brief Rebuilds the selector names from each pack's translations in the current language.
 */
void Misc::IconPacks::updateLocalizedNames()
{
  const auto language = Core::services().translator.language();
  const auto key      = LanguageTable::qmName(language);

  QStringList names;
  names.append(QCoreApplication::translate("Misc::IconPacks", "Follow theme"));
  names.append(QCoreApplication::translate("Misc::IconPacks", "Office (default)"));
  const auto packs = m_registry.visiblePacks();
  for (const auto& info : packs) {
    QString localized;
    if (language != Translator::English)
      localized = info.translations.value(key);

    names.append(localized.isEmpty() ? info.title : localized);
  }

  if (names == m_names)
    return;

  m_names = names;
  Q_EMIT availablePacksChanged();
}

/**
 * @brief Maps the QML tone name to the registry's enum; anything but "strip" is the text tone.
 */
Misc::IconRegistry::Tone Misc::IconPacks::toneFrom(const QString& tone)
{
  return tone == QStringLiteral("strip") ? IconRegistry::Tone::Strip : IconRegistry::Tone::Text;
}
