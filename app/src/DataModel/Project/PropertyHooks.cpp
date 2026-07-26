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

#include "DataModel/Project/PropertyHooks.h"

#include <QCoreApplication>

#include "DataModel/ProjectModel.h"
#include "SSAssert.h"
#include "UI/WidgetExtensions.h"

//--------------------------------------------------------------------------------------------------
// Option sources
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the shared base of every option adapter.
 */
DataModel::PropertyHooks::OptionSource::OptionSource() = default;

/**
 * @brief Destroys an option adapter; out-of-line so the vtable has one home.
 */
DataModel::PropertyHooks::OptionSource::~OptionSource() = default;

/**
 * @brief Builds a key -> label adapter over a static table, translated in @p context.
 */
DataModel::PropertyHooks::StaticMapOptions::StaticMapOptions(const StaticOptionEntry* entries,
                                                             int count,
                                                             const char* context)
  : m_entries(entries), m_context(context), m_count(count)
{
  SS_ASSERT_LOG(entries != nullptr);
  SS_ASSERT_LOG(count > 0);
}

/**
 * @brief Returns the translated labels in table order.
 */
QStringList DataModel::PropertyHooks::StaticMapOptions::labels(const ProjectModel& pm) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  QStringList out;
  out.reserve(m_count);
  for (int i = 0; i < m_count; ++i)
    out.append(QCoreApplication::translate(m_context, m_entries[i].label));

  return out;
}

/**
 * @brief Returns the row index storing @p val, or 0 when the value is not in the table.
 */
int DataModel::PropertyHooks::StaticMapOptions::indexForValue(const ProjectModel& pm,
                                                              const QVariant& val) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  const auto key = val.toString();
  for (int i = 0; i < m_count; ++i)
    if (key == QLatin1StringView(m_entries[i].value))
      return i;

  return 0;
}

/**
 * @brief Returns the stored value for a row index, or an empty string when out of range.
 */
QVariant DataModel::PropertyHooks::StaticMapOptions::valueForIndex(const ProjectModel& pm,
                                                                   int index) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  if (index < 0 || index >= m_count)
    return QString();

  return QString::fromLatin1(m_entries[index].value);
}

/**
 * @brief Builds a key -> label adapter whose fixed table @p provider extends at runtime.
 */
DataModel::PropertyHooks::ExtensibleMapOptions::ExtensibleMapOptions(
  const StaticOptionEntry* entries, int count, const char* context, EntryProvider provider)
  : m_entries(entries), m_context(context), m_count(count), m_provider(provider)
{
  SS_ASSERT_LOG(entries != nullptr);
  SS_ASSERT_LOG(provider != nullptr);
  SS_ASSERT_LOG(count > 0);
}

/**
 * @brief Returns the translated built-in labels followed by the provider's, which the provider
 *        already translated or took from package metadata.
 */
QStringList DataModel::PropertyHooks::ExtensibleMapOptions::labels(const ProjectModel& pm) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);
  SS_ASSERT_LOG(m_provider != nullptr);

  QStringList out;
  out.reserve(m_count);
  for (int i = 0; i < m_count; ++i)
    out.append(QCoreApplication::translate(m_context, m_entries[i].label));

  const auto appended = m_provider();
  for (const auto& entry : appended)
    out.append(entry.second);

  return out;
}

/**
 * @brief Returns the row index storing @p val, searching the fixed table before the appended rows
 *        and answering 0 when the value belongs to neither.
 */
int DataModel::PropertyHooks::ExtensibleMapOptions::indexForValue(const ProjectModel& pm,
                                                                  const QVariant& val) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);
  SS_ASSERT_LOG(m_provider != nullptr);

  const auto key = val.toString();
  for (int i = 0; i < m_count; ++i)
    if (key == QLatin1StringView(m_entries[i].value))
      return i;

  const auto appended = m_provider();
  for (int i = 0; i < appended.size(); ++i)
    if (key == appended.at(i).first)
      return m_count + i;

  return 0;
}

/**
 * @brief Returns the stored value for a row index, or an empty string when out of range.
 */
QVariant DataModel::PropertyHooks::ExtensibleMapOptions::valueForIndex(const ProjectModel& pm,
                                                                       int index) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);
  SS_ASSERT_LOG(m_provider != nullptr);

  if (index >= 0 && index < m_count)
    return QString::fromLatin1(m_entries[index].value);

  const auto appended = m_provider();
  const int offset    = index - m_count;
  if (offset < 0 || offset >= appended.size())
    return QString();

  return appended.at(offset).first;
}

/**
 * @brief Builds a label/value adapter over a static table; @p context may be null for raw labels.
 */
DataModel::PropertyHooks::ParallelValueOptions::ParallelValueOptions(const IntOptionEntry* entries,
                                                                     int count,
                                                                     const char* context,
                                                                     int notFoundIndex)
  : m_entries(entries), m_context(context), m_count(count), m_notFound(notFoundIndex)
{
  SS_ASSERT_LOG(entries != nullptr);
  SS_ASSERT_LOG(count > 0);
}

/**
 * @brief Returns the labels in table order, translated only when a context was declared.
 */
QStringList DataModel::PropertyHooks::ParallelValueOptions::labels(const ProjectModel& pm) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  QStringList out;
  out.reserve(m_count);
  for (int i = 0; i < m_count; ++i)
    if (m_context)
      out.append(QCoreApplication::translate(m_context, m_entries[i].label));
    else
      out.append(QString::fromLatin1(m_entries[i].label));

  return out;
}

/**
 * @brief Returns the row index holding @p val, or the declared not-found index.
 */
int DataModel::PropertyHooks::ParallelValueOptions::indexForValue(const ProjectModel& pm,
                                                                  const QVariant& val) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  const int wanted = val.toInt();
  for (int i = 0; i < m_count; ++i)
    if (m_entries[i].value == wanted)
      return i;

  return m_notFound;
}

/**
 * @brief Returns the stored value for a row index, falling back to the not-found entry.
 */
QVariant DataModel::PropertyHooks::ParallelValueOptions::valueForIndex(const ProjectModel& pm,
                                                                       int index) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  if (index < 0 || index >= m_count)
    return m_entries[m_notFound].value;

  return m_entries[index].value;
}

/**
 * @brief Builds an adapter over two ProjectModel accessors that compute the domain live.
 */
DataModel::PropertyHooks::LiveProviderOptions::LiveProviderOptions(LabelProvider labelProvider,
                                                                   ValueProvider valueProvider,
                                                                   int notFoundValue)
  : m_labels(labelProvider), m_values(valueProvider), m_notFoundValue(notFoundValue)
{
  SS_ASSERT_LOG(labelProvider != nullptr);
  SS_ASSERT_LOG(valueProvider != nullptr);
}

/**
 * @brief Returns the labels the model currently offers for this domain.
 */
QStringList DataModel::PropertyHooks::LiveProviderOptions::labels(const ProjectModel& pm) const
{
  SS_ASSERT_LOG(m_labels != nullptr);
  return (pm.*m_labels)();
}

/**
 * @brief Returns the row index holding @p val, or 0 when the value is no longer offered.
 */
int DataModel::PropertyHooks::LiveProviderOptions::indexForValue(const ProjectModel& pm,
                                                                 const QVariant& val) const
{
  SS_ASSERT_LOG(m_values != nullptr);

  const auto ids   = (pm.*m_values)();
  const int wanted = val.toInt();
  for (int i = 0; i < ids.size(); ++i)
    if (ids.at(i) == wanted)
      return i;

  return 0;
}

/**
 * @brief Returns the stored value for a row index, or the declared fallback when out of range.
 */
QVariant DataModel::PropertyHooks::LiveProviderOptions::valueForIndex(const ProjectModel& pm,
                                                                      int index) const
{
  SS_ASSERT_LOG(m_values != nullptr);

  const auto ids = (pm.*m_values)();
  if (index < 0 || index >= ids.size())
    return m_notFoundValue;

  return ids.at(index);
}

/**
 * @brief Builds an adapter for a row that drives two boolean fields at once.
 */
DataModel::PropertyHooks::TupleOptions::TupleOptions(const TupleOptionEntry* entries,
                                                     int count,
                                                     const char* context)
  : m_entries(entries), m_context(context), m_count(count)
{
  SS_ASSERT_LOG(entries != nullptr);
  SS_ASSERT_LOG(count > 0);
}

/**
 * @brief Returns the translated labels in table order.
 */
QStringList DataModel::PropertyHooks::TupleOptions::labels(const ProjectModel& pm) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  QStringList out;
  out.reserve(m_count);
  for (int i = 0; i < m_count; ++i)
    out.append(QCoreApplication::translate(m_context, m_entries[i].label));

  return out;
}

/**
 * @brief Returns the row index for a [first, second] pair packed into a variant list.
 */
int DataModel::PropertyHooks::TupleOptions::indexForValue(const ProjectModel& pm,
                                                          const QVariant& val) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  const auto pair = val.toList();
  if (pair.size() != 2)
    return 0;

  return indexForPair(pair.at(0).toBool(), pair.at(1).toBool());
}

/**
 * @brief Returns the [first, second] pair stored at a row index as a variant list.
 */
QVariant DataModel::PropertyHooks::TupleOptions::valueForIndex(const ProjectModel& pm,
                                                               int index) const
{
  Q_UNUSED(pm);
  SS_ASSERT_LOG(m_entries != nullptr);

  QVariantList out;
  out.append(firstForIndex(index));
  out.append(secondForIndex(index));
  return out;
}

/**
 * @brief Returns the row index for an explicit boolean pair, or 0 when the pair is unknown.
 */
int DataModel::PropertyHooks::TupleOptions::indexForPair(bool first, bool second) const
{
  SS_ASSERT_LOG(m_entries != nullptr);
  SS_ASSERT_LOG(m_count > 0);

  for (int i = 0; i < m_count; ++i)
    if (m_entries[i].first == first && m_entries[i].second == second)
      return i;

  return 0;
}

/**
 * @brief Returns the first field's value at a row index, false when out of range.
 */
bool DataModel::PropertyHooks::TupleOptions::firstForIndex(int index) const
{
  SS_ASSERT_LOG(m_entries != nullptr);

  if (index < 0 || index >= m_count)
    return false;

  return m_entries[index].first;
}

/**
 * @brief Returns the second field's value at a row index, false when out of range.
 */
bool DataModel::PropertyHooks::TupleOptions::secondForIndex(int index) const
{
  SS_ASSERT_LOG(m_entries != nullptr);

  if (index < 0 || index >= m_count)
    return false;

  return m_entries[index].second;
}

//--------------------------------------------------------------------------------------------------
// Option providers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns one (widget string, title) pair per installed dataset-scope widget extension, in
 *        catalog order. A bundled package that ships as a built-in implementation is skipped: its
 *        built-in row already exists and owns that widget string.
 */
QList<QPair<QString, QString>> DataModel::PropertyHooks::widgetExtensionOptions()
{
  static auto& catalog = UI::WidgetExtensions::instance();

  QList<QPair<QString, QString>> out;
  const auto packages = catalog.idsForScope(UI::WidgetExtensions::DatasetScope);
  for (const auto& id : packages) {
    const auto& package = catalog.descriptor(id);
    if (package.replaces.isEmpty() && !package.title.isEmpty())
      out.append({id, package.title});
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Validators that read project state (the ProjectModel-free ones live in PropertyValidators.cpp)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when @p alias already belongs to a dataset other than @p selfUniqueId.
 */
bool DataModel::PropertyHooks::aliasInUseByOtherDataset(const ProjectModel& pm,
                                                        const QString& alias,
                                                        int selfUniqueId)
{
  SS_ASSERT_LOG(!alias.isEmpty());

  for (const auto& group : pm.groups()) {
    for (const auto& other : group.datasets)
      if (other.uniqueId != selfUniqueId && other.alias == alias)
        return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Row visibility and enablement predicates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the dataset's owning group renders a painter widget.
 */
bool DataModel::PropertyHooks::insidePainterGroup(const Dataset& d, const ProjectModel& pm)
{
  const auto& groups = pm.groups();
  if (d.groupId < 0 || static_cast<size_t>(d.groupId) >= groups.size())
    return false;

  return groups[d.groupId].widget == QLatin1String("painter");
}

/**
 * @brief Returns true when the dataset takes its value from a frame slot rather than a transform.
 */
bool DataModel::PropertyHooks::notVirtual(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return !d.virtual_;
}

/**
 * @brief Returns true when the dataset draws a plot widget.
 */
bool DataModel::PropertyHooks::plotEnabled(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.plt;
}

/**
 * @brief Returns true when the plot is on and its X axis is not the time axis.
 */
bool DataModel::PropertyHooks::plotEnabledNonTimeX(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.plt && d.xAxisId != kXAxisTime;
}

/**
 * @brief Returns true when frequency-domain analysis is on.
 */
bool DataModel::PropertyHooks::fftEnabled(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.fft;
}

/**
 * @brief Returns true when either consumer of the FFT settings is on.
 */
bool DataModel::PropertyHooks::fftOrWaterfallEnabled(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.fft || d.waterfall;
}

/**
 * @brief Returns true when the waterfall (spectrogram) widget is on.
 */
bool DataModel::PropertyHooks::waterfallEnabled(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.waterfall;
}

/**
 * @brief Returns true when the owning group leaves the dataset widget freely selectable.
 */
bool DataModel::PropertyHooks::widgetSelectable(const Dataset& d, const ProjectModel& pm)
{
  const auto& groups = pm.groups();
  if (d.groupId >= 0 && static_cast<size_t>(d.groupId) < groups.size()) {
    const auto& widget = groups[d.groupId].widget;
    if (widget != "" && widget != "multiplot" && widget != "datagrid" && widget != "painter")
      return false;
  }

  return true;
}

/**
 * @brief Returns true when the dataset draws a widget that honours an explicit display range.
 */
bool DataModel::PropertyHooks::widgetRangeApplicable(const Dataset& d, const ProjectModel& pm)
{
  if (!widgetSelectable(d, pm))
    return false;

  return d.widget == "bar" || d.widget == "gauge" || d.widget == "meter";
}

/**
 * @brief Returns true when the dataset appears in the LED panel.
 */
bool DataModel::PropertyHooks::ledEnabled(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.led;
}

/**
 * @brief Returns true while no alarm band exists, which is when the legacy LED threshold applies.
 */
bool DataModel::PropertyHooks::ledBandsAbsent(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(pm);
  return d.alarmBands.empty();
}

//--------------------------------------------------------------------------------------------------
// Dynamic placeholders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the frame-index placeholder: the next free slot in the project.
 */
QVariant DataModel::PropertyHooks::datasetIndexPlaceholder(const Dataset& d, const ProjectModel& pm)
{
  Q_UNUSED(d);
  return pm.datasetCount() + 1;
}

//--------------------------------------------------------------------------------------------------
// Commit side effects
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports the rebuild work a title edit needs; the caller owns the tree-item patch.
 */
DataModel::PropertyHooks::RebuildHint DataModel::PropertyHooks::onTitleChanged(Dataset& d)
{
  Q_UNUSED(d);
  return RebuildHint::None;
}

/**
 * @brief Applies the compass range rewrite that follows a widget change and asks for a rebuild.
 */
DataModel::PropertyHooks::RebuildHint DataModel::PropertyHooks::onWidgetChanged(Dataset& d)
{
  if (d.widget == "compass") {
    d.wgtMin = 0;
    d.wgtMax = 360;
    d.alarmBands.clear();
  }

  return RebuildHint::Sync;
}

/**
 * @brief Reports that a virtual-flag change needs the deferred rebuild (the row set changes).
 */
DataModel::PropertyHooks::RebuildHint DataModel::PropertyHooks::onVirtualChanged(Dataset& d)
{
  Q_UNUSED(d);
  return RebuildHint::Deferred;
}

/**
 * @brief Reports that an X-axis change needs the deferred rebuild (log-X enablement follows it).
 */
DataModel::PropertyHooks::RebuildHint DataModel::PropertyHooks::onXAxisChanged(Dataset& d)
{
  Q_UNUSED(d);
  return RebuildHint::Deferred;
}

/**
 * @brief Reports that a visualization toggle reshapes the form and needs a synchronous rebuild.
 */
DataModel::PropertyHooks::RebuildHint DataModel::PropertyHooks::onReshape(Dataset& d)
{
  Q_UNUSED(d);
  return RebuildHint::Sync;
}
