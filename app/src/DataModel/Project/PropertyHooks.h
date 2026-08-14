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

#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "DataModel/Frame.h"

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace DataModel::PropertyHooks {

/**
 * @brief Rebuild work a commit hook asks its caller to perform; the hook never rebuilds a form
 *        itself, so the editor keeps owning today's synchronous-versus-deferred split.
 */
enum class RebuildHint : quint8 {
  None,     ///< Field write only
  Sync,     ///< Rebuild the form model before returning
  Deferred  ///< Rebuild through a zero-timer, re-checking the selection uniqueId
};

/**
 * @brief One (value, untranslated label) pair of a string-keyed choice domain.
 */
struct StaticOptionEntry {
  const char* value;
  const char* label;
};

/**
 * @brief One (value, untranslated label) pair of an integer-valued choice domain.
 */
struct IntOptionEntry {
  int value;
  const char* label;
};

/**
 * @brief One (first, second, untranslated label) triple of a two-field choice domain.
 */
struct TupleOptionEntry {
  bool first;
  bool second;
  const char* label;
};

/**
 * @brief Maps a choice property's stored value to the positional index the editor row carries,
 *        and back. The row keeps storing an index, so the generic QML delegate is untouched.
 */
class OptionSource {
public:
  OptionSource();
  virtual ~OptionSource();

  OptionSource(OptionSource&&)                 = delete;
  OptionSource(const OptionSource&)            = delete;
  OptionSource& operator=(OptionSource&&)      = delete;
  OptionSource& operator=(const OptionSource&) = delete;

  [[nodiscard]] virtual QStringList labels(const ProjectModel& pm) const                     = 0;
  [[nodiscard]] virtual int indexForValue(const ProjectModel& pm, const QVariant& val) const = 0;
  [[nodiscard]] virtual QVariant valueForIndex(const ProjectModel& pm, int index) const      = 0;
};

/**
 * @brief Choice domain backed by a fixed key -> label table; the stored value is the key.
 */
class StaticMapOptions : public OptionSource {
public:
  StaticMapOptions(const StaticOptionEntry* entries, int count, const char* context);

  [[nodiscard]] QStringList labels(const ProjectModel& pm) const override;
  [[nodiscard]] int indexForValue(const ProjectModel& pm, const QVariant& val) const override;
  [[nodiscard]] QVariant valueForIndex(const ProjectModel& pm, int index) const override;

private:
  const StaticOptionEntry* m_entries;
  const char* m_context;
  int m_count;
};

/**
 * @brief Choice domain backed by a fixed table that a provider extends at runtime: the built-in
 *        rows keep their indices, appended rows follow. Widget extension packages arrive this way
 *        (spec 0038), so a project with no package installed sees exactly the built-in domain.
 */
class ExtensibleMapOptions : public OptionSource {
public:
  using EntryProvider = QList<QPair<QString, QString>> (*)();

  ExtensibleMapOptions(const StaticOptionEntry* entries,
                       int count,
                       const char* context,
                       EntryProvider provider);

  [[nodiscard]] QStringList labels(const ProjectModel& pm) const override;
  [[nodiscard]] int indexForValue(const ProjectModel& pm, const QVariant& val) const override;
  [[nodiscard]] QVariant valueForIndex(const ProjectModel& pm, int index) const override;

private:
  const StaticOptionEntry* m_entries;
  const char* m_context;
  int m_count;
  EntryProvider m_provider;
};

/**
 * @brief Choice domain backed by parallel label and value lists; the stored value is the value.
 */
class ParallelValueOptions : public OptionSource {
public:
  ParallelValueOptions(const IntOptionEntry* entries,
                       int count,
                       const char* context,
                       int notFoundIndex);

  [[nodiscard]] QStringList labels(const ProjectModel& pm) const override;
  [[nodiscard]] int indexForValue(const ProjectModel& pm, const QVariant& val) const override;
  [[nodiscard]] QVariant valueForIndex(const ProjectModel& pm, int index) const override;

private:
  const IntOptionEntry* m_entries;
  const char* m_context;
  int m_count;
  int m_notFound;
};

/**
 * @brief Choice domain computed from live project state; labels and values come from a pair of
 *        ProjectModel accessors, so no hook ever reaches for a singleton.
 */
class LiveProviderOptions : public OptionSource {
public:
  using LabelProvider = QStringList (ProjectModel::*)() const;
  using ValueProvider = QList<int> (ProjectModel::*)() const;

  LiveProviderOptions(LabelProvider labelProvider, ValueProvider valueProvider, int notFoundValue);

  [[nodiscard]] QStringList labels(const ProjectModel& pm) const override;
  [[nodiscard]] int indexForValue(const ProjectModel& pm, const QVariant& val) const override;
  [[nodiscard]] QVariant valueForIndex(const ProjectModel& pm, int index) const override;

private:
  LabelProvider m_labels;
  ValueProvider m_values;
  int m_notFoundValue;
};

/**
 * @brief Choice domain where one row drives two boolean fields (plot enable + logging).
 */
class TupleOptions : public OptionSource {
public:
  TupleOptions(const TupleOptionEntry* entries, int count, const char* context);

  [[nodiscard]] QStringList labels(const ProjectModel& pm) const override;
  [[nodiscard]] int indexForValue(const ProjectModel& pm, const QVariant& val) const override;
  [[nodiscard]] QVariant valueForIndex(const ProjectModel& pm, int index) const override;

  [[nodiscard]] int indexForPair(bool first, bool second) const;
  [[nodiscard]] bool firstForIndex(int index) const;
  [[nodiscard]] bool secondForIndex(int index) const;

private:
  const TupleOptionEntry* m_entries;
  const char* m_context;
  int m_count;
};

//--------------------------------------------------------------------------------------------------
// Option providers
//--------------------------------------------------------------------------------------------------

[[nodiscard]] QList<QPair<QString, QString>> widgetExtensionOptions();

//--------------------------------------------------------------------------------------------------
// Validators
//--------------------------------------------------------------------------------------------------

[[nodiscard]] bool aliasInUseByOtherDataset(const ProjectModel& pm,
                                            const QString& alias,
                                            int selfUniqueId);
[[nodiscard]] bool isValidColor(const QString& color);
[[nodiscard]] bool isValidDatasetIndex(int index);
[[nodiscard]] bool isValidFftWindow(int window);
[[nodiscard]] bool isValidTransformLanguage(int language);

//--------------------------------------------------------------------------------------------------
// Row visibility and enablement predicates
//--------------------------------------------------------------------------------------------------

[[nodiscard]] bool insidePainterGroup(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool notVirtual(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool plotEnabled(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool plotEnabledNonTimeX(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool fftEnabled(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool fftOrWaterfallEnabled(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool waterfallEnabled(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool widgetSelectable(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool widgetRangeApplicable(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool extremeHoldApplicable(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool ledEnabled(const Dataset& d, const ProjectModel& pm);
[[nodiscard]] bool ledBandsAbsent(const Dataset& d, const ProjectModel& pm);

//--------------------------------------------------------------------------------------------------
// Dynamic placeholders
//--------------------------------------------------------------------------------------------------

[[nodiscard]] QVariant datasetIndexPlaceholder(const Dataset& d, const ProjectModel& pm);

//--------------------------------------------------------------------------------------------------
// Commit side effects
//--------------------------------------------------------------------------------------------------

[[nodiscard]] RebuildHint onTitleChanged(Dataset& d);
[[nodiscard]] RebuildHint onWidgetChanged(Dataset& d);
[[nodiscard]] RebuildHint onVirtualChanged(Dataset& d);
[[nodiscard]] RebuildHint onXAxisChanged(Dataset& d);
[[nodiscard]] RebuildHint onReshape(Dataset& d);

}  // namespace DataModel::PropertyHooks
