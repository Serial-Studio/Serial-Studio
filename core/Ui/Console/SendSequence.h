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

#include <functional>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

namespace Console {

/**
 * @brief Bounds and default of a pinned command's delay in a command list (spec 0127 R1), and
 *        how often a list held by a paused link looks again.
 */
inline constexpr int kSequenceDelayMin     = 0;
inline constexpr int kSequenceDelayMax     = 3600000;
inline constexpr int kSequenceDelayDefault = 1000;
inline constexpr int kSequencePausePollMs  = 100;

/**
 * @brief What one console transmission is made of: the payload and the framing it was saved
 *        with. Pins and auto-reply rules both store one, so a list or a reply sends with its own
 *        settings instead of whatever the send bar shows at that moment.
 */
struct TxPreset {
  QString payload;
  QString checksum;
  int lineEnding = 0;
  int encoding   = 0;
  bool hex       = false;
};

[[nodiscard]] QByteArray lineEndingBytes(int lineEnding);
[[nodiscard]] TxPreset txPresetFromMap(const QVariantMap& map);
[[nodiscard]] QByteArray encodeTxPreset(const TxPreset& preset);
void writeTxPreset(const TxPreset& preset, QVariantMap& map);

[[nodiscard]] bool pinEnabled(const QVariantMap& pin);
[[nodiscard]] int pinDelayMs(const QVariantMap& pin);
[[nodiscard]] int nextEnabledPin(const QVariantList& pins, int after);

/**
 * @brief The link a command list sends through, as three callbacks, so the runner needs no
 *        console handler and a test can stand in for the device.
 */
struct SequenceLink {
  std::function<bool()> connected;
  std::function<bool()> paused;
  std::function<void(const QByteArray&)> send;
};

/**
 * @brief Runs a list of pinned commands in order (spec 0127 R2/R3): each enabled pin waits its
 *        own delay and is sent with its own framing, once through or in a loop. The list is
 *        copied when the run starts, so editing pins never shifts a running one. A closed link
 *        ends the run, a paused link holds its position, and a sealed session never starts one.
 */
class SendSequencer : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void stateChanged();

public:
  explicit SendSequencer(SequenceLink link, QObject* parent = nullptr);

  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] bool looping() const noexcept;
  [[nodiscard]] int currentIndex() const noexcept;
  [[nodiscard]] bool start(const QVariantList& pins, bool loop);

public slots:
  void stop();

private slots:
  void onTick();

private:
  void schedule(int index);

private:
  SequenceLink m_link;
  QVariantList m_pins;
  QTimer m_timer;
  int m_index;
  bool m_loop;
};

}  // namespace Console
