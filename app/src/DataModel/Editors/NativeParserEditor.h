/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QVariantList>

class QStandardItem;
class QAbstractItemModel;

namespace DataModel {

class CustomModel;
class INativeTemplate;
struct PipelineResult;
struct Source;

/**
 * @brief QML bridge for the native frame-parser editor pane: template selection, the dynamic
 * parameter form model, inline pipeline controls and the live dry-run preview.
 */
class NativeParserEditor : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int sourceId
             READ sourceId
             WRITE setSourceId
             NOTIFY sourceIdChanged)
  Q_PROPERTY(int templateIndex
             READ templateIndex
             WRITE setTemplateIndex
             NOTIFY templateIndexChanged)
  Q_PROPERTY(QStringList templateNames
             READ templateNames
             NOTIFY templatesChanged)
  Q_PROPERTY(QString templateDescription
             READ templateDescription
             NOTIFY templateIndexChanged)
  Q_PROPERTY(QString templateDocumentation
             READ templateDocumentation
             NOTIFY templateIndexChanged)
  Q_PROPERTY(QAbstractItemModel* parameterModel
             READ parameterModel
             NOTIFY parameterModelChanged)
  Q_PROPERTY(int parameterCount
             READ parameterCount
             NOTIFY parameterModelChanged)
  Q_PROPERTY(QString paramError
             READ paramError
             NOTIFY paramErrorChanged)
  Q_PROPERTY(int decoderIndex
             READ decoderIndex
             WRITE setDecoderIndex
             NOTIFY pipelineChanged)
  Q_PROPERTY(int detectionIndex
             READ detectionIndex
             WRITE setDetectionIndex
             NOTIFY pipelineChanged)
  Q_PROPERTY(int checksumIndex
             READ checksumIndex
             WRITE setChecksumIndex
             NOTIFY pipelineChanged)
  Q_PROPERTY(QString frameStart
             READ frameStart
             WRITE setFrameStart
             NOTIFY pipelineChanged)
  Q_PROPERTY(QString frameEnd
             READ frameEnd
             WRITE setFrameEnd
             NOTIFY pipelineChanged)
  Q_PROPERTY(bool hexDelimiters
             READ hexDelimiters
             WRITE setHexDelimiters
             NOTIFY pipelineChanged)
  Q_PROPERTY(QStringList decoderOptions
             READ decoderOptions
             NOTIFY pipelineOptionsChanged)
  Q_PROPERTY(QStringList detectionOptions
             READ detectionOptions
             NOTIFY pipelineOptionsChanged)
  Q_PROPERTY(QStringList checksumOptions
             READ checksumOptions
             NOTIFY pipelineOptionsChanged)
  // clang-format on

signals:
  void sourceIdChanged();
  void templatesChanged();
  void pipelineChanged();
  void paramErrorChanged();
  void templateIndexChanged();
  void parameterModelChanged();
  void pipelineOptionsChanged();
  void previewReady(const QVariantList& frames, const QString& error, const QString& stats);

public:
  explicit NativeParserEditor(QObject* parent = nullptr);

  [[nodiscard]] int sourceId() const noexcept;
  [[nodiscard]] int templateIndex() const;
  [[nodiscard]] QString paramError() const;
  [[nodiscard]] QString templateDescription() const;
  [[nodiscard]] QString templateDocumentation() const;
  [[nodiscard]] int parameterCount() const;
  [[nodiscard]] const QStringList& templateNames() const;
  [[nodiscard]] QAbstractItemModel* parameterModel() const;

  [[nodiscard]] int decoderIndex() const;
  [[nodiscard]] int detectionIndex() const;
  [[nodiscard]] int checksumIndex() const;
  [[nodiscard]] QString frameStart() const;
  [[nodiscard]] QString frameEnd() const;
  [[nodiscard]] bool hexDelimiters() const;
  [[nodiscard]] QStringList decoderOptions() const;
  [[nodiscard]] QStringList detectionOptions() const;
  [[nodiscard]] QStringList checksumOptions() const;

  Q_INVOKABLE [[nodiscard]] bool inputContainsDelimiters(const QString& input, bool hex) const;

public slots:
  void setSourceId(int sourceId);
  void setTemplateIndex(int index);
  void setDecoderIndex(int index);
  void setDetectionIndex(int index);
  void setChecksumIndex(int index);
  void setFrameStart(const QString& sequence);
  void setFrameEnd(const QString& sequence);
  void setHexDelimiters(bool hexadecimal);
  void resetToDefaults();
  void dryRun(const QString& input, bool hex);

private slots:
  void onItemChanged(QStandardItem* item);
  void onSourceTemplateChanged(int sourceId);
  void onSourceParamsChanged(int sourceId);
  void onLanguageChanged();

private:
  void rebuildTemplateNames();
  void rebuildParameterModel();
  void setParamError(const QString& error);
  void emitPreview(const PipelineResult& result, int decoderMethod);
  [[nodiscard]] Source currentSource() const;
  [[nodiscard]] const INativeTemplate* currentTemplate() const;
  [[nodiscard]] bool validateParams(const QJsonObject& params, QString& error) const;

private:
  int m_sourceId;
  bool m_applying;
  QString m_paramError;
  QJsonObject m_params;
  QStringList m_templateNames;
  CustomModel* m_parameterModel;
};

}  // namespace DataModel
