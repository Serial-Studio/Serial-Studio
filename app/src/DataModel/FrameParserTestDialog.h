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

#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

namespace DataModel {
class FrameParser;

/**
 * @brief Dialog for testing frame parser functions with sample data
 *
 * Provides an interface for users to:
 * - Input sample frame data (text or hex)
 * - Execute the parser function
 * - View the parsed output in a table
 * - Test multiple frames sequentially
 */
class FrameParserTestDialog : public QDialog {
  Q_OBJECT

public:
  explicit FrameParserTestDialog(FrameParser* parser, QWidget* parent = nullptr);

public slots:
  void clear();
  void parseData();

private slots:
  void onThemeChanged();
  void onLanguageChanged();
  void onInputModeChanged(Qt::CheckState state);
  void onInputDataChanged(const QString& text);

private:
  bool validateHexInput(const QString& text);
  QString formatHexInput(const QString& text);
  void displayOutput(const QString& input, const QStringList& output);

private:
  QLabel* m_inputTitle;
  QLabel* m_outputTitle;
  FrameParser* m_parser;
  QTableWidget* m_table;
  QLineEdit* m_userInput;
  QGroupBox* m_inputGroup;
  QGroupBox* m_outputGroup;
  QCheckBox* m_hexCheckBox;
  QPushButton* m_parseButton;
  QPushButton* m_clearButton;
  QPlainTextEdit* m_inputDisplay;
};

}  // namespace DataModel
