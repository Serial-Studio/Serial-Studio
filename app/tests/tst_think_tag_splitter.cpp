/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QString>
#include <QTest>
#include <vector>

#include "AI/Providers/ThinkTagSplitter.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Drives the inline <think> scanner with synthetic streams, including tags split across
 *        chunk boundaries -- the case a local server produces whenever a tag straddles a packet.
 */
class TstThinkTagSplitter : public QObject {
  Q_OBJECT

private slots:
  void plainTextPassesThrough();
  void partialOpenTagIsHeldBack();
  void thinkingBlockRoutesToThinkingChannel();
  void openTagSplitAcrossChunks();
  void closeTagSplitAcrossChunks();
  void textAfterBlockIsPassthrough();
  void flushReleasesUnterminatedBlock();
  void leadingWhitespaceBeforeOpenTag();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Concatenates the runs of one channel, so a test asserts on content rather than on how
 *        many pieces the scanner happened to publish it in.
 */
static QString joined(const std::vector<AI::ThinkChunk>& chunks, AI::ThinkChannel channel)
{
  QString out;
  for (const auto& chunk : chunks)
    if (chunk.channel == channel)
      out.append(chunk.text);

  return out;
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief A stream that never opens a tag reaches the transcript verbatim.
 */
void TstThinkTagSplitter::plainTextPassesThrough()
{
  AI::ThinkTagSplitter splitter;
  const auto first = splitter.append(QStringLiteral("hello "));
  const auto rest  = splitter.append(QStringLiteral("world"));

  QCOMPARE(joined(first, AI::ThinkChannel::Text) + joined(rest, AI::ThinkChannel::Text),
           QStringLiteral("hello world"));
  QCOMPARE(joined(first, AI::ThinkChannel::Thinking), QString());
}

/**
 * @brief A prefix that could still become <think> is buffered rather than published as text.
 */
void TstThinkTagSplitter::partialOpenTagIsHeldBack()
{
  AI::ThinkTagSplitter splitter;
  const auto out = splitter.append(QStringLiteral("<thi"));

  QCOMPARE(joined(out, AI::ThinkChannel::Text), QString());
  QCOMPARE(joined(out, AI::ThinkChannel::Thinking), QString());
}

/**
 * @brief A complete block in one chunk lands entirely on the thinking channel.
 */
void TstThinkTagSplitter::thinkingBlockRoutesToThinkingChannel()
{
  AI::ThinkTagSplitter splitter;
  const auto out = splitter.append(QStringLiteral("<think>reasoning</think>answer"));

  QCOMPARE(joined(out, AI::ThinkChannel::Thinking), QStringLiteral("reasoning"));
  QCOMPARE(joined(out, AI::ThinkChannel::Text), QStringLiteral("answer"));
}

/**
 * @brief An opening tag split across two chunks is still recognised as a tag.
 */
void TstThinkTagSplitter::openTagSplitAcrossChunks()
{
  AI::ThinkTagSplitter splitter;
  const auto first  = splitter.append(QStringLiteral("<thi"));
  const auto second = splitter.append(QStringLiteral("nk>deep</think>done"));

  QCOMPARE(joined(first, AI::ThinkChannel::Text), QString());
  QCOMPARE(joined(second, AI::ThinkChannel::Thinking), QStringLiteral("deep"));
  QCOMPARE(joined(second, AI::ThinkChannel::Text), QStringLiteral("done"));
}

/**
 * @brief A closing tag split across chunks never leaks its own characters into the transcript.
 */
void TstThinkTagSplitter::closeTagSplitAcrossChunks()
{
  AI::ThinkTagSplitter splitter;
  const auto first  = splitter.append(QStringLiteral("<think>abc</thi"));
  const auto second = splitter.append(QStringLiteral("nk>tail"));

  const auto thinking =
    joined(first, AI::ThinkChannel::Thinking) + joined(second, AI::ThinkChannel::Thinking);

  QCOMPARE(thinking, QStringLiteral("abc"));
  QCOMPARE(joined(first, AI::ThinkChannel::Text), QString());
  QCOMPARE(joined(second, AI::ThinkChannel::Text), QStringLiteral("tail"));
}

/**
 * @brief Once the stream is past the decision, later chunks go straight to text.
 */
void TstThinkTagSplitter::textAfterBlockIsPassthrough()
{
  AI::ThinkTagSplitter splitter;
  (void)splitter.append(QStringLiteral("plain"));

  const auto out = splitter.append(QStringLiteral(" <think>not a tag anymore"));
  QCOMPARE(joined(out, AI::ThinkChannel::Text), QStringLiteral(" <think>not a tag anymore"));
  QCOMPARE(joined(out, AI::ThinkChannel::Thinking), QString());
}

/**
 * @brief End of stream releases whatever is still buffered instead of dropping it.
 */
void TstThinkTagSplitter::flushReleasesUnterminatedBlock()
{
  AI::ThinkTagSplitter splitter;
  const auto streamed = splitter.append(QStringLiteral("<think>unterminated"));
  const auto flushed  = splitter.flush();

  const auto thinking =
    joined(streamed, AI::ThinkChannel::Thinking) + joined(flushed, AI::ThinkChannel::Thinking);

  QCOMPARE(thinking, QStringLiteral("unterminated"));
  QCOMPARE(joined(flushed, AI::ThinkChannel::Text), QString());
}

/**
 * @brief Whitespace ahead of the opening tag is consumed, not published as an empty text run.
 */
void TstThinkTagSplitter::leadingWhitespaceBeforeOpenTag()
{
  AI::ThinkTagSplitter splitter;
  const auto out = splitter.append(QStringLiteral("\n  <think>x</think>y"));

  QCOMPARE(joined(out, AI::ThinkChannel::Thinking), QStringLiteral("x"));
  QCOMPARE(joined(out, AI::ThinkChannel::Text), QStringLiteral("y"));
}

QTEST_APPLESS_MAIN(TstThinkTagSplitter)

#include "tst_think_tag_splitter.moc"
