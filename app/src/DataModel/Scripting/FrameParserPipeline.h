/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QByteArray>
#include <QByteArrayView>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVarLengthArray>
#include <QVector>

#include "SerialStudio.h"

namespace DataModel {

class FrameParser;
class IScriptEngine;

/**
 * @brief Inputs to a one-shot pipeline run -- mirrors the live FrameReader + FrameBuilder
 *        decoder switch but without touching the running project.
 */
struct PipelineSpec {
  SerialStudio::OperationMode operationMode   = SerialStudio::ProjectFile;
  SerialStudio::FrameDetection frameDetection = SerialStudio::EndDelimiterOnly;
  QList<QByteArray> startSequences;
  QList<QByteArray> finishSequences;
  QString checksumAlgorithm;
  SerialStudio::DecoderMethod decoderMethod = SerialStudio::PlainText;
};

/**
 * @brief Single extracted frame after decoder + parser have run.
 */
struct PipelineFrame {
  QByteArray rawBytes;
  QString decoderOutput;
  bool decoderProducedBinary = false;
  QList<QStringList> rows;
};

/**
 * @brief Aggregate result of a pipeline run, including extraction stats.
 */
struct PipelineResult {
  QVector<PipelineFrame> frames;
  int extractedCount       = 0;
  qsizetype consumedBytes  = 0;
  qsizetype remainingBytes = 0;
  quint64 droppedFrames    = 0;
  QString stageError;
  QString stageWhere;
};

/**
 * @brief Live-parser variant of the decoder seam: applies the decoder method then parses.
 */
void decodeAndParseFrame(const QByteArray& rawFrame,
                         SerialStudio::DecoderMethod decoderMethod,
                         DataModel::FrameParser& parser,
                         int sourceId,
                         QList<QStringList>& outChannels);

/**
 * @brief Engine-override variant of the decoder seam: used by the dryRun API.
 */
void decodeAndParseFrame(const QByteArray& rawFrame,
                         SerialStudio::DecoderMethod decoderMethod,
                         DataModel::IScriptEngine& engine,
                         QList<QStringList>& outChannels);

/**
 * @brief QuickPlot comma split: one channel row per frame, no script engine involved.
 */
void splitQuickPlotChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels);

/**
 * @brief Joins replay cells into one comma-separated row, RFC-4180-quoting any cell that
 *        contains a comma, quote or newline. Counterpart of splitReplayRow.
 */
[[nodiscard]] QByteArray joinReplayRow(const QStringList& cells);

/**
 * @brief Quote-aware comma split of one synthesized replay row (RFC-4180 double-quote escape).
 */
[[nodiscard]] QStringList splitReplayRow(QStringView row);

/**
 * @brief Reusable per-cell view buffer for splitReplayRowSpans (stack storage for the
 *        common column counts).
 */
using ReplayCellViews = QVarLengthArray<QByteArrayView, 64>;

/**
 * @brief Byte-level twin of splitReplayRow for disk-backed replay rows: identical quote,
 *        trim and injection-guard semantics, but cells come back as views into @p row (or
 *        into @p scratch for cells that need RFC-4180 unescaping). A trailing CR is chopped
 *        (QTextStream did that upstream); trimming covers ASCII whitespace, the one
 *        documented divergence from QString::trimmed for exotic Unicode spaces. Views stay
 *        valid while @p row's bytes and @p scratch are alive and untouched; scratch-backed
 *        cells are resolved after the final append, so they survive a scratch reallocation.
 *        @p separator swaps the cell boundary (CSV player sniffing, spec 0048); quoting,
 *        trimming and guard semantics are separator-independent.
 */
void splitReplayRowSpans(QByteArrayView row,
                         ReplayCellViews& out,
                         QByteArray& scratch,
                         char separator = ',');

/**
 * @brief Replay twin of splitQuickPlotChannels: one quote-aware row per non-empty line.
 */
void splitReplayChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels);

/**
 * @brief Extraction + decoder + parser against the live FrameParser engine for @p sourceId.
 */
[[nodiscard]] PipelineResult runFrameParserPipeline(const QByteArray& input,
                                                    const PipelineSpec& spec,
                                                    int sourceId);

/**
 * @brief Extraction + decoder + parser with a throwaway engine (dryRun). For the Native
 *        language, @p parserCode carries the JSON descriptor {"template": ..., "params": ...}.
 */
[[nodiscard]] PipelineResult runFrameParserPipelineWithCode(const QByteArray& input,
                                                            const PipelineSpec& spec,
                                                            const QString& parserCode,
                                                            int parserLanguage);

/**
 * @brief Extraction + decoder + a throwaway native template engine (live preview / dryRun).
 */
[[nodiscard]] PipelineResult runNativeTemplatePipeline(const QByteArray& input,
                                                       const PipelineSpec& spec,
                                                       const QString& templateId,
                                                       const QJsonObject& params);

/**
 * @brief Returns the decoder-side representation the parser would receive for @p raw.
 */
[[nodiscard]] QString previewDecoderOutput(const QByteArray& raw,
                                           SerialStudio::DecoderMethod method,
                                           bool* producedBinary = nullptr);

}  // namespace DataModel
