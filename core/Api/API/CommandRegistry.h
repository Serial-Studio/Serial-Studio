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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <functional>
#include <optional>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVector>

#include "API/ICheckpointStore.h"
#include "API/PathPolicy.h"
#include "Core/Api/CommandProtocol.h"

namespace API {
/**
 * @brief Signature implemented by every API command handler.
 */
using CommandFunction =
  std::function<CommandResponse(const QString& id, const QJsonObject& params)>;

/**
 * @brief Describes a registered command.
 */
struct CommandDefinition {
  QString name;
  QString description;
  QJsonObject inputSchema;
  QVector<PathParamPolicy> pathParams;
  CommandFunction handler;
};

/**
 * @brief Policy a caller with tiers of its own (the in-app assistant, spec 0098) installs for the
 *        commands dispatched under it: the refusal to send, or nothing when the command may run.
 *        @p depth is 0 for the outermost execute() under the scope.
 */
using DispatchPolicy = std::function<std::optional<CommandResponse>(
  const QString& id, const QString& name, const QJsonObject& params, int depth)>;

/**
 * @brief RAII marker like RemoteDispatchScope, on a stack of its own so the remote-origin probes
 *        never see it: every CommandRegistry::execute under it, outer or nested, answers to
 *        @p policy. A trusted dispatch opens one with an empty policy, shadowing it, so a
 *        consented script keeps full trust under an assistant call.
 */
class DispatchPolicyScope {
public:
  explicit DispatchPolicyScope(DispatchPolicy policy);
  ~DispatchPolicyScope();
  DispatchPolicyScope(DispatchPolicyScope&&)                 = delete;
  DispatchPolicyScope(const DispatchPolicyScope&)            = delete;
  DispatchPolicyScope& operator=(DispatchPolicyScope&&)      = delete;
  DispatchPolicyScope& operator=(const DispatchPolicyScope&) = delete;

public:
  [[nodiscard]] static const DispatchPolicyScope* active() noexcept;
  [[nodiscard]] std::optional<CommandResponse> authorize(const QString& id,
                                                         const QString& name,
                                                         const QJsonObject& params) const;

private:
  int m_baseDepth;
  DispatchPolicy m_policy;
  const DispatchPolicyScope* m_previous;
};

/**
 * @brief Policy a remote entry point installs for the commands dispatched under it: the refusal
 *        to send, or nothing when the command may run.
 */
using RemoteGate = std::function<std::optional<CommandResponse>(
  const QString& id, const QString& name, const QJsonObject& params)>;

/**
 * @brief RAII marker: every CommandRegistry::execute under it, outer or nested, answers to @p gate
 *        (the server's policy for TCP/MCP/gRPC; empty, so trusted, for a script). It also shadows
 *        a caller's DispatchPolicyScope (spec 0098): a request served in an assistant call's
 *        nested event loop answers to its own origin, not the assistant's tiers.
 */
class RemoteDispatchScope {
public:
  explicit RemoteDispatchScope(RemoteGate gate);
  ~RemoteDispatchScope();
  RemoteDispatchScope(RemoteDispatchScope&&)                 = delete;
  RemoteDispatchScope(const RemoteDispatchScope&)            = delete;
  RemoteDispatchScope& operator=(RemoteDispatchScope&&)      = delete;
  RemoteDispatchScope& operator=(const RemoteDispatchScope&) = delete;

public:
  [[nodiscard]] static const RemoteDispatchScope* active() noexcept;
  [[nodiscard]] std::optional<CommandResponse> authorize(const QString& id,
                                                         const QString& name,
                                                         const QJsonObject& params) const;

private:
  RemoteGate m_gate;
  const RemoteDispatchScope* m_previous;
  DispatchPolicyScope m_policyShadow;
};

/**
 * @brief Central registry for all available API commands. The pre-mutation checkpoint goes
 *        through the bound ICheckpointStore (spec 0077 T63); every root binds one before a
 *        command can run, so an unbound store is a logged assert and no snapshot.
 */
class CommandRegistry {
private:
  CommandRegistry();
  CommandRegistry(CommandRegistry&&)                 = delete;
  CommandRegistry(const CommandRegistry&)            = delete;
  CommandRegistry& operator=(CommandRegistry&&)      = delete;
  CommandRegistry& operator=(const CommandRegistry&) = delete;

public:
  [[nodiscard]] static CommandRegistry& instance();

  void bindCheckpointStore(ICheckpointStore& store) noexcept;
  [[nodiscard]] ICheckpointStore* checkpointStore() const noexcept;

  void registerCommand(const QString& name, const QString& description, CommandFunction handler);
  void registerCommand(const QString& name,
                       const QString& description,
                       const QJsonObject& inputSchema,
                       CommandFunction handler);

  [[nodiscard]] bool hasCommand(const QString& name) const;
  [[nodiscard]] CommandResponse execute(const QString& name,
                                        const QString& id,
                                        const QJsonObject& params);

  [[nodiscard]] QStringList availableCommands() const;
  [[nodiscard]] const QMap<QString, CommandDefinition>& commands() const;

private:
  CommandResponse buildUnknownCommandResponse(const QString& name, const QString& id) const;
  [[nodiscard]] bool commandHonorsDryRun(const QString& name) const;
  void attachErrorMetadata(const QString& name, CommandResponse& response) const;
  static QString classifyErrorCategory(const QString& commandName, const CommandResponse& response);
  static QString dryRunHintForScriptCommand(const QString& commandName);

  ICheckpointStore* m_checkpoints;
  QMap<QString, CommandDefinition> m_commands;
};

}  // namespace API
