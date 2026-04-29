/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/NotificationsHandler.h"

#  include <QJsonArray>
#  include <QJsonObject>

#  include "API/CommandRegistry.h"
#  include "DataModel/NotificationCenter.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Reusable "accepts channel/title/subtitle strings" schema chunk.
 */
[[nodiscard]] QJsonObject stringProp(const QString& description)
{
  QJsonObject p;
  p[QStringLiteral("type")]        = QStringLiteral("string");
  p[QStringLiteral("description")] = description;
  return p;
}

/**
 * @brief Builds a schema object from a properties map and required-keys list.
 */
[[nodiscard]] QJsonObject makeSchema(const QJsonObject& properties, const QJsonArray& required)
{
  QJsonObject schema;
  schema[QStringLiteral("type")]       = QStringLiteral("object");
  schema[QStringLiteral("properties")] = properties;
  if (!required.isEmpty())
    schema[QStringLiteral("required")] = required;

  return schema;
}

/**
 * @brief Reads channel, title, subtitle from params (all optional, default empty).
 */
void readEventStrings(const QJsonObject& params, QString& channel, QString& title, QString& sub)
{
  channel = params.value(QStringLiteral("channel")).toString();
  title   = params.value(QStringLiteral("title")).toString();
  sub     = params.value(QStringLiteral("subtitle")).toString();
}

}  // anonymous namespace

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all notifications.* commands with the command registry.
 */
void API::Handlers::NotificationsHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  // Shared event-shape property set
  QJsonObject eventProps;
  eventProps[QStringLiteral("channel")]  = stringProp(QStringLiteral("Channel ID (free-form)"));
  eventProps[QStringLiteral("title")]    = stringProp(QStringLiteral("Event title"));
  eventProps[QStringLiteral("subtitle")] = stringProp(QStringLiteral("Event detail (optional)"));

  // notifications.post — full form with level
  {
    QJsonObject props              = eventProps;
    props[QStringLiteral("level")] = QJsonObject{
      {       QStringLiteral("type"),                             QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("0 = Info, 1 = Warning, 2 = Critical")}
    };
    registry.registerCommand(QStringLiteral("notifications.post"),
                             QStringLiteral("Post a notification (params: level, channel, title, "
                                            "subtitle)"),
                             makeSchema(props, QJsonArray{QStringLiteral("level")}),
                             &post);
  }

  // notifications.postInfo / postWarning / postCritical
  registry.registerCommand(QStringLiteral("notifications.postInfo"),
                           QStringLiteral("Post an Info-level notification"),
                           makeSchema(eventProps, QJsonArray()),
                           &postInfo);
  registry.registerCommand(QStringLiteral("notifications.postWarning"),
                           QStringLiteral("Post a Warning-level notification"),
                           makeSchema(eventProps, QJsonArray()),
                           &postWarning);
  registry.registerCommand(QStringLiteral("notifications.postCritical"),
                           QStringLiteral("Post a Critical-level notification"),
                           makeSchema(eventProps, QJsonArray()),
                           &postCritical);

  // notifications.resolve — emit a companion Info "Resolved: <title>"
  registry.registerCommand(QStringLiteral("notifications.resolve"),
                           QStringLiteral("Emit a companion 'Resolved' Info event"),
                           makeSchema(eventProps, QJsonArray()),
                           &resolve);

  // notifications.list — optional channel filter + limit
  {
    QJsonObject props;
    props[QStringLiteral("channel")] = stringProp(QStringLiteral("Filter by channel (optional)"));
    props[QStringLiteral("limit")]   = QJsonObject{
        {       QStringLiteral("type"),QStringLiteral("integer")                                       },
        {QStringLiteral("description"),
         QStringLiteral("Max entries to return (0 = all, default 0)")}
    };
    registry.registerCommand(
      QStringLiteral("notifications.list"),
      QStringLiteral("List historical events, newest first (params: channel?, limit?)"),
      makeSchema(props, QJsonArray()),
      &list);
  }

  // notifications.channels
  registry.registerCommand(QStringLiteral("notifications.channels"),
                           QStringLiteral("List channel IDs currently present in history"),
                           emptySchema,
                           &channels);

  // notifications.unreadCount
  registry.registerCommand(QStringLiteral("notifications.unreadCount"),
                           QStringLiteral("Return the number of unread Warning/Critical events"),
                           emptySchema,
                           &unreadCount);

  // notifications.clearChannel
  {
    QJsonObject props;
    props[QStringLiteral("channel")] = stringProp(QStringLiteral("Channel to clear"));
    registry.registerCommand(QStringLiteral("notifications.clearChannel"),
                             QStringLiteral("Erase every event posted to the given channel"),
                             makeSchema(props, QJsonArray{QStringLiteral("channel")}),
                             &clearChannel);
  }

  // notifications.clearAll
  registry.registerCommand(QStringLiteral("notifications.clearAll"),
                           QStringLiteral("Erase the entire notification history"),
                           emptySchema,
                           &clearAll);

  // notifications.markRead
  registry.registerCommand(QStringLiteral("notifications.markRead"),
                           QStringLiteral("Clear the unread badge counter"),
                           emptySchema,
                           &markRead);
}

//--------------------------------------------------------------------------------------------------
// Post handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Post a notification at the explicitly specified level.
 */
API::CommandResponse API::Handlers::NotificationsHandler::post(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("level")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: level"));

  const int level = params.value(QStringLiteral("level")).toInt(0);
  QString channel, title, subtitle;
  readEventStrings(params, channel, title, subtitle);

  DataModel::NotificationCenter::instance().post(level, channel, title, subtitle);

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("posted"), true}
  });
}

/**
 * @brief Post an Info-level notification.
 */
API::CommandResponse API::Handlers::NotificationsHandler::postInfo(const QString& id,
                                                                   const QJsonObject& params)
{
  QString channel, title, subtitle;
  readEventStrings(params, channel, title, subtitle);

  DataModel::NotificationCenter::instance().postInfo(channel, title, subtitle);

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("posted"), true}
  });
}

/**
 * @brief Post a Warning-level notification.
 */
API::CommandResponse API::Handlers::NotificationsHandler::postWarning(const QString& id,
                                                                      const QJsonObject& params)
{
  QString channel, title, subtitle;
  readEventStrings(params, channel, title, subtitle);

  DataModel::NotificationCenter::instance().postWarning(channel, title, subtitle);

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("posted"), true}
  });
}

/**
 * @brief Post a Critical-level notification.
 */
API::CommandResponse API::Handlers::NotificationsHandler::postCritical(const QString& id,
                                                                       const QJsonObject& params)
{
  QString channel, title, subtitle;
  readEventStrings(params, channel, title, subtitle);

  DataModel::NotificationCenter::instance().postCritical(channel, title, subtitle);

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("posted"), true}
  });
}

/**
 * @brief Emit a companion Info-level event marking the issue as resolved.
 */
API::CommandResponse API::Handlers::NotificationsHandler::resolve(const QString& id,
                                                                  const QJsonObject& params)
{
  QString channel, title, subtitle;
  readEventStrings(params, channel, title, subtitle);

  DataModel::NotificationCenter::instance().resolve(channel, title, subtitle);

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("resolved"), true}
  });
}

//--------------------------------------------------------------------------------------------------
// Query handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Return historical events, newest first, optionally filtered by channel.
 */
API::CommandResponse API::Handlers::NotificationsHandler::list(const QString& id,
                                                               const QJsonObject& params)
{
  const QString channel = params.value(QStringLiteral("channel")).toString();
  const int limit       = params.value(QStringLiteral("limit")).toInt(0);

  const auto events = DataModel::NotificationCenter::instance().history(channel, limit);

  QJsonObject result;
  result[QStringLiteral("events")] = QJsonArray::fromVariantList(events);
  result[QStringLiteral("count")]  = static_cast<int>(events.size());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List the distinct channel IDs currently present in history.
 */
API::CommandResponse API::Handlers::NotificationsHandler::channels(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto list = DataModel::NotificationCenter::instance().channels();

  QJsonArray arr;
  for (const auto& c : list)
    arr.append(c);

  QJsonObject result;
  result[QStringLiteral("channels")] = arr;
  result[QStringLiteral("count")]    = static_cast<int>(list.size());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Return the number of unread Warning/Critical events.
 */
API::CommandResponse API::Handlers::NotificationsHandler::unreadCount(const QString& id,
                                                                      const QJsonObject& params)
{
  Q_UNUSED(params)

  const int unread = DataModel::NotificationCenter::instance().unreadCount();

  QJsonObject result;
  result[QStringLiteral("unreadCount")] = unread;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Mutation handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Erase every event posted to the given channel.
 */
API::CommandResponse API::Handlers::NotificationsHandler::clearChannel(const QString& id,
                                                                       const QJsonObject& params)
{
  const QString channel = params.value(QStringLiteral("channel")).toString();
  if (channel.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: channel"));

  DataModel::NotificationCenter::instance().clearChannel(channel);

  QJsonObject result;
  result[QStringLiteral("channel")] = channel;
  result[QStringLiteral("cleared")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Erase the entire notification history.
 */
API::CommandResponse API::Handlers::NotificationsHandler::clearAll(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::NotificationCenter::instance().clearAll();

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("cleared"), true}
  });
}

/**
 * @brief Clear the unread badge counter.
 */
API::CommandResponse API::Handlers::NotificationsHandler::markRead(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::NotificationCenter::instance().markRead();

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("marked"), true}
  });
}

#endif  // BUILD_COMMERCIAL
