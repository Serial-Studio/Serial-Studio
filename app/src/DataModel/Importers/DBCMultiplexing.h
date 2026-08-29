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

#pragma once

#include <QCanMessageDescription>
#include <QCanSignalDescription>
#include <QList>
#include <QString>

namespace DataModel::DBCMux {

/**
 * @brief Multiplexing role of a CAN signal as classified during DBC import. Selector covers
 *        both the top-level MultiplexorSwitch and an SG_MUL_VAL_ switch that is itself gated;
 *        ExtendedMuxed is the sole drop verdict, reserved for switch ranges that do not fit a
 *        qint64.
 */
enum class MuxRole {
  Plain,
  Muxed,
  Selector,
  ExtendedMuxed
};

/**
 * @brief One inclusive range of raw multiplexor values; a single value has lo == hi.
 */
struct MuxRange {
  qint64 lo;
  qint64 hi;
};

/**
 * @brief One gating condition: the selector signal's name plus the ranges of it, sorted by
 *        lower bound, that switch the dependent signal into the frame.
 */
struct MuxSpec {
  QString parent;
  QList<MuxRange> ranges;
};

/**
 * @brief One importable signal in decode order, carrying every condition that gates it. A
 *        nested SG_MUL_VAL_ switch is a Selector with a non-empty gate list.
 */
struct OrderedSignal {
  MuxRole role;
  QList<MuxSpec> gates;
  QCanSignalDescription signal;
};

[[nodiscard]] QList<OrderedSignal> orderedSignals(const QCanMessageDescription& message);

[[nodiscard]] QString rootSelectorName(const QList<OrderedSignal>& entries);

[[nodiscard]] QString muxTitleSuffix(const QList<MuxSpec>& gates);

[[nodiscard]] bool simpleMuxValue(const QList<MuxSpec>& gates,
                                  const QString& rootSelector,
                                  qint64& outValue);

}  // namespace DataModel::DBCMux
