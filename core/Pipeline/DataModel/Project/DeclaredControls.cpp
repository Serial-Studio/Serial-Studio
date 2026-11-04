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

#include "DataModel/Project/DeclaredControls.h"

#include <algorithm>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <utility>

#include "Core/SSAssert.h"
#include "DataModel/Generated/DeclarationVocabulary.h"

namespace Declaration = DataModel::Declaration;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief Longest command name a control may declare, in bytes (the names are ASCII).
 */
static constexpr qsizetype kMaxNameBytes = 32;

/**
 * @brief The template line a declared control's name replaces.
 */
static constexpr QLatin1StringView kMarkedLine("const CONTROL = \"control\";");

//--------------------------------------------------------------------------------------------------
// Command template
//--------------------------------------------------------------------------------------------------

/**
 * @brief The shipped command template, read once from the resource tree; empty when the resource
 *        is missing, which refuses declared controls instead of sending through another script.
 */
[[nodiscard]] static const QString& templateCode()
{
  static const QString code = [] {
    QFile file(QStringLiteral(":/scripts/output/serial_studio_command.js"));
    return file.open(QFile::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
  }();

  return code;
}

//--------------------------------------------------------------------------------------------------
// Names and transmit functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when @p name may be a command name: 1 to 32 of [A-Za-z0-9_ .-], with no leading,
 *        trailing or doubled space, so the project reader's simplification never changes it.
 */
bool DataModel::DeclaredControls::validName(const QString& name)
{
  SS_ASSERT(kMaxNameBytes > 0, return false);
  if (name.isEmpty() || name.size() > kMaxNameBytes || name != name.simplified())
    return false;

  return std::all_of(name.cbegin(), name.cend(), [](const QChar c) {
    const auto u = c.unicode();
    return (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') || (u >= '0' && u <= '9') || u == '_'
        || u == ' ' || u == '.' || u == '-';
  });
}

/**
 * @brief The command template with @p name JSON-encoded into its one marked line, or empty for an
 *        invalid name or a template that lacks the line or carries it twice.
 */
QString DataModel::DeclaredControls::transmitFunction(const QString& name)
{
  const auto& code   = templateCode();
  const auto at      = code.indexOf(kMarkedLine);
  const bool usable  = at >= 0 && code.indexOf(kMarkedLine, at + 1) < 0;
  const auto encoded = QJsonDocument(QJsonArray{name}).toJson(QJsonDocument::Compact);
  if (!validName(name) || !usable)
    return QString();

  SS_ASSERT(encoded.size() > 4, return QString());
  QString function = code;
  function.replace(at,
                   kMarkedLine.size(),
                   QStringLiteral("const CONTROL = %1;")
                     .arg(QString::fromUtf8(encoded.sliced(1, encoded.size() - 2))));
  return function;
}

//--------------------------------------------------------------------------------------------------
// Output groups
//--------------------------------------------------------------------------------------------------

/**
 * @brief Finishes one read output group (spec 0111 R7): within the declaration's control cap, no
 *        group widget and no dataset, every control sending through the command template, and a
 *        text field always a Number field, so spec 0109's admission bounds what it sends.
 */
bool DataModel::DeclaredControls::attach(Group& group, DeviceDeclaration& out)
{
  SS_ASSERT(group.groupType == GroupType::Output, return false);
  SS_ASSERT(Declaration::kMaxControls > 0, return false);

  size_t total = group.outputWidgets.size();
  for (const auto& declared : out.groups)
    total += declared.outputWidgets.size();

  if (group.outputWidgets.empty() || total > static_cast<size_t>(Declaration::kMaxControls)) {
    out.reason =
      QStringLiteral("a declaration may carry 1 to %1 controls").arg(Declaration::kMaxControls);
    return false;
  }

  group.widget.clear();
  group.datasets.clear();
  for (auto& control : group.outputWidgets) {
    if (control.type == OutputWidgetType::TextField)
      control.inputKind = OutputInputKind::Number;

    control.transmitFunction = transmitFunction(control.title);
    if (control.transmitFunction.isEmpty()) {
      out.reason = QStringLiteral("the command template is unavailable");
      return false;
    }
  }

  return true;
}

/**
 * @brief Gives a text declaration with controls its reply channels (spec 0110 through 0111 R7):
 *        three hidden datasets after the last declared position (token, code, message) in a group
 *        of their own, and every control bound to the unique ids that the Quick Plot builder and
 *        a project load both derive from the group and dataset positions.
 */
bool DataModel::DeclaredControls::appendReplies(DeviceDeclaration& out)
{
  SS_ASSERT(out.channels > 0, return false);
  SS_ASSERT(!out.binary, return false);
  const auto hasControls = [](const Group& g) {
    return !g.outputWidgets.empty();
  };
  if (std::none_of(out.groups.cbegin(), out.groups.cend(), hasControls))
    return true;

  if (out.channels + 3 > Declaration::kMaxDatasets
      || std::ssize(out.groups) >= Declaration::kMaxGroups) {
    out.reason = QStringLiteral("no room left for the three control reply channels");
    return false;
  }

  Group replies;
  replies.groupId      = static_cast<int>(out.groups.size());
  replies.title        = QStringLiteral("Control Replies");
  const char* titles[] = {"Reply Token", "Reply Code", "Reply Message"};
  for (int i = 0; i < 3; ++i) {
    Dataset dataset;
    dataset.groupId         = replies.groupId;
    dataset.datasetId       = i;
    dataset.index           = out.channels + i + 1;
    dataset.title           = QLatin1String(titles[i]);
    dataset.hideOnDashboard = true;
    replies.datasets.push_back(std::move(dataset));
  }

  const int token = dataset_unique_id(0, replies.groupId, 0);
  for (auto& group : out.groups)
    for (auto& control : group.outputWidgets) {
      control.replyTokenDatasetId   = token;
      control.replyCodeDatasetId    = dataset_unique_id(0, replies.groupId, 1);
      control.replyMessageDatasetId = dataset_unique_id(0, replies.groupId, 2);
    }

  out.groups.push_back(std::move(replies));
  out.channels += 3;
  return true;
}
