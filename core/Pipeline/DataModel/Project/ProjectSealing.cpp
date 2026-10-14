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

#include "DataModel/Project/ProjectSealing.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "Core/DataModel/FrameKeys.h"
#include "Core/Runtime.h"
#include "Core/SSAssert.h"

/**
 * @brief Removes every listed key from each object of an array; non-objects pass through.
 */
[[nodiscard]] static QJsonArray stripEach(const QJsonArray& array,
                                          std::initializer_list<QLatin1StringView> keys)
{
  QJsonArray out;
  for (const auto& value : array) {
    if (!value.isObject()) {
      out.append(value);
      continue;
    }

    auto object = value.toObject();
    for (const auto key : keys)
      object.remove(key);

    out.append(object);
  }

  SS_ASSERT_LOG(out.size() == array.size());
  return out;
}

/**
 * @brief Strips one group: its Canvas code, its datasets' transforms and its output widgets'
 *        transmit functions.
 */
[[nodiscard]] static QJsonObject stripGroup(QJsonObject group)
{
  group.remove(Keys::PainterCode);
  if (group.contains(Keys::Datasets))
    group.insert(Keys::Datasets,
                 stripEach(group.value(Keys::Datasets).toArray(),
                           {Keys::TransformCode, Keys::TransformParams}));

  if (group.contains(Keys::OutputWidgets))
    group.insert(Keys::OutputWidgets,
                 stripEach(group.value(Keys::OutputWidgets).toArray(), {Keys::TransmitFunction}));

  return group;
}

/**
 * @brief Removes script source, credentials and integration settings from a project document in
 *        place; keys that are absent stay absent, so the rest of the document is unchanged.
 */
void DataModel::stripSealedContent(QJsonObject& project)
{
  for (const auto key : {Keys::ControlScriptCode,
                         Keys::TransformLibrary,
                         Keys::TransformLibraryJs,
                         Keys::PasswordHash,
                         Keys::MqttPublisher,
                         Keys::InfluxSink})
    project.remove(key);

  if (project.contains(Keys::Sources))
    project.insert(Keys::Sources,
                   stripEach(project.value(Keys::Sources).toArray(),
                             {Keys::SourceConn, Keys::FrameParserCode, Keys::FrameParserParams}));

  if (!project.contains(Keys::Groups))
    return;

  const auto groups = project.value(Keys::Groups).toArray();
  QJsonArray stripped;
  for (const auto& group : groups)
    stripped.append(group.isObject() ? QJsonValue(stripGroup(group.toObject())) : group);

  SS_ASSERT_LOG(stripped.size() == groups.size());
  project.insert(Keys::Groups, stripped);
}

/**
 * @brief The struct twin of the JSON strip, for frame templates that are serialized later: clears
 *        the same script, params and connection fields in place.
 */
void DataModel::stripSealedContent(Frame& frame)
{
  frame.controlScriptCode.clear();
  frame.transformLibrary.clear();
  frame.transformLibraryJs.clear();
  for (auto& source : frame.sources) {
    source.connectionSettings = QJsonObject();
    source.frameParserParams  = QJsonObject();
    source.frameParserCode.clear();
  }

  for (auto& group : frame.groups) {
    group.painterCode.clear();
    for (auto& dataset : group.datasets) {
      dataset.transformCode.clear();
      dataset.transformParams.clear();
    }

    for (auto& output : group.outputWidgets)
      output.transmitFunction.clear();
  }
}

/**
 * @brief The copy a recording stores: unchanged in an ordinary session; stripped and marked with
 *        kSealedRecordingKey in a sealed one, so the verify and regress tools can refuse it.
 */
QJsonObject DataModel::sealedCopy(QJsonObject project)
{
  if (!Core::Runtime::sealed())
    return project;

  stripSealedContent(project);
  project.insert(kSealedRecordingKey, true);
  return project;
}

/**
 * @brief True for a stored project that sealedCopy() marked, i.e. a recording made from a
 *        deployment package: its scripts were never stored, so it cannot be re-parsed.
 */
bool DataModel::isSealedRecording(const QString& projectJson)
{
  const auto document = QJsonDocument::fromJson(projectJson.toUtf8());
  return document.isObject() && document.object().value(kSealedRecordingKey).toBool(false);
}
