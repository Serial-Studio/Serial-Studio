/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

#include "AI/Conversation/ToolCallStatus.h"
#include "AI/DocSearch.h"

namespace AI {

class HelpFetcher;
class ToolDispatcher;

/**
 * @brief The conversation-side surface a meta-tool handler drives: tool-call cards, the
 *        pending tool-result queue, and the escapes back into the facade's turn state.
 *        Abstract on purpose: the streaming state stays facade-private, so a handler
 *        cannot touch the assistant row, the dirty flags, or the flush timer.
 */
class MetaToolSink {
public:
  virtual ~MetaToolSink() = default;

  virtual void releaseOutstandingToolResult()                                               = 0;
  virtual void noteSkillLoaded(const QString& skillId)                                      = 0;
  virtual void recordToolResult(const QString& callId,
                                const QString& name,
                                const QJsonObject& payload)                                 = 0;
  virtual void dispatchByCallSafety(const QString& callId,
                                    const QString& name,
                                    const QJsonObject& arguments)                           = 0;
  virtual void appendToolCallCard(const QString& callId,
                                  const QString& name,
                                  const QJsonObject& arguments,
                                  ToolCallStatus status)                                    = 0;
  virtual void updateToolCallCard(const QString& callId,
                                  ToolCallStatus status,
                                  const QJsonObject& result       = {},
                                  const QJsonObject& verification = {})                     = 0;
  [[nodiscard]] virtual QList<DocSearch::Hit> searchDocs(const QString& query, int k) const = 0;
};

/**
 * @brief Runs the meta.* discovery surface: catalog listing, command description, snapshot,
 *        doc/help retrieval, skill loading, and the executeCommand indirection. Every
 *        handler is self-contained (card, tool result, outstanding-count release) except
 *        meta.executeCommand and meta.fetchHelp, which hand the turn back to the facade.
 */
class MetaToolRunner {
public:
  MetaToolRunner(MetaToolSink& sink, HelpFetcher& helpFetcher);

  void setDispatcher(ToolDispatcher* dispatcher);
  [[nodiscard]] bool dispatch(const QString& callId,
                              const QString& name,
                              const QJsonObject& arguments);

private:
  void runSearch(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runHowTo(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runSnapshot(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runDescribe(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runLoadSkill(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runFetchHelp(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runSearchDocs(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runListCommands(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runScriptingDocs(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runListCategories(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runExecuteCommand(const QString& callId, const QString& name, const QJsonObject& arguments);

private:
  MetaToolSink& m_sink;
  HelpFetcher& m_helpFetcher;
  ToolDispatcher* m_dispatcher;
};

}  // namespace AI
