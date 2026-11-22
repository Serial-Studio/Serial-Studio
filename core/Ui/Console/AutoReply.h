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
#include <QElapsedTimer>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <unordered_map>
#include <vector>

#include "Console/AutoReplyRule.h"

namespace Console {

/**
 * @brief The link auto-reply answers through, as callbacks, so the responder needs no console
 *        handler and a test can stand in for the device.
 */
struct ReplyLink {
  std::function<bool()> connected;
  std::function<void(int, const QByteArray&)> send;
  std::function<void(const QString&)> note;
};

/**
 * @brief Answers received messages from a rule table (spec 0127 R4-R7). Rules are app-wide
 *        settings; the armed state is not stored, so nothing answers a device until the user arms
 *        it in the current session, and a closed link or a reply storm disarms it. Disarmed, a
 *        received chunk costs one early return. A sealed deployment session never arms.
 */
class AutoReply : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QVariantList rules
             READ rules
             NOTIFY rulesChanged)
  Q_PROPERTY(bool armed
             READ armed
             NOTIFY armedChanged)
  Q_PROPERTY(int gapMs
             READ gapMs
             WRITE setGapMs
             NOTIFY gapChanged)
  Q_PROPERTY(quint64 replyCount
             READ replyCount
             NOTIFY replyCountChanged)
  Q_PROPERTY(bool available
             READ available
             CONSTANT)
  Q_PROPERTY(int maxRules
             READ maxRules
             CONSTANT)
  Q_PROPERTY(QStringList checksumNames
             READ checksumNames
             CONSTANT)
  // clang-format on

signals:
  void gapChanged();
  void rulesChanged();
  void armedChanged();
  void replyCountChanged();

public:
  explicit AutoReply(ReplyLink link, QObject* parent = nullptr);

  [[nodiscard]] bool armed() const noexcept;
  [[nodiscard]] int gapMs() const noexcept;
  [[nodiscard]] int maxRules() const noexcept;
  [[nodiscard]] bool available() const noexcept;
  [[nodiscard]] quint64 replyCount() const noexcept;
  [[nodiscard]] QStringList checksumNames() const;
  [[nodiscard]] const QVariantList& rules() const noexcept;

  [[nodiscard]] Q_INVOKABLE QString ruleProblem(const QVariantMap& rule) const;

  void feed(int deviceId, const QByteArray& bytes);

public slots:
  void arm();
  void disarm();
  void onLinkChanged();
  void removeRule(int index);
  void setGapMs(int gapMs);
  void moveRule(int from, int to);
  void addRule(const QVariantMap& rule);
  void setRuleEnabled(int index, bool enabled);
  void updateRule(int index, const QVariantMap& rule);

private slots:
  void onGapTimeout();

private:
  /**
   * @brief What one device has sent that no rule has judged yet.
   */
  struct DeviceState {
    QByteArray line;
    QByteArray packet;
    qint64 lastRxMs = 0;
    bool skipping   = false;
  };

  void loadRules();
  void saveRules();
  void recompile();
  void feedLines(int deviceId, DeviceState& state, const QByteArray& bytes);
  void dispatch(int deviceId, QByteArrayView message, bool hex);

private:
  ReplyLink m_link;
  QSettings m_settings;
  QVariantList m_rules;
  std::vector<CompiledReplyRule> m_compiled;
  std::unordered_map<int, DeviceState> m_devices;
  ReplyRateLimiter m_limiter;
  QElapsedTimer m_clock;
  QTimer m_gapTimer;
  quint64 m_replyCount;
  int m_gapMs;
  bool m_armed;
  bool m_anyText;
  bool m_anyHex;
};

}  // namespace Console
