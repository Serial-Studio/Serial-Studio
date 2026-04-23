/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QColor>
#include <QQuickItem>

#include "IO/HAL_Driver.h"

namespace Widgets {

/**
 * @brief Per-widget secondary frame reader that extracts binary image frames
 *        from the raw HAL driver byte stream.
 */
class ImageFrameReader : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void frameReady(QByteArray imageData);

public:
  enum class DetectionMode {
    Autodetect,
    Manual
  };

  explicit ImageFrameReader(QObject* parent = nullptr);
  ImageFrameReader(QByteArray startSeq, QByteArray endSeq, QObject* parent = nullptr);

public slots:
  void processData(const IO::CapturedDataPtr& data);

private:
  void processAutodetect();
  void processManual();

  DetectionMode m_mode;
  QByteArray m_startSeq;
  QByteArray m_endSeq;
  QByteArray m_accumulator;
  bool m_inFrame;
  qsizetype m_frameStartPos;
};

/**
 * @brief Dashboard widget model for live binary image streams.
 */
class ImageView : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY exportEnabledChanged)
  Q_PROPERTY(int frameCount
             READ frameCount
             NOTIFY imageReady)
  Q_PROPERTY(int imageWidth
             READ imageWidth
             NOTIFY imageReady)
  Q_PROPERTY(int imageHeight
             READ imageHeight
             NOTIFY imageReady)
  Q_PROPERTY(QColor primaryColor
             READ primaryColor
             NOTIFY imageReady)
  Q_PROPERTY(QString groupTitle
             READ groupTitle
             CONSTANT)
  Q_PROPERTY(QString imageUrl
             READ imageUrl
             NOTIFY imageReady)
  Q_PROPERTY(QString imageFormat
             READ imageFormat
             NOTIFY imageReady)
  // clang-format on

signals:
  void imageReady();
  void exportEnabledChanged();

public:
  explicit ImageView(int index = -1, QQuickItem* parent = nullptr);
  ~ImageView();

  [[nodiscard]] int frameCount() const noexcept;
  [[nodiscard]] int imageWidth() const noexcept;
  [[nodiscard]] int imageHeight() const noexcept;
  [[nodiscard]] bool exportEnabled() const noexcept;
  [[nodiscard]] QString imageUrl() const;
  [[nodiscard]] QColor primaryColor() const;
  [[nodiscard]] const QString& imageFormat() const noexcept;
  [[nodiscard]] const QString& groupTitle() const noexcept;

public slots:
  void setExportEnabled(bool enabled);

private slots:
  void onFrameReady(const QByteArray& data);
  void reconfigureReader();

private:
  static QString detectFormat(const QByteArray& data);

  int m_index;
  int m_groupId;
  int m_sourceId;
  int m_frameCount;
  int m_imageWidth;
  int m_imageHeight;
  bool m_exportEnabled;
  QColor m_primaryColor;
  QString m_imageFormat;
  QString m_providerKey;
  QString m_groupTitle;
  ImageFrameReader* m_reader;
};

}  // namespace Widgets
