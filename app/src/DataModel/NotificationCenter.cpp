/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include "DataModel/NotificationCenter.h"

#include <lauxlib.h>
#include <lua.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QIcon>
#include <QJSEngine>
#include <QQmlEngine>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QThread>

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Settings keys
//--------------------------------------------------------------------------------------------------

static const QString kSettingsKeySysNotify =
  QStringLiteral("NotificationCenter/systemNotifications");
static const QString kSettingsKeyRouteWarnings =
  QStringLiteral("NotificationCenter/routeWarningsToNotifications");

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the NotificationCenter singleton and restores user preferences.
 */
DataModel::NotificationCenter::NotificationCenter()
  : QObject(nullptr)
  , m_unreadCount(0)
  , m_systemNotificationsEnabled(false)
  , m_routeWarningsToNotifications(false)
  , m_tray(nullptr)
{
  // Pin affinity to the GUI thread so post()'s thread-check is meaningful
  if (QCoreApplication::instance())
    moveToThread(QCoreApplication::instance()->thread());

  // Restore user preferences (defaults to off)
  QSettings settings;
  m_systemNotificationsEnabled   = settings.value(kSettingsKeySysNotify, false).toBool();
  m_routeWarningsToNotifications = settings.value(kSettingsKeyRouteWarnings, false).toBool();
}

/**
 * @brief Default destructor.
 */
DataModel::NotificationCenter::~NotificationCenter() = default;

/**
 * @brief Returns the global instance; created on first access.
 */
DataModel::NotificationCenter& DataModel::NotificationCenter::instance()
{
  static NotificationCenter self;
  return self;
}

//--------------------------------------------------------------------------------------------------
// Read-only getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the count of unread non-Info notifications.
 */
int DataModel::NotificationCenter::unreadCount() const noexcept
{
  return m_unreadCount;
}

/**
 * @brief Returns the sorted list of channels that currently hold events.
 */
QStringList DataModel::NotificationCenter::channels() const
{
  auto keys = m_channelCounts.keys();
  std::sort(keys.begin(), keys.end());
  return keys;
}

/**
 * @brief Returns true if system tray notifications are enabled.
 */
bool DataModel::NotificationCenter::systemNotificationsEnabled() const noexcept
{
  return m_systemNotificationsEnabled;
}

/**
 * @brief Returns true when qWarning() output should be mirrored as Warning notifications.
 */
bool DataModel::NotificationCenter::routeWarningsToNotifications() const noexcept
{
  return m_routeWarningsToNotifications;
}

/**
 * @brief Returns the ring-buffer size cap for retained notifications.
 */
int DataModel::NotificationCenter::maxHistory() const noexcept
{
  return kMaxHistory;
}

//--------------------------------------------------------------------------------------------------
// Post API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Primary posting entry point; normalizes inputs and fans out to the bus.
 */
void DataModel::NotificationCenter::post(int level,
                                         const QString& channel,
                                         const QString& title,
                                         const QString& subtitle)
{
  // Main-thread only — see class doc; workers must use QueuedConnection
  Q_ASSERT(thread() == QThread::currentThread());

  // Clamp level to enum range
  const int clamped = qBound(static_cast<int>(Info), level, static_cast<int>(Critical));

  // Trim inputs so "power " and "power" don't create two channels
  const QString chan = channel.trimmed();
  const QString ttl  = title.trimmed();

  // Reject empty channel + title — nothing to display
  if (chan.isEmpty() && ttl.isEmpty())
    return;

  // De-dup on full key (subtitle included so rising-value alarms aren't collapsed)
  const auto now = QDateTime::currentMSecsSinceEpoch();
  const DedupKey key{clamped, chan, ttl, subtitle};
  if (shouldDropDuplicate(key, now))
    return;

  // Append to history with current wall-clock timestamp
  Event e;
  e.timestampMs = now;
  e.level       = clamped;
  e.channel     = chan;
  e.title       = ttl;
  e.subtitle    = subtitle;
  appendEvent(std::move(e));
}

/**
 * @brief Convenience wrapper that posts an Info-level event.
 */
void DataModel::NotificationCenter::postInfo(const QString& channel,
                                             const QString& title,
                                             const QString& subtitle)
{
  post(Info, channel, title, subtitle);
}

/**
 * @brief Convenience wrapper that posts a Warning-level event.
 */
void DataModel::NotificationCenter::postWarning(const QString& channel,
                                                const QString& title,
                                                const QString& subtitle)
{
  post(Warning, channel, title, subtitle);
}

/**
 * @brief Convenience wrapper that posts a Critical-level event.
 */
void DataModel::NotificationCenter::postCritical(const QString& channel,
                                                 const QString& title,
                                                 const QString& subtitle)
{
  post(Critical, channel, title, subtitle);
}

/**
 * @brief Emits a companion "resolved" Info event; leaves history intact.
 */
void DataModel::NotificationCenter::resolve(const QString& channel,
                                            const QString& title,
                                            const QString& subtitle)
{
  // Reuse post() so de-dup + fan-out rules apply uniformly
  const QString resolvedTitle = title.trimmed().isEmpty()
                                ? QStringLiteral("Resolved")
                                : QStringLiteral("Resolved: %1").arg(title.trimmed());
  post(Info, channel, resolvedTitle, subtitle);
}

//--------------------------------------------------------------------------------------------------
// History accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns history, optionally filtered by channel and capped to limit.
 */
QVariantList DataModel::NotificationCenter::history(const QString& channel, int limit) const
{
  QVariantList list;
  list.reserve(static_cast<int>(m_history.size()));

  const QString filter = channel.trimmed();
  const bool hasFilter = !filter.isEmpty();

  // Walk newest-to-oldest so the `limit` cap keeps the most recent events
  for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
    if (hasFilter && it->channel != filter)
      continue;

    list.append(toVariant(*it));
    if (limit > 0 && list.size() >= limit)
      break;
  }

  return list;
}

//--------------------------------------------------------------------------------------------------
// Mutation slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears all events posted to the given channel.
 */
void DataModel::NotificationCenter::clearChannel(const QString& channel)
{
  const QString chan = channel.trimmed();
  if (chan.isEmpty())
    return;

  // Erase entries for the channel in a single pass
  int removed = 0;
  for (auto it = m_history.begin(); it != m_history.end();) {
    if (it->channel == chan) {
      it = m_history.erase(it);
      ++removed;
    } else {
      ++it;
    }
  }

  if (removed == 0)
    return;

  // Drop channel from bookkeeping
  m_channelCounts.remove(chan);

  // Reset dedup memory for this channel so identical events can post again.
  for (auto it = m_lastSeen.begin(); it != m_lastSeen.end();)
    if (it.key().channel == chan)
      it = m_lastSeen.erase(it);
    else
      ++it;

  Q_EMIT channelCleared(chan);
  Q_EMIT channelsChanged();
}

/**
 * @brief Drops all events and resets unread count.
 */
void DataModel::NotificationCenter::clearAll()
{
  if (m_history.empty() && m_unreadCount == 0)
    return;

  m_history.clear();
  m_lastSeen.clear();
  m_channelCounts.clear();

  const bool hadUnread = m_unreadCount > 0;
  m_unreadCount        = 0;

  Q_EMIT historyCleared();
  Q_EMIT channelsChanged();
  if (hadUnread)
    Q_EMIT unreadCountChanged();
}

/**
 * @brief Clears the unread badge counter without touching history.
 */
void DataModel::NotificationCenter::markRead()
{
  if (m_unreadCount == 0)
    return;

  m_unreadCount = 0;
  Q_EMIT unreadCountChanged();
}

/**
 * @brief Persists the system-notification preference and lazily creates the tray.
 */
void DataModel::NotificationCenter::setSystemNotificationsEnabled(bool enabled)
{
  if (m_systemNotificationsEnabled == enabled)
    return;

  m_systemNotificationsEnabled = enabled;

  QSettings settings;
  settings.setValue(kSettingsKeySysNotify, enabled);

  if (enabled)
    ensureTrayIcon();

  Q_EMIT systemNotificationsEnabledChanged();
}

/**
 * @brief Persists the route-warnings preference.
 */
void DataModel::NotificationCenter::setRouteWarningsToNotifications(bool enabled)
{
  if (m_routeWarningsToNotifications == enabled)
    return;

  m_routeWarningsToNotifications = enabled;

  QSettings settings;
  settings.setValue(kSettingsKeyRouteWarnings, enabled);

  Q_EMIT routeWarningsToNotificationsChanged();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes an event to a QML-friendly variant map.
 */
QVariantMap DataModel::NotificationCenter::toVariant(const Event& e) const
{
  QVariantMap m;
  m.insert(QStringLiteral("timestamp"), e.timestampMs);
  m.insert(QStringLiteral("level"), e.level);
  m.insert(QStringLiteral("channel"), e.channel);
  m.insert(QStringLiteral("title"), e.title);
  m.insert(QStringLiteral("subtitle"), e.subtitle);
  return m;
}

/**
 * @brief Lazily creates the tray icon using the platform-agnostic logo.
 */
void DataModel::NotificationCenter::ensureTrayIcon()
{
  // Skip if tray already exists or platform lacks a system tray
  if (m_tray || !QSystemTrayIcon::isSystemTrayAvailable())
    return;

  // Use the same logo path as the Qt main window (see app/src/main.cpp)
  QIcon icon(QStringLiteral(":/rcc/logo/icon.svg"));
  if (icon.isNull())
    return;

  m_tray = new QSystemTrayIcon(icon, this);
  m_tray->setToolTip(QCoreApplication::applicationName());
  m_tray->show();
}

/**
 * @brief Returns true when an identical event was posted within the dedup window.
 */
bool DataModel::NotificationCenter::shouldDropDuplicate(const DedupKey& k, qint64 now)
{
  auto it = m_lastSeen.find(k);
  if (it != m_lastSeen.end() && (now - it.value()) < kDedupWindowMs)
    return true;

  m_lastSeen.insert(k, now);
  return false;
}

/**
 * @brief Appends an event, enforces ring-buffer cap, updates counters, fans out.
 */
void DataModel::NotificationCenter::appendEvent(Event&& e)
{
  // Track channel before moving the event
  const QString chan     = e.channel;
  const bool newChannel  = !chan.isEmpty() && !m_channelCounts.contains(chan);
  const int level        = e.level;
  const QString title    = e.title;
  const QString subtitle = e.subtitle;

  // Snapshot for notificationPosted + tray — moves invalidate e
  const QVariantMap variant = toVariant(e);

  // Push into the ring buffer
  m_history.push_back(std::move(e));
  if (m_history.size() > static_cast<size_t>(kMaxHistory)) {
    const auto& evicted = m_history.front();
    auto cit            = m_channelCounts.find(evicted.channel);
    if (cit != m_channelCounts.end()) {
      if (--cit.value() <= 0)
        m_channelCounts.erase(cit);
    }
    m_history.pop_front();
  }

  // Bump per-channel count
  if (!chan.isEmpty())
    m_channelCounts[chan] += 1;

  // Skip unread bumps for Info; background chatter shouldn't raise the badge.
  if (level != Info) {
    ++m_unreadCount;
    Q_EMIT unreadCountChanged();
  }

  Q_EMIT notificationPosted(variant);
  if (newChannel)
    Q_EMIT channelsChanged();

  // Surface Warning/Critical via the OS tray when user opted in
  if (m_systemNotificationsEnabled && level != Info) {
    ensureTrayIcon();
    if (m_tray) {
      const auto icon = (level == Critical) ? QSystemTrayIcon::Critical : QSystemTrayIcon::Warning;
      const QString header = chan.isEmpty() ? title : QStringLiteral("%1 — %2").arg(chan, title);
      m_tray->showMessage(header, subtitle, icon, kTrayTimeoutMs);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Script-API injection helpers (Lua + JS)
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Returns true when the current runtime has a valid Pro-or-higher license.
 */
[[nodiscard]] bool isProTierActive()
{
#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  return tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Pro;
#else
  return false;
#endif
}

/**
 * @brief Resolves positional notify* arguments into (channel, title, subtitle).
 *
 * `baseIdx` is the 1-based index of the first string argument. Arg-count
 * overloads (relative to baseIdx): 1 → (title); 2 → (title, subtitle);
 * 3 → (channel, title, subtitle). The default channel is "Dashboard".
 */
void resolveLuaArgs(lua_State* L, int baseIdx, QString& channel, QString& title, QString& subtitle)
{
  const int total  = lua_gettop(L);
  const int remain = total - (baseIdx - 1);

  if (remain <= 1) {
    channel  = QStringLiteral("Dashboard");
    title    = QString::fromUtf8(luaL_optstring(L, baseIdx, ""));
    subtitle = QString();
  } else if (remain == 2) {
    channel  = QStringLiteral("Dashboard");
    title    = QString::fromUtf8(luaL_optstring(L, baseIdx, ""));
    subtitle = QString::fromUtf8(luaL_optstring(L, baseIdx + 1, ""));
  } else {
    channel  = QString::fromUtf8(luaL_optstring(L, baseIdx, ""));
    title    = QString::fromUtf8(luaL_optstring(L, baseIdx + 1, ""));
    subtitle = QString::fromUtf8(luaL_optstring(L, baseIdx + 2, ""));
  }
}

/**
 * @brief Lua C closure that posts a notification event by level integer.
 */
int luaNotify(lua_State* L)
{
  const int level = static_cast<int>(luaL_checkinteger(L, 1));
  QString channel;
  QString title;
  QString subtitle;
  resolveLuaArgs(L, 2, channel, title, subtitle);

  DataModel::NotificationCenter::instance().post(level, channel, title, subtitle);
  return 0;
}

/**
 * @brief Lua C closure that posts an Info-level notification.
 */
int luaNotifyInfo(lua_State* L)
{
  QString channel;
  QString title;
  QString subtitle;
  resolveLuaArgs(L, 1, channel, title, subtitle);

  DataModel::NotificationCenter::instance().postInfo(channel, title, subtitle);
  return 0;
}

/**
 * @brief Lua C closure that posts a Warning-level notification.
 */
int luaNotifyWarning(lua_State* L)
{
  QString channel;
  QString title;
  QString subtitle;
  resolveLuaArgs(L, 1, channel, title, subtitle);

  DataModel::NotificationCenter::instance().postWarning(channel, title, subtitle);
  return 0;
}

/**
 * @brief Lua C closure that posts a Critical-level notification.
 */
int luaNotifyCritical(lua_State* L)
{
  QString channel;
  QString title;
  QString subtitle;
  resolveLuaArgs(L, 1, channel, title, subtitle);

  DataModel::NotificationCenter::instance().postCritical(channel, title, subtitle);
  return 0;
}

/**
 * @brief Lua C closure that emits a companion "resolved" Info event.
 */
int luaNotifyClear(lua_State* L)
{
  QString channel;
  QString title;
  QString subtitle;
  resolveLuaArgs(L, 1, channel, title, subtitle);

  DataModel::NotificationCenter::instance().resolve(channel, title, subtitle);
  return 0;
}

/**
 * @brief Lua C closure used as the stub when Pro tier is not active.
 */
int luaNotifyStub(lua_State* L)
{
  return luaL_error(L,
                    "notify() requires a Pro license. "
                    "See https://serial-studio.com/pricing");
}

}  // anonymous namespace

/**
 * @brief Installs notify* globals + Info/Warning/Critical constants into a Lua state.
 */
void DataModel::NotificationCenter::installScriptApi(lua_State* L)
{
  Q_ASSERT(L);

  // Always-available integer constants
  lua_pushinteger(L, 0);
  lua_setglobal(L, "Info");
  lua_pushinteger(L, 1);
  lua_setglobal(L, "Warning");
  lua_pushinteger(L, 2);
  lua_setglobal(L, "Critical");

  const bool proActive = isProTierActive();

  lua_CFunction fnNotify   = proActive ? luaNotify : luaNotifyStub;
  lua_CFunction fnInfo     = proActive ? luaNotifyInfo : luaNotifyStub;
  lua_CFunction fnWarning  = proActive ? luaNotifyWarning : luaNotifyStub;
  lua_CFunction fnCritical = proActive ? luaNotifyCritical : luaNotifyStub;
  lua_CFunction fnClear    = proActive ? luaNotifyClear : luaNotifyStub;

  lua_pushcfunction(L, fnNotify);
  lua_setglobal(L, "notify");

  lua_pushcfunction(L, fnInfo);
  lua_setglobal(L, "notifyInfo");

  lua_pushcfunction(L, fnWarning);
  lua_setglobal(L, "notifyWarning");

  lua_pushcfunction(L, fnCritical);
  lua_setglobal(L, "notifyCritical");

  lua_pushcfunction(L, fnClear);
  lua_setglobal(L, "notifyClear");
}

/**
 * @brief Installs notify* globals + Info/Warning/Critical constants into a QJSEngine.
 */
void DataModel::NotificationCenter::installScriptApi(QJSEngine* js)
{
  Q_ASSERT(js);

  // Level constants first so scripts parse-load cleanly regardless of tier
  js->evaluate(QStringLiteral("var Info = 0, Warning = 1, Critical = 2;\n"));

  if (isProTierActive()) {
    // Route through the singleton with CppOwnership; the engine must not delete it.
    auto* nc = &instance();
    QQmlEngine::setObjectOwnership(nc, QQmlEngine::CppOwnership);

    auto global    = js->globalObject();
    auto bridgeVal = js->newQObject(nc);
    global.setProperty(QStringLiteral("__nc"), bridgeVal);

    js->evaluate(
      QStringLiteral("function __ssNotifyArgs(args) {\n"
                     "  // Accept (title), (title, subtitle), or (channel, title, subtitle).\n"
                     "  // 1 arg  → title on 'Dashboard' channel\n"
                     "  // 2 args → (title, subtitle) on 'Dashboard' channel\n"
                     "  // 3 args → (channel, title, subtitle)\n"
                     "  if (args.length <= 1)\n"
                     "    return ['Dashboard', args[0] || '', ''];\n"
                     "  if (args.length === 2)\n"
                     "    return ['Dashboard', args[0] || '', args[1] || ''];\n"
                     "  return [args[0] || '', args[1] || '', args[2] || ''];\n"
                     "}\n"
                     "function notify(level) {\n"
                     "  var rest = Array.prototype.slice.call(arguments, 1);\n"
                     "  var a = __ssNotifyArgs(rest);\n"
                     "  __nc.post(level, a[0], a[1], a[2]);\n"
                     "}\n"
                     "function notifyInfo() {\n"
                     "  var a = __ssNotifyArgs(arguments);\n"
                     "  __nc.postInfo(a[0], a[1], a[2]);\n"
                     "}\n"
                     "function notifyWarning() {\n"
                     "  var a = __ssNotifyArgs(arguments);\n"
                     "  __nc.postWarning(a[0], a[1], a[2]);\n"
                     "}\n"
                     "function notifyCritical() {\n"
                     "  var a = __ssNotifyArgs(arguments);\n"
                     "  __nc.postCritical(a[0], a[1], a[2]);\n"
                     "}\n"
                     "function notifyClear() {\n"
                     "  var a = __ssNotifyArgs(arguments);\n"
                     "  __nc.resolve(a[0], a[1], a[2]);\n"
                     "}\n"));
  } else {
    // Throwing stubs with an actionable message
    js->evaluate(QStringLiteral("function __ssNoPro() {\n"
                                "  throw new Error('notify() requires a Pro license. "
                                "See https://serial-studio.com/pricing');\n"
                                "}\n"
                                "var notify = __ssNoPro;\n"
                                "var notifyInfo = __ssNoPro;\n"
                                "var notifyWarning = __ssNoPro;\n"
                                "var notifyCritical = __ssNoPro;\n"
                                "var notifyClear = __ssNoPro;\n"));
  }
}
