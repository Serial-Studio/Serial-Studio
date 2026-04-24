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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/FrameParserTestDialog.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor function
//--------------------------------------------------------------------------------------------------

DataModel::FrameParserTestDialog::FrameParserTestDialog(FrameParser* parser, QWidget* parent)
  : QDialog(parent), m_sourceId(0), m_parser(parser)
{
  // Configure window dimensions
  resize(640, 480);
  setMinimumSize(640, 480);

  auto* commonFonts = &Misc::CommonFonts::instance();

  // Create widgets
  m_inputTitle   = new QLabel(this);
  m_outputTitle  = new QLabel(this);
  m_table        = new QTableWidget(this);
  m_userInput    = new QLineEdit(this);
  m_inputGroup   = new QGroupBox(this);
  m_outputGroup  = new QGroupBox(this);
  m_hexCheckBox  = new QCheckBox(this);
  m_clearButton  = new QPushButton(this);
  m_parseButton  = new QPushButton(this);
  m_inputDisplay = new QPlainTextEdit(this);

  // Create layouts
  auto* mainLayout   = new QVBoxLayout(this);
  auto* inputLayout  = new QHBoxLayout(m_inputGroup);
  auto* outputLayout = new QVBoxLayout(m_outputGroup);

  // Configure buttons and table
  m_parseButton->setDefault(true);
  m_clearButton->setIcon(QIcon(":/rcc/icons/buttons/clear.svg"));
  m_parseButton->setIcon(QIcon(":/rcc/icons/buttons/media-play.svg"));

  m_table->setColumnCount(2);
  m_table->verticalHeader()->hide();
  m_table->setAlternatingRowColors(true);
  m_table->setFont(commonFonts->monoFont());
  m_table->horizontalHeader()->setDefaultSectionSize(156);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->horizontalHeader()->setFont(commonFonts->boldUiFont());
  m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_inputDisplay->setReadOnly(true);

  // Configure title fonts
  auto titleFont = commonFonts->customUiFont(0.8, true);
  titleFont.setCapitalization(QFont::AllUppercase);
  m_inputTitle->setFont(titleFont);
  m_outputTitle->setFont(titleFont);

  // Assemble layouts
  inputLayout->addWidget(m_userInput);
  inputLayout->addWidget(m_hexCheckBox);
  inputLayout->addWidget(m_clearButton);
  inputLayout->addWidget(m_parseButton);
  outputLayout->addWidget(m_inputDisplay);
  outputLayout->addWidget(m_table);

  outputLayout->setStretch(0, 1);
  outputLayout->setStretch(1, 2);

  // Build main layout
  mainLayout->setSpacing(4);
  mainLayout->addWidget(m_inputTitle);
  mainLayout->addWidget(m_inputGroup);
  mainLayout->addSpacing(4);
  mainLayout->addWidget(m_outputTitle);
  mainLayout->addWidget(m_outputGroup);

  // Connect widget signals
  connect(m_parseButton, &QPushButton::clicked, this, &FrameParserTestDialog::parseData);
  connect(m_clearButton, &QPushButton::clicked, this, &FrameParserTestDialog::clear);
  connect(
    m_hexCheckBox, &QCheckBox::checkStateChanged, this, &FrameParserTestDialog::onInputModeChanged);
  connect(m_userInput, &QLineEdit::returnPressed, this, &FrameParserTestDialog::parseData);
  connect(m_userInput, &QLineEdit::textChanged, this, &FrameParserTestDialog::onInputDataChanged);

  // Connect theme and language change signals
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &FrameParserTestDialog::onThemeChanged);
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &FrameParserTestDialog::onLanguageChanged);

  // Apply initial theme and translations
  onThemeChanged();
  onLanguageChanged();

  // Make window stay on top
  setWindowFlag(Qt::WindowStaysOnTopHint, true);
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the source ID used when executing the parser.
 */
void DataModel::FrameParserTestDialog::setSourceId(int sourceId)
{
  m_sourceId = sourceId;
}

//--------------------------------------------------------------------------------------------------
// Button actions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears the input, output table, and parser execution context.
 */
void DataModel::FrameParserTestDialog::clear()
{
  m_userInput->clear();
  m_table->setRowCount(0);
  m_inputDisplay->clear();
  m_parser->clearContext();
  m_inputDisplay->setFont(Misc::CommonFonts::instance().uiFont());
}

/**
 * @brief Runs the parser on the current input and displays the results.
 */
void DataModel::FrameParserTestDialog::parseData()
{
  const auto input = m_userInput->text();
  if (input.isEmpty())
    return;

  if (m_hexCheckBox->isChecked() && !validateHexInput(input)) {
    QMessageBox::warning(this,
                         tr("Invalid Hex Input"),
                         tr("Please enter valid hexadecimal bytes.\n\nValid format: 01 A2 FF 3C"));
    return;
  }

  // Pick up uncommitted edits to shared tables
  DataModel::FrameBuilder::instance().refreshTableStoreFromProjectModel();

  QStringList result;
  if (m_hexCheckBox->isChecked()) {
    const auto frames = m_parser->parseMultiFrame(SerialStudio::hexToBytes(input), m_sourceId);
    if (!frames.isEmpty())
      result = frames.first();
  } else {
    const auto frames = m_parser->parseMultiFrame(input, m_sourceId);
    if (!frames.isEmpty())
      result = frames.first();
  }

  displayOutput(input, result);
  m_userInput->clear();
  m_userInput->setFocus();
}

//--------------------------------------------------------------------------------------------------
// Singleton module slot functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the dialog palette when the application theme changes.
 */
void DataModel::FrameParserTestDialog::onThemeChanged()
{
  // Apply palette and sync input mode
  setPalette(Misc::ThemeManager::instance().palette());
  onInputModeChanged(m_hexCheckBox->checkState());

  // Style groupboxes
  const auto* tm = &Misc::ThemeManager::instance();
  const auto groupBoxStyle =
    QStringLiteral(
      "QGroupBox {  border: 1px solid %1;  border-radius: 2px;  background-color: %2;}")
      .arg(tm->getColor("groupbox_border").name())
      .arg(tm->getColor("groupbox_background").name());
  m_inputGroup->setStyleSheet(groupBoxStyle);
  m_outputGroup->setStyleSheet(groupBoxStyle);
}

/**
 * @brief Retranslates all UI text when the application language changes.
 */
void DataModel::FrameParserTestDialog::onLanguageChanged()
{
  m_hexCheckBox->setText(tr("HEX"));
  m_clearButton->setText(tr("Clear"));
  m_parseButton->setText(tr("Evaluate"));
  m_inputTitle->setText(tr("Frame Data Input"));
  m_outputTitle->setText(tr("Frame Parser Results"));
  m_userInput->setPlaceholderText(tr("Enter frame data here..."));
  m_table->setHorizontalHeaderLabels({tr("Dataset Index"), tr("Value")});
  m_inputDisplay->setPlaceholderText(tr(
    "Enter frame data above, enable HEX mode if needed, then click \"Evaluate\" to run the frame parser.\n\nExample (Text): a,b,c,d,e,f\nExample (HEX):  48 65 6C 6C 6F"));

  onInputModeChanged(m_hexCheckBox->checkState());
  if (m_table->rowCount() > 0)
    parseData();

  setWindowTitle(tr("Test Frame Parser"));
}

//--------------------------------------------------------------------------------------------------
// Widget slot functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates placeholder text and formatting when hex mode toggles.
 */
void DataModel::FrameParserTestDialog::onInputModeChanged(Qt::CheckState state)
{
  if (state == Qt::Checked) {
    m_userInput->setPlaceholderText(tr("Enter hex bytes (e.g., 01 A2 FF)"));
    if (!m_userInput->text().isEmpty()) {
      QString formatted = formatHexInput(m_userInput->text());
      m_userInput->setText(formatted);
    }
  }

  else {
    m_userInput->setPlaceholderText(tr("Enter frame data here..."));
    m_userInput->setPalette(QPalette());
  }
}

/**
 * @brief Formats and validates the input when hex mode is active.
 */
void DataModel::FrameParserTestDialog::onInputDataChanged(const QString& t)
{
  // Format and validate hex input
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

    // Highlight invalid hex in red
    auto palette = m_userInput->palette();
    if (isValid || fmt.isEmpty())
      palette.setColor(QPalette::Text, Qt::black);
    else
      palette.setColor(QPalette::Text, Qt::red);

    m_userInput->setPalette(palette);
    m_userInput->blockSignals(false);
  }

  else
    m_userInput->setPalette(QPalette());
}

//--------------------------------------------------------------------------------------------------
// HEX string validation & formatting functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats the input into uppercase space-separated hex byte pairs.
 */
QString DataModel::FrameParserTestDialog::formatHexInput(const QString& text)
{
  // Strip non-hex characters and uppercase
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
 * @brief Returns true if the text is empty or forms valid complete hex bytes.
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

//--------------------------------------------------------------------------------------------------
// Script output display function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates the results table with the parsed frame output.
 */
void DataModel::FrameParserTestDialog::displayOutput(const QString& input,
                                                     const QStringList& output)
{
  // Clear previous results
  m_table->setRowCount(0);
  m_inputDisplay->setPlainText(input);
  if (input.isEmpty())
    m_inputDisplay->setFont(Misc::CommonFonts::instance().uiFont());
  else
    m_inputDisplay->setFont(Misc::CommonFonts::instance().monoFont());

  if (output.isEmpty()) {
    m_table->insertRow(0);
    m_table->setItem(0, 0, new QTableWidgetItem(tr("(empty)")));
    m_table->setItem(0, 1, new QTableWidgetItem(tr("No data returned")));
    m_table->item(0, 1)->setForeground(Qt::gray);
  }

  else {
    for (int i = 0; i < output.size(); ++i) {
      int row = m_table->rowCount();
      m_table->insertRow(row);
      if (i == 0) {
        auto* inputItem = new QTableWidgetItem(input);
        inputItem->setData(Qt::UserRole, output.size());
        m_table->setItem(row, 0, inputItem);
      }

      m_table->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
      m_table->setItem(row, 1, new QTableWidgetItem(output[i]));
    }
  }

  m_table->scrollToBottom();
}
