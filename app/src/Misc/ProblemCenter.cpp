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

#include "Misc/ProblemCenter.h"

#include <algorithm>
#include <QLocale>
#include <QStringList>
#include <QVariantMap>
#include <utility>

#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Problems/ExtensionCheckers.h"
#include "Misc/Problems/LinkCheckers.h"
#include "Misc/Problems/ProjectCheckers.h"
#include "Misc/Problems/ScriptCheckers.h"
#include "Misc/TimerEvents.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Finding comparison
//--------------------------------------------------------------------------------------------------

/**
 * @brief Field-wise equality; drives the "skip the model reset when nothing changed" compare, so
 *        every user-visible field participates.
 */
bool Misc::ProblemCenter::Finding::operator==(const Finding& other) const noexcept
{
  return severity == other.severity && entityUniqueId == other.entityUniqueId && code == other.code
      && jump == other.jump && title == other.title && remedy == other.remedy
      && checkerId == other.checkerId && explanation == other.explanation;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the collector with no dependencies at all: the spec-0001 ctor-edge proof
 *        holds only while this constructor stays a leaf (see the class documentation).
 */
Misc::ProblemCenter::ProblemCenter()
  : m_infoCount(0), m_errorCount(0), m_warningCount(0), m_lastRun(), m_notifications(nullptr)
{}

/**
 * @brief Returns the singleton ProblemCenter instance.
 */
Misc::ProblemCenter& Misc::ProblemCenter::instance()
{
  static ProblemCenter singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Public accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of information-severity findings currently reported.
 */
int Misc::ProblemCenter::infoCount() const noexcept
{
  return m_infoCount;
}

/**
 * @brief Returns the number of error-severity findings currently reported.
 */
int Misc::ProblemCenter::errorCount() const noexcept
{
  return m_errorCount;
}

/**
 * @brief Returns the total number of findings currently reported.
 */
int Misc::ProblemCenter::totalCount() const noexcept
{
  return static_cast<int>(m_findings.size());
}

/**
 * @brief Returns the number of warning-severity findings currently reported.
 */
int Misc::ProblemCenter::warningCount() const noexcept
{
  return m_warningCount;
}

/**
 * @brief Returns the localized time of the last checker run, or an empty string before the first.
 */
QString Misc::ProblemCenter::lastRunTime() const
{
  if (!m_lastRun.isValid())
    return QString();

  return QLocale::system().toString(m_lastRun.time(), QLocale::ShortFormat);
}

/**
 * @brief Returns the current findings for the API handler and the checkers' own bookkeeping.
 */
const QList<Misc::ProblemCenter::Finding>& Misc::ProblemCenter::findings() const noexcept
{
  return m_findings;
}

/**
 * @brief Lists every registered checker as {id, triggers} so an agent can see what runs and when.
 */
QVariantList Misc::ProblemCenter::checkerCatalog() const
{
  QVariantList list;
  list.reserve(static_cast<qsizetype>(m_checkers.size()));

  for (const auto& entry : m_checkers) {
    QStringList triggers;
    if (entry.triggers & ProjectChanged)
      triggers.append(QStringLiteral("projectChanged"));

    if (entry.triggers & LinkSample)
      triggers.append(QStringLiteral("linkSample"));

    if (entry.triggers & OnDemand)
      triggers.append(QStringLiteral("onDemand"));

    QVariantMap map;
    map.insert(QStringLiteral("id"), entry.id);
    map.insert(QStringLiteral("triggers"), triggers);
    list.append(map);
  }

  return list;
}

//--------------------------------------------------------------------------------------------------
// List model interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of findings exposed to the panel.
 */
int Misc::ProblemCenter::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return static_cast<int>(m_findings.size());
}

/**
 * @brief Returns one field of the finding at @p index for the requested role.
 */
QVariant Misc::ProblemCenter::data(const QModelIndex& index, int role) const
{
  const int row = index.row();
  if (row < 0 || row >= static_cast<int>(m_findings.size()))
    return QVariant();

  const auto& finding = m_findings.at(row);
  switch (role) {
    case SeverityRole:
      return static_cast<int>(finding.severity);
    case CodeRole:
      return finding.code;
    case JumpRole:
      return finding.jump;
    case TitleRole:
      return finding.title;
    case RemedyRole:
      return finding.remedy;
    case EntityIdRole:
      return finding.entityUniqueId;
    case CheckerIdRole:
      return finding.checkerId;
    case ExplanationRole:
      return finding.explanation;
    default:
      return QVariant();
  }
}

/**
 * @brief Maps the finding roles to the property names the QML delegate reads.
 */
QHash<int, QByteArray> Misc::ProblemCenter::roleNames() const
{
  return QHash<int, QByteArray>{
    {   SeverityRole,    "severity"},
    {       CodeRole,        "code"},
    {       JumpRole,        "jump"},
    {      TitleRole,       "title"},
    {     RemedyRole,      "remedy"},
    {   EntityIdRole,    "entityId"},
    {  CheckerIdRole,   "checkerId"},
    {ExplanationRole, "explanation"},
  };
}

//--------------------------------------------------------------------------------------------------
// Checker registration & dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers @p checker under @p id, to be run whenever a matching trigger fires. Adding a
 *        checker requires no change to this class, the panel, the badge or the API handler.
 */
void Misc::ProblemCenter::registerChecker(const QString& id, quint8 triggers, Checker checker)
{
  SS_ASSERT(!id.isEmpty(), return);
  SS_ASSERT(triggers != NoTrigger, return);

  if (!checker)
    return;

  m_checkers.push_back(CheckerEntry{id, triggers, std::move(checker)});
  m_checkerFindings.emplace_back();
}

/**
 * @brief Runs every checker registered for @p trigger, replacing each one's previous findings
 *        wholesale. Slices are capped as a defensive bound; a checker that can produce many
 *        findings is expected to cap itself and append its own "and N more" entry.
 */
void Misc::ProblemCenter::run(quint8 trigger)
{
  SS_ASSERT(trigger != NoTrigger, return);
  SS_ASSERT(m_checkers.size() == m_checkerFindings.size(), return);

  QList<Finding> scratch;
  for (size_t i = 0; i < m_checkers.size(); ++i) {
    if ((m_checkers[i].triggers & trigger) == 0)
      continue;

    scratch.clear();
    m_checkers[i].checker(scratch);
    if (scratch.size() > kMaxFindingsPerChecker)
      scratch.resize(kMaxFindingsPerChecker);

    for (auto& finding : scratch)
      finding.checkerId = m_checkers[i].id;

    m_checkerFindings[i] = scratch;
  }

  m_lastRun = QDateTime::currentDateTime();
  rebuildFindings();
  Q_EMIT lastRunChanged();
}

/**
 * @brief Flattens the per-checker slices and publishes them only when the result actually differs
 *        from the previous run: a 1 Hz reset of an unchanged model would repaint the panel every
 *        second.
 */
void Misc::ProblemCenter::rebuildFindings()
{
  SS_ASSERT(m_checkers.size() == m_checkerFindings.size(), return);

  QList<Finding> flattened;
  for (const auto& slice : m_checkerFindings)
    flattened.append(slice);

  if (flattened == m_findings)
    return;

  beginResetModel();
  m_findings = std::move(flattened);
  endResetModel();

  m_infoCount    = 0;
  m_errorCount   = 0;
  m_warningCount = 0;
  for (const auto& finding : m_findings)
    if (finding.severity == Error)
      ++m_errorCount;
    else if (finding.severity == Warning)
      ++m_warningCount;
    else
      ++m_infoCount;

  notifyNewFindings();
  Q_EMIT findingsChanged();
}

/**
 * @brief Builds the dedup key that decides whether a finding is new since the previous run.
 */
QString Misc::ProblemCenter::findingKey(const Finding& finding)
{
  return QStringLiteral("%1/%2/%3")
    .arg(finding.checkerId, finding.code, QString::number(finding.entityUniqueId));
}

/**
 * @brief Posts one aggregated notification per run that introduced findings; an unchanged re-run
 *        and a run that only cleared findings post nothing, since the panel and the badge already
 *        show the change.
 */
void Misc::ProblemCenter::notifyNewFindings()
{
  QSet<QString> keys;
  keys.reserve(m_findings.size());

  int added = 0;
  int level = Info;
  for (const auto& finding : m_findings) {
    const auto key = findingKey(finding);
    keys.insert(key);
    if (m_previousKeys.contains(key))
      continue;

    ++added;
    level = std::max(level, static_cast<int>(finding.severity));
  }

  m_previousKeys = std::move(keys);
  if (added == 0)
    return;

  SS_ASSERT(m_notifications != nullptr, return);

  m_notifications->post(
    level,
    QStringLiteral("Problems"),
    tr("Problems detected"),
    tr("%1 new problem(s) were detected. Open the problem center to review them.").arg(added));
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every standing finding without re-running any checker. A condition that still
 *        holds reappears on its next trigger; the dedup keys survive so neither the dismissal
 *        nor the reappearance posts a notification.
 */
void Misc::ProblemCenter::clear()
{
  SS_ASSERT(m_checkers.size() == m_checkerFindings.size(), return);

  for (auto& slice : m_checkerFindings)
    slice.clear();

  if (m_findings.isEmpty())
    return;

  beginResetModel();
  m_findings.clear();
  endResetModel();

  m_infoCount    = 0;
  m_errorCount   = 0;
  m_warningCount = 0;
  Q_EMIT findingsChanged();
}

/**
 * @brief Re-runs every registered checker regardless of its trigger mask.
 */
void Misc::ProblemCenter::runNow()
{
  run(ProjectChanged | LinkSample | OnDemand);
}

/**
 * @brief Runs the checkers that sample link statistics; wired to the shared 1 Hz tick.
 */
void Misc::ProblemCenter::onLinkSample()
{
  run(LinkSample);
}

/**
 * @brief Runs the checkers that inspect the project document after a load, edit or save.
 */
void Misc::ProblemCenter::onProjectChanged()
{
  run(ProjectChanged);
}

/**
 * @brief Requests navigation to the entity a finding points at; QML performs the navigation so
 *        this class keeps no dependency on the project editor. Returns false when the finding
 *        carries no jump target.
 */
bool Misc::ProblemCenter::activate(int row)
{
  if (row < 0 || row >= static_cast<int>(m_findings.size()))
    return false;

  const auto& finding = m_findings.at(row);
  if (finding.jump.isEmpty())
    return false;

  Q_EMIT jumpRequested(finding.jump, finding.entityUniqueId);
  return true;
}

/**
 * @brief Registers the built-in checkers and wires the project-change signals and the shared 1 Hz
 *        sample tick. Called once from the composition root after every module exists and before
 *        the last project is restored; the constructor stays inert.
 */
void Misc::ProblemCenter::setupExternalConnections()
{
  Misc::ProjectCheckers::registerAll();
  Misc::LinkCheckers::registerAll();
  Misc::ScriptCheckers::registerAll();
  Misc::ExtensionCheckers::registerAll();

  m_notifications = &DataModel::NotificationCenter::instance();

  auto* project = &DataModel::ProjectModel::instance();
  connect(
    project, &DataModel::ProjectModel::groupsChanged, this, &Misc::ProblemCenter::onProjectChanged);
  connect(project,
          &DataModel::ProjectModel::jsonFileChanged,
          this,
          &Misc::ProblemCenter::onProjectChanged);
  connect(project,
          &DataModel::ProjectModel::frameDetectionChanged,
          this,
          &Misc::ProblemCenter::onProjectChanged);

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &Misc::ProblemCenter::onLinkSample);
}
