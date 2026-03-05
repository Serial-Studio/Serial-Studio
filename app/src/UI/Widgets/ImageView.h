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

#ifdef BUILD_COMMERCIAL

#  include <QQuickItem>

#  include "IO/HAL_Driver.h"

namespace Widgets {

/**
 * @class ImageFrameReader
 * @brief Per-widget secondary frame reader that extracts binary image frames
 *        from the raw HAL driver byte stream.
 *
 * Connects directly to @c IO::HAL_Driver::dataReceived via a
 * @c Qt::QueuedConnection, completely independently of the main telemetry
 * @c IO::FrameReader. This means image data and CSV/JSON telemetry can
 * co-exist in the same byte stream without interfering with each other.
 *
 * ### Detection modes
 * - **Autodetect** (default): scans the accumulator for JPEG and PNG magic bytes
 *   and pairs them with their natural end markers:
 *   | Format | Start       | End                     |
 *   |--------|-------------|-------------------------|
 *   | JPEG   | FF D8 FF    | FF D9                   |
 *   | PNG    | 89 50 4E 47 0D 0A 1A 0A | 49 45 4E 44 AE 42 60 82 |
 *
 * - **Manual**: user supplies explicit hex or ASCII start/end byte sequences
 *   (stored in @c ProjectModel::widgetSettings under @c "imgStartSequence" /
 *   @c "imgEndSequence"). Useful for proprietary framing such as
 *   @c "$IMG_START$...$IMG_END$".
 *
 * ### Immutability
 * Config (mode + sequences) is fixed at construction time, matching the same
 * immutability contract as @c IO::FrameReader. If the user changes delimiter
 * settings in the Project Editor, @c Widgets::ImageView destroys and recreates
 * the reader via @c reconfigureReader().
 *
 * ### Accumulator
 * A plain @c QByteArray accumulator is used instead of @c IO::CircularBuffer
 * because images arrive infrequently and can be megabytes in size. The
 * accumulator is capped at 16 MiB and reset if exceeded.
 */
class ImageFrameReader : public QObject {
  Q_OBJECT

public:
  enum class DetectionMode {
    Autodetect,
    Manual
  };

  explicit ImageFrameReader(QObject* parent = nullptr);
  ImageFrameReader(QByteArray startSeq, QByteArray endSeq, QObject* parent = nullptr);

public slots:
  void processData(const IO::ByteArrayPtr& data);

signals:
  void frameReady(QByteArray imageData);

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
 * @class ImageView
 * @brief Dashboard widget model for live binary image streams.
 *
 * Owns an @c ImageFrameReader that taps directly into the HAL driver's
 * @c dataReceived signal, decodes incoming JPEG/PNG/BMP/WebP frames, and
 * pushes them to @c UI::ImageProvider. QML binds to the @c imageUrl property
 * to display the latest frame via a @c QQuickImageProvider URL.
 *
 * ### Cache-busting
 * @c imageUrl() appends @c "/<frameCount>" to the provider key on every call,
 * producing a unique URL per frame (e.g. @c "image://serial-studio-img/imageview:3/42").
 * This forces Qt's scene graph to re-request the texture on every frame instead
 * of serving a stale cached copy, which was the root cause of the "frozen image"
 * symptom seen when @c cache:false alone was insufficient.
 * @c UI::ImageProvider::requestImage() strips the suffix before the hash lookup.
 *
 * ### Exposed QML properties
 * | Property      | Type    | Description                          |
 * |---------------|---------|--------------------------------------|
 * | imageUrl      | string  | Provider URL for the current frame   |
 * | imageFormat   | string  | "JPEG", "PNG", "BMP", "WebP", or "Unknown" |
 * | frameCount    | int     | Total decoded frames since connect   |
 * | imageWidth    | int     | Pixel width of the last decoded frame |
 * | imageHeight   | int     | Pixel height of the last decoded frame |
 *
 * ### Connection lifecycle
 * 1. @c IO::Manager::driverChanged → @c reconfigureReader() (queued)
 * 2. @c reconfigureReader() reads delimiter config from
 *    @c ProjectModel::widgetSettings and recreates @c ImageFrameReader.
 * 3. On driver disconnect the driver object is destroyed, severing the
 *    @c dataReceived connection automatically.
 */
class ImageView : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString imageUrl    READ imageUrl    NOTIFY imageReady)
  Q_PROPERTY(QString imageFormat READ imageFormat NOTIFY imageReady)
  Q_PROPERTY(int     frameCount  READ frameCount  NOTIFY imageReady)
  Q_PROPERTY(int     imageWidth  READ imageWidth  NOTIFY imageReady)
  Q_PROPERTY(int     imageHeight READ imageHeight NOTIFY imageReady)
  // clang-format on

signals:
  void imageReady();

public:
  explicit ImageView(int index = -1, QQuickItem* parent = nullptr);
  ~ImageView();

  [[nodiscard]] QString imageUrl() const;
  [[nodiscard]] const QString& imageFormat() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int imageWidth() const;
  [[nodiscard]] int imageHeight() const;

private slots:
  void onFrameReady(const QByteArray& data);
  void reconfigureReader();

private:
  static QString detectFormat(const QByteArray& data);

  int m_index;
  int m_groupId;
  int m_frameCount;
  int m_imageWidth;
  int m_imageHeight;
  QString m_imageFormat;
  QString m_providerKey;
  ImageFrameReader* m_reader;
};

}  // namespace Widgets

#endif  // BUILD_COMMERCIAL
