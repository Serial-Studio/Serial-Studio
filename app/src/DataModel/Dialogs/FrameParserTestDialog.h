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

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>

namespace DataModel {
class FrameParser;

/**
 * @brief Editor dialog that drives the FrameParserPipeline against user-supplied bytes.
 */
class FrameParserTestDialog : public QDialog {
  Q_OBJECT

public:
  explicit FrameParserTestDialog(FrameParser* parser, QWidget* parent = nullptr);

  void setSourceId(int sourceId);

public slots:
  void clear();
  void parseData();

private slots:
  void onThemeChanged();
  void onLanguageChanged();
  void onInputModeChanged(Qt::CheckState state);
  void onInputDataChanged(const QString& text);

  void onDetectionChanged(int index);
  void onDecoderChanged(int index);
  void onChecksumChanged(const QString& text);
  void onStartSequenceEdited();
  void onFinishSequenceEdited();
  void onHexDelimitersToggled(Qt::CheckState state);
  void onSourceChanged(int sourceId);
  void reloadFromSource();

private:
  bool validateHexInput(const QString& text);
  QString formatHexInput(const QString& text);
  void refreshPipelineControls();
  void applyDelimitersToProject();
  void updateStageVisibility();
  void populateChecksumCombo();

  void buildPipelineGroup();
  void buildInputGroup();
  void buildOutputGroup();
  void connectControls();

protected:
  void closeEvent(QCloseEvent* event) override;

private:
  int m_sourceId;
  bool m_suspendSync;
  FrameParser* m_parser;

  // Group boxes + titles
  QLabel* m_pipelineTitle;
  QLabel* m_inputTitle;
  QLabel* m_outputTitle;
  QGroupBox* m_pipelineGroup;
  QGroupBox* m_inputGroup;
  QGroupBox* m_outputGroup;

  // Pipeline controls
  QComboBox* m_detectionCombo;
  QComboBox* m_decoderCombo;
  QComboBox* m_checksumCombo;
  QLineEdit* m_startEdit;
  QLineEdit* m_finishEdit;
  QCheckBox* m_hexDelimiters;
  QPushButton* m_reloadButton;
  QLabel* m_startLabel;
  QLabel* m_finishLabel;
  QLabel* m_detectionLabel;
  QLabel* m_decoderLabel;
  QLabel* m_checksumLabel;

  // Input row
  QLineEdit* m_userInput;
  QCheckBox* m_hexCheckBox;
  QPushButton* m_clearButton;
  QPushButton* m_parseButton;

  // Output area
  QLabel* m_statsLabel;
  QTreeWidget* m_results;
};

}  // namespace DataModel
