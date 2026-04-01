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

#include "UI/Widgets/ImageView.h"

#include <QImage>

#include "IO/ConnectionManager.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"
#include "UI/ImageProvider.h"
#include "UI/Widgets/ImageExport.h"

//--------------------------------------------------------------------------------------------------
// ImageFrameReader
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an autodetect-mode reader.
 * @param parent Optional QObject parent.
 */
Widgets::ImageFrameReader::ImageFrameReader(QObject* parent)
  : QObject(parent), m_mode(DetectionMode::Autodetect), m_inFrame(false), m_frameStartPos(-1)
{}

/**
 * @brief Constructs a manual-mode reader with explicit start/end byte sequences.
 * @param startSeq  Raw bytes marking the beginning of an image frame.
 * @param endSeq    Raw bytes marking the end of an image frame.
 * @param parent    Optional QObject parent.
 */
Widgets::ImageFrameReader::ImageFrameReader(QByteArray startSeq, QByteArray endSeq, QObject* parent)
  : QObject(parent)
  , m_mode(DetectionMode::Manual)
  , m_startSeq(std::move(startSeq))
  , m_endSeq(std::move(endSeq))
  , m_inFrame(false)
  , m_frameStartPos(-1)
{}

/**
 * @brief Appends incoming bytes to the accumulator and dispatches to the
 *        active detection mode.
 * @param data  Shared pointer to the raw byte chunk from the HAL driver.
 */
void Widgets::ImageFrameReader::processData(const IO::ByteArrayPtr& data)
{
  // Append incoming bytes and dispatch to the active detection mode
  if (!data || data->isEmpty())
    return;

  m_accumulator.append(*data);

  if (m_mode == DetectionMode::Autodetect)
    processAutodetect();
  else
    processManual();
}

/**
 * @brief Scans the accumulator for JPEG, PNG, BMP, and WebP magic bytes and
 *        emits @c frameReady() for each complete frame found.
 */
void Widgets::ImageFrameReader::processAutodetect()
{
  static const QByteArray kJpegStart("\xFF\xD8\xFF", 3);
  static const QByteArray kPngStart("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
  static const QByteArray kBmpStart("BM", 2);
  static const QByteArray kWebpStart("RIFF", 4);
  static const QByteArray kJpegEnd("\xFF\xD9", 2);
  static const QByteArray kPngEnd("\x49\x45\x4E\x44\xAE\x42\x60\x82", 8);

  while (true) {
    if (!m_inFrame) {
      qsizetype jpegPos = m_accumulator.indexOf(kJpegStart);
      qsizetype pngPos  = m_accumulator.indexOf(kPngStart);
      qsizetype bmpPos  = m_accumulator.indexOf(kBmpStart);
      qsizetype webpPos = m_accumulator.indexOf(kWebpStart);

      if (webpPos >= 0 && m_accumulator.size() >= webpPos + 12) {
        if (m_accumulator[webpPos + 8] != 'W' || m_accumulator[webpPos + 9] != 'E'
            || m_accumulator[webpPos + 10] != 'B' || m_accumulator[webpPos + 11] != 'P')
          webpPos = -1;
      } else {
        webpPos = -1;
      }

      qsizetype startPos = -1;
      auto pickEarliest  = [&](qsizetype a, qsizetype b) {
        if (a >= 0 && (b < 0 || a <= b))
          return a;

        return b;
      };

      startPos = pickEarliest(jpegPos, pngPos);
      startPos = pickEarliest(startPos, bmpPos);
      startPos = pickEarliest(startPos, webpPos);

      if (startPos < 0)
        break;

      m_accumulator.remove(0, startPos);
      m_inFrame       = true;
      m_frameStartPos = 0;
    }

    if (m_accumulator.startsWith(kWebpStart) && m_accumulator.size() >= 12
        && m_accumulator[8] == 'W' && m_accumulator[9] == 'E' && m_accumulator[10] == 'B'
        && m_accumulator[11] == 'P') {
      const auto* raw         = reinterpret_cast<const quint8*>(m_accumulator.constData());
      const quint32 chunkSize = static_cast<quint32>(raw[4]) | (static_cast<quint32>(raw[5]) << 8)
                              | (static_cast<quint32>(raw[6]) << 16)
                              | (static_cast<quint32>(raw[7]) << 24);
      const qsizetype frameLen = 8 + static_cast<qsizetype>(chunkSize);

      if (chunkSize > 64 * 1024 * 1024) {
        m_accumulator.remove(0, 4);
        m_inFrame       = false;
        m_frameStartPos = -1;
        continue;
      }

      if (m_accumulator.size() < frameLen)
        break;

      QByteArray frame = m_accumulator.left(frameLen);
      m_accumulator.remove(0, frameLen);
      m_inFrame       = false;
      m_frameStartPos = -1;

      Q_EMIT frameReady(frame);
      continue;
    }

    if (m_accumulator.startsWith(kBmpStart)) {
      if (m_accumulator.size() < 6)
        break;

      const auto* raw  = reinterpret_cast<const quint8*>(m_accumulator.constData());
      const quint32 sz = static_cast<quint32>(raw[2]) | (static_cast<quint32>(raw[3]) << 8)
                       | (static_cast<quint32>(raw[4]) << 16)
                       | (static_cast<quint32>(raw[5]) << 24);

      if (sz < 14 || sz > 64 * 1024 * 1024) {
        m_accumulator.remove(0, 2);
        m_inFrame       = false;
        m_frameStartPos = -1;
        continue;
      }

      if (m_accumulator.size() < static_cast<qsizetype>(sz))
        break;

      QByteArray frame = m_accumulator.left(static_cast<qsizetype>(sz));
      m_accumulator.remove(0, static_cast<qsizetype>(sz));
      m_inFrame       = false;
      m_frameStartPos = -1;

      Q_EMIT frameReady(frame);
      continue;
    }

    QByteArray endMarker;
    qsizetype endMarkerLen = 0;
    if (m_accumulator.startsWith(kJpegStart)) {
      endMarker    = kJpegEnd;
      endMarkerLen = kJpegEnd.size();
    } else if (m_accumulator.startsWith(kPngStart)) {
      endMarker    = kPngEnd;
      endMarkerLen = kPngEnd.size();
    } else {
      m_accumulator.clear();
      m_inFrame       = false;
      m_frameStartPos = -1;
      break;
    }

    qsizetype endPos = m_accumulator.indexOf(endMarker, 1);
    if (endPos < 0)
      break;

    qsizetype frameLen = endPos + endMarkerLen;
    QByteArray frame   = m_accumulator.left(frameLen);
    m_accumulator.remove(0, frameLen);
    m_inFrame       = false;
    m_frameStartPos = -1;

    Q_EMIT frameReady(frame);
  }

  // Prevent unbounded growth (cap at 16 MiB)
  constexpr qsizetype kMaxAccumulator = 16 * 1024 * 1024;
  if (m_accumulator.size() > kMaxAccumulator) {
    m_accumulator.clear();
    m_inFrame       = false;
    m_frameStartPos = -1;
  }
}

/**
 * @brief Searches the accumulator for the user-supplied start/end sequences
 *        and emits @c frameReady() for each complete frame found.
 */
void Widgets::ImageFrameReader::processManual()
{
  // Require both delimiters to be configured
  if (m_startSeq.isEmpty() || m_endSeq.isEmpty())
    return;

  // Scan the accumulator for complete frames between start/end sequences
  while (true) {
    if (!m_inFrame) {
      qsizetype startPos = m_accumulator.indexOf(m_startSeq);
      if (startPos < 0)
        break;

      m_accumulator.remove(0, startPos + m_startSeq.size());
      m_inFrame       = true;
      m_frameStartPos = 0;
    }

    qsizetype endPos = m_accumulator.indexOf(m_endSeq);
    if (endPos < 0)
      break;

    QByteArray frame = m_accumulator.left(endPos);
    m_accumulator.remove(0, endPos + m_endSeq.size());
    m_inFrame       = false;
    m_frameStartPos = -1;

    if (!frame.isEmpty())
      Q_EMIT frameReady(frame);
  }

  // Prevent unbounded growth (cap at 16 MiB)
  constexpr qsizetype kMaxAccumulator = 16 * 1024 * 1024;
  if (m_accumulator.size() > kMaxAccumulator) {
    m_accumulator.clear();
    m_inFrame       = false;
    m_frameStartPos = -1;
  }
}

//--------------------------------------------------------------------------------------------------
// ImageView
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ImageView widget model and wires the driver connection.
 * @param index   Dashboard widget index used to look up group metadata.
 * @param parent  Optional QQuickItem parent.
 */
Widgets::ImageView::ImageView(int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_groupId(-1)
  , m_sourceId(0)
  , m_frameCount(0)
  , m_imageWidth(0)
  , m_imageHeight(0)
  , m_exportEnabled(ImageExport::instance().exportEnabled())
  , m_primaryColor(Qt::black)
  , m_imageFormat(QStringLiteral("Unknown"))
  , m_reader(nullptr)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardImageView, m_index)) {
    const auto& group = GET_GROUP(SerialStudio::DashboardImageView, m_index);
    m_groupId         = group.groupId;
    m_sourceId        = group.sourceId;
    m_groupTitle      = group.title;
  }

  m_providerKey = QStringLiteral("imageview:%1").arg(m_groupId);

  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::driverChanged,
          this,
          &ImageView::reconfigureReader);

  reconfigureReader();
}

/**
 * @brief Destroys the widget and its associated @c ImageFrameReader.
 */
Widgets::ImageView::~ImageView()
{
  if (m_reader) {
    m_reader->deleteLater();
    m_reader = nullptr;
  }
}

/**
 * @brief Returns a cache-busting URL for the current frame.
 *
 * The frame counter is appended so each call produces a unique URL, forcing
 * Qt's scene graph to re-request the texture instead of serving a cached copy.
 */
QString Widgets::ImageView::imageUrl() const
{
  return QStringLiteral("image://serial-studio-img/") + m_providerKey + QStringLiteral("/")
       + QString::number(m_frameCount);
}

/**
 * @brief Returns the detected format of the last decoded frame (e.g. "JPEG").
 */
const QString& Widgets::ImageView::imageFormat() const noexcept
{
  return m_imageFormat;
}

/**
 * @brief Returns the total number of frames decoded since the last connection.
 */
int Widgets::ImageView::frameCount() const noexcept
{
  return m_frameCount;
}

/**
 * @brief Returns the pixel width of the last decoded frame.
 */
int Widgets::ImageView::imageWidth() const noexcept
{
  return m_imageWidth;
}

/**
 * @brief Returns the pixel height of the last decoded frame.
 */
int Widgets::ImageView::imageHeight() const noexcept
{
  return m_imageHeight;
}

/**
 * @brief Returns the dominant color of the last decoded frame.
 */
QColor Widgets::ImageView::primaryColor() const
{
  return m_primaryColor;
}

/**
 * @brief Returns the dashboard group title associated with this widget.
 */
const QString& Widgets::ImageView::groupTitle() const noexcept
{
  return m_groupTitle;
}

/**
 * @brief Returns whether per-widget image export is currently enabled.
 */
bool Widgets::ImageView::exportEnabled() const noexcept
{
  return m_exportEnabled;
}

/**
 * @brief Enables or disables per-widget image export.
 * @param enabled  @c true to start exporting frames, @c false to stop.
 */
void Widgets::ImageView::setExportEnabled(bool enabled)
{
  // Guard unchanged value
  if (m_exportEnabled == enabled)
    return;

  // Update state and close the export session when disabling
  m_exportEnabled = enabled;

  if (!enabled)
    ImageExport::instance().closeSession(m_groupId);

  Q_EMIT exportEnabledChanged();
}

/**
 * @brief Decodes a complete image frame and pushes it to the image provider.
 *
 * Detects the format, decodes via @c QImage::fromData, updates dimensions,
 * publishes to @c UI::ImageProvider, and enqueues for export when enabled.
 * @param data  Raw bytes of a single image frame.
 */
void Widgets::ImageView::onFrameReady(const QByteArray& data)
{
  // Ignore empty frames and paused state
  if (data.isEmpty())
    return;

  if (IO::ConnectionManager::instance().paused())
    return;

  // Detect format, decode, and publish the image
  m_imageFormat = detectFormat(data);

  QImage img = QImage::fromData(data);
  if (img.isNull())
    return;

  m_imageWidth  = img.width();
  m_imageHeight = img.height();

  {
    const QImage thumb = img.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::FastTransformation)
                           .convertToFormat(QImage::Format_RGB32);
    quint64 r = 0, g = 0, b = 0;
    const int n = thumb.width() * thumb.height();
    for (int y = 0; y < thumb.height(); ++y) {
      const QRgb* line = reinterpret_cast<const QRgb*>(thumb.constScanLine(y));
      for (int x = 0; x < thumb.width(); ++x) {
        r += qRed(line[x]);
        g += qGreen(line[x]);
        b += qBlue(line[x]);
      }
    }
    m_primaryColor =
      QColor(static_cast<int>(r / n), static_cast<int>(g / n), static_cast<int>(b / n));
  }

  if (auto* prov = UI::ImageProvider::global())
    prov->setImage(m_providerKey, img);

  if (m_exportEnabled) {
    const auto& dash = UI::Dashboard::instance();
    ImageExport::instance().enqueueImage(
      data, m_imageFormat, m_groupId, m_groupTitle, dash.title());
  }

  ++m_frameCount;
  Q_EMIT imageReady();
}

/**
 * @brief Destroys the current @c ImageFrameReader and creates a new one with
 *        the latest delimiter configuration from the project model.
 *
 * Called whenever the active driver changes. The old reader is scheduled for
 * deletion via @c deleteLater() to avoid destroying it mid-signal.
 */
void Widgets::ImageView::reconfigureReader()
{
  // Destroy any existing reader before creating a new one
  if (m_reader) {
    m_reader->deleteLater();
    m_reader = nullptr;
  }

  auto* driver = IO::ConnectionManager::instance().driver(m_sourceId);
  if (!driver)
    return;

  QString mode;
  QString startHex;
  QString endHex;

  if (VALIDATE_WIDGET(SerialStudio::DashboardImageView, m_index)) {
    const auto& group = GET_GROUP(SerialStudio::DashboardImageView, m_index);
    mode              = group.imgDetectionMode;
    startHex          = group.imgStartSequence;
    endHex            = group.imgEndSequence;
  }

  if (mode == QLatin1String("manual") && !startHex.isEmpty() && !endHex.isEmpty()) {
    m_reader = new ImageFrameReader(
      SerialStudio::hexToBytes(startHex), SerialStudio::hexToBytes(endHex), this);
  } else {
    m_reader = new ImageFrameReader(this);
  }

  connect(driver, &IO::HAL_Driver::dataReceived, m_reader, &ImageFrameReader::processData);

  connect(
    m_reader, &ImageFrameReader::frameReady, this, &ImageView::onFrameReady, Qt::QueuedConnection);
}

/**
 * @brief Identifies the image format from the first few bytes of @p data.
 * @param data  Raw image bytes.
 * @return "JPEG", "PNG", "BMP", "WebP", or "Unknown".
 */
QString Widgets::ImageView::detectFormat(const QByteArray& data)
{
  // Match magic bytes against known image format headers
  if (data.size() >= 3 && static_cast<quint8>(data[0]) == 0xFF
      && static_cast<quint8>(data[1]) == 0xD8 && static_cast<quint8>(data[2]) == 0xFF)
    return QStringLiteral("JPEG");

  if (data.size() >= 8 && static_cast<quint8>(data[0]) == 0x89 && data[1] == 'P' && data[2] == 'N'
      && data[3] == 'G')
    return QStringLiteral("PNG");

  if (data.size() >= 2 && data[0] == 'B' && data[1] == 'M')
    return QStringLiteral("BMP");

  if (data.size() >= 12 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F'
      && data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P')
    return QStringLiteral("WebP");

  return QStringLiteral("Unknown");
}
