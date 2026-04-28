/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/TransmitTestDialog.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QJSEngine>
#include <QMessageBox>

#include "DataModel/FrameBuilder.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"
#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/Output/Base.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the transmit function test dialog.
 *
 * @param parent Parent widget.
 */
DataModel::TransmitTestDialog::TransmitTestDialog(QWidget* parent) : QDialog(parent)
{
  resize(640, 480);
  setMinimumSize(640, 480);

  auto* commonFonts = &Misc::CommonFonts::instance();

  // Initialize widgets
  m_inputTitle     = new QLabel(this);
  m_outputTitle    = new QLabel(this);
  m_byteCountLabel = new QLabel(this);
  m_userInput      = new QLineEdit(this);
  m_inputGroup     = new QGroupBox(this);
  m_outputGroup    = new QGroupBox(this);
  m_hexCheckBox    = new QCheckBox(this);
  m_clearButton    = new QPushButton(this);
  m_evaluateButton = new QPushButton(this);
  m_rawOutput      = new QPlainTextEdit(this);
  m_hexOutput      = new QPlainTextEdit(this);

  auto* mainLayout   = new QVBoxLayout(this);
  auto* inputLayout  = new QHBoxLayout(m_inputGroup);
  auto* outputLayout = new QVBoxLayout(m_outputGroup);

  m_evaluateButton->setDefault(true);
  m_clearButton->setIcon(QIcon(":/rcc/icons/buttons/clear.svg"));
  m_evaluateButton->setIcon(QIcon(":/rcc/icons/buttons/media-play.svg"));

  m_rawOutput->setReadOnly(true);
  m_hexOutput->setReadOnly(true);
  m_rawOutput->setFont(commonFonts->monoFont());
  m_hexOutput->setFont(commonFonts->monoFont());

  m_byteCountLabel->setFont(commonFonts->boldUiFont());
  m_byteCountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  auto titleFont = commonFonts->customUiFont(0.8, true);
  titleFont.setCapitalization(QFont::AllUppercase);
  m_inputTitle->setFont(titleFont);
  m_outputTitle->setFont(titleFont);

  // Populate layouts
  inputLayout->addWidget(m_userInput);
  inputLayout->addWidget(m_hexCheckBox);
  inputLayout->addWidget(m_clearButton);
  inputLayout->addWidget(m_evaluateButton);
  outputLayout->addWidget(m_rawOutput);
  outputLayout->addWidget(m_hexOutput);
  outputLayout->addWidget(m_byteCountLabel);

  outputLayout->setStretch(0, 1);
  outputLayout->setStretch(1, 1);
  outputLayout->setStretch(2, 0);

  mainLayout->setSpacing(4);
  mainLayout->addWidget(m_inputTitle);
  mainLayout->addWidget(m_inputGroup);
  mainLayout->addSpacing(4);
  mainLayout->addWidget(m_outputTitle);
  mainLayout->addWidget(m_outputGroup);

  // Connect widget signals
  connect(m_evaluateButton, &QPushButton::clicked, this, &TransmitTestDialog::evaluate);
  connect(m_clearButton, &QPushButton::clicked, this, &TransmitTestDialog::clear);
  connect(
    m_hexCheckBox, &QCheckBox::checkStateChanged, this, &TransmitTestDialog::onInputModeChanged);
  connect(m_userInput, &QLineEdit::returnPressed, this, &TransmitTestDialog::evaluate);
  connect(m_userInput, &QLineEdit::textChanged, this, &TransmitTestDialog::onInputDataChanged);

  // React to theme and language changes
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &TransmitTestDialog::onThemeChanged);
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &TransmitTestDialog::onLanguageChanged);

  onThemeChanged();
  onLanguageChanged();

  setWindowFlag(Qt::WindowStaysOnTopHint, true);
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the transmit function code to evaluate.
 * @param code JavaScript source containing a transmit(value) function.
 */
void DataModel::TransmitTestDialog::setTransmitCode(const QString& code)
{
  m_transmitCode = code;
}

//--------------------------------------------------------------------------------------------------
// Button actions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears all input and output fields.
 */
void DataModel::TransmitTestDialog::clear()
{
  m_userInput->clear();
  m_rawOutput->clear();
  m_hexOutput->clear();
  m_byteCountLabel->clear();
}

/**
 * @brief Compiles and runs the transmit function with the user's input value.
 *
 * Creates a temporary JS engine, installs protocol helpers, wraps and compiles
 * the transmit code, then calls it with the input value. Auto-detects whether
 * the input is a number or a string.
 */
void DataModel::TransmitTestDialog::evaluate()
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

  // No transmit code to evaluate
  if (m_transmitCode.trimmed().isEmpty()) {
    displayOutput({}, tr("No transmit function code to evaluate."));
    return;
  }

  // Pick up uncommitted edits to shared tables
  DataModel::FrameBuilder::instance().refreshTableStoreFromProjectModel();

  // Temporary JS engine: protocol helpers + shared-memory table API
  QJSEngine engine;
#ifdef BUILD_COMMERCIAL
  Widgets::Output::Base::installProtocolHelpers(engine);
#endif
  DataModel::FrameBuilder::instance().injectTableApiJS(&engine);

  // Compile and validate the transmit function
  const auto wrapped =
    QStringLiteral("(function() { %1; return transmit; })()").arg(m_transmitCode);
  auto transmitFn = engine.evaluate(wrapped);
  if (!transmitFn.isCallable()) {
    const auto msg =
      transmitFn.isError() ? transmitFn.toString() : tr("transmit function is not callable");
    displayOutput({}, msg);
    return;
  }

  // Convert user input to a JS value (hex bytes or auto-detect number/string)
  QJSValue jsValue;
  if (m_hexCheckBox->isChecked()) {
    const auto bytes = SerialStudio::hexToBytes(input);
    jsValue          = engine.toScriptValue(QString::fromLatin1(bytes));
  } else {
    bool ok;
    double num = input.toDouble(&ok);
    if (ok)
      jsValue = engine.toScriptValue(num);
    else
      jsValue = engine.toScriptValue(input);
  }

  auto result = transmitFn.call(QJSValueList{jsValue});
  if (result.isError()) {
    displayOutput({}, result.toString());
    return;
  }

  // Convert JS result to byte array
  QByteArray payload;
  if (result.isString())
    payload = result.toString().toLatin1();
  else
    payload = result.toVariant().toByteArray();

  displayOutput(payload, {});
}

//--------------------------------------------------------------------------------------------------
// Singleton module slot functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the dialog palette when the application theme changes.
 */
void DataModel::TransmitTestDialog::onThemeChanged()
{
  setPalette(Misc::ThemeManager::instance().palette());
  onInputModeChanged(m_hexCheckBox->checkState());

  // Style groupboxes with theme colors
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
void DataModel::TransmitTestDialog::onLanguageChanged()
{
  m_hexCheckBox->setText(tr("HEX"));
  m_clearButton->setText(tr("Clear"));
  m_evaluateButton->setText(tr("Evaluate"));
  m_inputTitle->setText(tr("Input Value"));
  m_outputTitle->setText(tr("Transmit Function Output"));
  m_userInput->setPlaceholderText(tr("Enter value to transmit…"));
  m_rawOutput->setPlaceholderText(tr("Raw string output appears here"));
  m_hexOutput->setPlaceholderText(tr("Hex byte output appears here"));

  onInputModeChanged(m_hexCheckBox->checkState());
  setWindowTitle(tr("Test Transmit Function"));
}

//--------------------------------------------------------------------------------------------------
// Widget slot functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles hex mode checkbox state changes.
 *
 * @param state The new checkbox state.
 */
void DataModel::TransmitTestDialog::onInputModeChanged(Qt::CheckState state)
{
  if (state == Qt::Checked) {
    m_userInput->setPlaceholderText(tr("Enter hex bytes (e.g., 01 A2 FF)"));
    if (!m_userInput->text().isEmpty()) {
      QString formatted = formatHexInput(m_userInput->text());
      m_userInput->setText(formatted);
    }
  } else {
    m_userInput->setPlaceholderText(tr("Enter value to transmit…"));
    m_userInput->setPalette(QPalette());
  }
}

/**
 * @brief Handles input data changes, formats and validates hex input.
 *
 * @param t The new input text.
 */
void DataModel::TransmitTestDialog::onInputDataChanged(const QString& t)
{
  // Auto-format and validate hex input
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
  }

  // Reset to default palette in text mode
  else
    m_userInput->setPalette(QPalette());
}

//--------------------------------------------------------------------------------------------------
// HEX string validation & formatting functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats input text into space-separated hexadecimal byte pairs.
 *
 * @param text Raw input text.
 * @return Formatted hex string (e.g., "01 A2 FF").
 */
QString DataModel::TransmitTestDialog::formatHexInput(const QString& text)
{
  // Strip non-hex characters and normalize to uppercase
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
 * @brief Validates that input contains valid hexadecimal byte pairs.
 *
 * @param text Input text to validate.
 * @return @c true if the text is valid hex with complete byte pairs.
 */
bool DataModel::TransmitTestDialog::validateHexInput(const QString& text)
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
// Output display
//--------------------------------------------------------------------------------------------------

/**
 * @brief Displays the transmit function output or an error message.
 *
 * @param result The byte array produced by the transmit function.
 * @param errorMsg Error message to display (empty on success).
 */
void DataModel::TransmitTestDialog::displayOutput(const QByteArray& result, const QString& errorMsg)
{
  auto* commonFonts = &Misc::CommonFonts::instance();

  // Display error message
  if (!errorMsg.isEmpty()) {
    m_rawOutput->setFont(commonFonts->uiFont());
    m_rawOutput->setPlainText(errorMsg);
    m_hexOutput->clear();
    m_byteCountLabel->clear();
    return;
  }

  // Handle empty payload
  if (result.isEmpty()) {
    m_rawOutput->setFont(commonFonts->uiFont());
    m_rawOutput->setPlainText(tr("(empty) No data returned"));
    m_hexOutput->clear();
    m_byteCountLabel->setText(tr("0 bytes"));
    return;
  }

  // Build escaped raw string, showing control chars as \xNN
  QString rawStr;
  for (int i = 0; i < result.size(); ++i) {
    unsigned char ch = static_cast<unsigned char>(result.at(i));
    if (ch == '\r')
      rawStr += QStringLiteral("\\r");
    else if (ch == '\n')
      rawStr += QStringLiteral("\\n");
    else if (ch == '\t')
      rawStr += QStringLiteral("\\t");
    else if (ch < 0x20 || ch >= 0x7F)
      rawStr += QStringLiteral("\\x%1").arg(ch, 2, 16, QLatin1Char('0'));
    else
      rawStr += QChar(ch);
  }

  // Build space-separated hex representation
  QString hexStr;
  for (int i = 0; i < result.size(); ++i) {
    if (i > 0)
      hexStr += ' ';

    hexStr +=
      QStringLiteral("%1").arg(static_cast<unsigned char>(result.at(i)), 2, 16, QLatin1Char('0'));
  }

  m_rawOutput->setFont(commonFonts->monoFont());
  m_rawOutput->setPlainText(rawStr);
  m_hexOutput->setPlainText(hexStr.toUpper());
  m_byteCountLabel->setText(tr("%1 byte(s)").arg(result.size()));
}
