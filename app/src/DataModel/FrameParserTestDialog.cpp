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

#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

#include "SerialStudio.h"
#include "Misc/Translator.h"
#include "Misc/CommonFonts.h"
#include "DataModel/FrameParser.h"
#include "Misc/ThemeManager.h"
#include "DataModel/FrameParserTestDialog.h"

//------------------------------------------------------------------------------
// Constructor function
//------------------------------------------------------------------------------

/**
 * @brief Constructs the frame parser test dialog
 *
 * @param parser Pointer to the FrameParser instance to test
 * @param parent Parent widget
 */
DataModel::FrameParserTestDialog::FrameParserTestDialog(FrameParser *parser,
                                                        QWidget *parent)
  : QDialog(parent)
  , m_parser(parser)
{
  // Set window geometry and title
  resize(640, 480);
  setMinimumSize(640, 480);

  // Get pointer to fonts module
  auto *commonFonts = &Misc::CommonFonts::instance();

  // Initialize widgets
  m_inputTitle = new QLabel(this);
  m_outputTitle = new QLabel(this);
  m_table = new QTableWidget(this);
  m_userInput = new QLineEdit(this);
  m_inputGroup = new QGroupBox(this);
  m_outputGroup = new QGroupBox(this);
  m_hexCheckBox = new QCheckBox(this);
  m_clearButton = new QPushButton(this);
  m_parseButton = new QPushButton(this);
  m_inputDisplay = new QPlainTextEdit(this);

  // Create layout objects
  auto *mainLayout = new QVBoxLayout(this);
  auto *inputLayout = new QHBoxLayout(m_inputGroup);
  auto *outputLayout = new QVBoxLayout(m_outputGroup);

  // Configure buttons
  m_parseButton->setDefault(true);
  m_clearButton->setIcon(QIcon(":/rcc/icons/buttons/clear.svg"));
  m_parseButton->setIcon(QIcon(":/rcc/icons/buttons/media-play.svg"));

  // Configure table widget
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

  // Configure input text edit
  m_inputDisplay->setReadOnly(true);

  // Configure titles
  auto titleFont = commonFonts->customUiFont(0.8, true);
  titleFont.setCapitalization(QFont::AllUppercase);
  m_inputTitle->setFont(titleFont);
  m_outputTitle->setFont(titleFont);

  // Configure layouts
  inputLayout->addWidget(m_userInput);
  inputLayout->addWidget(m_hexCheckBox);
  inputLayout->addWidget(m_clearButton);
  inputLayout->addWidget(m_parseButton);
  outputLayout->addWidget(m_inputDisplay);
  outputLayout->addWidget(m_table);

  // Set layout stretch factors
  outputLayout->setStretch(0, 1);
  outputLayout->setStretch(1, 2);

  // Set main layout
  mainLayout->setSpacing(4);
  mainLayout->addWidget(m_inputTitle);
  mainLayout->addWidget(m_inputGroup);
  mainLayout->addSpacing(4);
  mainLayout->addWidget(m_outputTitle);
  mainLayout->addWidget(m_outputGroup);

  // Connect signals
  connect(m_parseButton, &QPushButton::clicked, this,
          &FrameParserTestDialog::parseData);
  connect(m_clearButton, &QPushButton::clicked, this,
          &FrameParserTestDialog::clear);
  connect(m_hexCheckBox, &QCheckBox::checkStateChanged, this,
          &FrameParserTestDialog::onInputModeChanged);
  connect(m_userInput, &QLineEdit::returnPressed, this,
          &FrameParserTestDialog::parseData);

  // Add hex formatting and validation
  connect(m_userInput, &QLineEdit::textChanged, this,
          &FrameParserTestDialog::onInputDataChanged);

  // Singleton module connections
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &FrameParserTestDialog::onThemeChanged);
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &FrameParserTestDialog::onLanguageChanged);

  // Load theme & translations
  onThemeChanged();
  onLanguageChanged();

  // Make window stay on top
  setWindowFlag(Qt::WindowStaysOnTopHint, true);
}

//------------------------------------------------------------------------------
// Button actions
//------------------------------------------------------------------------------

/**
 * @brief Clears all results from the table
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
 * @brief Handles the Parse button click
 *
 * Executes the parser function with the input data and displays results
 */
void DataModel::FrameParserTestDialog::parseData()
{
  const auto input = m_userInput->text();
  if (input.isEmpty())
    return;

  if (m_hexCheckBox->isChecked() && !validateHexInput(input))
  {
    QMessageBox::warning(this, tr("Invalid Hex Input"),
                         tr("Please enter valid hexadecimal bytes.\n\n"
                            "Valid format: 01 A2 FF 3C"));
    return;
  }

  QStringList result;
  if (m_hexCheckBox->isChecked())
    result = m_parser->parse(SerialStudio::hexToBytes(input));
  else
    result = m_parser->parse(input);

  displayOutput(input, result);
  m_userInput->clear();
  m_userInput->setFocus();
}

//------------------------------------------------------------------------------
// Singleton module slot functions
//------------------------------------------------------------------------------

/**
 * @brief Updates the dialog palette when the application theme changes.
 */
void DataModel::FrameParserTestDialog::onThemeChanged()
{
  // Load theme colors
  setPalette(Misc::ThemeManager::instance().palette());
  onInputModeChanged(m_hexCheckBox->checkState());

  // Define QSS for groupboxes
  const auto *tm = &Misc::ThemeManager::instance();
  const auto groupBoxStyle
      = QStringLiteral("QGroupBox {"
                       "  border: 1px solid %1;"
                       "  border-radius: 2px;"
                       "  background-color: %2;"
                       "}")
            .arg(tm->getColor("groupbox_border").name())
            .arg(tm->getColor("groupbox_background").name());

  // Set groupbox style
  m_inputGroup->setStyleSheet(groupBoxStyle);
  m_outputGroup->setStyleSheet(groupBoxStyle);
}

/**
 * @brief Retranslates all UI text when the application language changes.
 *
 * Updates all labels, tooltips, placeholders, and re-parses existing data
 * to reflect the new language.
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
  m_inputDisplay->setPlaceholderText(
      tr("Enter frame data above, enable HEX mode if needed, then click "
         "\"Evaluate\" to run the frame parser.\n\n"
         "Example (Text): a,b,c,d,e,f\n"
         "Example (HEX):  48 65 6C 6C 6F"));

  onInputModeChanged(m_hexCheckBox->checkState());
  if (m_table->rowCount() > 0)
    parseData();

  setWindowTitle(tr("Test Frame Parser"));
}

//------------------------------------------------------------------------------
// Widget slot functions
//------------------------------------------------------------------------------

/**
 * @brief Handles hex mode checkbox state changes, updating placeholder text
 *        and formatting.
 *
 * When hex mode is enabled, sets hex-specific placeholder text and formats
 * existing input as hex. When disabled, restores default placeholder and
 * palette.
 *
 * @param state The new checkbox state (Qt::Checked or Qt::Unchecked)
 */
void DataModel::FrameParserTestDialog::onInputModeChanged(Qt::CheckState state)
{
  if (state == Qt::Checked)
  {
    m_userInput->setPlaceholderText(tr("Enter hex bytes (e.g., 01 A2 FF)"));
    if (!m_userInput->text().isEmpty())
    {
      QString formatted = formatHexInput(m_userInput->text());
      m_userInput->setText(formatted);
    }
  }

  else
  {
    m_userInput->setPlaceholderText(tr("Enter frame data here..."));
    m_userInput->setPalette(QPalette());
  }
}

/**
 * @brief Handles input data changes, and formats & validates input in hex mode.
 *
 * In hex mode, automatically formats hex input with spaces, validates the data,
 * and highlights invalid input in red.  Maintains cursor position during
 * formatting.
 *
 * In text mode, it uses default palette always.
 *
 * @param t The new input text from the line edit
 */
void DataModel::FrameParserTestDialog::onInputDataChanged(const QString &t)
{
  // Automatically add spaces & highlight invalid hex data
  if (m_hexCheckBox->isChecked())
  {
    // Block signals to prevent recursive calls
    m_userInput->blockSignals(true);

    // Update text if formatting changed
    const auto fmt = formatHexInput(t);
    const auto isValid = validateHexInput(fmt);
    if (t != fmt)
    {
      const int pos = m_userInput->cursorPosition();
      const int diff = fmt.left(pos).count(' ') - t.left(pos).count(' ');

      m_userInput->setText(fmt);
      m_userInput->setCursorPosition(pos + diff);
    }

    // Update text color based on validity
    auto palette = m_userInput->palette();
    if (isValid || fmt.isEmpty())
      palette.setColor(QPalette::Text, Qt::black);
    else
      palette.setColor(QPalette::Text, Qt::red);

    m_userInput->setPalette(palette);
    m_userInput->blockSignals(false);
  }

  // Reset to default palette in text mode
  else
    m_userInput->setPalette(QPalette());
}

//------------------------------------------------------------------------------
// HEX string validation & formatting functions
//------------------------------------------------------------------------------

/**
 * @brief Formats user input text into properly spaced hexadecimal byte pairs
 *
 * Converts raw text input into formatted hexadecimal string with spaces
 * separating each byte pair. Non-hexadecimal characters are removed, and
 * letters are converted to uppercase.
 *
 * The formatting process:
 * 1. Removes all non-alphanumeric characters (including existing spaces)
 * 2. Converts all letters to uppercase
 * 3. Inserts a space after every 2 characters (one byte)
 *
 * Examples:
 *   - "01a2ff" → "01 A2 FF"
 *   - "1 2 3 4" → "12 34"
 *   - "abcdef123" → "AB CD EF 12 3"
 *
 * @param text Raw input text from the user
 * @return Formatted hexadecimal string with space-separated byte pairs
 *
 * @note Incomplete byte pairs (odd number of characters) are left without
 *       a trailing space
 *
 * @see validateHexInput()
 */
QString DataModel::FrameParserTestDialog::formatHexInput(const QString &text)
{
  QString cleaned;
  for (const QChar &c : text)
  {
    if (c.isLetterOrNumber())
      cleaned.append(c.toUpper());
  }

  QString formatted;
  for (int i = 0; i < cleaned.length(); ++i)
  {
    formatted.append(cleaned[i]);
    if (i % 2 == 1 && i < cleaned.length() - 1)
      formatted.append(' ');
  }

  return formatted;
}

/**
 * @brief Validates that input text contains valid hexadecimal byte data
 *
 * Checks if the input string is valid hexadecimal data suitable for
 * parsing. Validation ensures:
 * 1. All characters (excluding spaces) are valid hex digits (0-9, A-F, a-f)
 * 2. The total number of hex digits forms complete byte pairs (even count)
 *
 * Empty strings are considered valid to allow progressive input.
 *
 * Examples:
 *   - "01 A2 FF" → true (3 complete bytes)
 *   - "1A2B" → true (2 complete bytes)
 *   - "01 A" → false (incomplete byte)
 *   - "01 GH" → false (invalid hex character 'G')
 *   - "" → true (empty input allowed)
 *
 * @param text Input text to validate
 * @return true if the text is valid hexadecimal with complete byte pairs,
 *         false otherwise
 *
 * @note This function is case-insensitive for hex letters (A-F, a-f)
 *
 * @see formatHexInput()
 */
bool DataModel::FrameParserTestDialog::validateHexInput(const QString &text)
{
  if (text.isEmpty())
    return true;

  QString cleaned = text;
  cleaned.remove(' ');

  for (const QChar &c : cleaned)
  {
    if (!c.isDigit() && (c.toUpper() < 'A' || c.toUpper() > 'F'))
      return false;
  }

  return cleaned.length() % 2 == 0;
}

//------------------------------------------------------------------------------
// Script output display function
//------------------------------------------------------------------------------

/**
 * @brief Adds a result row to the results table
 *
 * @param input The input frame data
 * @param output The parsed output array
 */
void DataModel::FrameParserTestDialog::displayOutput(const QString &input,
                                                     const QStringList &output)
{
  m_table->setRowCount(0);
  m_inputDisplay->setPlainText(input);
  if (input.isEmpty())
    m_inputDisplay->setFont(Misc::CommonFonts::instance().uiFont());
  else
    m_inputDisplay->setFont(Misc::CommonFonts::instance().monoFont());

  if (output.isEmpty())
  {
    m_table->insertRow(0);
    m_table->setItem(0, 0, new QTableWidgetItem(tr("(empty)")));
    m_table->setItem(0, 1, new QTableWidgetItem(tr("No data returned")));
    m_table->item(0, 1)->setForeground(Qt::gray);
  }

  else
  {
    for (int i = 0; i < output.size(); ++i)
    {
      int row = m_table->rowCount();
      m_table->insertRow(row);
      if (i == 0)
      {
        auto *inputItem = new QTableWidgetItem(input);
        inputItem->setData(Qt::UserRole, output.size());
        m_table->setItem(row, 0, inputItem);
      }

      m_table->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
      m_table->setItem(row, 1, new QTableWidgetItem(output[i]));
    }
  }

  m_table->scrollToBottom();
}
