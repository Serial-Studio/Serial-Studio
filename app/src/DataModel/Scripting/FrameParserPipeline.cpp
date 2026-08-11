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

#include "DataModel/Scripting/FrameParserPipeline.h"

#include <memory>

#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/IScriptEngine.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "IO/FrameReader.h"
#include "IO/HAL_Driver.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps raw frame bytes to the representation the parser receives.
 */
static QString decoderRepresentation(const QByteArray& raw,
                                     SerialStudio::DecoderMethod method,
                                     bool& producedBinary)
{
  producedBinary = false;
  switch (method) {
    case SerialStudio::Hexadecimal:
      return QString::fromLatin1(raw.toHex());
    case SerialStudio::Base64:
      return QString::fromLatin1(raw.toBase64());
    case SerialStudio::Binary:
      producedBinary = true;
      return QString::fromLatin1(raw.toHex(' '));
    case SerialStudio::PlainText:
    default:
      return QString::fromUtf8(raw);
  }
}

/**
 * @brief Configures a throwaway FrameReader for one-shot extraction and feeds the input bytes.
 */
static void configureAndFeed(IO::FrameReader& reader,
                             const QByteArray& input,
                             const DataModel::PipelineSpec& spec)
{
  reader.setOperationMode(spec.operationMode);
  reader.setFrameDetectionMode(spec.frameDetection);
  reader.setStartSequences(spec.startSequences);
  reader.setFinishSequences(spec.finishSequences);
  reader.setChecksum(spec.checksumAlgorithm);

  if (input.isEmpty())
    return;

  reader.processData(IO::makeCapturedData(input));
}

/**
 * @brief Drains all extracted frames from a one-shot FrameReader run.
 */
static QList<IO::CapturedDataPtr> drainExtractedFrames(IO::FrameReader& reader)
{
  QList<IO::CapturedDataPtr> out;
  auto& queue = reader.queue();
  IO::CapturedDataPtr next;
  while (queue.try_dequeue(next))
    out.append(next);

  return out;
}

/**
 * @brief Builds a one-shot script engine for the dryRun path.
 */
static std::unique_ptr<DataModel::IScriptEngine> makeEngineForLanguage(int language)
{
  if (language == SerialStudio::Native)
    return std::make_unique<DataModel::CFrameParser>();

  if (language == SerialStudio::Lua)
    return std::make_unique<DataModel::LuaScriptEngine>();

  return std::make_unique<DataModel::JsScriptEngine>();
}

//--------------------------------------------------------------------------------------------------
// Decoder seam
//--------------------------------------------------------------------------------------------------

/**
 * @brief Live-parser overload: applies the decoder + the source's parser engine to one frame.
 */
void DataModel::decodeAndParseFrame(const QByteArray& rawFrame,
                                    SerialStudio::DecoderMethod decoderMethod,
                                    DataModel::FrameParser& parser,
                                    int sourceId,
                                    QList<QStringList>& outChannels)
{
  switch (decoderMethod) {
    case SerialStudio::Hexadecimal:
      outChannels = parser.parseMultiFrame(QString::fromLatin1(rawFrame.toHex()), sourceId);
      break;
    case SerialStudio::Base64:
      outChannels = parser.parseMultiFrame(QString::fromLatin1(rawFrame.toBase64()), sourceId);
      break;
    case SerialStudio::Binary:
      outChannels = parser.parseMultiFrame(rawFrame, sourceId);
      break;
    case SerialStudio::PlainText:
    default:
      outChannels = parser.parseMultiFrameUtf8(rawFrame, sourceId);
      break;
  }
}

/**
 * @brief Engine-override variant: same decoder switch against a caller-owned engine (dryRun).
 */
void DataModel::decodeAndParseFrame(const QByteArray& rawFrame,
                                    SerialStudio::DecoderMethod decoderMethod,
                                    DataModel::IScriptEngine& engine,
                                    QList<QStringList>& outChannels)
{
  outChannels.clear();
  if (rawFrame.isEmpty() || !engine.isLoaded())
    return;

  switch (decoderMethod) {
    case SerialStudio::Hexadecimal:
      outChannels = engine.parseString(QString::fromLatin1(rawFrame.toHex()));
      break;
    case SerialStudio::Base64:
      outChannels = engine.parseString(QString::fromLatin1(rawFrame.toBase64()));
      break;
    case SerialStudio::Binary:
      outChannels = engine.parseBinary(rawFrame);
      break;
    case SerialStudio::PlainText:
    default:
      outChannels = engine.parseUtf8(rawFrame);
      break;
  }
}

/**
 * @brief Comma split (whitespace stripped) producing one channel row per frame.
 */
void DataModel::splitQuickPlotChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels)
{
  outChannels.clear();
  if (rawFrame.isEmpty())
    return;

  QStringList row;
  row.reserve(16);

  const char* raw      = rawFrame.constData();
  const int dataLength = rawFrame.size();
  int start            = 0;
  for (int i = 0; i <= dataLength; ++i) {
    if (i == dataLength || raw[i] == ',') {
      int s = start;
      int e = i;
      while (s < e && (raw[s] == ' ' || raw[s] == '\t' || raw[s] == '\r' || raw[s] == '\n'))
        ++s;
      while (
        e > s
        && (raw[e - 1] == ' ' || raw[e - 1] == '\t' || raw[e - 1] == '\r' || raw[e - 1] == '\n'))
        --e;

      if (e > s)
        row.append(QString::fromUtf8(raw + s, e - s));
      else
        row.append(QString());

      start = i + 1;
    }
  }

  if (!row.isEmpty())
    outChannels.append(std::move(row));
}

/**
 * @brief Joins replay cells into one comma-separated row, RFC-4180-quoting any cell that
 *        contains a comma, quote or newline. Counterpart of splitReplayRow.
 */
QByteArray DataModel::joinReplayRow(const QStringList& cells)
{
  QString row;
  for (int i = 0; i < cells.size(); ++i) {
    if (i > 0)
      row.append(QChar(','));

    const QString& cell      = cells.at(i);
    const bool needs_quoting = cell.contains(QChar(',')) || cell.contains(QChar('"'))
                            || cell.contains(QChar('\n')) || cell.contains(QChar('\r'));
    if (!needs_quoting) {
      row.append(cell);
      continue;
    }

    QString quoted = cell;
    quoted.replace(QChar('"'), QStringLiteral("\"\""));
    row.append(QChar('"'));
    row.append(quoted);
    row.append(QChar('"'));
  }

  return row.toUtf8();
}

/**
 * @brief Undoes the CSV export's formula-injection guard: a leading apostrophe before a
 *        dangerous first char is a sanitizer artifact, not data. Restores recordings written
 *        before numeric fields were exempted from the guard ("'-0.5" -> "-0.5").
 */
static void stripCsvInjectionGuard(QString& cell)
{
  if (cell.size() < 2 || cell.at(0) != QChar('\''))
    return;

  const QChar c     = cell.at(1);
  const bool danger = c == QChar('=') || c == QChar('+') || c == QChar('-') || c == QChar('@')
                   || c == QChar('\t') || c == QChar('\r');
  if (danger)
    cell.remove(0, 1);
}

/**
 * @brief Quote-aware comma split of one synthesized replay row (RFC-4180 double-quote escape).
 */
QStringList DataModel::splitReplayRow(QStringView row)
{
  QStringList cells;
  QString cell;
  bool in_quotes  = false;
  bool was_quoted = false;

  const qsizetype length = row.size();
  for (qsizetype i = 0; i < length; ++i) {
    const QChar c = row.at(i);

    if (in_quotes) {
      const bool escaped = c == QChar('"') && i + 1 < length && row.at(i + 1) == QChar('"');
      if (escaped) {
        cell.append(QChar('"'));
        ++i;
        continue;
      }

      if (c == QChar('"')) {
        in_quotes = false;
        continue;
      }

      cell.append(c);
      continue;
    }

    if (c == QChar(',')) {
      QString value = was_quoted ? cell : cell.trimmed();
      stripCsvInjectionGuard(value);
      cells.append(std::move(value));
      cell.clear();
      was_quoted = false;
      continue;
    }

    if (c == QChar('"') && !was_quoted && cell.trimmed().isEmpty()) {
      in_quotes  = true;
      was_quoted = true;
      cell.clear();
      continue;
    }

    cell.append(c);
  }

  QString last = was_quoted ? cell : cell.trimmed();
  stripCsvInjectionGuard(last);
  cells.append(std::move(last));
  return cells;
}

/**
 * @brief True when every byte in [begin, end) of @p row is ASCII whitespace.
 */
[[nodiscard]] static bool allAsciiSpace(QByteArrayView row, qsizetype begin, qsizetype end)
{
  for (qsizetype i = begin; i < end; ++i) {
    const char c = row.at(i);
    if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
      return false;
  }

  return true;
}

/**
 * @brief ASCII-whitespace trim of a cell view (byte twin of QString::trimmed for the
 *        character set replay rows actually contain).
 */
[[nodiscard]] static QByteArrayView trimmedCellView(QByteArrayView v)
{
  qsizetype b = 0;
  qsizetype e = v.size();
  while (b < e && allAsciiSpace(v, b, b + 1))
    ++b;
  while (e > b && allAsciiSpace(v, e - 1, e))
    --e;

  return v.sliced(b, e - b);
}

/**
 * @brief View twin of stripCsvInjectionGuard: drops the sanitizer apostrophe by advancing
 *        the view instead of mutating a string.
 */
[[nodiscard]] static QByteArrayView stripGuardView(QByteArrayView v)
{
  if (v.size() < 2 || v.at(0) != '\'')
    return v;

  const char c      = v.at(1);
  const bool danger = c == '=' || c == '+' || c == '-' || c == '@' || c == '\t' || c == '\r';
  return danger ? v.sliced(1) : v;
}

/**
 * @brief Byte replica of splitReplayRow's per-character machine for one cell region that
 *        needs rewriting (escaped quotes or content around a closed quote); appends the
 *        finalized cell bytes to @p scratch.
 */
static void appendRewrittenCell(QByteArrayView row,
                                qsizetype begin,
                                qsizetype end,
                                QByteArray& scratch)
{
  SS_ASSERT(begin <= end, return);
  SS_ASSERT(end <= row.size(), return);

  bool in_quotes            = false;
  bool was_quoted           = false;
  const qsizetype cell_base = scratch.size();
  for (qsizetype i = begin; i < end; ++i) {
    const char c = row.at(i);
    if (in_quotes) {
      const bool escaped = c == '"' && i + 1 < end && row.at(i + 1) == '"';
      if (escaped) {
        scratch.append('"');
        ++i;
        continue;
      }

      if (c == '"') {
        in_quotes = false;
        continue;
      }

      scratch.append(c);
      continue;
    }

    if (c == '"' && !was_quoted
        && allAsciiSpace(QByteArrayView(scratch), cell_base, scratch.size())) {
      in_quotes  = true;
      was_quoted = true;
      scratch.resize(cell_base);
      continue;
    }

    scratch.append(c);
  }
}

namespace detail {

/**
 * @brief A cell whose bytes had to be rewritten into the scratch buffer: the slot it occupies
 *        in the output array plus the scratch byte range it owns.
 */
struct RewrittenCell {
  qsizetype slot = 0;
  qsizetype base = 0;
  qsizetype size = 0;
};

/**
 * @brief Scratch-backed cells awaiting resolution. Stack storage covers the rows that carry
 *        escaped quotes at all; plain numeric rows never touch it.
 */
using RewrittenCells = QVarLengthArray<RewrittenCell, 8>;

}  // namespace detail

using detail::RewrittenCell;
using detail::RewrittenCells;

/**
 * @brief Byte-level twin of splitReplayRow: identical quote/trim/guard semantics, cells
 *        returned as views into @p row or @p scratch. Scratch-backed cells stay offsets until
 *        after the final append, so a reallocation cannot strand an emitted view; the reserve
 *        only keeps the append loop allocation-free.
 */
void DataModel::splitReplayRowSpans(QByteArrayView row,
                                    ReplayCellViews& out,
                                    QByteArray& scratch,
                                    char separator)
{
  out.clear();
  scratch.resize(0);
  if (row.endsWith('\r'))
    row.chop(1);

  if (scratch.capacity() < row.size())
    scratch.reserve(row.size());

  SS_ASSERT_LOG(scratch.capacity() >= row.size());

  const qsizetype length  = row.size();
  qsizetype cell_start    = 0;
  qsizetype interior_from = -1;
  qsizetype interior_to   = -1;
  bool in_quotes          = false;
  bool was_quoted         = false;
  bool needs_rewrite      = false;
  RewrittenCells rewritten;

  const auto finalize = [&](qsizetype end) {
    QByteArrayView value;
    if (!was_quoted)
      value = trimmedCellView(row.sliced(cell_start, end - cell_start));
    else if (!needs_rewrite) {
      const qsizetype to = (interior_to >= 0) ? interior_to : end;
      value              = row.sliced(interior_from, to - interior_from);
    } else {
      const qsizetype base = scratch.size();
      appendRewrittenCell(row, cell_start, end, scratch);
      SS_ASSERT_LOG(scratch.size() <= row.size());
      rewritten.append(RewrittenCell{out.size(), base, scratch.size() - base});
    }

    out.append(stripGuardView(value));
    interior_from = -1;
    interior_to   = -1;
    was_quoted    = false;
    needs_rewrite = false;
  };

  for (qsizetype i = 0; i < length; ++i) {
    const char c = row.at(i);

    if (in_quotes) {
      const bool escaped = c == '"' && i + 1 < length && row.at(i + 1) == '"';
      if (escaped) {
        needs_rewrite = true;
        ++i;
        continue;
      }

      if (c == '"') {
        in_quotes   = false;
        interior_to = i;
      }

      continue;
    }

    if (c == separator) {
      finalize(i);
      cell_start = i + 1;
      continue;
    }

    if (c == '"' && !was_quoted && allAsciiSpace(row, cell_start, i)) {
      in_quotes     = true;
      was_quoted    = true;
      interior_from = i + 1;
      continue;
    }

    if (was_quoted)
      needs_rewrite = true;
  }

  finalize(length);

  for (const auto& cell : rewritten)
    out[cell.slot] = stripGuardView(QByteArrayView(scratch).sliced(cell.base, cell.size));
}

/**
 * @brief Replay twin of splitQuickPlotChannels: one quote-aware row per non-empty line.
 */
void DataModel::splitReplayChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels)
{
  outChannels.clear();
  if (rawFrame.isEmpty())
    return;

  const QString text = QString::fromUtf8(rawFrame);
  const auto lines   = QStringView(text).split(QChar('\n'), Qt::SkipEmptyParts);
  for (const auto& line : lines) {
    const auto trimmed = line.trimmed();
    if (trimmed.isEmpty())
      continue;

    outChannels.append(splitReplayRow(trimmed));
  }
}

//--------------------------------------------------------------------------------------------------
// Pipeline runners (extraction + decode + parse)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Per-frame helper: QuickPlot -> comma split, other modes -> decoder seam + engine.
 */
static DataModel::PipelineFrame buildPipelineFrame(const IO::CapturedDataPtr& f,
                                                   const DataModel::PipelineSpec& spec,
                                                   DataModel::FrameParser* liveParser,
                                                   DataModel::IScriptEngine* throwawayEngine,
                                                   int sourceId)
{
  DataModel::PipelineFrame frame;
  frame.rawBytes = f ? f->data : QByteArray();
  frame.decoderOutput =
    decoderRepresentation(frame.rawBytes, spec.decoderMethod, frame.decoderProducedBinary);

  if (frame.rawBytes.isEmpty())
    return frame;

  if (spec.operationMode == SerialStudio::QuickPlot) {
    DataModel::splitQuickPlotChannels(frame.rawBytes, frame.rows);
    return frame;
  }

  if (throwawayEngine)
    DataModel::decodeAndParseFrame(
      frame.rawBytes, spec.decoderMethod, *throwawayEngine, frame.rows);
  else if (liveParser)
    DataModel::decodeAndParseFrame(
      frame.rawBytes, spec.decoderMethod, *liveParser, sourceId, frame.rows);

  return frame;
}

/**
 * @brief Fills extraction-side stats (consumed / remaining / dropped) onto a PipelineResult.
 */
static void populateExtractionStats(DataModel::PipelineResult& result,
                                    qsizetype inputBytes,
                                    const QList<IO::CapturedDataPtr>& extracted,
                                    quint64 droppedFrames)
{
  result.extractedCount = extracted.size();
  result.droppedFrames  = droppedFrames;

  qsizetype consumed = 0;
  for (const auto& f : extracted)
    if (f)
      consumed += f->data.size();

  result.consumedBytes  = consumed;
  result.remainingBytes = std::max<qsizetype>(0, inputBytes - consumed);
}

/**
 * @brief Runs the live parser engine for @p sourceId against decoded extracted frames.
 *        QuickPlot mode bypasses the parser and comma-splits each frame.
 */
DataModel::PipelineResult DataModel::runFrameParserPipeline(const QByteArray& input,
                                                            const DataModel::PipelineSpec& spec,
                                                            int sourceId)
{
  SS_ASSERT(sourceId >= 0, return {});

  IO::FrameReader reader;
  configureAndFeed(reader, input, spec);
  const auto extracted = drainExtractedFrames(reader);

  PipelineResult result;
  populateExtractionStats(result, input.size(), extracted, reader.droppedFrameCount());

  static auto& parser = DataModel::FrameParser::instance();
  result.frames.reserve(extracted.size());
  for (const auto& f : extracted)
    result.frames.append(buildPipelineFrame(f, spec, &parser, nullptr, sourceId));

  return result;
}

/**
 * @brief Runs a throwaway engine over decoded extracted frames; used by project.frameParser.dryRun.
 *        QuickPlot mode bypasses the script engine compilation entirely.
 */
DataModel::PipelineResult DataModel::runFrameParserPipelineWithCode(
  const QByteArray& input,
  const DataModel::PipelineSpec& spec,
  const QString& parserCode,
  int parserLanguage)
{
  const bool needsEngine = (spec.operationMode != SerialStudio::QuickPlot);

  std::unique_ptr<DataModel::IScriptEngine> engine;
  if (needsEngine) {
    engine = makeEngineForLanguage(parserLanguage);
    if (!engine->loadScript(parserCode, 0, false)) {
      PipelineResult failed;
      failed.stageError = QStringLiteral("Frame parser failed to compile or define parse(frame).");
      failed.stageWhere = QStringLiteral("compile");
      return failed;
    }
  }

  IO::FrameReader reader;
  configureAndFeed(reader, input, spec);
  const auto extracted = drainExtractedFrames(reader);

  PipelineResult result;
  populateExtractionStats(result, input.size(), extracted, reader.droppedFrameCount());

  result.frames.reserve(extracted.size());
  for (const auto& f : extracted)
    result.frames.append(buildPipelineFrame(f, spec, nullptr, engine.get(), 0));

  return result;
}

/**
 * @brief Runs a throwaway native template engine over the input; used by the live preview pane.
 */
DataModel::PipelineResult DataModel::runNativeTemplatePipeline(const QByteArray& input,
                                                               const DataModel::PipelineSpec& spec,
                                                               const QString& templateId,
                                                               const QJsonObject& params)
{
  SS_ASSERT(!templateId.isEmpty(), return {});

  const auto descriptor = CFrameParser::buildDescriptor(templateId, params);
  return runFrameParserPipelineWithCode(input, spec, descriptor, SerialStudio::Native);
}

/**
 * @brief Returns the decoder-side representation that the parser would receive for @p raw.
 */
QString DataModel::previewDecoderOutput(const QByteArray& raw,
                                        SerialStudio::DecoderMethod method,
                                        bool* producedBinary)
{
  bool isBinary   = false;
  const auto repr = decoderRepresentation(raw, method, isBinary);
  if (producedBinary)
    *producedBinary = isBinary;

  return repr;
}
