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

#include <map>
#include <memory>
#include <QObject>
#include <QStringList>

#include "DataModel/IScriptEngine.h"

namespace DataModel {

/**
 * @brief Singleton script engine manager for frame parsing.
 *
 * Maintains one IScriptEngine per source ID (source 0 = global / single-source
 * projects). All parse, load, and template operations are sourceId-scoped so
 * that multi-source projects can run independent parser code per device.
 *
 * Supports multiple scripting languages via the IScriptEngine interface
 * (JavaScript via QJSEngine, Lua via embedded Lua 5.4).
 */
class FrameParser : public QObject {
  Q_OBJECT

signals:
  void modifiedChanged();
  void templateNamesChanged();

private:
  explicit FrameParser();
  FrameParser(FrameParser&&)                 = delete;
  FrameParser(const FrameParser&)            = delete;
  FrameParser& operator=(FrameParser&&)      = delete;
  FrameParser& operator=(const FrameParser&) = delete;

public:
  [[nodiscard]] static FrameParser& instance();

  [[nodiscard]] static QString defaultTemplateCode(int language = 0);

  [[nodiscard]] QString templateCode(int sourceId = 0) const;
  [[nodiscard]] const QStringList& templateNames() const;
  [[nodiscard]] const QStringList& templateFiles() const;

  [[nodiscard]] QList<QStringList> parseMultiFrame(const QString& frame, int sourceId);
  [[nodiscard]] QList<QStringList> parseMultiFrame(const QByteArray& frame, int sourceId);

  [[nodiscard]] bool loadScript(int sourceId, const QString& script, bool showMessageBoxes = true);

  void setSuppressMessageBoxes(bool suppress);
  void setSourceCode(int sourceId, const QString& code);
  void clearSourceEngine(int sourceId);

public slots:
  void readCode();
  void clearContext();
  void collectGarbage();
  void loadTemplateNames();
  void setupExternalConnections();
  void setTemplateIdx(int sourceId, int idx);
  void loadDefaultTemplate(int sourceId, bool guiTrigger = false);

private:
  [[nodiscard]] IScriptEngine& engineForSource(int sourceId);
  [[nodiscard]] int languageForSource(int sourceId) const;

private:
  bool m_suppressMessageBoxes;

  QStringList m_templateFiles;
  QStringList m_templateNames;

  std::map<int, std::unique_ptr<IScriptEngine>> m_engines;
};

}  // namespace DataModel
