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

#include "DataModel/Dialogs/FrameParserTestDialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSpacerItem>
#include <QVBoxLayout>

#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "IO/Checksum.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Internal helpers (file-local)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves an active source by id from ProjectModel, or returns a defaulted Source.
 */
static DataModel::Source sourceById(int sourceId)
{
  const auto& sources = DataModel::ProjectModel::instance().sources();
  for (const auto& s : sources)
    if (s.sourceId == sourceId)
      return s;

  return {};
}

/**
 * @brief Returns the bytes encoded by either the FrameStart / FrameEnd field, honoring hex mode.
 */
static QByteArray delimiterFromField(const QString& text, bool hex)
{
  if (text.isEmpty())
    return {};

  if (hex) {
    const auto resolved = SerialStudio::resolveEscapeSequences(text);
    return QByteArray::fromHex(QString(resolved).remove(' ').toUtf8());
  }

  return SerialStudio::resolveEscapeSequences(text).toUtf8();
}

/**
 * @brief Truncates a printable hex preview to 64 bytes so the tree stays readable.
 */
static QString hexPreview(const QByteArray& bytes, qsizetype maxBytes = 64)
{
  if (bytes.size() <= maxBytes)
    return QString::fromLatin1(bytes.toHex(' '));

  return QString::fromLatin1(bytes.left(maxBytes).toHex(' '))
       + QStringLiteral(" ... (+%1 bytes)").arg(bytes.size() - maxBytes);
}

/**
 * @brief Renders one parsed row (and its per-cell children) under @p parent.
 */
static void appendRowItem(QTreeWidgetItem* parent, int rowIndex, const QStringList& row)
{
  auto* rowItem = new QTreeWidgetItem(parent);
  rowItem->setText(0, QObject::tr("Row %1").arg(rowIndex + 1));
  rowItem->setText(1, row.join(QStringLiteral(", ")));

  for (int c = 0; c < row.size(); ++c) {
    auto* cellItem = new QTreeWidgetItem(rowItem);
    cellItem->setText(0, QObject::tr("[%1]").arg(c + 1));
    cellItem->setText(1, row[c]);
  }
}

/**
 * @brief Renders one extracted frame (raw + decoder + rows) as a top-level tree node.
 */
static void appendFrameItem(QTreeWidget* tree, int frameIndex, const DataModel::PipelineFrame& f)
{
  auto* frameItem = new QTreeWidgetItem(tree);
  frameItem->setText(0, QObject::tr("Frame %1").arg(frameIndex + 1));
  frameItem->setText(1, hexPreview(f.rawBytes));

  auto* decodedItem = new QTreeWidgetItem(frameItem);
  decodedItem->setText(0, QObject::tr("Decoder"));
  decodedItem->setText(1, f.decoderOutput);

  auto* rowsItem = new QTreeWidgetItem(frameItem);
  rowsItem->setText(0, QObject::tr("Rows"));
  rowsItem->setText(1, QObject::tr("%1 row(s)").arg(f.rows.size()));

  for (int r = 0; r < f.rows.size(); ++r)
    appendRowItem(rowsItem, r, f.rows[r]);

  frameItem->setExpanded(true);
  rowsItem->setExpanded(true);
}

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the singleton dialog -- it outlives the QML editor pane that triggers it.
 */
DataModel::FrameParserTestDialog& DataModel::FrameParserTestDialog::instance()
{
  static FrameParserTestDialog singleton;
  return singleton;
}

/**
 * @brief Builds the dialog, wires controls, and binds to ProjectModel for live source sync.
 */
DataModel::FrameParserTestDialog::FrameParserTestDialog()
  : QDialog(nullptr)
  , m_sourceId(0)
  , m_suspendSync(false)
  , m_parser(&DataModel::FrameParser::instance())
{
  resize(720, 560);
  setMinimumSize(720, 560);

  buildPipelineGroup();
  buildInputGroup();
  buildOutputGroup();

  // Section title font shared by all three titles
  auto titleFont = Misc::CommonFonts::instance().customUiFont(0.8, true);
  titleFont.setCapitalization(QFont::AllUppercase);
  m_pipelineTitle->setFont(titleFont);
  m_inputTitle->setFont(titleFont);
  m_outputTitle->setFont(titleFont);

  // Main layout
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(4);
  mainLayout->addWidget(m_pipelineTitle);
  mainLayout->addWidget(m_pipelineGroup);
  mainLayout->addSpacing(2);
  mainLayout->addWidget(m_inputTitle);
  mainLayout->addWidget(m_inputGroup);
  mainLayout->addSpacing(2);
  mainLayout->addWidget(m_outputTitle);
  mainLayout->addWidget(m_outputGroup);

  connectControls();

  onThemeChanged();
  onLanguageChanged();
  refreshPipelineControls();

  setWindowFlag(Qt::WindowStaysOnTopHint, true);
}

/**
 * @brief Builds the pipeline-config group (detection / start / end / decoder / checksum + reload).
 */
void DataModel::FrameParserTestDialog::buildPipelineGroup()
{
  m_pipelineTitle  = new QLabel(this);
  m_pipelineGroup  = new QGroupBox(this);
  m_detectionLabel = new QLabel(this);
  m_decoderLabel   = new QLabel(this);
  m_checksumLabel  = new QLabel(this);
  m_startLabel     = new QLabel(this);
  m_finishLabel    = new QLabel(this);
  m_detectionCombo = new QComboBox(this);
  m_decoderCombo   = new QComboBox(this);
  m_checksumCombo  = new QComboBox(this);
  m_startEdit      = new QLineEdit(this);
  m_finishEdit     = new QLineEdit(this);
  m_hexDelimiters  = new QCheckBox(this);

  m_detectionCombo->addItem(QString(), int(SerialStudio::EndDelimiterOnly));
  m_detectionCombo->addItem(QString(), int(SerialStudio::StartAndEndDelimiter));
  m_detectionCombo->addItem(QString(), int(SerialStudio::StartDelimiterOnly));
  m_detectionCombo->addItem(QString(), int(SerialStudio::NoDelimiters));

  m_decoderCombo->addItem(QString(), int(SerialStudio::PlainText));
  m_decoderCombo->addItem(QString(), int(SerialStudio::Hexadecimal));
  m_decoderCombo->addItem(QString(), int(SerialStudio::Base64));
  m_decoderCombo->addItem(QString(), int(SerialStudio::Binary));

  populateChecksumCombo();

  auto* pipelineForm = new QFormLayout(m_pipelineGroup);
  pipelineForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
  pipelineForm->setHorizontalSpacing(8);
  pipelineForm->setVerticalSpacing(4);

  // Start delimiter row: edit fills, 4 px gap, then the hex toggle hugs the right edge.
  auto* startRow = new QHBoxLayout;
  startRow->setContentsMargins(0, 0, 0, 0);
  startRow->setSpacing(4);
  startRow->addWidget(m_startEdit, 1);
  startRow->addWidget(m_hexDelimiters, 0);

  pipelineForm->addRow(m_detectionLabel, m_detectionCombo);
  pipelineForm->addRow(m_startLabel, startRow);
  pipelineForm->addRow(m_finishLabel, m_finishEdit);
  pipelineForm->addRow(m_decoderLabel, m_decoderCombo);
  pipelineForm->addRow(m_checksumLabel, m_checksumCombo);
}

/**
 * @brief Populates the checksum combo, mapping the empty algorithm name to a "None" label
 *        so the dropdown reads cleanly while the underlying value stays "" on the wire.
 */
void DataModel::FrameParserTestDialog::populateChecksumCombo()
{
  m_checksumCombo->clear();
  for (const auto& name : IO::availableChecksums()) {
    const auto label = name.isEmpty() ? tr("None") : name;
    m_checksumCombo->addItem(label, name);
  }
}

/**
 * @brief Builds the input row (text / hex toggle + clear / evaluate buttons).
 */
void DataModel::FrameParserTestDialog::buildInputGroup()
{
  m_inputTitle  = new QLabel(this);
  m_inputGroup  = new QGroupBox(this);
  m_userInput   = new QLineEdit(this);
  m_hexCheckBox = new QCheckBox(this);
  m_clearButton = new QPushButton(this);
  m_parseButton = new QPushButton(this);

  m_parseButton->setDefault(true);
  m_clearButton->setIcon(QIcon(":/icons/buttons/clear.svg"));
  m_parseButton->setIcon(QIcon(":/icons/buttons/media-play.svg"));

  auto* inputLayout = new QHBoxLayout(m_inputGroup);
  inputLayout->addWidget(m_userInput);
  inputLayout->addWidget(m_hexCheckBox);
  inputLayout->addWidget(m_clearButton);
  inputLayout->addWidget(m_parseButton);
}

/**
 * @brief Builds the output area (stats label + per-frame results tree).
 */
void DataModel::FrameParserTestDialog::buildOutputGroup()
{
  auto* commonFonts = &Misc::CommonFonts::instance();

  m_outputTitle = new QLabel(this);
  m_outputGroup = new QGroupBox(this);
  m_statsLabel  = new QLabel(this);
  m_results     = new QTreeWidget(this);

  m_results->setColumnCount(2);
  m_results->setUniformRowHeights(true);
  m_results->setAlternatingRowColors(true);
  m_results->setFont(commonFonts->monoFont());
  m_results->setRootIsDecorated(true);
  m_results->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_results->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_results->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_results->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_results->header()->setFont(commonFonts->boldUiFont());

  m_statsLabel->setFont(commonFonts->customUiFont(0.9, false));

  auto* outputLayout = new QVBoxLayout(m_outputGroup);
  outputLayout->setSpacing(4);
  outputLayout->addWidget(m_statsLabel);
  outputLayout->addWidget(m_results);
}

/**
 * @brief Wires all widget signals + the live ProjectModel/theme/language subscriptions.
 */
void DataModel::FrameParserTestDialog::connectControls()
{
  connect(m_parseButton, &QPushButton::clicked, this, &FrameParserTestDialog::parseData);
  connect(m_clearButton, &QPushButton::clicked, this, &FrameParserTestDialog::clear);
  connect(
    m_hexCheckBox, &QCheckBox::checkStateChanged, this, &FrameParserTestDialog::onInputModeChanged);
  connect(m_userInput, &QLineEdit::returnPressed, this, &FrameParserTestDialog::parseData);
  connect(m_userInput, &QLineEdit::textChanged, this, &FrameParserTestDialog::onInputDataChanged);

  connect(m_detectionCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &FrameParserTestDialog::onDetectionChanged);
  connect(m_decoderCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &FrameParserTestDialog::onDecoderChanged);
  connect(m_checksumCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
    onChecksumChanged(m_checksumCombo->currentData().toString());
  });
  connect(
    m_startEdit, &QLineEdit::editingFinished, this, &FrameParserTestDialog::onStartSequenceEdited);
  connect(m_finishEdit,
          &QLineEdit::editingFinished,
          this,
          &FrameParserTestDialog::onFinishSequenceEdited);
  connect(m_hexDelimiters,
          &QCheckBox::checkStateChanged,
          this,
          &FrameParserTestDialog::onHexDelimitersToggled);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceChanged,
          this,
          &FrameParserTestDialog::onSourceChanged);

  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &FrameParserTestDialog::onThemeChanged);
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &FrameParserTestDialog::onLanguageChanged);
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active source id and re-syncs pipeline controls from the matching source.
 */
void DataModel::FrameParserTestDialog::setSourceId(int sourceId)
{
  m_sourceId = sourceId;
  refreshPipelineControls();
}

//--------------------------------------------------------------------------------------------------
// Button actions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears the input field, results tree, and parser scratch context.
 */
void DataModel::FrameParserTestDialog::clear()
{
  m_userInput->clear();
  m_results->clear();
  m_statsLabel->clear();
  m_parser->clearContext();
}

/**
 * @brief Runs the full pipeline (extraction + decoder + parser) against the current input.
 */
void DataModel::FrameParserTestDialog::parseData()
{
  const auto text = m_userInput->text();
  if (text.isEmpty())
    return;

  if (m_hexCheckBox->isChecked() && !validateHexInput(text)) {
    QMessageBox::warning(this,
                         tr("Invalid Hex Input"),
                         tr("Please enter valid hexadecimal bytes.\n\nValid format: 01 A2 FF 3C"));
    return;
  }

  // Pick up uncommitted edits to shared tables before the pipeline runs.
  DataModel::FrameBuilder::instance().refreshTableStoreFromProjectModel();

  // Build pipeline spec from the (now project-backed) controls.
  PipelineSpec spec;
  spec.operationMode = SerialStudio::ProjectFile;
  spec.frameDetection =
    static_cast<SerialStudio::FrameDetection>(m_detectionCombo->currentData().toInt());
  spec.decoderMethod =
    static_cast<SerialStudio::DecoderMethod>(m_decoderCombo->currentData().toInt());
  spec.checksumAlgorithm = m_checksumCombo->currentData().toString();

  const bool hex   = m_hexDelimiters->isChecked();
  const auto start = delimiterFromField(m_startEdit->text(), hex);
  const auto end   = delimiterFromField(m_finishEdit->text(), hex);
  if (!start.isEmpty())
    spec.startSequences.append(start);

  if (!end.isEmpty())
    spec.finishSequences.append(end);

  const QByteArray input =
    m_hexCheckBox->isChecked() ? SerialStudio::hexToBytes(text) : text.toUtf8();

  const auto result = runFrameParserPipeline(input, spec, m_sourceId);

  // Populate results tree
  m_results->clear();
  if (result.extractedCount == 0) {
    auto* item = new QTreeWidgetItem(m_results);
    item->setText(0, tr("(no frames)"));
    item->setText(1,
                  tr("Extraction did not produce a complete frame. "
                     "Check the start / end delimiters and the detection mode."));
    item->setForeground(1, Qt::gray);
  } else {
    for (int i = 0; i < result.frames.size(); ++i)
      appendFrameItem(m_results, i, result.frames[i]);
  }

  m_statsLabel->setText(tr("%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | "
                           "%4 dropped")
                          .arg(result.extractedCount)
                          .arg(result.consumedBytes)
                          .arg(result.remainingBytes)
                          .arg(result.droppedFrames));

  m_userInput->clear();
  m_userInput->setFocus();
}

//--------------------------------------------------------------------------------------------------
// Pipeline-config slots (write through to ProjectModel)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Persists detection mode to the source so the live driver reconfigures.
 */
void DataModel::FrameParserTestDialog::onDetectionChanged(int index)
{
  if (m_suspendSync || index < 0)
    return;

  auto src           = sourceById(m_sourceId);
  src.frameDetection = m_detectionCombo->itemData(index).toInt();
  DataModel::ProjectModel::instance().updateSource(m_sourceId, src);
  updateStageVisibility();
}

/**
 * @brief Persists decoder choice; the live FrameBuilder picks it up via the same source field.
 */
void DataModel::FrameParserTestDialog::onDecoderChanged(int index)
{
  if (m_suspendSync || index < 0)
    return;

  auto src          = sourceById(m_sourceId);
  src.decoderMethod = m_decoderCombo->itemData(index).toInt();
  DataModel::ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Persists the checksum algorithm name; empty string disables checksum validation.
 */
void DataModel::FrameParserTestDialog::onChecksumChanged(const QString& text)
{
  if (m_suspendSync)
    return;

  auto src              = sourceById(m_sourceId);
  src.checksumAlgorithm = text;
  DataModel::ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Pushes the start delimiter (raw or hex) to the project source.
 */
void DataModel::FrameParserTestDialog::onStartSequenceEdited()
{
  if (m_suspendSync)
    return;

  applyDelimitersToProject();
}

/**
 * @brief Pushes the end delimiter (raw or hex) to the project source.
 */
void DataModel::FrameParserTestDialog::onFinishSequenceEdited()
{
  if (m_suspendSync)
    return;

  applyDelimitersToProject();
}

/**
 * @brief Hex-delimiter toggle: triggers a re-encode of both fields into the source.
 */
void DataModel::FrameParserTestDialog::onHexDelimitersToggled(Qt::CheckState)
{
  if (m_suspendSync)
    return;

  applyDelimitersToProject();
}

/**
 * @brief Re-pulls pipeline controls when ProjectModel reports the active source has changed.
 */
void DataModel::FrameParserTestDialog::onSourceChanged(int sourceId)
{
  if (sourceId != m_sourceId)
    return;

  refreshPipelineControls();
}

//--------------------------------------------------------------------------------------------------
// Pipeline-config private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads source[m_sourceId] and updates every pipeline control without re-emitting writes.
 */
void DataModel::FrameParserTestDialog::refreshPipelineControls()
{
  m_suspendSync = true;

  const auto src = sourceById(m_sourceId);

  const int detIdx = m_detectionCombo->findData(src.frameDetection);
  m_detectionCombo->setCurrentIndex(detIdx >= 0 ? detIdx : 0);

  const int decIdx = m_decoderCombo->findData(src.decoderMethod);
  m_decoderCombo->setCurrentIndex(decIdx >= 0 ? decIdx : 0);

  const int chkIdx = m_checksumCombo->findData(src.checksumAlgorithm);
  m_checksumCombo->setCurrentIndex(chkIdx >= 0 ? chkIdx : 0);

  m_hexDelimiters->setChecked(src.hexadecimalDelimiters);
  m_startEdit->setText(src.frameStart);
  m_finishEdit->setText(src.frameEnd);

  m_suspendSync = false;

  updateStageVisibility();
}

/**
 * @brief Pushes the current delimiter / hex-mode trio into the source's frameStart/frameEnd.
 */
void DataModel::FrameParserTestDialog::applyDelimitersToProject()
{
  auto src                  = sourceById(m_sourceId);
  src.frameStart            = m_startEdit->text();
  src.frameEnd              = m_finishEdit->text();
  src.hexadecimalDelimiters = m_hexDelimiters->isChecked();
  DataModel::ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Hides delimiter rows that the current detection mode doesn't consult.
 */
void DataModel::FrameParserTestDialog::updateStageVisibility()
{
  const auto mode =
    static_cast<SerialStudio::FrameDetection>(m_detectionCombo->currentData().toInt());

  const bool needsStart =
    (mode == SerialStudio::StartDelimiterOnly || mode == SerialStudio::StartAndEndDelimiter);
  const bool needsEnd =
    (mode == SerialStudio::EndDelimiterOnly || mode == SerialStudio::StartAndEndDelimiter);

  m_startLabel->setVisible(needsStart);
  m_startEdit->setVisible(needsStart);
  m_finishLabel->setVisible(needsEnd);
  m_finishEdit->setVisible(needsEnd);
  m_hexDelimiters->setVisible(needsStart || needsEnd);
}

//--------------------------------------------------------------------------------------------------
// Theme + language slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reapplies palette + group-box styling when the application theme changes.
 */
void DataModel::FrameParserTestDialog::onThemeChanged()
{
  setPalette(Misc::ThemeManager::instance().palette());
  onInputModeChanged(m_hexCheckBox->checkState());

  const auto* tm = &Misc::ThemeManager::instance();
  const auto groupBoxStyle =
    QStringLiteral(
      "QGroupBox {  border: 1px solid %1;  border-radius: 2px;  background-color: %2;}")
      .arg(tm->getColor("groupbox_border").name())
      .arg(tm->getColor("groupbox_background").name());

  m_pipelineGroup->setStyleSheet(groupBoxStyle);
  m_inputGroup->setStyleSheet(groupBoxStyle);
  m_outputGroup->setStyleSheet(groupBoxStyle);
}

/**
 * @brief Re-translates every label/button when the language changes.
 */
void DataModel::FrameParserTestDialog::onLanguageChanged()
{
  m_pipelineTitle->setText(tr("Pipeline Configuration"));
  m_inputTitle->setText(tr("Frame Data Input"));
  m_outputTitle->setText(tr("Pipeline Results"));

  m_detectionLabel->setText(tr("Detection"));
  m_decoderLabel->setText(tr("Decoder"));
  m_checksumLabel->setText(tr("Checksum"));
  m_startLabel->setText(tr("Start Delimiter"));
  m_finishLabel->setText(tr("End Delimiter"));
  m_hexDelimiters->setText(tr("Hex Delimiters"));

  m_detectionCombo->setItemText(0, tr("End delimiter only"));
  m_detectionCombo->setItemText(1, tr("Start + end delimiters"));
  m_detectionCombo->setItemText(2, tr("Start delimiter only"));
  m_detectionCombo->setItemText(3, tr("No delimiters (whole chunk)"));

  m_decoderCombo->setItemText(0, tr("Plain text (UTF-8)"));
  m_decoderCombo->setItemText(1, tr("Hexadecimal"));
  m_decoderCombo->setItemText(2, tr("Base64"));
  m_decoderCombo->setItemText(3, tr("Binary (raw bytes)"));

  m_hexCheckBox->setText(tr("HEX"));
  m_clearButton->setText(tr("Clear"));
  m_parseButton->setText(tr("Evaluate"));
  m_userInput->setPlaceholderText(tr("Enter raw stream bytes here..."));
  m_results->setHeaderLabels({tr("Stage"), tr("Value")});

  onInputModeChanged(m_hexCheckBox->checkState());

  if (m_results->topLevelItemCount() > 0)
    parseData();

  setWindowTitle(tr("Test Frame Parser"));
}

//--------------------------------------------------------------------------------------------------
// Input-mode + validation slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates placeholder text and formatting when HEX input mode toggles.
 */
void DataModel::FrameParserTestDialog::onInputModeChanged(Qt::CheckState state)
{
  if (state == Qt::Checked) {
    m_userInput->setPlaceholderText(tr("Enter hex bytes (e.g., 01 A2 FF)"));
    if (!m_userInput->text().isEmpty()) {
      const auto formatted = formatHexInput(m_userInput->text());
      m_userInput->setText(formatted);
    }
  } else {
    m_userInput->setPlaceholderText(tr("Enter raw stream bytes here..."));
    m_userInput->setPalette(QPalette());
  }
}

/**
 * @brief Formats and validates the input as space-separated hex pairs when HEX mode is on.
 */
void DataModel::FrameParserTestDialog::onInputDataChanged(const QString& t)
{
  if (m_hexCheckBox->isChecked()) {
    m_userInput->blockSignals(true);
    const auto fmt     = formatHexInput(t);
    const auto isValid = validateHexInput(fmt);
    if (t != fmt) {
      const int pos  = m_userInput->cursorPosition();
      const int diff = fmt.left(pos).count(' ') - t.left(pos).count(' ');

      m_userInput->setText(fmt);
      m_userInput->setCursorPosition(pos + diff);
    }

    auto palette = m_userInput->palette();
    if (isValid || fmt.isEmpty())
      palette.setColor(QPalette::Text, Qt::black);
    else
      palette.setColor(QPalette::Text, Qt::red);

    m_userInput->setPalette(palette);
    m_userInput->blockSignals(false);
  } else {
    m_userInput->setPalette(QPalette());
  }
}

//--------------------------------------------------------------------------------------------------
// HEX string validation + formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats the input into uppercase space-separated hex byte pairs.
 */
QString DataModel::FrameParserTestDialog::formatHexInput(const QString& text)
{
  QString cleaned;
  for (const QChar& c : text)
    if (c.isLetterOrNumber())
      cleaned.append(c.toUpper());

  QString formatted;
  for (int i = 0; i < cleaned.length(); ++i) {
    formatted.append(cleaned[i]);
    if (i % 2 == 1 && i < cleaned.length() - 1)
      formatted.append(' ');
  }

  return formatted;
}

/**
 * @brief Returns true if @p text is empty or forms valid complete hex bytes.
 */
bool DataModel::FrameParserTestDialog::validateHexInput(const QString& text)
{
  if (text.isEmpty())
    return true;

  QString cleaned = text;
  cleaned.remove(' ');

  for (const QChar& c : cleaned)
    if (!c.isDigit() && (c.toUpper() < 'A' || c.toUpper() > 'F'))
      return false;

  return cleaned.length() % 2 == 0;
}
