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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QJsonArray>
#include <QJsonObject>

#include "DataModel/Scripting/IScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

namespace DataModel {

/**
 * @brief Native (C++) frame parser engine. The "script" is a canonical JSON descriptor of the
 * form {"params": {...}, "template": "<id>"} selecting a parametrized native template.
 */
class CFrameParser final : public IScriptEngine {
public:
  CFrameParser();

  [[nodiscard]] static QJsonArray templateCatalog();
  [[nodiscard]] static QJsonObject templateSchema(const QString& id);
  [[nodiscard]] static QString buildDescriptor(const QString& templateId,
                                               const QJsonObject& params);

  [[nodiscard]] bool loadScript(const QString& script,
                                int sourceId,
                                bool showMessageBoxes) override;

  [[nodiscard]] QList<QStringList> parseString(const QString& frame) override;
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override;
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override;
  [[nodiscard]] qsizetype parseUtf8Spans(QByteArrayView frame,
                                         QByteArrayView* out,
                                         qsizetype maxSpans) noexcept override;

  [[nodiscard]] bool isLoaded() const noexcept override;
  [[nodiscard]] int language() const noexcept override;

  [[nodiscard]] QString lastError() const;
  [[nodiscard]] QString templateId() const;

  void collectGarbage() override;
  void reset() override;

private:
  void reportLoadError(const QString& error, int sourceId, bool showMessageBoxes);

private:
  QString m_lastError;
  QString m_templateId;
  std::unique_ptr<INativeParser> m_parser;
};

}  // namespace DataModel
