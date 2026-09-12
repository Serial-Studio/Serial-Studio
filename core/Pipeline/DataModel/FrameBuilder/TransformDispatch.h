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

#include <QtGlobal>
#include <QVariant>

#include "DataModel/FrameBuilder/TransformCompiler.h"

namespace DataModel {

/**
 * @brief Per-frame context handed to a transform: the frame's ordinal for its source and the
 *        capture time the driver stamped, both surfaced to the script as `info`.
 */
struct TransformFrameInfo {
  quint64 frameNumber = 0;
  int sourceId        = 0;
  qint64 timestampMs  = 0;
};

/**
 * @brief Runs one dataset's transform on the engine cached for its source (spec 0086 extraction
 *        of the frame builder's dispatch): the Lua, JavaScript and expression engines of the
 *        current source are resolved once per source change, never per dataset, and the JS
 *        watchdog verdict is latched here for the pass that armed it.
 */
class TransformDispatch {
public:
  explicit TransformDispatch(TransformCompiler& transforms);
  TransformDispatch(TransformDispatch&&)                 = delete;
  TransformDispatch(const TransformDispatch&)            = delete;
  TransformDispatch& operator=(TransformDispatch&&)      = delete;
  TransformDispatch& operator=(const TransformDispatch&) = delete;

  [[nodiscard]] int cachedSourceId() const noexcept { return m_cacheSourceId; }

  [[nodiscard]] TransformEngine* jsEngine() const noexcept { return m_js; }

  [[nodiscard]] TransformEngine* exprEngine() const noexcept { return m_expr; }

  [[nodiscard]] TransformEngine* const& exprEngineRef() const noexcept { return m_expr; }

  [[nodiscard]] bool jsTimedOut() const noexcept { return m_jsTimedOut; }

  [[nodiscard]] QVariant apply(int language,
                               int uniqueId,
                               const QVariant& rawValue,
                               const TransformFrameInfo& info);

  void select(int sourceId);

  void clearJsTimedOut() noexcept { m_jsTimedOut = false; }

  void reset() noexcept;

private:
  [[nodiscard]] QVariant applyLua(TransformEngine& engine,
                                  int uniqueId,
                                  const QVariant& rawValue,
                                  const TransformFrameInfo& info);
  [[nodiscard]] QVariant applyExpr(TransformEngine& engine,
                                   int uniqueId,
                                   const QVariant& rawValue,
                                   const TransformFrameInfo& info);
  [[nodiscard]] QVariant applyJs(TransformEngine& engine,
                                 int uniqueId,
                                 const QVariant& rawValue,
                                 const TransformFrameInfo& info);

private:
  TransformCompiler& m_transforms;
  int m_cacheSourceId;
  TransformEngine* m_lua;
  TransformEngine* m_js;
  TransformEngine* m_expr;
  bool m_jsTimedOut;
};

}  // namespace DataModel
