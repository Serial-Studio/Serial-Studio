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

#include "Core/DataModel/LedPanelGroup.h"

#include <QCoreApplication>
#include <QJsonObject>

#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

/**
 * @brief Builds the LED Panel group showing @p source's datasets at @p positions: value copies
 *        keeping their frame index and taking their position as datasetId, output widgets
 *        stripped, uniqueId left -1 for the caller to allocate; the group takes the source's type
 * and source id, the "ledpanel" widget and a translated "<title> LEDs" title. An out-of-range
 * position is skipped.
 */
DataModel::Group DataModel::deriveLedPanelGroup(const Group& source,
                                                const std::vector<int>& positions)
{
  SS_ASSERT_LOG(positions.size() <= source.datasets.size());

  Group panel;
  panel.uniqueId       = -1;
  panel.parentFolderId = source.parentFolderId;
  panel.sourceId       = source.sourceId;
  panel.groupType      = source.groupType;
  panel.enabled        = source.enabled;
  panel.widget         = QLatin1String(kLedPanelWidget);
  panel.title          = QCoreApplication::translate("ProjectModel", "%1 LEDs").arg(source.title);

  const auto count = static_cast<int>(source.datasets.size());
  for (const int position : positions) {
    SS_ASSERT_LOG(position >= 0 && position < count);
    if (position < 0 || position >= count)
      continue;

    Dataset copy   = source.datasets[static_cast<size_t>(position)];
    copy.uniqueId  = -1;
    copy.datasetId = static_cast<int>(panel.datasets.size());
    panel.datasets.push_back(std::move(copy));
  }

  return panel;
}

/**
 * @brief Serializes @p group into @p out and, when @p leds names members, the LED Panel derived
 *        from it right after: the shape every boolean-tag project generator emits.
 */
void DataModel::appendGroupWithLedPanel(QJsonArray& out,
                                        const Group& group,
                                        const std::vector<int>& leds)
{
  out.append(serialize(group));
  if (!leds.empty())
    out.append(serialize(deriveLedPanelGroup(group, leds)));
}

/**
 * @brief The in-memory twin of the QJsonArray overload, for importers that build Group vectors.
 */
void DataModel::appendGroupWithLedPanel(std::vector<Group>& out,
                                        Group group,
                                        const std::vector<int>& leds)
{
  if (!leds.empty()) {
    auto panel = deriveLedPanelGroup(group, leds);
    out.push_back(std::move(group));
    out.push_back(std::move(panel));
    return;
  }

  out.push_back(std::move(group));
}

/**
 * @brief Retargets saved LED Panel references from @p sourceUid (where the phantom panel lived)
 *        to @p panelUid, so a customised workspace keeps its panel after the migration.
 */
static void retargetLedRefs(std::vector<DataModel::Workspace>& workspaces,
                            int sourceUid,
                            int panelUid)
{
  for (auto& workspace : workspaces)
    for (auto& ref : workspace.widgetRefs)
      if (ref.widgetType == static_cast<int>(SerialStudio::DashboardLED)
          && ref.groupUniqueId == sourceUid)
        ref.groupUniqueId = panelUid;
}

/**
 * @brief The raw JSON object at @p guess when its @p key matches @p wanted, else the first entry of
 *        @p array that does; empty when none does. The reader stores raw indices in groupId and
 *        datasetId, so the guess is exact on a plain load and the scan covers an id remap.
 */
static QJsonObject rawEntryFor(const QJsonArray& array,
                               int guess,
                               QLatin1StringView key,
                               const QString& wanted)
{
  const auto matches = [&](const QJsonObject& obj) {
    return DataModel::ss_jsr(obj, key, QString()).toString().simplified() == wanted;
  };

  if (guess >= 0 && guess < array.count() && matches(array.at(guess).toObject()))
    return array.at(guess).toObject();

  for (const auto& entry : array)
    if (matches(entry.toObject()))
      return entry.toObject();

  return {};
}

/**
 * @brief Positions of @p group's datasets whose raw JSON carries the pre-0114 LED flag and is not
 *        hidden (a hidden flagged dataset showed no light). Entries are matched by title, the
 *        positional id being only the first guess, and a dataset match also needs the frame index
 *        (absent reads as 0, the loader's default, or an index-less flagged dataset loses its LED).
 */
std::vector<int> DataModel::legacyLedPositions(const Group& group, const QJsonArray& rawGroups)
{
  std::vector<int> positions;
  const auto rawGroup = rawEntryFor(rawGroups, group.groupId, Keys::Title, group.title);
  if (rawGroup.isEmpty())
    return positions;

  const auto rawDatasets = rawGroup.value(Keys::Datasets).toArray();
  for (size_t i = 0; i < group.datasets.size(); ++i) {
    const auto& dataset = group.datasets[i];
    if (dataset.hideOnDashboard)
      continue;

    const auto raw = rawEntryFor(rawDatasets, dataset.datasetId, Keys::Title, dataset.title);
    const int rawIndex = ss_jsr(raw, Keys::Index, 0).toInt();
    SS_ASSERT_LOG(rawIndex >= 0);
    if (rawIndex != dataset.index)
      continue;

    if (ss_jsr(raw, Keys::LED, false).toBool())
      positions.push_back(static_cast<int>(i));
  }

  return positions;
}

/**
 * @brief The pre-0114 migration: every group whose raw JSON flags datasets gains a derived LED
 *        Panel group right after it, identities drawn from @p allocateUniqueId, so the relative
 *        order the old phantom panels had is kept and saved workspace references follow the
 *        panel. Group ids are left for the caller to renumber. Returns whether anything moved.
 */
bool DataModel::insertLegacyLedPanels(std::vector<Group>& groups,
                                      std::vector<Workspace>& workspaces,
                                      const QJsonArray& rawGroups,
                                      const std::function<int()>& allocateUniqueId)
{
  SS_ASSERT(allocateUniqueId, return false);

  bool inserted = false;
  for (size_t g = 0; g < groups.size(); ++g) {
    const auto positions = legacyLedPositions(groups[g], rawGroups);
    if (positions.empty())
      continue;

    auto panel     = deriveLedPanelGroup(groups[g], positions);
    panel.uniqueId = allocateUniqueId();
    SS_ASSERT_LOG(panel.datasets.size() <= positions.size());
    for (auto& light : panel.datasets)
      light.uniqueId = allocateUniqueId();

    retargetLedRefs(workspaces, groups[g].uniqueId, panel.uniqueId);
    groups.insert(groups.begin() + static_cast<std::ptrdiff_t>(g) + 1, std::move(panel));
    inserted = true;
    ++g;
  }

  return inserted;
}
