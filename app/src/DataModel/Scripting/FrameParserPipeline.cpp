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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
      cells.append(was_quoted ? cell : cell.trimmed());
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

  cells.append(was_quoted ? cell : cell.trimmed());
  return cells;
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
  Q_ASSERT(sourceId >= 0);

  IO::FrameReader reader;
  configureAndFeed(reader, input, spec);
  const auto extracted = drainExtractedFrames(reader);

  PipelineResult result;
  populateExtractionStats(result, input.size(), extracted, reader.droppedFrameCount());

  auto& parser = DataModel::FrameParser::instance();
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
  Q_ASSERT(!templateId.isEmpty());

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
