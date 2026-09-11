/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "Misc/SimdSettings.h"

#include <optional>
#include <QDebug>
#include <QLatin1StringView>
#include <string_view>

#include "Core/SSAssert.h"

static constexpr QLatin1StringView kAutoId("auto");

//--------------------------------------------------------------------------------------------------
// Ids and labels
//--------------------------------------------------------------------------------------------------

const char* Misc::SimdSettings::settingsKey() noexcept
{
  return "App/SimdLevel";
}

/**
 * @brief Display name of a level; the ids stay lowercase and untranslated.
 */
QString Misc::SimdSettings::levelLabel(const DSP::SimdLevel level)
{
  if (level == DSP::SimdLevel::Sse4)
    return QStringLiteral("SSE4");

  if (level == DSP::SimdLevel::Avx2)
    return QStringLiteral("AVX2");

  if (level == DSP::SimdLevel::Neon)
    return QStringLiteral("NEON");

  return tr("Scalar");
}

/**
 * @brief Lowercases and trims an id so a hand-edited settings file or a shouted --simd AVX2 still
 *        resolves; anything unknown or unsupported on this machine collapses to auto.
 */
QString Misc::SimdSettings::normalizedId(const QString& id)
{
  QString clean = id.trimmed().toLower();
  if (clean.isEmpty() || clean == kAutoId)
    return QString(kAutoId);

  const QByteArray latin = clean.toLatin1();
  const auto parsed =
    DSP::parseSimdLevelId(std::string_view(latin.constData(), static_cast<size_t>(latin.size())));
  if (!parsed.has_value() || !DSP::isSimdLevelSupported(*parsed))
    return QString(kAutoId);

  return clean;
}

/**
 * @brief Installs the level behind an already-normalized id; auto means the best supported one.
 */
bool Misc::SimdSettings::applyId(const QString& id)
{
  if (id == kAutoId)
    return DSP::setActiveSimdLevel(DSP::bestSupportedSimdLevel());

  const QByteArray latin = id.toLatin1();
  const auto parsed =
    DSP::parseSimdLevelId(std::string_view(latin.constData(), static_cast<size_t>(latin.size())));
  SS_ASSERT(parsed.has_value(), return DSP::setActiveSimdLevel(DSP::bestSupportedSimdLevel()));
  return DSP::setActiveSimdLevel(*parsed);
}

//--------------------------------------------------------------------------------------------------
// Startup resolution (before any module exists)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves pin > preference > auto and installs the result for the whole process. Only a
 *        rejected preference or pin is logged; the chosen level is visible in Preferences.
 */
void Misc::SimdSettings::applyConfiguredLevel(const QString& pinnedId)
{
  QSettings settings;
  const QString preference = settings.value(settingsKey(), QString(kAutoId)).toString();

  QString id           = normalizedId(preference);
  const QString pinned = pinnedId.trimmed();
  const QString stored = preference.trimmed();
  if (id == kAutoId && !stored.isEmpty() && stored.toLower() != kAutoId)
    qInfo().noquote()
      << QStringLiteral("[simd] ignoring preference %1: unknown or unsupported here").arg(stored);

  if (!pinned.isEmpty()) {
    const QString candidate = normalizedId(pinned);
    const bool honoured     = candidate != kAutoId || pinned.toLower() == kAutoId;
    if (honoured)
      id = candidate;
    else
      qInfo().noquote()
        << QStringLiteral("[simd] ignoring --simd %1: unknown or unsupported here").arg(pinned);
  }

  const bool applied = applyId(id);
  SS_ASSERT(applied, (void)0);
}

//--------------------------------------------------------------------------------------------------
// Object lifetime
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors the persisted id (an unsupported one shows as auto without rewriting the file)
 *        and builds the combobox model once. applyConfiguredLevel() installed the level long
 *        before this runs; when a --simd pin made it differ from the preference, the property
 *        follows the lane actually running. Construct after the Translator: labels use tr().
 */
Misc::SimdSettings::SimdSettings(QObject* parent) : QObject(parent), m_currentLevel(kAutoId)
{
  m_currentLevel = normalizedId(m_settings.value(settingsKey(), QString(kAutoId)).toString());

  const DSP::SimdLevel active = DSP::activeSimdLevel();
  const QByteArray latin      = m_currentLevel.toLatin1();
  const auto preferred        = (m_currentLevel == kAutoId)
                                ? std::optional(DSP::bestSupportedSimdLevel())
                                : DSP::parseSimdLevelId(std::string_view(
                             latin.constData(), static_cast<size_t>(latin.size())));
  if (!preferred.has_value() || *preferred != active)
    m_currentLevel = idString(active);

  rebuildAvailableLevels();
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

const QString& Misc::SimdSettings::currentLevel() const noexcept
{
  return m_currentLevel;
}

const QVariantList& Misc::SimdSettings::availableLevels() const noexcept
{
  return m_availableLevels;
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Persists and applies a level live; every level is bit-identical, so no restart prompt.
 *        Re-selecting the lane a --simd pin is showing still writes it, so a pinned run can be
 *        kept from the dialog.
 */
void Misc::SimdSettings::setCurrentLevel(const QString& id)
{
  const QString clean = normalizedId(id);
  const QString persisted =
    normalizedId(m_settings.value(settingsKey(), QString(kAutoId)).toString());
  if (clean == m_currentLevel && clean == persisted)
    return;

  const bool applied = applyId(clean);
  SS_ASSERT(applied, return);

  const bool changed = clean != m_currentLevel;
  m_currentLevel     = clean;
  m_settings.setValue(settingsKey(), clean);
  m_settings.sync();
  if (changed)
    Q_EMIT currentLevelChanged();
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Auto first, labelled with the level it resolves to, then every supported level ascending.
 */
void Misc::SimdSettings::rebuildAvailableLevels()
{
  m_availableLevels.clear();
  m_availableLevels.append(
    levelEntry(QString(kAutoId), tr("Auto (%1)").arg(levelLabel(DSP::bestSupportedSimdLevel()))));
  for (const DSP::SimdLevel level : DSP::supportedSimdLevels())
    m_availableLevels.append(levelEntry(idString(level), levelLabel(level)));
}

/**
 * @brief The stable id as a QString, sized from the view rather than assuming NUL termination.
 */
QString Misc::SimdSettings::idString(const DSP::SimdLevel level)
{
  const std::string_view id = DSP::simdLevelId(level);
  return QString::fromLatin1(id.data(), static_cast<qsizetype>(id.size()));
}

/**
 * @brief One combobox row: the id the setter accepts and the label the user sees.
 */
QVariantMap Misc::SimdSettings::levelEntry(const QString& id, const QString& label)
{
  QVariantMap entry;
  entry.insert(QStringLiteral("id"), id);
  entry.insert(QStringLiteral("label"), label);
  return entry;
}
