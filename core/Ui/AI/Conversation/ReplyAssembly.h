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

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace AI::ReplyAssembly {

[[nodiscard]] QString neutralizeHistoryDelimiter(const QString& payload);

[[nodiscard]] QString rewriteHelpLinks(const QString& text);

[[nodiscard]] QString wrapUntrusted(const QString& sourceTag, const QByteArray& contentBytes);

[[nodiscard]] QJsonObject makeToolUseBlock(const QString& callId,
                                           const QString& name,
                                           const QJsonObject& arguments,
                                           const QJsonObject& extras);

[[nodiscard]] QJsonObject makeAssistantMessage(const QJsonArray& thinkingBlocks,
                                               const QString& text,
                                               const QJsonArray& toolUseBlocks);

[[nodiscard]] QJsonObject makeTruncatedResult(const QJsonObject& scrubbed,
                                              const QByteArray& fullBytes,
                                              int budgetBytes);

[[nodiscard]] QJsonObject makeToolResultBlock(const QString& callId,
                                              const QString& name,
                                              const QJsonObject& effective,
                                              const QByteArray& contentBytes);

}  // namespace AI::ReplyAssembly
