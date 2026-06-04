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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QList>
#include <QString>
#include <QStringList>

namespace DataModel {

/**
 * @brief Abstract script engine interface used by the frame parser.
 */
class IScriptEngine {
public:
  virtual ~IScriptEngine() = default;

  [[nodiscard]] virtual bool loadScript(const QString& script,
                                        int sourceId,
                                        bool showMessageBoxes) = 0;

  [[nodiscard]] virtual QList<QStringList> parseString(const QString& frame)    = 0;
  [[nodiscard]] virtual QList<QStringList> parseBinary(const QByteArray& frame) = 0;

  /**
   * @brief Parses a text frame from raw UTF-8 bytes. Engines that store text as UTF-16 (JS) just
   * transcode; byte-string engines (Lua) override this to skip the QString round-trip.
   */
  [[nodiscard]] virtual QList<QStringList> parseUtf8(const QByteArray& frame)
  {
    return parseString(QString::fromUtf8(frame));
  }

  /**
   * @brief Allocation-free single-row fast path; -1 = no span-capable parser (native only).
   */
  [[nodiscard]] virtual qsizetype parseUtf8Spans(QByteArrayView frame,
                                                 QByteArrayView* out,
                                                 qsizetype maxSpans) noexcept
  {
    Q_UNUSED(frame)
    Q_UNUSED(out)
    Q_UNUSED(maxSpans)
    return -1;
  }

  [[nodiscard]] virtual bool isLoaded() const noexcept = 0;
  [[nodiscard]] virtual int language() const noexcept  = 0;

  virtual void collectGarbage() = 0;
  virtual void reset()          = 0;

  int templateIdx = -1;
};

}  // namespace DataModel
