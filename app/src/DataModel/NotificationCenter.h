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

#include <deque>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QSystemTrayIcon;
class QJSEngine;
struct lua_State;

namespace DataModel {

/**
 * @class NotificationCenter
 * @brief Central in-memory event bus (ring buffer, dedup, tray) for dashboard
 *        notifications. Main thread only -- workers route via QueuedConnection.
 */
class NotificationCenter : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int unreadCount
             READ  unreadCount
             NOTIFY unreadCountChanged)
  Q_PROPERTY(bool systemNotificationsEnabled
             READ  systemNotificationsEnabled
             WRITE setSystemNotificationsEnabled
             NOTIFY systemNotificationsEnabledChanged)
  Q_PROPERTY(bool routeWarningsToNotifications
             READ  routeWarningsToNotifications
             WRITE setRouteWarningsToNotifications
             NOTIFY routeWarningsToNotificationsChanged)
  Q_PROPERTY(QStringList channels
             READ  channels
             NOTIFY channelsChanged)
  // clang-format on

public:
  enum Level : int {
    Info     = 0,
    Warning  = 1,
    Critical = 2,
  };
  Q_ENUM(Level)

signals:
  void notificationPosted(const QVariantMap& event);
  void channelCleared(const QString& channel);
  void historyCleared();
  void unreadCountChanged();
  void channelsChanged();
  void systemNotificationsEnabledChanged();
  void routeWarningsToNotificationsChanged();

private:
  explicit NotificationCenter();
  ~NotificationCenter();
  NotificationCenter(NotificationCenter&&)                 = delete;
  NotificationCenter(const NotificationCenter&)            = delete;
  NotificationCenter& operator=(NotificationCenter&&)      = delete;
  NotificationCenter& operator=(const NotificationCenter&) = delete;

public:
  [[nodiscard]] static NotificationCenter& instance();

  static void installScriptApi(lua_State* L);
  static void installScriptApi(QJSEngine* js);

  [[nodiscard]] int unreadCount() const noexcept;
  [[nodiscard]] QStringList channels() const;
  [[nodiscard]] bool systemNotificationsEnabled() const noexcept;
  [[nodiscard]] bool routeWarningsToNotifications() const noexcept;

  Q_INVOKABLE [[nodiscard]] QVariantList history(const QString& channel = QString(),
                                                 int limit              = 0) const;
  Q_INVOKABLE [[nodiscard]] int maxHistory() const noexcept;

public slots:
  void post(int level, const QString& channel, const QString& title, const QString& subtitle);
  void postInfo(const QString& channel, const QString& title, const QString& subtitle);
  void postWarning(const QString& channel, const QString& title, const QString& subtitle);
  void postCritical(const QString& channel, const QString& title, const QString& subtitle);
  void resolve(const QString& channel, const QString& title, const QString& subtitle);

  void clearChannel(const QString& channel);
  void clearAll();
  void markRead();
  void setSystemNotificationsEnabled(bool enabled);
  void setRouteWarningsToNotifications(bool enabled);

public:
  struct Event {
    qint64 timestampMs;
    int level;
    QString channel;
    QString title;
    QString subtitle;
  };

  struct DedupKey {
    int level;
    QString channel;
    QString title;
    QString subtitle;

    bool operator==(const DedupKey& o) const noexcept
    {
      return level == o.level && channel == o.channel && title == o.title && subtitle == o.subtitle;
    }
  };

private:
  [[nodiscard]] QVariantMap toVariant(const Event& e) const;
  void ensureTrayIcon();
  bool shouldDropDuplicate(const DedupKey& k, qint64 now);
  void appendEvent(Event&& e);

private:
  static constexpr int kMaxHistory    = 1024;
  static constexpr int kDedupWindowMs = 100;
  static constexpr int kTrayTimeoutMs = 5000;

  std::deque<Event> m_history;
  QHash<DedupKey, qint64> m_lastSeen;
  QHash<QString, int> m_channelCounts;
  int m_unreadCount;
  bool m_systemNotificationsEnabled;
  bool m_routeWarningsToNotifications;
  QSystemTrayIcon* m_tray;
};

inline size_t qHash(const NotificationCenter::DedupKey& k, size_t seed = 0) noexcept
{
  return qHashMulti(seed, k.level, k.channel, k.title, k.subtitle);
}

}  // namespace DataModel
