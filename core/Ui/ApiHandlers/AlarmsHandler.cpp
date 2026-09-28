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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "ApiHandlers/AlarmsHandler.h"

#include <QJsonArray>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/PathPolicy.h"
#include "Core/DataModel/FrameKeys.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "UI/Alarms/AlarmAnnunciator.h"

//--------------------------------------------------------------------------------------------------
// Bound facade
//--------------------------------------------------------------------------------------------------

static const std::unique_ptr<UI::Alarms::AlarmAnnunciator>* s_slot = nullptr;

/**
 * @brief The annunciator the root currently owns, or nullptr before it exists or after release.
 */
[[nodiscard]] static UI::Alarms::AlarmAnnunciator* bound()
{
  return s_slot ? s_slot->get() : nullptr;
}

/**
 * @brief The refusal every verb returns when no annunciator was bound (a schema dump root).
 */
[[nodiscard]] static API::CommandResponse unavailable(const QString& id)
{
  return API::CommandResponse::makeError(
    id, QStringLiteral("UNAVAILABLE"), QStringLiteral("The alarm annunciator is not running"));
}

/**
 * @brief Builds a schema object from a properties map and required-keys list.
 */
[[nodiscard]] static QJsonObject makeSchema(const QJsonObject& properties,
                                            const QJsonArray& required = QJsonArray())
{
  QJsonObject schema;
  schema[QStringLiteral("type")]       = QStringLiteral("object");
  schema[QStringLiteral("properties")] = properties;
  if (!required.isEmpty())
    schema[QStringLiteral("required")] = required;

  return schema;
}

/**
 * @brief The reply shared by the four operator actions.
 */
[[nodiscard]] static QJsonObject actionResult()
{
  QJsonObject r;
  r[QStringLiteral("ok")]                  = true;
  r[QStringLiteral("unacknowledgedCount")] = bound()->unacknowledgedCount();
  r[QStringLiteral("highestPriority")]     = bound()->highestPriority();
  return r;
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the alarms.* commands; @p annunciator is the root's owning slot (may be null).
 */
void API::Handlers::AlarmsHandler::registerCommands(
  const std::unique_ptr<UI::Alarms::AlarmAnnunciator>* annunciator)
{
  s_slot                  = annunciator;
  auto& registry          = API::handlerContext().registry;
  const QJsonObject empty = makeSchema(QJsonObject());

  registry.registerCommand(
    QStringLiteral("alarms.state"),
    QStringLiteral("Annunciator state: sequence, mute, highest priority, unacknowledged count, "
                   "the sounding burst and every active point"),
    empty,
    &state);
  registry.registerCommand(QStringLiteral("alarms.acknowledge"),
                           QStringLiteral("Acknowledge every alerting point (ISA-18.1)"),
                           empty,
                           &acknowledge);
  registry.registerCommand(QStringLiteral("alarms.silence"),
                           QStringLiteral("Silence the audible; points stay unacknowledged"),
                           empty,
                           &silence);
  registry.registerCommand(QStringLiteral("alarms.reset"),
                           QStringLiteral("Reset return-to-normal points (sequences M and R)"),
                           empty,
                           &reset);
  registry.registerCommand(QStringLiteral("alarms.clear"),
                           QStringLiteral("Drop every point, acknowledged ones included"),
                           empty,
                           &clear);
  registry.registerCommand(QStringLiteral("alarms.test"),
                           QStringLiteral("Play every priority sound and walk the annunciator"),
                           empty,
                           &test);

  QJsonObject mutedProps;
  mutedProps[QStringLiteral("muted")] = QJsonObject{
    {       QStringLiteral("type"),                   QStringLiteral("boolean")},
    {QStringLiteral("description"), QStringLiteral("true silences every sound")}
  };
  registry.registerCommand(QStringLiteral("alarms.setMuted"),
                           QStringLiteral("Mute or unmute all alarm and event sounds"),
                           makeSchema(mutedProps, QJsonArray{QStringLiteral("muted")}),
                           &setMuted);

  registry.registerCommand(QStringLiteral("alarms.getProjectSounds"),
                           QStringLiteral("Return the project's sound overrides object"),
                           empty,
                           &getProjectSounds);

  QJsonObject soundsProps;
  soundsProps[QStringLiteral("sounds")] = QJsonObject{
    {       QStringLiteral("type"),QStringLiteral("object")                                 },
    {QStringLiteral("description"),
     QStringLiteral(
     "{ sequence: 'A-4'|'A'|'M-4'|'M'|'R-4'|'R', channels: { <channel>: { warning, caution, "
     "advisory } } } with WAV paths, relative to the project file")}
  };
  registry.registerCommand(
    QStringLiteral("alarms.setProjectSounds"),
    QStringLiteral("Replace the project's sound overrides (validates paths)"),
    makeSchema(soundsProps, QJsonArray{QStringLiteral("sounds")}),
    &setProjectSounds);
}

//--------------------------------------------------------------------------------------------------
// Verbs
//--------------------------------------------------------------------------------------------------

/**
 * @brief alarms.state: the full snapshot.
 */
API::CommandResponse API::Handlers::AlarmsHandler::state(const QString& id, const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  return CommandResponse::makeSuccess(id, QJsonObject::fromVariantMap(bound()->stateSnapshot()));
}

/**
 * @brief alarms.acknowledge.
 */
API::CommandResponse API::Handlers::AlarmsHandler::acknowledge(const QString& id,
                                                               const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  bound()->acknowledge();
  return CommandResponse::makeSuccess(id, actionResult());
}

/**
 * @brief alarms.silence.
 */
API::CommandResponse API::Handlers::AlarmsHandler::silence(const QString& id, const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  bound()->silence();
  return CommandResponse::makeSuccess(id, actionResult());
}

/**
 * @brief alarms.reset.
 */
API::CommandResponse API::Handlers::AlarmsHandler::reset(const QString& id, const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  bound()->reset();
  return CommandResponse::makeSuccess(id, actionResult());
}

/**
 * @brief alarms.clear.
 */
API::CommandResponse API::Handlers::AlarmsHandler::clear(const QString& id, const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  bound()->clear();
  return CommandResponse::makeSuccess(id, actionResult());
}

/**
 * @brief alarms.test.
 */
API::CommandResponse API::Handlers::AlarmsHandler::test(const QString& id, const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  bound()->test();
  return CommandResponse::makeSuccess(id, actionResult());
}

/**
 * @brief alarms.setMuted.
 */
API::CommandResponse API::Handlers::AlarmsHandler::setMuted(const QString& id,
                                                            const QJsonObject& params)
{
  if (!bound())
    return unavailable(id);

  bound()->setMuted(params.value(QStringLiteral("muted")).toBool());
  QJsonObject r;
  r[QStringLiteral("muted")] = bound()->muted();
  return CommandResponse::makeSuccess(id, r);
}

/**
 * @brief alarms.getProjectSounds: the raw project object.
 */
API::CommandResponse API::Handlers::AlarmsHandler::getProjectSounds(const QString& id,
                                                                    const QJsonObject&)
{
  if (!bound())
    return unavailable(id);

  return CommandResponse::makeSuccess(id, DataModel::pipelineModules().projectModel.sounds());
}

/**
 * @brief Applies the one API path policy to every file the channel map names (spec 0075 I3):
 *        a path outside the allowed roots is reported and removed before the object is stored,
 *        so an API client cannot make the app open an arbitrary file.
 */
[[nodiscard]] static QJsonObject stripDisallowedPaths(const QJsonObject& sounds,
                                                      QVariantList& rejected)
{
  QJsonObject channels = sounds.value(Keys::Channels).toObject();
  const QStringList keys{Keys::SoundWarning, Keys::SoundCaution, Keys::SoundAdvisory};
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    QJsonObject entry = it.value().toObject();
    for (const auto& key : keys) {
      const QString raw = entry.value(key).toString().trimmed();
      if (raw.isEmpty())
        continue;

      const QString path = bound()->resolveProjectPath(raw);
      if (!path.isEmpty() && API::isPathAllowed(path, true))
        continue;

      QVariantMap r;
      r.insert(QStringLiteral("channel"), it.key());
      r.insert(QStringLiteral("priority"), key);
      r.insert(QStringLiteral("path"), raw);
      r.insert(QStringLiteral("reason"), QStringLiteral("Path is outside the allowed roots"));
      rejected.append(r);
      entry.remove(key);
    }

    it.value() = entry;
  }

  QJsonObject out = sounds;
  out.insert(Keys::Channels, channels);
  return out;
}

/**
 * @brief Validates every path, stores the object, and reports what will fall back (R12, R14).
 */
API::CommandResponse API::Handlers::AlarmsHandler::setProjectSounds(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!bound())
    return unavailable(id);

  const QJsonValue value = params.value(QStringLiteral("sounds"));
  if (!value.isObject())
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), QStringLiteral("'sounds' must be an object"));

  QVariantList rejected;
  const QJsonObject sounds  = stripDisallowedPaths(value.toObject(), rejected);
  rejected                 += bound()->validateProjectSounds(sounds);
  DataModel::pipelineModules().projectModel.setSounds(sounds);

  QJsonObject r;
  r[QStringLiteral("ok")]       = true;
  r[QStringLiteral("rejected")] = QJsonArray::fromVariantList(rejected);
  return CommandResponse::makeSuccess(id, r);
}
