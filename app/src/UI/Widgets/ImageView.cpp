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

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/ImageView.h"

#  include <QImage>

#  include "IO/Manager.h"
#  include "SerialStudio.h"
#  include "UI/Dashboard.h"
#  include "UI/ImageProvider.h"

//--------------------------------------------------------------------------------------------------
// ImageFrameReader
//--------------------------------------------------------------------------------------------------

Widgets::ImageFrameReader::ImageFrameReader(QObject* parent)
  : QObject(parent), m_mode(DetectionMode::Autodetect), m_inFrame(false), m_frameStartPos(-1)
{}

Widgets::ImageFrameReader::ImageFrameReader(QByteArray startSeq, QByteArray endSeq, QObject* parent)
  : QObject(parent)
  , m_mode(DetectionMode::Manual)
  , m_startSeq(std::move(startSeq))
  , m_endSeq(std::move(endSeq))
  , m_inFrame(false)
  , m_frameStartPos(-1)
{}

void Widgets::ImageFrameReader::processData(const IO::ByteArrayPtr& data)
{
  if (!data || data->isEmpty())
    return;

  m_accumulator.append(*data);

  if (m_mode == DetectionMode::Autodetect)
    processAutodetect();
  else
    processManual();
}

void Widgets::ImageFrameReader::processAutodetect()
{
  static const QByteArray kJpegStart("\xFF\xD8\xFF", 3);
  static const QByteArray kPngStart("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
  static const QByteArray kJpegEnd("\xFF\xD9", 2);
  static const QByteArray kPngEnd("\x49\x45\x4E\x44\xAE\x42\x60\x82", 8);

  while (true) {
    if (!m_inFrame) {
      qsizetype jpegPos = m_accumulator.indexOf(kJpegStart);
      qsizetype pngPos  = m_accumulator.indexOf(kPngStart);

      qsizetype startPos = -1;
      if (jpegPos >= 0 && (pngPos < 0 || jpegPos <= pngPos))
        startPos = jpegPos;
      else if (pngPos >= 0)
        startPos = pngPos;

      if (startPos < 0)
        break;

      m_accumulator.remove(0, startPos);
      m_inFrame       = true;
      m_frameStartPos = 0;
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

void Widgets::ImageFrameReader::processManual()
{
  if (m_startSeq.isEmpty() || m_endSeq.isEmpty())
    return;

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

Widgets::ImageView::ImageView(int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_groupId(-1)
  , m_frameCount(0)
  , m_imageWidth(0)
  , m_imageHeight(0)
  , m_imageFormat(QStringLiteral("Unknown"))
  , m_reader(nullptr)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardImageView, m_index)) {
    const auto& group = GET_GROUP(SerialStudio::DashboardImageView, m_index);
    m_groupId         = group.groupId;
  }

  m_providerKey = QStringLiteral("imageview:%1").arg(m_groupId);

  connect(&IO::Manager::instance(),
          &IO::Manager::driverChanged,
          this,
          &ImageView::reconfigureReader,
          Qt::QueuedConnection);

  reconfigureReader();
}

Widgets::ImageView::~ImageView()
{
  if (m_reader) {
    m_reader->deleteLater();
    m_reader = nullptr;
  }
}

QString Widgets::ImageView::imageUrl() const
{
  return QStringLiteral("image://serial-studio-img/") + m_providerKey + QStringLiteral("/")
       + QString::number(m_frameCount);
}

const QString& Widgets::ImageView::imageFormat() const
{
  return m_imageFormat;
}

int Widgets::ImageView::frameCount() const
{
  return m_frameCount;
}

int Widgets::ImageView::imageWidth() const
{
  return m_imageWidth;
}

int Widgets::ImageView::imageHeight() const
{
  return m_imageHeight;
}

void Widgets::ImageView::onFrameReady(const QByteArray& data)
{
  if (data.isEmpty())
    return;

  m_imageFormat = detectFormat(data);

  QImage img = QImage::fromData(data);
  if (img.isNull())
    return;

  m_imageWidth  = img.width();
  m_imageHeight = img.height();

  if (auto* prov = UI::ImageProvider::global())
    prov->setImage(m_providerKey, img);
  ++m_frameCount;
  Q_EMIT imageReady();
}

void Widgets::ImageView::reconfigureReader()
{
  if (m_reader) {
    m_reader->deleteLater();
    m_reader = nullptr;
  }

  auto* driver = IO::Manager::instance().driver();
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

  connect(driver,
          &IO::HAL_Driver::dataReceived,
          m_reader,
          &ImageFrameReader::processData,
          Qt::QueuedConnection);

  connect(
    m_reader, &ImageFrameReader::frameReady, this, &ImageView::onFrameReady, Qt::QueuedConnection);
}

QString Widgets::ImageView::detectFormat(const QByteArray& data)
{
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

#endif  // BUILD_COMMERCIAL
