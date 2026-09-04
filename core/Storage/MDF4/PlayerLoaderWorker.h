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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <atomic>
#include <memory>
#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>

namespace MDF4 {

/**
 * @brief Fully decoded MDF4 recording in columnar form: one contiguous vector per channel
 *        (numeric doubles, or per-frame text for string channels) plus the sorted frame
 *        timestamps and per-channel activity bits. No mdflib object crosses this boundary.
 */
struct PlayerDecodePayload {
  bool ok          = false;
  bool cancelled   = false;
  bool partialData = false;
  QString errorTitle;
  QString errorBody;
  QString filePath;
  bool isSerialStudioFile = false;
  quint64 generation      = 0;
  QStringList channelNames;
  std::vector<bool> channelIsString;
  std::vector<double> timestamps;
  std::vector<std::vector<double>> numeric;
  std::vector<std::vector<QString>> text;
  std::vector<std::vector<bool>> active;
  QVector<quint8> channelSourceBit;  ///< Per channel: bit of its source (input, may be empty)
  QVector<quint8> rowSourceBits;     ///< Per row: OR of channelSourceBit over active channels
};

/**
 * @brief Shared pointer alias for PlayerDecodePayload, exchanged across threads.
 */
using PlayerDecodePayloadPtr = std::shared_ptr<PlayerDecodePayload>;

/**
 * @brief Worker that performs the whole mdflib decode off the main thread; the reader and
 *        every mdf::* pointer stay confined to the worker's slot scope.
 */
class PlayerLoaderWorker : public QObject {
  Q_OBJECT

signals:
  void progressUpdate(double fraction, quint64 generation);
  void finished(const MDF4::PlayerDecodePayloadPtr& payload);

public:
  explicit PlayerLoaderWorker(QObject* parent = nullptr);

  PlayerLoaderWorker(PlayerLoaderWorker&&)                 = delete;
  PlayerLoaderWorker(const PlayerLoaderWorker&)            = delete;
  PlayerLoaderWorker& operator=(PlayerLoaderWorker&&)      = delete;
  PlayerLoaderWorker& operator=(const PlayerLoaderWorker&) = delete;

  void requestCancel();
  [[nodiscard]] bool recordTick();
  [[nodiscard]] bool cancelRequested() const;

public slots:
  void decodeFile(const QString& filePath,
                  quint64 generation,
                  const QVector<quint8>& channelSourceBit);

private:
  std::atomic<bool> m_cancelRequested;
  quint64 m_recordsSeen;
  quint64 m_recordsTotal;
  quint64 m_activeGeneration;
};

}  // namespace MDF4

Q_DECLARE_METATYPE(MDF4::PlayerDecodePayloadPtr)
