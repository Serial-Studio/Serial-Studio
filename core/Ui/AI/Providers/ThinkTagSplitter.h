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

#include <QString>
#include <vector>

namespace AI {

/**
 * @brief Stream channel a scanned run of characters belongs to.
 */
enum class ThinkChannel {
  Text,
  Thinking
};

/**
 * @brief One run of characters and the channel it must be published on.
 */
struct ThinkChunk {
  ThinkChannel channel;
  QString text;
};

/**
 * @brief Scanner for inline <think>...</think> blocks in an OpenAI-compatible content stream:
 *        llama.cpp / Ollama servers stream reasoning inside the content delta, so it must be
 *        split before reaching the transcript. Bytes that could still complete a tag are held
 *        back until the next append(), or released verbatim by flush() at end of stream.
 */
class ThinkTagSplitter {
public:
  ThinkTagSplitter();

  [[nodiscard]] std::vector<ThinkChunk> append(const QString& chunk);
  [[nodiscard]] std::vector<ThinkChunk> flush();

private:
  /**
   * @brief Scanner position: looking for an opening tag, inside a block, or past the decision.
   */
  enum class Scan : int {
    Detect      = 0,
    Thinking    = 1,
    Passthrough = 2
  };

  void drain(bool atEnd, std::vector<ThinkChunk>& out);

private:
  Scan m_scan;
  QString m_carry;
};

}  // namespace AI
