/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Importers/DBCMultiplexing.h"

#include <algorithm>
#include <QSet>
#include <QStringList>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// File-local classification helpers
//--------------------------------------------------------------------------------------------------

namespace detail::dbcmux {

/**
 * @brief Converts Qt's multiplex ranges into gates sorted by switch name and lower bound, so the
 *        emitted spec is stable across runs (Qt hands the parents back in a QHash). Returns false
 *        when a bound does not fit a qint64, which is the only reason the import drops a signal.
 */
[[nodiscard]] static bool buildMuxGates(const QCanSignalDescription& signal,
                                        QList<DataModel::DBCMux::MuxSpec>& outGates)
{
  using DataModel::DBCMux::MuxRange;
  using DataModel::DBCMux::MuxSpec;

  outGates.clear();

  const auto parents = signal.multiplexSignals();
  auto names         = parents.keys();
  std::sort(names.begin(), names.end());

  for (const auto& name : names) {
    MuxSpec spec;
    spec.parent = name;

    for (const auto& range : parents.value(name)) {
      bool loOk     = false;
      bool hiOk     = false;
      const auto lo = range.minimum.toLongLong(&loOk);
      const auto hi = range.maximum.toLongLong(&hiOk);
      if (!loOk || !hiOk)
        return false;

      spec.ranges.append({std::min(lo, hi), std::max(lo, hi)});
    }

    if (spec.ranges.isEmpty())
      return false;

    std::sort(spec.ranges.begin(), spec.ranges.end(), [](const MuxRange& a, const MuxRange& b) {
      return a.lo < b.lo;
    });

    outGates.append(spec);
  }

  return !outGates.isEmpty();
}

/**
 * @brief Classifies a signal's multiplexing role and collects every gate that switches it on.
 *        SwitchAndSignal (an SG_MUL_VAL_ switch that is itself multiplexed) is a Selector with
 *        gates, so nested chains import; ExtendedMuxed is returned only when a switch range does
 *        not fit a qint64 and the signal has to be dropped.
 */
[[nodiscard]] static DataModel::DBCMux::MuxRole classifyMux(
  const QCanSignalDescription& signal, QList<DataModel::DBCMux::MuxSpec>& outGates)
{
  using DataModel::DBCMux::MuxRole;

  outGates.clear();

  const auto state = signal.multiplexState();
  if (state == QtCanBus::MultiplexState::None)
    return MuxRole::Plain;

  const bool selector = (state == QtCanBus::MultiplexState::MultiplexorSwitch
                         || state == QtCanBus::MultiplexState::SwitchAndSignal);

  if (signal.multiplexSignals().isEmpty())
    return selector ? MuxRole::Selector : MuxRole::Plain;

  if (!buildMuxGates(signal, outGates))
    return MuxRole::ExtendedMuxed;

  return selector ? MuxRole::Selector : MuxRole::Muxed;
}

/**
 * @brief Returns true once every switch a gated signal names has been emitted as a selector; a
 *        gate naming a signal the message never declares as one can never match at runtime.
 */
[[nodiscard]] static bool gatesResolved(const QList<DataModel::DBCMux::MuxSpec>& gates,
                                        const QSet<QString>& resolved)
{
  for (const auto& gate : gates)
    if (!resolved.contains(gate.parent))
      return false;

  return true;
}

/**
 * @brief Routes one gated signal into the emitted list or back into the pending set, marking a
 *        newly emitted switch resolved so a chain declared in order settles in a single pass.
 */
static void appendResolved(const DataModel::DBCMux::OrderedSignal& entry,
                           QList<DataModel::DBCMux::OrderedSignal>& ordered,
                           QList<DataModel::DBCMux::OrderedSignal>& pending,
                           QSet<QString>& resolved)
{
  if (!gatesResolved(entry.gates, resolved)) {
    pending.append(entry);
    return;
  }

  if (entry.role == DataModel::DBCMux::MuxRole::Selector)
    resolved.insert(entry.signal.name());

  ordered.append(entry);
}

}  // namespace detail::dbcmux

//--------------------------------------------------------------------------------------------------
// Decode ordering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the message's importable signals in decode order: ungated selectors, plain
 *        signals, then each gated signal once every switch it names has been emitted; the
 *        generated Lua reads selector values as it walks the spec. Signals left unreachable
 *        by circular or dangling SG_MUL_VAL_ parentage are dropped for the caller to count.
 */
QList<DataModel::DBCMux::OrderedSignal> DataModel::DBCMux::orderedSignals(
  const QCanMessageDescription& message)
{
  QList<OrderedSignal> selectors;
  QList<OrderedSignal> plain;
  QList<OrderedSignal> gated;

  const auto signalList = message.signalDescriptions();
  for (const auto& signal : signalList) {
    QList<MuxSpec> gates;
    const auto role = ::detail::dbcmux::classifyMux(signal, gates);
    if (role == MuxRole::ExtendedMuxed)
      continue;

    if (!gates.isEmpty())
      gated.append({role, gates, signal});
    else if (role == MuxRole::Selector)
      selectors.append({role, gates, signal});
    else
      plain.append({role, gates, signal});
  }

  auto ordered = selectors + plain;
  SS_ASSERT(ordered.size() + gated.size() <= signalList.size(), return ordered);

  QSet<QString> resolved;
  for (const auto& entry : selectors)
    resolved.insert(entry.signal.name());

  for (qsizetype pass = 0; pass < signalList.size() && !gated.isEmpty(); ++pass) {
    const auto before = ordered.size();
    QList<OrderedSignal> pending;
    for (const auto& entry : gated)
      ::detail::dbcmux::appendResolved(entry, ordered, pending, resolved);

    if (ordered.size() == before)
      break;

    gated = pending;
  }

  SS_ASSERT(ordered.size() <= signalList.size(), return ordered);
  return ordered;
}

/**
 * @brief Returns the name the generated parse() latches as `root`: the first ungated selector in
 *        decode order, which is the switch a bare numeric mux field is compared against.
 */
QString DataModel::DBCMux::rootSelectorName(const QList<OrderedSignal>& entries)
{
  for (const auto& entry : entries)
    if (entry.role == MuxRole::Selector && entry.gates.isEmpty())
      return entry.signal.name();

  return QString();
}

//--------------------------------------------------------------------------------------------------
// Gate rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders a gate list as the switch values a reader can match against the DBC: "3" for a
 *        single value, "1,4-6" for several, and "Mode=1/Page=2-3" once more than one selector
 *        gates the signal.
 */
QString DataModel::DBCMux::muxTitleSuffix(const QList<MuxSpec>& gates)
{
  QStringList conditions;
  for (const auto& gate : gates) {
    QStringList values;
    for (const auto& range : gate.ranges)
      values.append(range.lo == range.hi ? QString::number(range.lo)
                                         : QStringLiteral("%1-%2").arg(QString::number(range.lo),
                                                                       QString::number(range.hi)));

    const auto joined = values.join(QLatin1Char(','));
    conditions.append(gates.size() > 1 ? QStringLiteral("%1=%2").arg(gate.parent, joined) : joined);
  }

  return conditions.join(QLatin1Char('/'));
}

/**
 * @brief Recognizes the single-point gate on the message's top-level multiplexor, the one shape
 *        the generated Lua compares as a bare number.
 */
bool DataModel::DBCMux::simpleMuxValue(const QList<MuxSpec>& gates,
                                       const QString& rootSelector,
                                       qint64& outValue)
{
  outValue = 0;
  if (gates.size() != 1 || rootSelector.isEmpty())
    return false;

  const auto& gate = gates.constFirst();
  if (gate.parent != rootSelector || gate.ranges.size() != 1)
    return false;

  const auto& range = gate.ranges.constFirst();
  if (range.lo != range.hi)
    return false;

  outValue = range.lo;
  return true;
}
