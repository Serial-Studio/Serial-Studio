/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace AI::ProviderJson {

/**
 * @brief Rewrites a tool name into the ^[a-zA-Z0-9_-]+ shape both Anthropic and OpenAI enforce.
 */
[[nodiscard]] QString sanitizeToolName(const QString& original);

/**
 * @brief Flattens structured system blocks into the single system string every
 *        Chat-Completions-shaped backend takes, blocks joined by a blank line.
 */
[[nodiscard]] QString flattenSystemBlocks(const QJsonArray& blocks);

/**
 * @brief Converts Anthropic-shaped history into the OpenAI Chat Completions shape, prepending
 *        the system text as a system (or developer) message and back-filling stub replies for
 *        assistant tool calls no tool message answers.
 */
[[nodiscard]] QJsonArray translateHistory(const QJsonArray& history,
                                          const QString& systemText,
                                          bool useDeveloperRole);

/**
 * @brief Converts AI-tool definitions into the OpenAI tool-choice schema.
 */
[[nodiscard]] QJsonArray translateTools(const QJsonArray& tools);

/**
 * @brief Builds the request body shared by every OpenAI-compatible endpoint. Vendor-specific
 *        fields (caching keys, reasoning effort, parallel tool calls) are added by the caller.
 */
[[nodiscard]] QJsonObject chatCompletionsBody(const QString& model,
                                              const QJsonArray& history,
                                              const QString& systemText,
                                              const QJsonArray& tools,
                                              bool forbidToolUse,
                                              bool useDeveloperRole = false);

/**
 * @brief Extracts `error.message` from an HTTP error body, empty when the body carries none.
 *        The caller owns the user-facing wording so its tr() context stays with its class.
 */
[[nodiscard]] QString errorMessageFromBody(const QByteArray& body);

/**
 * @brief True when @p status is worth retrying: request timeout, rate limit, or a server fault.
 */
[[nodiscard]] bool isTransientHttpStatus(int status);

}  // namespace AI::ProviderJson
