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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/ProjectModel.h"

#include <algorithm>
#include <QApplication>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#endif

/**
 * @brief Returns a unique title for a duplicated item using a numbered " (N)" suffix.
 */
static QString nextDuplicateTitle(const QString& title, const QStringList& taken)
{
  static const QRegularExpression kSuffixRe(QStringLiteral("^(.*?)\\s*\\((\\d+)\\)\\s*$"));

  QString base        = title;
  const auto stripped = kSuffixRe.match(title);
  if (stripped.hasMatch())
    base = stripped.captured(1).trimmed();

  const QString basePattern = QStringLiteral("^") + QRegularExpression::escape(base)
                            + QStringLiteral("(?:\\s*\\((\\d+)\\))?\\s*$");
  const QRegularExpression baseRe(basePattern);

  int maxN = -1;
  for (const auto& t : taken) {
    const auto m = baseRe.match(t);
    if (!m.hasMatch())
      continue;

    const QString suffix = m.captured(1);
    if (suffix.isEmpty()) {
      maxN = qMax(maxN, 0);
      continue;
    }

    bool ok     = false;
    const int n = suffix.toInt(&ok);
    if (ok)
      maxN = qMax(maxN, n);
  }

  if (maxN < 0)
    return QStringLiteral("%1 (1)").arg(base);

  return QStringLiteral("%1 (%2)").arg(base, QString::number(maxN + 1));
}

/**
 * @brief Increments the per-type counter for every eligible dataset widget.
 */
static void tallyDatasetWidgetTypes(const DataModel::Dataset& ds, QMap<int, int>& counts)
{
  const auto keys = SerialStudio::getDashboardWidgets(ds);
  for (const auto& k : keys)
    if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
      counts[static_cast<int>(k)] += 1;
}

/**
 * @brief Appends a dataset widget ref unless the widget type is the LED aggregator.
 */
static bool appendDatasetRef(SerialStudio::DashboardWidget k,
                             int groupUniqueId,
                             QMap<SerialStudio::DashboardWidget, int>& datasetIdx,
                             std::vector<DataModel::WidgetRef>& groupRefs,
                             std::vector<DataModel::WidgetRef>& allRefs)
{
  if (k == SerialStudio::DashboardLED)
    return true;

  if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
    return false;

  DataModel::WidgetRef r;
  r.widgetType    = static_cast<int>(k);
  r.groupUniqueId = groupUniqueId;
  r.relativeIndex = datasetIdx.value(k, 0);
  datasetIdx[k]   = r.relativeIndex + 1;

  groupRefs.push_back(r);
  allRefs.push_back(r);
  return false;
}

/**
 * @brief Collects per-dataset widget refs for a group, returning whether any LED is present.
 */
static bool collectGroupDatasetRefs(const DataModel::Group& group,
                                    QMap<SerialStudio::DashboardWidget, int>& datasetIdx,
                                    std::vector<DataModel::WidgetRef>& groupRefs,
                                    std::vector<DataModel::WidgetRef>& allRefs)
{
  bool groupHasLed = false;
  for (const auto& ds : group.datasets) {
    if (ds.hideOnDashboard)
      continue;

    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys)
      if (appendDatasetRef(k, group.uniqueId, datasetIdx, groupRefs, allRefs))
        groupHasLed = true;
  }
  return groupHasLed;
}

/**
 * @brief Pushes a tracked widget ref into the supplied output vectors.
 */
static void pushTrackedRef(SerialStudio::DashboardWidget key,
                           int groupUniqueId,
                           QMap<SerialStudio::DashboardWidget, int>& runningIdx,
                           std::vector<DataModel::WidgetRef>& groupRefs,
                           std::vector<DataModel::WidgetRef>& allRefs,
                           std::vector<DataModel::WidgetRef>& overviewRefs)
{
  DataModel::WidgetRef r;
  r.widgetType    = static_cast<int>(key);
  r.groupUniqueId = groupUniqueId;
  r.relativeIndex = runningIdx.value(key, 0);
  runningIdx[key] = r.relativeIndex + 1;

  groupRefs.push_back(r);
  allRefs.push_back(r);
  overviewRefs.push_back(r);
}

namespace detail {

/**
 * @brief Layout config for fixed three-axis group widgets (Accel/Gyro/GPS/Plot3D).
 */
struct ThreeAxisLayout {
  const char* widgetTag;
  const char* axisWidgets[3];
  QString units[3];
  QString titles[3];
  double wgtMin[3];
  double wgtMax[3];
  bool plt;
};

}  // namespace detail

using detail::ThreeAxisLayout;

/**
 * @brief Populates a group with three canonical axis datasets per supplied layout.
 */
static void populateThreeAxisDatasets(DataModel::Group& grp,
                                      int baseIndex,
                                      const ThreeAxisLayout& layout)
{
  grp.widget = QString::fromUtf8(layout.widgetTag);

  DataModel::Dataset axes[3];
  for (int i = 0; i < 3; ++i) {
    axes[i].datasetId = i;
    axes[i].groupId   = grp.groupId;
    axes[i].index     = baseIndex + i;
    axes[i].units     = layout.units[i];
    axes[i].widget    = QString::fromUtf8(layout.axisWidgets[i]);
    axes[i].title     = layout.titles[i];
    axes[i].wgtMin    = layout.wgtMin[i];
    axes[i].wgtMax    = layout.wgtMax[i];
    axes[i].plt       = layout.plt;

    grp.datasets.push_back(axes[i]);
  }
}

/**
 * @brief Builds widget refs for one group during auto-workspace synthesis.
 */
static std::vector<DataModel::WidgetRef> buildAutoRefsForGroup(
  const DataModel::Group& group,
  bool pro,
  QMap<SerialStudio::DashboardWidget, int>& groupIdx,
  QMap<SerialStudio::DashboardWidget, int>& datasetIdx,
  std::vector<DataModel::WidgetRef>& allRefs,
  std::vector<DataModel::WidgetRef>& overviewRefs)
{
  std::vector<DataModel::WidgetRef> groupRefs;

  auto groupKey = SerialStudio::getDashboardWidget(group);
  if (groupKey == SerialStudio::DashboardPlot3D && !pro)
    groupKey = SerialStudio::DashboardMultiPlot;

  const bool isEmptyOutputPanel =
    group.groupType == DataModel::GroupType::Output && group.outputWidgets.empty();

  if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
    pushTrackedRef(groupKey, group.uniqueId, groupIdx, groupRefs, allRefs, overviewRefs);

  const bool groupHasLed = collectGroupDatasetRefs(group, datasetIdx, groupRefs, allRefs);

  if (groupHasLed)
    pushTrackedRef(
      SerialStudio::DashboardLED, group.uniqueId, groupIdx, groupRefs, allRefs, overviewRefs);

  return groupRefs;
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton instance access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ProjectModel singleton and seeds an empty project.
 */
DataModel::ProjectModel::ProjectModel()
  : m_title("")
  , m_frameEndSequence("")
  , m_checksumAlgorithm("")
  , m_frameStartSequence("")
  , m_writerVersionAtCreation("")
  , m_hexadecimalDelimiters(false)
  , m_frameDecoder(SerialStudio::PlainText)
  , m_frameDetection(SerialStudio::EndDelimiterOnly)
  , m_pointCount(100)
  , m_plotTimeRange(10.0)
  , m_nextUniqueId(1)
  , m_modified(false)
  , m_silentReload(false)
  , m_filePath("")
  , m_suppressMessageBoxes(false)
  , m_customizeWorkspaces(false)
  , m_passwordHash("")
  , m_locked(false)
  , m_apiCallAllowFullSurface(false)
  , m_autoSaveTimer(new QTimer(this))
  , m_autoSaveSuspended(false)
  , m_mutationEpoch(0)
{
  m_autoSaveTimer->setSingleShot(true);
  m_autoSaveTimer->setInterval(1500);
  connect(m_autoSaveTimer, &QTimer::timeout, this, &ProjectModel::autoSave);

  const auto bumpEpoch = [this] {
    ++m_mutationEpoch;
  };
  connect(this, &ProjectModel::groupAdded, this, bumpEpoch);
  connect(this, &ProjectModel::groupDeleted, this, bumpEpoch);
  connect(this, &ProjectModel::datasetAdded, this, bumpEpoch);
  connect(this, &ProjectModel::datasetDeleted, this, bumpEpoch);
  connect(this, &ProjectModel::sourceAdded, this, bumpEpoch);
  connect(this, &ProjectModel::sourceDeleted, this, bumpEpoch);
  connect(this, &ProjectModel::sourceStructureChanged, this, bumpEpoch);
  connect(this, &ProjectModel::groupsChanged, this, bumpEpoch);

  connect(this, &ProjectModel::titleChanged, this, &ProjectModel::saveStatusChanged);
  connect(this, &ProjectModel::groupsChanged, this, &ProjectModel::saveStatusChanged);

  connect(this, &ProjectModel::widgetSettingsChanged, this, [this] {
    if (m_autoSaveSuspended || m_filePath.isEmpty() || m_locked)
      return;

    if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
      return;

    m_autoSaveTimer->start();
  });

  // code-verify off
  // Must run before the groupsChanged auto-regen connect below: newJsonFile()
  // emits groupsChanged while AppState is still mid-init, so wiring first would
  // fire the regen handler against half-initialized state.
  newJsonFile();
  // code-verify on

  connect(this, &ProjectModel::groupsChanged, this, [this] {
    if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
      return;

    if (m_customizeWorkspaces) {
      if (mergeAutoWorkspaceUpdates()) {
        Q_EMIT editorWorkspacesChanged();
        Q_EMIT activeWorkspacesChanged();
      }
      return;
    }

    regenerateAutoWorkspacesUnnotified();
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
  });
}

/**
 * @brief Returns the singleton ProjectModel instance.
 */
DataModel::ProjectModel& DataModel::ProjectModel::instance()
{
  static ProjectModel singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Document status
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the project has unsaved edits.
 */
bool DataModel::ProjectModel::modified() const noexcept
{
  return m_modified;
}

/**
 * @brief Returns the project-wide payload decoder method.
 */
SerialStudio::DecoderMethod DataModel::ProjectModel::decoderMethod() const noexcept
{
  return m_frameDecoder;
}

/**
 * @brief Returns the project-wide frame detection strategy.
 */
SerialStudio::FrameDetection DataModel::ProjectModel::frameDetection() const noexcept
{
  return m_frameDetection;
}

/**
 * @brief Returns @c true if the project configuration is sufficient to generate a valid
 *        dashboard configuration
 */
bool DataModel::ProjectModel::validateProject(const bool silent)
{
  if (m_title.isEmpty()) {
    if (!silent) {
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("Project title cannot be empty!"), QMessageBox::Warning);
    }

    return false;
  }

  if (groupCount() <= 0) {
    if (!silent) {
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one group!"), QMessageBox::Warning);
    }

    return false;
  }

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup) {
    if (!silent) {
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one dataset!"), QMessageBox::Warning);
    }

    return false;
  }

  return true;
}

/**
 * @brief Identifies which (if any) save prerequisite is currently missing.
 */
DataModel::ProjectModel::SaveBlocker DataModel::ProjectModel::saveBlockerCode() const
{
  if (m_title.isEmpty())
    return SaveBlocker::MissingTitle;

  if (groupCount() <= 0)
    return SaveBlocker::MissingGroup;

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup)
    return SaveBlocker::MissingDataset;

  return SaveBlocker::None;
}

/**
 * @brief Returns true when the project has everything saveJsonFile() needs.
 */
bool DataModel::ProjectModel::canSave() const
{
  return saveBlockerCode() == SaveBlocker::None;
}

/**
 * @brief Returns a short, HIG-style heading describing the current save blocker.
 */
QString DataModel::ProjectModel::saveBlockerTitle() const
{
  switch (saveBlockerCode()) {
    case SaveBlocker::None:
      return QString();
    case SaveBlocker::MissingTitle:
      return tr("Your project needs a title");
    case SaveBlocker::MissingGroup:
      return tr("Add a group to get started");
    case SaveBlocker::MissingDataset:
      return tr("Add a dataset to a group");
  }
  return QString();
}

/**
 * @brief Returns one or two sentences of HIG-style guidance on how to fix the blocker.
 */
QString DataModel::ProjectModel::saveBlockerDetail() const
{
  switch (saveBlockerCode()) {
    case SaveBlocker::None:
      return QString();
    case SaveBlocker::MissingTitle:
      return tr("Open the Project view at the top of the tree and enter "
                "a name. You can rename the project at any time.");
    case SaveBlocker::MissingGroup:
      return tr("Groups organize datasets into dashboard widgets. Use the "
                "Group button in the toolbar above to create one, then add "
                "datasets to it.");
    case SaveBlocker::MissingDataset:
      return tr("Datasets are the values that appear on the dashboard. "
                "Select a group in the tree and use the Dataset button in "
                "the toolbar to add one.");
  }
  return QString();
}

//--------------------------------------------------------------------------------------------------
// Project lock: UX read-only flag (plain MD5, not crypto)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the editor body is gated behind the lock screen.
 */
bool DataModel::ProjectModel::locked() const noexcept
{
  return m_locked;
}

/**
 * @brief Monotonic counter bumped on every structural project mutation.
 */
qint64 DataModel::ProjectModel::mutationEpoch() const noexcept
{
  return m_mutationEpoch;
}

/**
 * @brief Prompts for a password, hashes it, and locks the editor.
 */
void DataModel::ProjectModel::lockProject()
{
  bool ok          = false;
  const auto first = QInputDialog::getText(nullptr,
                                           tr("Lock Project"),
                                           tr("Choose a password to lock the project:"),
                                           QLineEdit::Password,
                                           QString(),
                                           &ok);
  if (!ok || first.isEmpty())
    return;

  const auto second = QInputDialog::getText(
    nullptr, tr("Lock Project"), tr("Confirm the password:"), QLineEdit::Password, QString(), &ok);

  if (first != second || !ok) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Passwords do not match"),
        tr("The two passwords you entered do not match. The project was not locked."),
        QMessageBox::Warning);
    });
    return;
  }

  m_passwordHash = Misc::PasswordHash::hashPassword(first);

  if (!m_locked) {
    m_locked = true;
    Q_EMIT lockedChanged();
  }

  if (validateProject(true)) {
    setModified(true);
    (void)saveJsonFile(false);
  }
}

/**
 * @brief Prompts for the password, verifies it, and clears the lock on success.
 */
void DataModel::ProjectModel::unlockProject()
{
  if (m_passwordHash.isEmpty()) {
    if (m_locked) {
      m_locked = false;
      Q_EMIT lockedChanged();
    }
    return;
  }

  bool ok        = false;
  const auto pwd = QInputDialog::getText(nullptr,
                                         tr("Unlock Project"),
                                         tr("Enter the project password:"),
                                         QLineEdit::Password,
                                         QString(),
                                         &ok);
  if (!ok)
    return;

  if (!Misc::PasswordHash::verifyPassword(pwd, m_passwordHash)) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Incorrect password"),
        tr("The password you entered does not match the one stored in the project file."),
        QMessageBox::Warning);
    });
    return;
  }

  m_passwordHash.clear();

  if (m_locked) {
    m_locked = false;
    Q_EMIT lockedChanged();
  }

  if (validateProject(true)) {
    setModified(true);
    (void)saveJsonFile(false);
  }
}

//--------------------------------------------------------------------------------------------------
// Document information
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the project filename, or "New Project" when none is loaded.
 */
QString DataModel::ProjectModel::jsonFileName() const
{
  if (!m_filePath.isEmpty())
    return QFileInfo(m_filePath).fileName();

  return tr("New Project");
}

/**
 * @brief Returns the workspace folder used for project files.
 */
QString DataModel::ProjectModel::jsonProjectsPath() const
{
  return Misc::WorkspaceManager::instance().path("Projects");
}

/**
 * @brief Allocates the next persistent uniqueId for a new group or dataset.
 */
int DataModel::ProjectModel::allocateUniqueId()
{
  return m_nextUniqueId++;
}

/**
 * @brief Back-fills missing uniqueIds and bumps the allocator past every assigned uid.
 */
void DataModel::ProjectModel::seedNextUniqueIdFromGroups()
{
  int maxUid = 0;

  for (auto& group : m_groups) {
    for (auto& dataset : group.datasets) {
      if (dataset.uniqueId < 0)
        dataset.uniqueId = dataset_unique_id(group.sourceId, dataset.groupId, dataset.datasetId);

      if (dataset.uniqueId > maxUid)
        maxUid = dataset.uniqueId;
    }

    if (group.uniqueId > maxUid)
      maxUid = group.uniqueId;
  }

  if (m_nextUniqueId <= maxUid)
    m_nextUniqueId = maxUid + 1;

  for (auto& group : m_groups)
    if (group.uniqueId < 0)
      group.uniqueId = m_nextUniqueId++;
}

/**
 * @brief Rewrites workspace refs whose groupUniqueId is actually a legacy positional groupId.
 */
void DataModel::ProjectModel::migrateLegacyWorkspaceRefs()
{
  for (auto& ws : m_workspaces) {
    for (auto& ref : ws.widgetRefs) {
      const int pos = ref.groupUniqueId;
      if (pos < 0 || static_cast<size_t>(pos) >= m_groups.size())
        continue;

      ref.groupUniqueId = m_groups[static_cast<size_t>(pos)].uniqueId;
    }
  }
}

/**
 * @brief Rebinds legacy index-based X-axis references to dataset uniqueIds.
 */
void DataModel::ProjectModel::migrateLegacyXAxisIds()
{
  QMap<int, QMap<int, int>> uidByIndex;
  for (const auto& group : m_groups)
    for (const auto& dataset : group.datasets)
      uidByIndex[group.sourceId].insert(dataset.index, dataset.uniqueId);

  for (auto& group : m_groups) {
    const auto indexMap = uidByIndex.value(group.sourceId);
    for (auto& dataset : group.datasets) {
      if (dataset.xAxisId == kXAxisTime)
        continue;

      if (dataset.xAxisId <= 0)
        dataset.xAxisId = kXAxisTime;
      else
        dataset.xAxisId = indexMap.value(dataset.xAxisId, kXAxisTime);
    }
  }
}

/**
 * @brief Translates one dataset's legacy index-based waterfall Y-axis to a uniqueId.
 */
static void remapWaterfallYAxisId(DataModel::Dataset& dataset,
                                  const QSet<int>& liveUids,
                                  const QMap<int, int>& indexMap)
{
  if (dataset.waterfallYAxis <= 0)
    dataset.waterfallYAxis = 0;
  else if (!liveUids.contains(dataset.waterfallYAxis))
    dataset.waterfallYAxis = indexMap.value(dataset.waterfallYAxis, 0);
}

/**
 * @brief Rebinds legacy index-based waterfall Y-axis references to dataset uniqueIds.
 */
void DataModel::ProjectModel::migrateLegacyWaterfallYAxisIds()
{
  QSet<int> liveUids;
  QMap<int, QMap<int, int>> uidByIndex;
  for (const auto& group : m_groups)
    for (const auto& dataset : group.datasets) {
      liveUids.insert(dataset.uniqueId);
      uidByIndex[group.sourceId].insert(dataset.index, dataset.uniqueId);
    }

  for (auto& group : m_groups) {
    const auto indexMap = uidByIndex.value(group.sourceId);
    for (auto& dataset : group.datasets)
      remapWaterfallYAxisId(dataset, liveUids, indexMap);
  }
}

/**
 * @brief Resolves a Group.uniqueId to its current positional groupId; returns -1 if absent.
 */
int DataModel::ProjectModel::groupIdForUniqueId(int uniqueId) const
{
  if (uniqueId < 0)
    return -1;

  for (const auto& group : m_groups)
    if (group.uniqueId == uniqueId)
      return group.groupId;

  return -1;
}

/**
 * @brief Resolves a positional groupId to its Group.uniqueId; returns -1 if out of range.
 */
int DataModel::ProjectModel::groupUniqueIdForGroupId(int groupId) const
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return -1;

  return m_groups[static_cast<size_t>(groupId)].uniqueId;
}

/**
 * @brief Returns "Time" plus every dataset label, sorted by uniqueId.
 */
QStringList DataModel::ProjectModel::xDataSources() const
{
  QStringList list;
  list.append(tr("Time"));

  QMap<int, QString> datasets;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!datasets.contains(uid))
        datasets.insert(uid, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Parallel to xDataSources(): the dataset uniqueId at each combo position
 *        (position 0 -> -2 "Time", then dataset uniqueIds).
 */
QList<int> DataModel::ProjectModel::xDataSourceUniqueIds() const
{
  QList<int> out;
  out.append(kXAxisTime);

  QMap<int, bool> seen;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!seen.contains(uid))
        seen.insert(uid, true);
    }
  }

  for (auto it = seen.cbegin(); it != seen.cend(); ++it)
    out.append(it.key());

  return out;
}

/**
 * @brief Returns "Time" plus every dataset label, sorted by uniqueId.
 */
QStringList DataModel::ProjectModel::yWaterfallSources() const
{
  QStringList list;
  list.append(tr("Time"));

  QMap<int, QString> datasets;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!datasets.contains(uid))
        datasets.insert(uid, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Parallel to yWaterfallSources(): the dataset uniqueId at each combo position
 *        (position 0 -> 0, the "Time" sentinel).
 */
QList<int> DataModel::ProjectModel::yWaterfallSourceUniqueIds() const
{
  QList<int> out;
  out.append(0);

  QMap<int, bool> seen;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!seen.contains(uid))
        seen.insert(uid, true);
    }
  }

  for (auto it = seen.cbegin(); it != seen.cend(); ++it)
    out.append(it.key());

  return out;
}

/**
 * @brief Suppresses modal dialogs when true (API/headless mode).
 */
void DataModel::ProjectModel::setSuppressMessageBoxes(const bool suppress)
{
  m_suppressMessageBoxes = suppress;
}

/**
 * @brief Returns true when modal dialogs are suppressed (API/headless mode).
 */
bool DataModel::ProjectModel::suppressMessageBoxes() const noexcept
{
  return m_suppressMessageBoxes;
}

/**
 * @brief Returns the current project title.
 */
const QString& DataModel::ProjectModel::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the absolute path of the loaded project file, or empty.
 */
const QString& DataModel::ProjectModel::jsonFilePath() const noexcept
{
  return m_filePath;
}

/**
 * @brief Returns the frame parser source code from source 0.
 */
QString DataModel::ProjectModel::frameParserCode() const
{
  if (m_sources.empty())
    return QString();

  return m_sources[0].frameParserCode;
}

/**
 * @brief Returns the scripting language for the global frame parser (source 0).
 */
int DataModel::ProjectModel::frameParserLanguage() const
{
  if (m_sources.empty())
    return 0;

  return m_sources[0].frameParserLanguage;
}

/**
 * @brief Returns the scripting language for the source, or source 0's.
 */
int DataModel::ProjectModel::frameParserLanguage(int sourceId) const
{
  for (const auto& src : m_sources)
    if (src.sourceId == sourceId)
      return src.frameParserLanguage;

  return frameParserLanguage();
}

/**
 * @brief Returns the native parser template id for the global frame parser (source 0).
 */
QString DataModel::ProjectModel::frameParserTemplate() const
{
  if (m_sources.empty())
    return QString();

  return m_sources[0].frameParserTemplate;
}

/**
 * @brief Returns the native parser template id for the source, or source 0's.
 */
QString DataModel::ProjectModel::frameParserTemplate(int sourceId) const
{
  for (const auto& src : m_sources)
    if (src.sourceId == sourceId)
      return src.frameParserTemplate;

  return frameParserTemplate();
}

/**
 * @brief Returns the native parser template params for the global frame parser (source 0).
 */
QJsonObject DataModel::ProjectModel::frameParserParams() const
{
  if (m_sources.empty())
    return QJsonObject();

  return m_sources[0].frameParserParams;
}

/**
 * @brief Returns the native parser template params for the source, or source 0's.
 */
QJsonObject DataModel::ProjectModel::frameParserParams(int sourceId) const
{
  for (const auto& src : m_sources)
    if (src.sourceId == sourceId)
      return src.frameParserParams;

  return frameParserParams();
}

/**
 * @brief Returns the active group ID for the dashboard tab bar, or -1.
 */
int DataModel::ProjectModel::activeGroupId() const
{
  return m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
}

/**
 * @brief Returns the persisted layout for the given group ID.
 */
QJsonObject DataModel::ProjectModel::groupLayout(int groupId) const
{
  return m_widgetSettings.value(Keys::layoutKey(groupId)).toObject().value("data").toObject();
}

/**
 * @brief Returns the persisted settings object for the given widget.
 */
QJsonObject DataModel::ProjectModel::widgetSettings(const QString& widgetId) const
{
  return m_widgetSettings.value(widgetId).toObject();
}

/**
 * @brief Returns the persisted state object for the given plugin.
 */
QJsonObject DataModel::ProjectModel::pluginState(const QString& pluginId) const
{
  return m_widgetSettings.value(QStringLiteral("plugin:") + pluginId).toObject();
}

/**
 * @brief Returns true if the project uses any commercial-only features.
 */
bool DataModel::ProjectModel::containsCommercialFeatures() const
{
  return SerialStudio::commercialCfg(m_groups);
}

/**
 * @brief Returns the dashboard point count (0 = use global default).
 */
int DataModel::ProjectModel::pointCount() const noexcept
{
  return m_pointCount;
}

/**
 * @brief Returns the project's plot time range in seconds (visible window for time-axis plots).
 */
double DataModel::ProjectModel::plotTimeRange() const noexcept
{
  return m_plotTimeRange;
}

/**
 * @brief Returns the number of groups in the project.
 */
int DataModel::ProjectModel::groupCount() const noexcept
{
  return static_cast<int>(m_groups.size());
}

/**
 * @brief Returns the total number of datasets across all groups.
 */
int DataModel::ProjectModel::datasetCount() const
{
  int count = 0;
  for (const auto& group : m_groups)
    count += static_cast<int>(group.datasets.size());

  return count;
}

/**
 * @brief Returns the project's group list.
 */
const std::vector<DataModel::Group>& DataModel::ProjectModel::groups() const noexcept
{
  return m_groups;
}

/**
 * @brief Returns the project's action list.
 */
const std::vector<DataModel::Action>& DataModel::ProjectModel::actions() const noexcept
{
  return m_actions;
}

/**
 * @brief Returns the project's data-source list.
 */
const std::vector<DataModel::Source>& DataModel::ProjectModel::sources() const noexcept
{
  return m_sources;
}

/**
 * @brief Returns the number of configured data sources.
 */
int DataModel::ProjectModel::sourceCount() const noexcept
{
  return static_cast<int>(m_sources.size());
}

/**
 * @brief Returns the editor-owned workspace list (always m_workspaces).
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::editorWorkspaces() const noexcept
{
  return m_workspaces;
}

/**
 * @brief Returns the workspace list currently rendered by the dashboard.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::activeWorkspaces() const
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return m_sessionWorkspaces;

  return m_workspaces;
}

/**
 * @brief Returns the set of hidden auto-generated group IDs.
 */
const QSet<int>& DataModel::ProjectModel::hiddenGroupIds() const noexcept
{
  return m_hiddenGroupIds;
}

/**
 * @brief Returns the number of workspaces defined in the project.
 */
int DataModel::ProjectModel::workspaceCount() const noexcept
{
  return static_cast<int>(m_workspaces.size());
}

/**
 * @brief Returns true when the auto-generated group workspace is hidden.
 */
bool DataModel::ProjectModel::isGroupHidden(int groupId) const
{
  return m_hiddenGroupIds.contains(groupId);
}

/**
 * @brief Returns the number of user-defined tables in the project.
 */
int DataModel::ProjectModel::tableCount() const noexcept
{
  return static_cast<int>(m_tables.size());
}

/**
 * @brief Returns the project's user-defined data table list.
 */
const std::vector<DataModel::TableDef>& DataModel::ProjectModel::tables() const noexcept
{
  return m_tables;
}

/**
 * @brief Returns the project's MQTT publisher configuration (empty when unset).
 */
const QJsonObject& DataModel::ProjectModel::mqttPublisher() const noexcept
{
  return m_mqttPublisher;
}

/**
 * @brief Replaces the MQTT publisher configuration and marks the project as modified.
 */
void DataModel::ProjectModel::setMqttPublisher(const QJsonObject& config)
{
  if (m_mqttPublisher == config)
    return;

  m_mqttPublisher = config;
  setModified(true);
  Q_EMIT mqttPublisherChanged();
}

/**
 * @brief Stages a single widget setting and marks the project dirty. QML callers pass JS
 *        arrays/objects as QJSValue-wrapped variants, which QJsonValue::fromVariant silently
 *        turns into null, so they are unwrapped first.
 */
void DataModel::ProjectModel::saveWidgetSetting(const QString& widgetId,
                                                const QString& key,
                                                const QVariant& value)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  auto normalized = value;
  if (normalized.userType() == qMetaTypeId<QJSValue>())
    normalized = normalized.value<QJSValue>().toVariant();

  auto obj            = m_widgetSettings.value(widgetId).toObject();
  const auto newValue = QJsonValue::fromVariant(normalized);
  if (obj.value(key) == newValue)
    return;

  obj.insert(key, newValue);
  m_widgetSettings.insert(widgetId, obj);

  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Stages a plugin's state in the project and marks it dirty.
 */
void DataModel::ProjectModel::savePluginState(const QString& pluginId, const QJsonObject& state)
{
  const auto key = QStringLiteral("plugin:") + pluginId;
  if (m_widgetSettings.value(key).toObject() == state)
    return;

  m_widgetSettings.insert(key, state);
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    setModified(true);

  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Seeds a source with the default parser: the Native CSV (delimited/comma) template.
 * Switching to a scripting language converts the template via the equivalence mapping.
 */
static void seedDefaultFrameParser(DataModel::Source& source)
{
  source.frameParserLanguage = static_cast<int>(SerialStudio::Native);
  source.frameParserTemplate = DataModel::defaultNativeTemplateId();

  const auto* tmpl = DataModel::nativeTemplateById(source.frameParserTemplate);
  Q_ASSERT(tmpl != nullptr);
  if (tmpl)
    source.frameParserParams = DataModel::nativeTemplateDefaults(*tmpl);
}

/**
 * @brief Adds a new source to the project (GPL: capped to one source).
 */
void DataModel::ProjectModel::addSource()
{
#ifndef BUILD_COMMERCIAL
  if (!m_sources.empty()) {
    if (!m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(
        tr("Multiple data sources require a Pro license"),
        tr("Serial Studio Pro allows connecting to multiple devices simultaneously. "
           "Please upgrade to unlock this feature."),
        QMessageBox::Information);

    return;
  }
#endif

  const int newId = static_cast<int>(m_sources.size());

  DataModel::Source source;
  source.sourceId              = newId;
  source.title                 = tr("Device %1").arg(QChar('A' + newId));
  source.busType               = static_cast<int>(SerialStudio::BusType::UART);
  source.frameStart            = m_frameStartSequence;
  source.frameEnd              = m_frameEndSequence;
  source.checksumAlgorithm     = m_checksumAlgorithm;
  source.frameDetection        = static_cast<int>(m_frameDetection);
  source.decoderMethod         = static_cast<int>(m_frameDecoder);
  source.hexadecimalDelimiters = m_hexadecimalDelimiters;
  seedDefaultFrameParser(source);

  m_sources.push_back(source);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(newId);
}

/**
 * @brief Deletes the source and reassigns dependent groups to source 0.
 */
void DataModel::ProjectModel::deleteSource(int sourceId, bool confirm)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  (void)confirm;
  return;
#else
  if (sourceId <= 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  if (confirm && !m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete data source \"%1\"?").arg(m_sources[sourceId].title),
      tr("Groups using this source will move to the default source. "
         "This action cannot be undone."),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
      return;
  }

  m_sources.erase(m_sources.begin() + sourceId);

  for (auto& group : m_groups)
    if (group.sourceId == sourceId)
      group.sourceId = 0;
    else if (group.sourceId > sourceId)
      --group.sourceId;

  for (size_t i = 0; i < m_sources.size(); ++i)
    m_sources[i].sourceId = static_cast<int>(i);

  setModified(true);
  Q_EMIT groupsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceDeleted();
#endif
}

/**
 * @brief Duplicates the source with the given @p sourceId.
 */
void DataModel::ProjectModel::duplicateSource(int sourceId)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  return;
#else
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  DataModel::Source copy  = m_sources[sourceId];
  copy.sourceId           = static_cast<int>(m_sources.size());
  copy.connectionSettings = QJsonObject();

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(m_sources.size()));
  for (const auto& s : m_sources)
    existingTitles.append(s.title);

  copy.title = nextDuplicateTitle(m_sources[sourceId].title, existingTitles);

  m_sources.push_back(copy);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(copy.sourceId);
#endif
}

/**
 * @brief Updates the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSource(int sourceId, const DataModel::Source& source)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId]          = source;
  m_sources[sourceId].sourceId = sourceId;

  if (sourceId == 0) {
    m_frameStartSequence    = source.frameStart;
    m_frameEndSequence      = source.frameEnd;
    m_checksumAlgorithm     = source.checksumAlgorithm;
    m_hexadecimalDelimiters = source.hexadecimalDelimiters;
    m_frameDetection        = static_cast<SerialStudio::FrameDetection>(source.frameDetection);
    m_frameDecoder          = static_cast<SerialStudio::DecoderMethod>(source.decoderMethod);
    Q_EMIT frameDetectionChanged();
  }

  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceChanged(sourceId);
}

/**
 * @brief Updates the title of the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSourceTitle(int sourceId, const QString& title)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].title = title.simplified();
  setModified(true);
  Q_EMIT sourcesChanged();
}

/**
 * @brief Updates the bus type of the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSourceBusType(int sourceId, int busType)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].busType = busType;
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
}

/**
 * @brief Updates the per-source JavaScript frame parser code.
 */
void DataModel::ProjectModel::updateSourceFrameParser(int sourceId, const QString& code)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].frameParserCode = code;
  DataModel::FrameParser::instance().setSourceCode(sourceId, code);
  setModified(true);

  Q_EMIT sourceFrameParserCodeChanged(sourceId);
}

/**
 * @brief Snapshots the current driver settings for source @p sourceId into
 * Source::connectionSettings.
 */
void DataModel::ProjectModel::captureSourceSettings(int sourceId)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  const auto busType     = static_cast<SerialStudio::BusType>(m_sources[sourceId].busType);
  IO::HAL_Driver* driver = IO::ConnectionManager::instance().uiDriverForBusType(busType);
  if (!driver)
    return;

  QJsonObject settings;
  for (const auto& prop : driver->driverProperties())
    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

  const auto deviceId = driver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  m_sources[sourceId].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Applies the source's saved connectionSettings to its live driver.
 */
void DataModel::ProjectModel::restoreSourceSettings(int sourceId)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  const auto& source = m_sources[sourceId];
  if (source.connectionSettings.isEmpty())
    return;

  IO::HAL_Driver* driver = IO::ConnectionManager::instance().driverForEditing(sourceId);
  if (!driver)
    return;

  driver->applyConnectionSettings(source.connectionSettings);
}

/**
 * @brief Overwrites source[0].connectionSettings without emitting sourcesChanged.
 */
void DataModel::ProjectModel::setSource0ConnectionSettings(const QJsonObject& settings)
{
  if (m_sources.empty())
    return;

  m_sources[0].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Sets source[0].busType without emitting sourceStructureChanged.
 */
void DataModel::ProjectModel::setSource0BusType(int busType)
{
  if (m_sources.empty())
    return;

  m_sources[0].busType = busType;
  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Document saving / export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts to save changes, returning false only on cancel.
 */
bool DataModel::ProjectModel::askSave()
{
  if (!modified())
    return true;

  const auto opMode = AppState::instance().operationMode();
  if (opMode != SerialStudio::ProjectFile && m_filePath.isEmpty())
    return true;

  if (m_suppressMessageBoxes) {
    qWarning() << "[ProjectModel] Discarding unsaved changes (API mode)";
    if (jsonFilePath().isEmpty())
      newJsonFile();
    else {
      const auto path = m_filePath;
      m_silentReload  = true;
      m_filePath.clear();
      openJsonFile(path);
      m_silentReload = false;
      if (opMode != SerialStudio::ProjectFile)
        AppState::instance().setOperationMode(opMode);
    }

    return true;
  }

  auto ret =
    Misc::Utilities::showMessageBox(tr("Do you want to save your changes?"),
                                    tr("You have unsaved modifications in this project!"),
                                    QMessageBox::Question,
                                    APP_NAME,
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Cancel)
    return false;

  if (ret == QMessageBox::Discard) {
    if (jsonFilePath().isEmpty())
      newJsonFile();
    else {
      const auto path = m_filePath;
      m_silentReload  = true;
      m_filePath.clear();
      openJsonFile(path);
      m_silentReload = false;
      if (opMode != SerialStudio::ProjectFile)
        AppState::instance().setOperationMode(opMode);
    }

    return true;
  }

  return saveJsonFile(false);
}

/**
 * @brief Validates and saves the project, optionally prompting for a path; the
 * path-accepted handler defers via a queued invoke because the macOS NSSavePanel
 * KVO callback must unwind before re-entering the model.
 */
bool DataModel::ProjectModel::saveJsonFile(const bool askPath)
{
  if (!validateProject(m_suppressMessageBoxes))
    return false;

  if (jsonFilePath().isEmpty() || askPath) {
    auto* dialog = new QFileDialog(qApp->activeWindow(),
                                   tr("Save Serial Studio Project"),
                                   jsonProjectsPath() + "/" + title() + ".ssproj",
                                   tr("Serial Studio Project Files (*.ssproj)"));

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto accepted = std::make_shared<bool>(false);
    connect(dialog, &QFileDialog::fileSelected, this, [this, accepted](const QString& path) {
      if (path.isEmpty())
        return;

      *accepted = true;

      QMetaObject::invokeMethod(
        this,
        [this, path]() {
          QString finalPath = path;
          if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
            finalPath += QStringLiteral(".ssproj");

          const QString chosenTitle = QFileInfo(finalPath).completeBaseName();
          if (m_title == tr("Untitled Project") && !chosenTitle.isEmpty()
              && chosenTitle != m_title) {
            m_title = chosenTitle;
            Q_EMIT titleChanged();
          }

          m_filePath = finalPath;
          (void)finalizeProjectSave();
        },
        Qt::QueuedConnection);
    });

    connect(dialog, &QFileDialog::finished, this, [this, accepted](int) {
      Q_EMIT saveDialogCompleted(*accepted);
    });

    dialog->open();
    return false;
  }

  return finalizeProjectSave();
}

/**
 * @brief Headless save to the given path (no file dialog).
 */
bool DataModel::ProjectModel::apiSaveJsonFile(const QString& path)
{
  if (path.isEmpty())
    return false;

  if (m_title.isEmpty()) {
    qWarning() << "[ProjectModel] Project title cannot be empty";
    return false;
  }

  if (groupCount() <= 0) {
    qWarning() << "[ProjectModel] Project needs at least one group";
    return false;
  }

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup) {
    qWarning() << "[ProjectModel] Project needs at least one dataset";
    return false;
  }

  QString finalPath = path;
  if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
    finalPath += QStringLiteral(".ssproj");

  m_filePath = finalPath;
  return finalizeProjectSave();
}

/**
 * @brief Serializes the complete project state to a QJsonObject.
 */
QJsonObject DataModel::ProjectModel::serializeToJson() const
{
  QJsonObject json;
  json.insert(Keys::Title, m_title);
  json.insert(Keys::PointCount, m_pointCount);
  json.insert(Keys::PlotTimeRange, m_plotTimeRange);
  json.insert(Keys::HexadecimalDelimiters, m_hexadecimalDelimiters);

  const QString writer  = DataModel::current_writer_version();
  const QString creator = m_writerVersionAtCreation.isEmpty() ? writer : m_writerVersionAtCreation;
  json.insert(Keys::SchemaVersion, DataModel::kSchemaVersion);
  json.insert(Keys::WriterVersion, writer);
  json.insert(Keys::WriterVersionAtCreation, creator);
  json.insert(Keys::NextUniqueId, m_nextUniqueId);

  if (!m_passwordHash.isEmpty())
    json.insert(Keys::PasswordHash, m_passwordHash);

  if (m_apiCallAllowFullSurface)
    json.insert(Keys::ApiCallAllowFullSurface, true);

  if (!m_controlScriptCode.isEmpty())
    json.insert(Keys::ControlScriptCode, m_controlScriptCode);

  QJsonArray groupArray;
  for (const auto& group : std::as_const(m_groups))
    groupArray.append(DataModel::serialize(group));

  json.insert(Keys::Groups, groupArray);

  QJsonArray actionsArray;
  for (const auto& action : std::as_const(m_actions))
    actionsArray.append(DataModel::serialize(action));

  json.insert(Keys::Actions, actionsArray);

  QJsonArray sourcesArray;
  for (const auto& source : std::as_const(m_sources))
    sourcesArray.append(DataModel::serialize(source));

  json.insert(Keys::Sources, sourcesArray);

  if (m_customizeWorkspaces) {
    json.insert(Keys::CustomizeWorkspaces, true);

    QJsonArray workspacesArray;
    for (const auto& ws : std::as_const(m_workspaces))
      workspacesArray.append(DataModel::serialize(ws));

    json.insert(Keys::Workspaces, workspacesArray);
  }

  if (!m_hiddenGroupIds.isEmpty()) {
    QJsonArray hiddenArray;
    for (const int id : std::as_const(m_hiddenGroupIds))
      hiddenArray.append(id);

    json.insert(Keys::HiddenGroups, hiddenArray);
  }

  if (!m_tables.empty()) {
    QJsonArray tablesArray;
    for (const auto& table : std::as_const(m_tables))
      tablesArray.append(DataModel::serialize(table));

    json.insert(Keys::Tables, tablesArray);
  }

  if (!m_widgetSettings.isEmpty())
    json.insert(Keys::WidgetSettings, m_widgetSettings);

  if (!m_mqttPublisher.isEmpty())
    json.insert(Keys::MqttPublisher, m_mqttPublisher);

  return json;
}

//--------------------------------------------------------------------------------------------------
// Signal/slot setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires Dashboard, ConnectionManager, and AppState signals to this model.
 */
void DataModel::ProjectModel::setupExternalConnections()
{
  connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this, [this]() {
    const auto opMode = AppState::instance().operationMode();
    if (opMode != SerialStudio::ProjectFile || m_filePath.isEmpty())
      return;

    const int points = UI::Dashboard::instance().points();
    if (m_pointCount == points)
      return;

    m_pointCount = points;

    if (!writeProjectFile(m_filePath))
      return;

    Q_EMIT pointCountChanged();
  });

  connect(&UI::Dashboard::instance(), &UI::Dashboard::widgetCountChanged, this, [this] {
    if (AppState::instance().operationMode() != SerialStudio::QuickPlot)
      return;

    m_sessionWorkspaces = buildAutoWorkspaces();
    Q_EMIT activeWorkspacesChanged();
  });

  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    const auto opMode = AppState::instance().operationMode();
    if (opMode == SerialStudio::ProjectFile)
      m_sessionWorkspaces.clear();
    else
      m_sessionWorkspaces = buildAutoWorkspaces();

    Q_EMIT activeWorkspacesChanged();
  });

  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (!IO::ConnectionManager::instance().isConnected())
        clearTransientState();
    });
}

//--------------------------------------------------------------------------------------------------
// Document initialisation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resets all project state to factory defaults.
 */
void DataModel::ProjectModel::newJsonFile()
{
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_autoSnapshot.clear();
  m_tables.clear();
  m_hiddenGroupIds.clear();
  m_customizeWorkspaces = false;

  const bool hadMqttPublisher = !m_mqttPublisher.isEmpty();
  m_mqttPublisher             = QJsonObject();

  const bool wasLocked = m_locked;
  m_passwordHash.clear();
  m_locked = false;

  m_frameEndSequence        = "\\n";
  m_checksumAlgorithm       = "";
  m_frameStartSequence      = "$";
  m_writerVersionAtCreation = "";
  m_hexadecimalDelimiters   = false;
  m_title                   = tr("Untitled Project");
  m_pointCount              = 100;
  m_plotTimeRange           = 10.0;
  m_nextUniqueId            = 1;
  m_controlScriptCode       = "";
  DataModel::ControlScript::instance().setCode(m_controlScriptCode);
  m_frameDecoder   = SerialStudio::PlainText;
  m_frameDetection = SerialStudio::EndDelimiterOnly;
  m_widgetSettings = QJsonObject();

  DataModel::Source defaultSource;
  defaultSource.sourceId              = 0;
  defaultSource.title                 = tr("Device A");
  defaultSource.busType               = static_cast<int>(SerialStudio::BusType::UART);
  defaultSource.frameStart            = m_frameStartSequence;
  defaultSource.frameEnd              = m_frameEndSequence;
  defaultSource.checksumAlgorithm     = m_checksumAlgorithm;
  defaultSource.frameDetection        = static_cast<int>(m_frameDetection);
  defaultSource.decoderMethod         = static_cast<int>(m_frameDecoder);
  defaultSource.hexadecimalDelimiters = m_hexadecimalDelimiters;
  seedDefaultFrameParser(defaultSource);
  m_sources.push_back(defaultSource);

  m_filePath = "";

  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT tablesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();
  Q_EMIT controlScriptChanged();

  if (wasLocked)
    Q_EMIT lockedChanged();

  if (hadMqttPublisher)
    Q_EMIT mqttPublisherChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  setModified(false);
}

/**
 * @brief Updates the project title and emits titleChanged.
 */
void DataModel::ProjectModel::setTitle(const QString& title)
{
  if (m_title != title) {
    m_title = title;
    setModified(true);
    Q_EMIT titleChanged();
  }
}

/**
 * @brief Returns the project setup()/loop() control script source.
 */
QString DataModel::ProjectModel::controlScriptCode() const
{
  return m_controlScriptCode;
}

/**
 * @brief Stages a new control script and pushes it to the live runtime.
 */
void DataModel::ProjectModel::setControlScriptCode(const QString& code)
{
  if (m_controlScriptCode == code)
    return;

  m_controlScriptCode = code;
  DataModel::ControlScript::instance().setCode(code);
  setModified(true);
  Q_EMIT controlScriptChanged();
}

/**
 * @brief Sets the dashboard point count and syncs it to the Dashboard.
 */
void DataModel::ProjectModel::setPointCount(const int points)
{
  if (m_pointCount == points)
    return;

  m_pointCount = points;

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    UI::Dashboard::instance().setPoints(points);

  setModified(true);
  Q_EMIT pointCountChanged();
}

/**
 * @brief Sets the project's plot time range (seconds) and syncs it to the Dashboard.
 */
void DataModel::ProjectModel::setPlotTimeRange(const double seconds)
{
  const double clamped = qMax(0.001, seconds);
  if (qFuzzyCompare(m_plotTimeRange, clamped))
    return;

  m_plotTimeRange = clamped;

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    UI::Dashboard::instance().setPlotTimeRange(clamped);

  setModified(true);
  Q_EMIT plotTimeRangeChanged();
}

/**
 * @brief Clears the project file path without changing project data.
 */
void DataModel::ProjectModel::clearJsonFilePath()
{
  if (!m_filePath.isEmpty()) {
    m_filePath.clear();
    Q_EMIT jsonFileChanged();
  }
}

/**
 * @brief Sets the frame start delimiter sequence.
 */
void DataModel::ProjectModel::setFrameStartSequence(const QString& sequence)
{
  if (m_frameStartSequence == sequence)
    return;

  m_frameStartSequence = sequence;

  if (m_sources.size() == 1)
    m_sources[0].frameStart = sequence;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Sets the frame end delimiter sequence.
 */
void DataModel::ProjectModel::setFrameEndSequence(const QString& sequence)
{
  if (m_frameEndSequence == sequence)
    return;

  m_frameEndSequence = sequence;

  if (m_sources.size() == 1)
    m_sources[0].frameEnd = sequence;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Sets the checksum algorithm name.
 */
void DataModel::ProjectModel::setChecksumAlgorithm(const QString& algorithm)
{
  if (m_checksumAlgorithm == algorithm)
    return;

  m_checksumAlgorithm = algorithm;

  if (m_sources.size() == 1)
    m_sources[0].checksumAlgorithm = algorithm;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Sets the frame detection strategy.
 */
void DataModel::ProjectModel::setFrameDetection(const SerialStudio::FrameDetection detection)
{
  if (m_frameDetection == detection)
    return;

  m_frameDetection = detection;

  if (m_sources.size() == 1)
    m_sources[0].frameDetection = static_cast<int>(detection);

  setModified(true);
  Q_EMIT frameDetectionChanged();
}

//--------------------------------------------------------------------------------------------------
// Document loading / import
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shows a file-open dialog and loads the selected project; the selection
 * handler defers via a queued invoke so the macOS NSSavePanel KVO callback can
 * unwind before re-entering the model.
 */
void DataModel::ProjectModel::openJsonFile()
{
  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Select Project File"),
                                 jsonProjectsPath(),
                                 tr("Project Files (*.json *.ssproj)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { openJsonFile(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Loads a project from the given .ssproj/.json path.
 */
bool DataModel::ProjectModel::openJsonFile(const QString& path)
{
  if (path.isEmpty())
    return false;

  QString resolved = path;
  if (!QFileInfo::exists(resolved)) {
    const QString remapped = Misc::WorkspaceManager::instance().remapLegacyPath(path);
    if (remapped != path && QFileInfo::exists(remapped))
      resolved = remapped;
  }

  if (m_filePath == resolved && !m_groups.empty())
    return true;

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  QFile file(resolved);
  QJsonDocument document;
  if (file.open(QFile::ReadOnly)) {
    auto result = Misc::JsonValidator::parseAndValidate(file.readAll());
    if (!result.valid) [[unlikely]] {
      if (m_suppressMessageBoxes)
        qWarning() << "[ProjectModel] JSON validation error:" << result.errorMessage;
      else
        Misc::Utilities::showMessageBox(
          tr("JSON validation error"), result.errorMessage, QMessageBox::Critical);

      return false;
    }

    document = result.document;
    file.close();
  }

  return loadFromJsonDocument(document, resolved);
}

/**
 * @brief Deserialises a project from an in-memory QJsonDocument; older-schema
 * loads deliberately drop customizeWorkspaces and clear m_workspaces because
 * migrated projects can carry stale refs that would blank the dashboard, so
 * forcing the auto layout is preferred over honouring those refs.
 */
bool DataModel::ProjectModel::loadFromJsonDocument(const QJsonDocument& document,
                                                   const QString& sourcePath)
{
  if (document.isEmpty())
    return false;

  m_autoSaveSuspended = true;
  if (m_autoSaveTimer)
    m_autoSaveTimer->stop();

  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_widgetSettings = QJsonObject();

  m_filePath = sourcePath;

  const auto json                = document.object();
  const QString legacyParserCode = json.value(QLatin1StringView("frameParser")).toString();
  const bool legacyUniqueIds     = !json.contains(Keys::NextUniqueId);

  const int loadedSchema = ss_jsr(json, Keys::SchemaVersion, 0).toInt();
  const bool olderSchema = loadedSchema < DataModel::kSchemaVersion;

  m_apiCallAllowFullSurface = ss_jsr(json, Keys::ApiCallAllowFullSurface, false).toBool();
  DataModel::ScriptApiCall::setAllowFullSurface(m_apiCallAllowFullSurface);

  m_controlScriptCode = ss_jsr(json, Keys::ControlScriptCode, "").toString();
  DataModel::ControlScript::instance().setCode(m_controlScriptCode);

  loadProjectRootScalars(json);
  loadProjectArrays(json, legacyParserCode);
  enforceGplSingleSource();
  resolveDatasetTransformLanguages();
  resolveDatasetVirtualFlags();

  seedNextUniqueIdFromGroups();
  loadWidgetSettingsAndWorkspaces(json);

  if (olderSchema) {
    m_customizeWorkspaces = false;
    m_workspaces.clear();
  }

  if (legacyUniqueIds) {
    migrateLegacyWorkspaceRefs();
    migrateLegacyXAxisIds();
  }

  migrateLegacyWaterfallYAxisIds();

  loadPointCount(json);
  loadPlotTimeRange(json);
  migrateLegacyLayoutKeys();
  migrateLegacyDashboardLayout(json);

  setModified(false);

  if (migrateLegacySeparator(json))
    return true;

  m_autoSnapshot = buildAutoWorkspaces();
  emitProjectLoadedSignals();

  if (legacyUniqueIds && !m_filePath.isEmpty())
    persistLegacyMigration();

  m_autoSaveSuspended = false;

  if (olderSchema && !sourcePath.isEmpty()) {
    const QString title = tr("Project upgraded from an earlier file format");
    const QString body  = tr("This project was saved with schema version %1; the current version "
                             "is %2. Defaults have been applied to any new fields. Save the "
                             "project to lock in the upgrade.")
                           .arg(loadedSchema)
                           .arg(DataModel::kSchemaVersion);
    QTimer::singleShot(0, this, [title, body] {
      DataModel::NotificationCenter::instance().postInfo(
        QStringLiteral("ProjectModel"), title, body);
    });
  }

  return true;
}

/**
 * @brief Prompts for a save path, writes the imported project, then opens it; the
 * work is queued so the UI dialog that launched the import (and the macOS
 * NSSavePanel KVO callback) can unwind before the model re-enters.
 */
void DataModel::ProjectModel::importProjectFromJson(const QJsonObject& project,
                                                    const QString& suggestedFileName)
{
  QMetaObject::invokeMethod(
    this,
    [this, project, suggestedFileName]() {
      QString suggested = suggestedFileName.isEmpty() ? tr("Untitled Project") : suggestedFileName;
      if (!suggested.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
        suggested += QStringLiteral(".ssproj");

      const QString defaultPath = jsonProjectsPath() + QStringLiteral("/") + suggested;

      auto* dialog = new QFileDialog(qApp->activeWindow(),
                                     tr("Save Imported Project"),
                                     defaultPath,
                                     tr("Serial Studio Project Files (*.ssproj)"));

      dialog->setAcceptMode(QFileDialog::AcceptSave);
      dialog->setFileMode(QFileDialog::AnyFile);
      dialog->setAttribute(Qt::WA_DeleteOnClose);

      auto accepted   = std::make_shared<bool>(false);
      auto chosenPath = std::make_shared<QString>();
      connect(
        dialog, &QFileDialog::fileSelected, this, [accepted, chosenPath](const QString& path) {
          if (path.isEmpty())
            return;

          *accepted   = true;
          *chosenPath = path;
          if (!chosenPath->endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
            *chosenPath += QStringLiteral(".ssproj");
        });

      connect(dialog, &QFileDialog::finished, this, [this, accepted, chosenPath, project](int) {
        if (!*accepted) {
          Q_EMIT importCompleted(false, QString());
          return;
        }

        QMetaObject::invokeMethod(
          this,
          [this, chosenPath, project]() {
            QFile file(*chosenPath);
            if (!file.open(QFile::WriteOnly)) {
              if (m_suppressMessageBoxes)
                qWarning() << "[ProjectModel] Import save error:" << file.errorString();
              else
                Misc::Utilities::showMessageBox(
                  tr("File open error"), file.errorString(), QMessageBox::Critical);

              Q_EMIT importCompleted(false, QString());
              return;
            }

            file.write(QJsonDocument(project).toJson(QJsonDocument::Indented));
            file.close();

            AppState::instance().setOperationMode(SerialStudio::ProjectFile);

            // code-verify off
            // Clear cached path so openJsonFile()'s redundant-reload guard doesn't skip the open.
            m_filePath.clear();
            // code-verify on
            const bool ok = openJsonFile(*chosenPath);
            Q_EMIT importCompleted(ok, ok ? *chosenPath : QString());
          },
          Qt::QueuedConnection);
      });

      dialog->open();
    },
    Qt::QueuedConnection);
}

/**
 * @brief Reads project-wide scalar fields (title, delimiters, decoder, lock state) from JSON.
 */
void DataModel::ProjectModel::loadProjectRootScalars(const QJsonObject& json)
{
  m_title                 = json.value(Keys::Title).toString();
  m_frameEndSequence      = json.value(Keys::FrameEnd).toString();
  m_frameStartSequence    = json.value(Keys::FrameStart).toString();
  m_hexadecimalDelimiters = json.value(Keys::HexadecimalDelimiters).toBool();
  m_frameDetection =
    static_cast<SerialStudio::FrameDetection>(json.value(Keys::FrameDetection).toInt());

  if (json.contains(Keys::ChecksumAlgorithm))
    m_checksumAlgorithm = json.value(Keys::ChecksumAlgorithm).toString();
  else
    m_checksumAlgorithm = json.value(Keys::Checksum).toString();

  if (json.contains(Keys::DecoderMethod))
    m_frameDecoder =
      static_cast<SerialStudio::DecoderMethod>(json.value(Keys::DecoderMethod).toInt());
  else
    m_frameDecoder = static_cast<SerialStudio::DecoderMethod>(json.value(Keys::Decoder).toInt());

  m_writerVersionAtCreation = json.value(Keys::WriterVersionAtCreation).toString();

  m_nextUniqueId = ss_jsr(json, Keys::NextUniqueId, 1).toInt();

  m_passwordHash       = json.value(Keys::PasswordHash).toString();
  const bool wasLocked = m_locked;
  m_locked             = !m_passwordHash.isEmpty();
  if (m_locked != wasLocked)
    Q_EMIT lockedChanged();

  if (!json.contains(Keys::FrameDetection))
    m_frameDetection = SerialStudio::StartAndEndDelimiter;
}

/**
 * @brief Deserializes groups, actions and sources arrays into m_groups/m_actions/m_sources.
 */
void DataModel::ProjectModel::loadProjectArrays(const QJsonObject& json,
                                                const QString& legacyParserCode)
{
  auto groups = json.value(Keys::Groups).toArray();
  for (int g = 0; g < groups.count(); ++g) {
    DataModel::Group group;
    group.groupId = g;
    if (DataModel::read(group, groups.at(g).toObject()))
      m_groups.push_back(group);
  }

  auto actions = json.value(Keys::Actions).toArray();
  for (int a = 0; a < actions.count(); ++a) {
    DataModel::Action action;
    action.actionId = a;
    if (DataModel::read(action, actions.at(a).toObject()))
      m_actions.push_back(action);
  }

  m_sources.clear();
  if (json.contains(Keys::Sources)) {
    auto sourcesArr = json.value(Keys::Sources).toArray();
    for (int s = 0; s < sourcesArr.count(); ++s) {
      DataModel::Source source;
      if (DataModel::read(source, sourcesArr.at(s).toObject()))
        m_sources.push_back(source);
    }
  }

  if (m_sources.empty()) {
    seedDefaultSourceFromUi(legacyParserCode);
    return;
  }

  if (m_sources[0].frameParserCode.isEmpty())
    m_sources[0].frameParserCode =
      legacyParserCode.isEmpty() ? FrameParser::defaultTemplateCode() : legacyParserCode;
}

/**
 * @brief Builds a default Source[0] from the UI driver state when JSON has no sources array.
 */
void DataModel::ProjectModel::seedDefaultSourceFromUi(const QString& legacyParserCode)
{
  DataModel::Source defaultSource;
  defaultSource.sourceId              = 0;
  defaultSource.title                 = tr("Device A");
  auto& cm                            = IO::ConnectionManager::instance();
  defaultSource.busType               = static_cast<int>(cm.busType());
  defaultSource.frameStart            = m_frameStartSequence;
  defaultSource.frameEnd              = m_frameEndSequence;
  defaultSource.checksumAlgorithm     = m_checksumAlgorithm;
  defaultSource.frameDetection        = static_cast<int>(m_frameDetection);
  defaultSource.decoderMethod         = static_cast<int>(m_frameDecoder);
  defaultSource.hexadecimalDelimiters = m_hexadecimalDelimiters;
  defaultSource.frameParserCode =
    legacyParserCode.isEmpty() ? FrameParser::defaultTemplateCode() : legacyParserCode;

  IO::HAL_Driver* uiDriver = cm.uiDriverForBusType(cm.busType());
  if (uiDriver) {
    QJsonObject settings;
    for (const auto& prop : uiDriver->driverProperties())
      settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

    const auto deviceId = uiDriver->deviceIdentifier();
    if (!deviceId.isEmpty())
      settings.insert(QStringLiteral("deviceId"), deviceId);

    defaultSource.connectionSettings = settings;
  }

  m_sources.push_back(defaultSource);
}

/**
 * @brief Truncates multi-source projects to one source on GPL builds with a user warning.
 */
void DataModel::ProjectModel::enforceGplSingleSource()
{
#ifndef BUILD_COMMERCIAL
  if (m_sources.size() <= 1)
    return;

  m_sources.resize(1);
  for (auto& g : m_groups)
    if (g.sourceId > 0)
      g.sourceId = 0;

  if (!m_suppressMessageBoxes)
    Misc::Utilities::showMessageBox(
      tr("Multi-source projects require a Pro license"),
      tr("This project contains multiple data sources. Only the first source "
         "has been loaded. A Serial Studio Pro license is required to use "
         "multi-source projects."),
      QMessageBox::Information);
  else
    qWarning() << "[ProjectModel] Multi-source project truncated to 1 source (GPL build)";
#endif
}

/**
 * @brief Resolves any unset dataset transformLanguage values from their owning source.
 */
void DataModel::ProjectModel::resolveDatasetTransformLanguages()
{
  const auto languageForSource = [&](int sourceId) {
    for (const auto& src : m_sources)
      if (src.sourceId == sourceId)
        return src.frameParserLanguage == SerialStudio::Native ? static_cast<int>(SerialStudio::Lua)
                                                               : src.frameParserLanguage;

    return 0;
  };

  for (auto& group : m_groups)
    for (auto& dataset : group.datasets)
      if (dataset.transformLanguage < 0 && !dataset.transformCode.isEmpty())
        dataset.transformLanguage = languageForSource(dataset.sourceId);
}

/**
 * @brief Tokenizer state for transformBodyReferencesValue.
 */
struct TransformScanner {
  bool inStr          = false;
  bool inLineComment  = false;
  bool inBlockComment = false;
  QChar quote;
};

/**
 * @brief Tries to enter a comment at code[i]; returns true and advances i when entered.
 */
static bool tryEnterComment(const QString& code, int n, int& i, bool isLua, TransformScanner& s)
{
  const QChar c    = code[i];
  const QChar next = (i + 1 < n) ? code[i + 1] : QChar();

  if (isLua && c == '-' && next == '-') {
    const bool isBlock  = (i + 3 < n && code[i + 2] == '[' && code[i + 3] == '[');
    s.inBlockComment    = isBlock;
    s.inLineComment     = !isBlock;
    i                  += isBlock ? 4 : 2;
    return true;
  }
  if (!isLua && c == '/' && next == '/') {
    s.inLineComment  = true;
    i               += 2;
    return true;
  }
  if (!isLua && c == '/' && next == '*') {
    s.inBlockComment  = true;
    i                += 2;
    return true;
  }

  return false;
}

/**
 * @brief Advances i through whatever comment/string state is currently active; returns true if so.
 */
static bool advanceInsideToken(const QString& code, int n, int& i, bool isLua, TransformScanner& s)
{
  const QChar c    = code[i];
  const QChar next = (i + 1 < n) ? code[i + 1] : QChar();

  if (s.inLineComment) {
    if (c == '\n')
      s.inLineComment = false;

    ++i;
    return true;
  }

  if (s.inBlockComment) {
    const bool luaEnd = (isLua && c == ']' && next == ']');
    const bool cEnd   = (!isLua && c == '*' && next == '/');
    if (luaEnd || cEnd) {
      s.inBlockComment  = false;
      i                += 2;
      return true;
    }
    ++i;
    return true;
  }

  if (s.inStr) {
    if (c == '\\' && i + 1 < n) {
      i += 2;
      return true;
    }
    if (c == s.quote)
      s.inStr = false;

    ++i;
    return true;
  }

  return false;
}

/**
 * @brief Scans an identifier at code[i] and returns true if it is a free reference to `value`.
 */
static bool identifierIsFreeValueRef(const QString& code, int n, int& i)
{
  const auto isIdCont = [](QChar c) {
    return c.isLetterOrNumber() || c == '_';
  };

  const int start = i;
  while (i < n && isIdCont(code[i]))
    ++i;

  const QStringView ident(code.constData() + start, i - start);
  if (ident != QLatin1String("value"))
    return false;

  const QChar prev = start > 0 ? code[start - 1] : QChar();
  return prev != '.' && prev != ':';
}

/**
 * @brief Returns true when 'value' appears as an identifier outside strings/comments.
 */
static bool transformBodyReferencesValue(const QString& code, int language)
{
  if (code.isEmpty())
    return true;

  const int n      = code.size();
  const bool isLua = (language == 1);
  TransformScanner s;
  int i = 0;

  const auto isIdStart = [](QChar c) {
    return c.isLetter() || c == '_';
  };

  while (i < n) {
    if (advanceInsideToken(code, n, i, isLua, s))
      continue;

    if (tryEnterComment(code, n, i, isLua, s))
      continue;

    const QChar c = code[i];
    if (c == '"' || c == '\'' || (isLua && c == '`')) {
      s.inStr = true;
      s.quote = c;
      ++i;
      continue;
    }

    if (isIdStart(c)) {
      if (identifierIsFreeValueRef(code, n, i))
        return true;

      continue;
    }

    ++i;
  }

  return false;
}

/**
 * @brief Auto-flags datasets whose transform body never reads `value` as virtual.
 */
void DataModel::ProjectModel::resolveDatasetVirtualFlags()
{
  for (auto& group : m_groups) {
    for (auto& dataset : group.datasets) {
      if (dataset.virtual_)
        continue;

      if (dataset.transformCode.isEmpty())
        continue;

      const int lang = dataset.transformLanguage < 0 ? 0 : dataset.transformLanguage;
      if (!transformBodyReferencesValue(dataset.transformCode, lang))
        dataset.virtual_ = true;
    }
  }
}

/**
 * @brief Reads widgetSettings, customised workspaces, hidden group ids, and tables from JSON.
 */
void DataModel::ProjectModel::loadWidgetSettingsAndWorkspaces(const QJsonObject& json)
{
  m_widgetSettings = json.value(Keys::WidgetSettings).toObject();

  m_workspaces.clear();
  m_customizeWorkspaces = json.value(Keys::CustomizeWorkspaces).toBool(false);

  if (m_customizeWorkspaces && json.contains(Keys::Workspaces)) {
    const auto wsArray = json.value(Keys::Workspaces).toArray();
    for (const auto& val : wsArray) {
      DataModel::Workspace ws;
      if (DataModel::read(ws, val.toObject()))
        m_workspaces.push_back(ws);
    }

    int collisions = 0;
    int nextId     = WorkspaceIds::UserStart;
    for (const auto& ws : std::as_const(m_workspaces))
      if (ws.workspaceId >= WorkspaceIds::UserStart && ws.workspaceId >= nextId)
        nextId = ws.workspaceId + 1;

    for (auto& ws : m_workspaces) {
      if (ws.workspaceId >= WorkspaceIds::AutoStart && ws.workspaceId < WorkspaceIds::UserStart) {
        ws.workspaceId = nextId++;
        ++collisions;
      }
    }

    if (collisions > 0) {
      const int n = collisions;
      QTimer::singleShot(0, this, [n] {
        DataModel::NotificationCenter::instance().postWarning(
          QStringLiteral("ProjectModel"),
          tr("Workspace IDs remapped on load"),
          tr("%1 custom workspace ID(s) overlapped the new reserved auto range and were "
             "moved into the user range. Save the project to make the remap permanent.")
            .arg(n));
      });
    }
  }

  m_hiddenGroupIds.clear();
  if (json.contains(Keys::HiddenGroups)) {
    const auto hiddenArray = json.value(Keys::HiddenGroups).toArray();
    for (const auto& val : hiddenArray)
      m_hiddenGroupIds.insert(val.toInt());
  }

  m_tables.clear();
  if (json.contains(Keys::Tables)) {
    const auto tablesArray = json.value(Keys::Tables).toArray();
    for (const auto& val : tablesArray) {
      DataModel::TableDef table;
      if (DataModel::read(table, val.toObject()))
        m_tables.push_back(table);
    }
  }

  m_mqttPublisher = json.value(Keys::MqttPublisher).toObject();
  Q_EMIT mqttPublisherChanged();
}

/**
 * @brief Resolves the project point-count from JSON or legacy widgetSettings, syncing dashboard.
 */
void DataModel::ProjectModel::loadPointCount(const QJsonObject& json)
{
  m_pointCount = UI::Dashboard::instance().points();
  if (json.contains(Keys::PointCount)) {
    const int pts = json.value(Keys::PointCount).toInt();
    if (pts > 0)
      m_pointCount = pts;
  } else if (m_widgetSettings.contains(QStringLiteral("__pointCount__"))) {
    const int pts = m_widgetSettings.value(QStringLiteral("__pointCount__")).toInt();
    if (pts > 0)
      m_pointCount = pts;

    m_widgetSettings.remove(QStringLiteral("__pointCount__"));
  }

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    UI::Dashboard::instance().setPoints(m_pointCount);
}

/**
 * @brief Resolves the project plot time range (seconds) from JSON, syncing the dashboard.
 */
void DataModel::ProjectModel::loadPlotTimeRange(const QJsonObject& json)
{
  m_plotTimeRange = UI::Dashboard::instance().plotTimeRange();
  if (json.contains(Keys::PlotTimeRange)) {
    const double secs = SerialStudio::toDouble(json.value(Keys::PlotTimeRange));
    if (secs > 0)
      m_plotTimeRange = secs;
  }

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    UI::Dashboard::instance().setPlotTimeRange(m_plotTimeRange);
}

/**
 * @brief Rewrites legacy "__layout__:N__" widgetSettings keys into canonical "layout:N" form.
 */
void DataModel::ProjectModel::migrateLegacyLayoutKeys()
{
  const auto keys = m_widgetSettings.keys();
  for (const auto& key : keys) {
    const bool isOldFormat = key.startsWith(QStringLiteral("__layout__:"));
    const bool isNewFormat = key.startsWith(QStringLiteral("layout:"));
    if (!isOldFormat && !isNewFormat)
      continue;

    auto entry = m_widgetSettings.value(key).toObject();
    if (!entry.contains(QStringLiteral("data")))
      continue;

    QJsonObject cleaned;
    cleaned[QStringLiteral("data")] = entry[QStringLiteral("data")];

    if (!isOldFormat) {
      m_widgetSettings.insert(key, cleaned);
      continue;
    }

    m_widgetSettings.remove(key);
    auto id = key.mid(11);
    id.chop(2);
    m_widgetSettings.insert(QStringLiteral("layout:") + id, cleaned);
  }
}

/**
 * @brief Migrates legacy dashboardLayout/activeGroupId fields into the widgetSettings store.
 */
void DataModel::ProjectModel::migrateLegacyDashboardLayout(const QJsonObject& json)
{
  if (!json.contains(QStringLiteral("dashboardLayout")))
    return;

  const int legacy_group_id = json.value(QStringLiteral("activeGroupId")).toInt(-1);
  const auto layout         = json.value(QStringLiteral("dashboardLayout")).toObject();
  if (legacy_group_id >= 0 && !layout.isEmpty())
    m_widgetSettings.insert(Keys::layoutKey(legacy_group_id), layout);

  if (legacy_group_id >= 0)
    m_widgetSettings.insert(Keys::kActiveGroupSubKey, legacy_group_id);
}

/**
 * @brief Rewrites a legacy parse(frame, separator) function into the modern split-by-string form.
 */
bool DataModel::ProjectModel::migrateLegacySeparator(const QJsonObject& json)
{
  if (!json.contains("separator"))
    return false;

  const auto separator = json.value("separator").toString();
  static QRegularExpression legacyRegex(
    R"(function\s+parse\s*\(\s*frame\s*,\s*separator\s*\)\s*\{\s*return\s+frame\.split\(separator\);\s*\})");

  if (m_sources.empty() || !legacyRegex.match(m_sources[0].frameParserCode).hasMatch())
    return false;

  if (separator.length() > 1)
    m_sources[0].frameParserCode =
      QStringLiteral("/**\n * Automatically migrated frame parser function.\n"
                     " */\nfunction parse(frame) {\n    return frame.split(\"%1\");\n}")
        .arg(separator);
  else
    m_sources[0].frameParserCode =
      QStringLiteral("/**\n * Automatically migrated frame parser function.\n"
                     " */\nfunction parse(frame) {\n    return frame.split(\'%1\');\n}")
        .arg(separator);

  if (!m_suppressMessageBoxes)
    Misc::Utilities::showMessageBox(
      tr("Legacy frame parser function updated"),
      tr("Your project used a legacy frame parser function with a 'separator' argument. "
         "It has been automatically migrated to the new format."),
      QMessageBox::Information);
  else
    qWarning() << "[ProjectModel] Legacy frame parser function automatically migrated";

  if (!m_filePath.isEmpty())
    (void)saveJsonFile(false);

  return true;
}

/**
 * @brief Emits the standard burst of "project loaded" signals for downstream views.
 */
void DataModel::ProjectModel::emitProjectLoadedSignals()
{
  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT tablesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();
  Q_EMIT controlScriptChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  if (m_widgetSettings.contains(Keys::kActiveGroupSubKey))
    Q_EMIT activeGroupIdChanged();

  if (!m_widgetSettings.isEmpty())
    Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Re-saves the project file to lock in a legacy-schema migration.
 */
void DataModel::ProjectModel::persistLegacyMigration()
{
  qInfo() << "[ProjectModel] Migrating legacy project to current schema, saving...";
  if (!writeProjectFile(m_filePath))
    qWarning() << "[ProjectModel] Legacy-migration save failed";
}

//--------------------------------------------------------------------------------------------------
// Selection state (used internally by group/dataset/action operations)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the editor's currently selected group.
 */
void DataModel::ProjectModel::setSelectedGroup(const DataModel::Group& group)
{
  m_selectedGroup = group;
}

/**
 * @brief Stores the editor's currently selected action.
 */
void DataModel::ProjectModel::setSelectedAction(const DataModel::Action& action)
{
  m_selectedAction = action;
}

/**
 * @brief Stores the editor's currently selected dataset.
 */
void DataModel::ProjectModel::setSelectedDataset(const DataModel::Dataset& dataset)
{
  m_selectedDataset = dataset;
}

/**
 * @brief Sets the frame decoder method and emits frameDetectionChanged.
 */
void DataModel::ProjectModel::setDecoderMethod(const SerialStudio::DecoderMethod method)
{
  if (m_frameDecoder == method)
    return;

  m_frameDecoder = method;

  if (m_sources.size() == 1)
    m_sources[0].decoderMethod = static_cast<int>(method);

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Toggles hexadecimal delimiter mode.
 */
void DataModel::ProjectModel::setHexadecimalDelimiters(const bool hexadecimal)
{
  if (m_hexadecimalDelimiters == hexadecimal)
    return;

  m_hexadecimalDelimiters = hexadecimal;

  if (m_sources.size() == 1)
    m_sources[0].hexadecimalDelimiters = hexadecimal;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Replaces the group at groupId and emits groupsChanged.
 */
void DataModel::ProjectModel::updateGroup(const int groupId,
                                          const DataModel::Group& group,
                                          const bool rebuildTree)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  m_groups[groupId] = group;

  if (rebuildTree)
    Q_EMIT groupsChanged();
  else
    Q_EMIT groupDataChanged();

  setModified(true);
}

/**
 * @brief Replaces the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectModel::updateDataset(const int groupId,
                                            const int datasetId,
                                            const DataModel::Dataset& dataset,
                                            const bool rebuildTree)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  DataModel::Dataset resolved = dataset;
  if (resolved.transformLanguage < 0 && !resolved.transformCode.isEmpty()) {
    for (const auto& src : m_sources)
      if (src.sourceId == resolved.sourceId) {
        resolved.transformLanguage = src.frameParserLanguage == SerialStudio::Native
                                     ? static_cast<int>(SerialStudio::Lua)
                                     : src.frameParserLanguage;
        break;
      }

    if (resolved.transformLanguage < 0)
      resolved.transformLanguage = 0;
  }

  m_groups[groupId].datasets[datasetId] = resolved;
  m_selectedDataset                     = resolved;

  if (rebuildTree)
    Q_EMIT groupsChanged();

  setModified(true);
}

/**
 * @brief Replaces the action at actionId and emits actionsChanged.
 */
void DataModel::ProjectModel::updateAction(const int actionId,
                                           const DataModel::Action& action,
                                           const bool rebuildTree)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  m_actions[actionId] = action;

  if (rebuildTree)
    Q_EMIT actionsChanged();

  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Group / dataset / action mutation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected group after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentGroup()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete group \"%1\"?").arg(m_selectedGroup.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_selectedGroup.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[gid]);

  m_groups.erase(m_groups.begin() + gid);

  int id = 0;
  for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
    g->groupId = id;
    for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
      d->groupId = id;
  }

  if (m_customizeWorkspaces)
    shiftWorkspaceRefsAfterGroupDelete(gid, deletedTypeCounts);

  shiftHiddenGroupIdsAfterGroupDelete(gid);
  shiftLayoutKeysAfterGroupDelete(gid);

  Q_EMIT groupsChanged();
  Q_EMIT groupDeleted();
  setModified(true);
}

/**
 * @brief Deletes the currently selected action after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentAction()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete action \"%1\"?").arg(m_selectedAction.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto aid = m_selectedAction.actionId;
  if (aid < 0 || static_cast<size_t>(aid) >= m_actions.size())
    return;

  m_actions.erase(m_actions.begin() + aid);

  int id = 0;
  for (auto a = m_actions.begin(); a != m_actions.end(); ++a, ++id)
    a->actionId = id;

  Q_EMIT actionsChanged();
  Q_EMIT actionDeleted();
  setModified(true);
}

/**
 * @brief Deletes the selected dataset, removing the group if it becomes empty.
 */
void DataModel::ProjectModel::deleteCurrentDataset()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete dataset \"%1\"?").arg(m_selectedDataset.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[groupId]);

  QMap<int, int> datasetTypeCounts;
  if (m_customizeWorkspaces) {
    const auto& ds  = m_groups[groupId].datasets[datasetId];
    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys)
      if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
        datasetTypeCounts[static_cast<int>(k)] += 1;
  }

  m_groups[groupId].datasets.erase(m_groups[groupId].datasets.begin() + datasetId);

  const auto& widgetId        = m_groups[groupId].widget;
  const bool widgetCanBeEmpty = (widgetId == QLatin1String("painter")
                                 || widgetId == QLatin1String("image") || widgetId.isEmpty());

  if (m_groups[groupId].datasets.empty() && !widgetCanBeEmpty) {
    m_groups.erase(m_groups.begin() + groupId);

    int id = 0;
    for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
      g->groupId = id;
      for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
        d->groupId = id;
    }

    if (m_customizeWorkspaces)
      shiftWorkspaceRefsAfterGroupDelete(groupId, deletedTypeCounts);

    shiftHiddenGroupIdsAfterGroupDelete(groupId);
    shiftLayoutKeysAfterGroupDelete(groupId);

    Q_EMIT groupsChanged();
    Q_EMIT datasetDeleted(-1);
    setModified(true);
    return;
  }

  int id     = 0;
  auto begin = m_groups[groupId].datasets.begin();
  auto end   = m_groups[groupId].datasets.end();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->datasetId = id;

  if (m_customizeWorkspaces)
    shiftWorkspaceRefsAfterDatasetDelete(groupId, datasetTypeCounts);

  Q_EMIT groupsChanged();
  Q_EMIT datasetDeleted(groupId);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected group to the project.
 */
void DataModel::ProjectModel::duplicateCurrentGroup()
{
  DataModel::Group group = m_selectedGroup;
  group.groupId          = m_groups.size();
  group.uniqueId         = allocateUniqueId();
  group.datasets.clear();
  group.outputWidgets.clear();

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(m_groups.size()));
  for (const auto& g : m_groups)
    existingTitles.append(g.title);

  group.title = nextDuplicateTitle(m_selectedGroup.title, existingTitles);

  for (size_t i = 0; i < m_selectedGroup.datasets.size(); ++i) {
    auto dataset     = m_selectedGroup.datasets[i];
    dataset.groupId  = group.groupId;
    dataset.index    = nextDatasetIndex() + static_cast<int>(i);
    dataset.uniqueId = allocateUniqueId();
    group.datasets.push_back(dataset);
  }

  for (const auto& ow : m_selectedGroup.outputWidgets) {
    auto copy    = ow;
    copy.groupId = group.groupId;
    group.outputWidgets.push_back(copy);
  }

  m_groups.push_back(group);
  m_selectedGroup = m_groups.back();

  Q_EMIT groupsChanged();
  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected action to the project.
 */
void DataModel::ProjectModel::duplicateCurrentAction()
{
  DataModel::Action action;
  action.actionId             = m_actions.size();
  action.icon                 = m_selectedAction.icon;
  action.txData               = m_selectedAction.txData;
  action.timerMode            = m_selectedAction.timerMode;
  action.repeatCount          = m_selectedAction.repeatCount;
  action.eolSequence          = m_selectedAction.eolSequence;
  action.timerIntervalMs      = m_selectedAction.timerIntervalMs;
  action.autoExecuteOnConnect = m_selectedAction.autoExecuteOnConnect;

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(m_actions.size()));
  for (const auto& a : m_actions)
    existingTitles.append(a.title);

  action.title = nextDuplicateTitle(m_selectedAction.title, existingTitles);

  m_actions.push_back(action);
  m_selectedAction = action;

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Group / dataset / workspace / action reorder
//--------------------------------------------------------------------------------------------------

namespace detail {
/**
 * @brief Stable anchor for a workspace ref across group/dataset reorders.
 */
struct RefAnchor {
  int widgetType;
  int sourceGid;
  int datasetFrameIndex;
  bool isGroupOrLed;
};
}  // namespace detail

/**
 * @brief Resolves a workspace ref into a stable RefAnchor before a reorder.
 */
static detail::RefAnchor anchorRef(const DataModel::WidgetRef& r,
                                   const std::vector<DataModel::Group>& groups)
{
  detail::RefAnchor a;
  a.widgetType        = r.widgetType;
  a.sourceGid         = r.groupUniqueId;
  a.datasetFrameIndex = -1;
  a.isGroupOrLed      = false;

  auto git = std::find_if(groups.begin(), groups.end(), [uid = r.groupUniqueId](const auto& g) {
    return g.uniqueId == uid;
  });
  if (git == groups.end())
    return a;

  const auto& g            = *git;
  const auto groupKey      = SerialStudio::getDashboardWidget(g);
  const bool emptyOutPanel = g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();
  const bool groupRef = SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !emptyOutPanel
                     && static_cast<int>(groupKey) == r.widgetType;
  const bool ledAggregate = (r.widgetType == static_cast<int>(SerialStudio::DashboardLED));
  if (groupRef || ledAggregate) {
    a.isGroupOrLed = true;
    return a;
  }

  int slot = 0;
  for (const auto& d : g.datasets) {
    const auto keys = SerialStudio::getDashboardWidgets(d);
    for (const auto& k : keys) {
      if (static_cast<int>(k) != r.widgetType)
        continue;

      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      if (slot == r.relativeIndex) {
        a.datasetFrameIndex = d.index;
        return a;
      }

      slot += 1;
    }
  }

  return a;
}

/**
 * @brief Re-resolves a RefAnchor into a per-type slot index in the given group.
 *        Returns -1 if the anchor's dataset is not present.
 */
static int slotForAnchor(const detail::RefAnchor& a, const DataModel::Group& g)
{
  if (a.datasetFrameIndex < 0)
    return -1;

  int slot = 0;
  for (const auto& d : g.datasets) {
    const auto keys = SerialStudio::getDashboardWidgets(d);
    for (const auto& k : keys) {
      if (static_cast<int>(k) != a.widgetType)
        continue;

      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      if (d.index == a.datasetFrameIndex)
        return slot;

      slot += 1;
    }
  }

  return -1;
}

/**
 * @brief Snapshots one anchor per workspace ref before a reorder.
 */
static std::vector<std::vector<detail::RefAnchor>> snapshotAllRefs(
  const std::vector<DataModel::Workspace>& workspaces, const std::vector<DataModel::Group>& groups)
{
  std::vector<std::vector<detail::RefAnchor>> out;
  out.resize(workspaces.size());
  for (size_t w = 0; w < workspaces.size(); ++w) {
    const auto& ws = workspaces[w];
    auto& bucket   = out[w];
    bucket.reserve(ws.widgetRefs.size());
    for (const auto& r : ws.widgetRefs)
      bucket.push_back(anchorRef(r, groups));
  }
  return out;
}

/**
 * @brief Walks one workspace's refs against the new group/dataset layout, refreshing
 *        the dataset slot. The group identity is uniqueId-based so it never needs remapping.
 */
static void resolveOneWorkspaceRefs(DataModel::Workspace& ws,
                                    const std::vector<detail::RefAnchor>& src,
                                    const std::vector<DataModel::Group>& groups)
{
  Q_ASSERT(src.size() == ws.widgetRefs.size());

  for (size_t i = 0; i < ws.widgetRefs.size(); ++i) {
    auto& r       = ws.widgetRefs[i];
    const auto& a = src[i];

    if (a.sourceGid < 0 || a.isGroupOrLed)
      continue;

    auto git = std::find_if(groups.begin(), groups.end(), [uid = r.groupUniqueId](const auto& g) {
      return g.uniqueId == uid;
    });
    if (git == groups.end())
      continue;

    const int newSlot = slotForAnchor(a, *git);
    if (newSlot >= 0)
      r.relativeIndex = newSlot;
  }
}

/**
 * @brief Moves a group from one position to another, preserving widget settings,
 *        workspace refs, hidden state, and auto-workspace IDs across the shift.
 */
void DataModel::ProjectModel::moveGroup(int fromGroupId, int toGroupId)
{
  const int n = static_cast<int>(m_groups.size());
  if (fromGroupId < 0 || fromGroupId >= n)
    return;

  const int target = std::clamp(toGroupId, 0, n - 1);
  if (target == fromGroupId)
    return;

  std::vector<int> oldToNewGid(static_cast<size_t>(n));
  for (int i = 0; i < n; ++i)
    oldToNewGid[static_cast<size_t>(i)] = i;

  if (fromGroupId < target)
    for (int i = fromGroupId + 1; i <= target; ++i)
      oldToNewGid[static_cast<size_t>(i)] = i - 1;

  else
    for (int i = target; i < fromGroupId; ++i)
      oldToNewGid[static_cast<size_t>(i)] = i + 1;

  oldToNewGid[static_cast<size_t>(fromGroupId)] = target;

  std::vector<std::vector<detail::RefAnchor>> anchors;
  if (m_customizeWorkspaces)
    anchors = snapshotAllRefs(m_workspaces, m_groups);

  auto group = m_groups[fromGroupId];
  m_groups.erase(m_groups.begin() + fromGroupId);
  m_groups.insert(m_groups.begin() + target, group);

  remapGroupIdsAfterReorder(oldToNewGid);

  remapLayoutKeysAfterReorder(oldToNewGid);
  remapHiddenGroupIdsAfterReorder(oldToNewGid);
  remapAutoWorkspaceIdsAfterReorder(oldToNewGid);

  if (m_customizeWorkspaces)
    for (size_t w = 0; w < m_workspaces.size(); ++w)
      resolveOneWorkspaceRefs(m_workspaces[w], anchors[w], m_groups);

  if (m_selectedGroup.groupId == fromGroupId)
    m_selectedGroup = m_groups[target];

  Q_EMIT groupsChanged();
  Q_EMIT widgetSettingsChanged();
  setModified(true);
}

/**
 * @brief Moves a dataset within its group, renumbering datasetIds and re-resolving
 *        workspace refs in customise mode.
 */
void DataModel::ProjectModel::moveDataset(int groupId, int fromDatasetId, int toDatasetId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& datasets = m_groups[groupId].datasets;
  const int n    = static_cast<int>(datasets.size());
  if (fromDatasetId < 0 || fromDatasetId >= n)
    return;

  const int target = std::clamp(toDatasetId, 0, n - 1);
  if (target == fromDatasetId)
    return;

  std::vector<std::vector<detail::RefAnchor>> anchors;
  if (m_customizeWorkspaces)
    anchors = snapshotAllRefs(m_workspaces, m_groups);

  auto dataset = datasets[fromDatasetId];
  datasets.erase(datasets.begin() + fromDatasetId);
  datasets.insert(datasets.begin() + target, dataset);

  for (size_t i = 0; i < datasets.size(); ++i)
    datasets[i].datasetId = static_cast<int>(i);

  if (m_customizeWorkspaces)
    for (size_t w = 0; w < m_workspaces.size(); ++w)
      resolveOneWorkspaceRefs(m_workspaces[w], anchors[w], m_groups);

  if (m_selectedDataset.groupId == groupId && m_selectedDataset.datasetId == fromDatasetId)
    m_selectedDataset = datasets[target];

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Moves a workspace to the given index in the editor list.
 *        Auto workspaces (id < UserStart) are pinned to the top and ignored.
 */
void DataModel::ProjectModel::moveWorkspace(int workspaceId, int targetIndex)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (workspaceId < WorkspaceIds::UserStart)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  const int from = static_cast<int>(std::distance(m_workspaces.begin(), it));

  int firstUserSlot = 0;
  for (const auto& ws : m_workspaces) {
    if (ws.workspaceId >= WorkspaceIds::UserStart)
      break;

    firstUserSlot += 1;
  }

  const int last   = static_cast<int>(m_workspaces.size()) - 1;
  const int target = std::clamp(targetIndex, firstUserSlot, last);
  if (target == from)
    return;

  auto ws = *it;
  m_workspaces.erase(it);
  m_workspaces.insert(m_workspaces.begin() + target, ws);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Moves an action within the project actions list, renumbering actionIds.
 */
void DataModel::ProjectModel::moveAction(int fromActionId, int toActionId)
{
  const int n = static_cast<int>(m_actions.size());
  if (fromActionId < 0 || fromActionId >= n)
    return;

  const int target = std::clamp(toActionId, 0, n - 1);
  if (target == fromActionId)
    return;

  auto action = m_actions[fromActionId];
  m_actions.erase(m_actions.begin() + fromActionId);
  m_actions.insert(m_actions.begin() + target, action);

  for (size_t i = 0; i < m_actions.size(); ++i)
    m_actions[i].actionId = static_cast<int>(i);

  if (m_selectedAction.actionId == fromActionId)
    m_selectedAction = m_actions[target];

  Q_EMIT actionsChanged();
  setModified(true);
}

/**
 * @brief Moves an output widget within its group's outputWidgets list.
 */
void DataModel::ProjectModel::moveOutputWidget(int groupId, int fromWidgetId, int toWidgetId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& widgets = m_groups[groupId].outputWidgets;
  const int n   = static_cast<int>(widgets.size());
  if (fromWidgetId < 0 || fromWidgetId >= n)
    return;

  const int target = std::clamp(toWidgetId, 0, n - 1);
  if (target == fromWidgetId)
    return;

  auto widget = widgets[fromWidgetId];
  widgets.erase(widgets.begin() + fromWidgetId);
  widgets.insert(widgets.begin() + target, widget);

  for (size_t i = 0; i < widgets.size(); ++i)
    widgets[i].widgetId = static_cast<int>(i);

  if (m_selectedOutputWidget.groupId == groupId && m_selectedOutputWidget.widgetId == fromWidgetId)
    m_selectedOutputWidget = widgets[target];

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Renumbers Group::groupId (and child Dataset::groupId) to match the new vector order.
 */
void DataModel::ProjectModel::remapGroupIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  Q_ASSERT(oldToNewGid.size() == m_groups.size());

  for (size_t i = 0; i < m_groups.size(); ++i) {
    auto& g   = m_groups[i];
    g.groupId = static_cast<int>(i);
    for (auto& d : g.datasets)
      d.groupId = g.groupId;
  }

  Q_UNUSED(oldToNewGid);
}

/**
 * @brief Updates m_hiddenGroupIds so each hidden ID follows its renamed group.
 */
void DataModel::ProjectModel::remapHiddenGroupIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  if (m_hiddenGroupIds.isEmpty())
    return;

  QSet<int> updated;
  for (const int id : std::as_const(m_hiddenGroupIds)) {
    if (id < 0 || static_cast<size_t>(id) >= oldToNewGid.size())
      continue;

    updated.insert(oldToNewGid[static_cast<size_t>(id)]);
  }

  m_hiddenGroupIds = std::move(updated);
}

/**
 * @brief Rewrites every layout:N widgetSettings entry to use the new groupId.
 */
void DataModel::ProjectModel::remapLayoutKeysAfterReorder(const std::vector<int>& oldToNewGid)
{
  if (m_widgetSettings.isEmpty())
    return;

  const QString prefix = QStringLiteral("layout:");
  QMap<int, QJsonObject> snapshot;

  for (const auto& key : m_widgetSettings.keys()) {
    if (!key.startsWith(prefix))
      continue;

    bool ok      = false;
    const int id = key.mid(prefix.length()).toInt(&ok);
    if (!ok || id < 0 || static_cast<size_t>(id) >= oldToNewGid.size())
      continue;

    snapshot.insert(id, m_widgetSettings.value(key).toObject());
    m_widgetSettings.remove(key);
  }

  for (auto it = snapshot.constBegin(); it != snapshot.constEnd(); ++it) {
    const int newId = oldToNewGid[static_cast<size_t>(it.key())];
    m_widgetSettings.insert(Keys::layoutKey(newId), it.value());
  }
}

/**
 * @brief Renames per-group auto workspaces (PerGroupStart + groupId) so they
 *        track their group across a reorder.
 */
void DataModel::ProjectModel::remapAutoWorkspaceIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  for (auto& ws : m_workspaces) {
    if (ws.workspaceId < WorkspaceIds::PerGroupStart || ws.workspaceId >= WorkspaceIds::UserStart)
      continue;

    const int oldGid = ws.workspaceId - WorkspaceIds::PerGroupStart;
    if (oldGid < 0 || static_cast<size_t>(oldGid) >= oldToNewGid.size())
      continue;

    ws.workspaceId = WorkspaceIds::PerGroupStart + oldToNewGid[static_cast<size_t>(oldGid)];
  }

  std::stable_sort(
    m_workspaces.begin(), m_workspaces.end(), [](const Workspace& a, const Workspace& b) {
      const bool aUser = a.workspaceId >= WorkspaceIds::UserStart;
      const bool bUser = b.workspaceId >= WorkspaceIds::UserStart;
      if (aUser != bUser)
        return !aUser;

      if (!aUser && !bUser)
        return a.workspaceId < b.workspaceId;

      return false;
    });
}

//--------------------------------------------------------------------------------------------------
// Output widget CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the editor's currently selected output widget.
 */
void DataModel::ProjectModel::setSelectedOutputWidget(const DataModel::OutputWidget& widget)
{
  m_selectedOutputWidget = widget;
}

/**
 * @brief Changes the type of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetType(int type)
{
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  const auto newType = static_cast<DataModel::OutputWidgetType>(
    qBound(0, type, static_cast<int>(DataModel::OutputWidgetType::Knob)));

  if (widgets[wid].type == newType)
    return;

  widgets[wid].type           = newType;
  m_selectedOutputWidget.type = newType;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(gid, wid);
  setModified(true);
}

/**
 * @brief Sets the icon of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetIcon(const QString& icon)
{
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  widgets[wid].icon           = icon;
  m_selectedOutputWidget.icon = icon;

  Q_EMIT groupDataChanged();
  Q_EMIT outputWidgetAdded(gid, wid);
  setModified(true);
}

/**
 * @brief Creates a new output group with a default button control.
 */
void DataModel::ProjectModel::addOutputPanel(int sourceId)
{
  addGroup(tr("Output Controls"), SerialStudio::NoGroupWidget, sourceId);
  auto& group     = m_groups.back();
  group.groupType = DataModel::GroupType::Output;
  m_selectedGroup = group;

  addOutputControl(SerialStudio::OutputButton, sourceId);
}

/**
 * @brief Adds an output control, creating a new output group if needed.
 */
void DataModel::ProjectModel::addOutputControl(const SerialStudio::OutputWidgetType type,
                                               int sourceId)
{
  int groupId    = -1;
  const auto sel = m_selectedGroup.groupId;
  if (sel >= 0 && static_cast<size_t>(sel) < m_groups.size()
      && m_groups[sel].groupType == DataModel::GroupType::Output
      && (sourceId < 0 || m_groups[sel].sourceId == sourceId))
    groupId = sel;

  if (groupId < 0) {
    for (const auto& g : std::as_const(m_groups)) {
      if (g.groupType != DataModel::GroupType::Output)
        continue;

      if (sourceId >= 0 && g.sourceId != sourceId)
        continue;

      groupId         = g.groupId;
      m_selectedGroup = g;
      break;
    }
  }

  if (groupId < 0) {
    addGroup(tr("Output Controls"), SerialStudio::NoGroupWidget, sourceId);
    auto& group     = m_groups.back();
    group.groupType = DataModel::GroupType::Output;
    groupId         = group.groupId;
    m_selectedGroup = group;
  }

  auto& group = m_groups[groupId];

  QString title;
  switch (type) {
    case SerialStudio::OutputButton:
      title = tr("New Button");
      break;
    case SerialStudio::OutputSlider:
      title = tr("New Slider");
      break;
    case SerialStudio::OutputToggle:
      title = tr("New Toggle");
      break;
    case SerialStudio::OutputTextField:
      title = tr("New Text Field");
      break;
    case SerialStudio::OutputKnob:
      title = tr("New Knob");
      break;
  }

  DataModel::OutputWidget ow;
  ow.widgetId         = static_cast<int>(group.outputWidgets.size());
  ow.groupId          = groupId;
  ow.sourceId         = group.sourceId;
  ow.title            = title;
  ow.type             = static_cast<DataModel::OutputWidgetType>(type);
  ow.transmitFunction = DataModel::OutputCodeEditor::defaultTemplate();

  group.outputWidgets.push_back(ow);
  m_selectedOutputWidget = ow;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(groupId, ow.widgetId);
  setModified(true);
}

/**
 * @brief Deletes the currently selected output widget after confirmation.
 */
void DataModel::ProjectModel::deleteCurrentOutputWidget()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete output widget \"%1\"?").arg(m_selectedOutputWidget.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  widgets.erase(widgets.begin() + wid);

  if (widgets.empty()) {
    m_groups.erase(m_groups.begin() + gid);

    for (int i = 0; i < static_cast<int>(m_groups.size()); ++i)
      m_groups[i].groupId = i;

    Q_EMIT groupsChanged();
    Q_EMIT groupDeleted();
    setModified(true);
    return;
  }

  for (int i = 0; i < static_cast<int>(widgets.size()); ++i)
    widgets[i].widgetId = i;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetDeleted(gid);
  setModified(true);
}

/**
 * @brief Duplicates the currently selected output widget.
 */
void DataModel::ProjectModel::duplicateCurrentOutputWidget()
{
  const auto gid = m_selectedOutputWidget.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;

  DataModel::OutputWidget ow = m_selectedOutputWidget;
  ow.widgetId                = static_cast<int>(widgets.size());

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(widgets.size()));
  for (const auto& w : widgets)
    existingTitles.append(w.title);

  ow.title = nextDuplicateTitle(m_selectedOutputWidget.title, existingTitles);

  widgets.push_back(ow);
  m_selectedOutputWidget = ow;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(gid, ow.widgetId);
  setModified(true);
}

/**
 * @brief Updates an output widget in place.
 */
void DataModel::ProjectModel::updateOutputWidget(int groupId,
                                                 int widgetId,
                                                 const DataModel::OutputWidget& widget,
                                                 bool rebuildTree)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& widgets = m_groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return;

  widgets[widgetId] = widget;

  if (rebuildTree)
    Q_EMIT groupsChanged();
  else
    Q_EMIT groupDataChanged();

  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected dataset to its parent group.
 */
void DataModel::ProjectModel::duplicateCurrentDataset()
{
  auto dataset = m_selectedDataset;

  if (dataset.groupId < 0 || static_cast<size_t>(dataset.groupId) >= m_groups.size())
    return;

  dataset.index     = nextDatasetIndex();
  dataset.datasetId = m_groups[dataset.groupId].datasets.size();
  dataset.uniqueId  = allocateUniqueId();

  const auto& siblings = m_groups[dataset.groupId].datasets;
  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(siblings.size()));
  for (const auto& d : siblings)
    existingTitles.append(d.title);

  dataset.title = nextDuplicateTitle(m_selectedDataset.title, existingTitles);

  m_groups[dataset.groupId].datasets.push_back(dataset);
  m_selectedDataset = dataset;

  Q_EMIT groupsChanged();
  Q_EMIT datasetAdded(dataset.groupId,
                      static_cast<int>(m_groups[dataset.groupId].datasets.size()) - 1);
  setModified(true);
}

/**
 * @brief Ensures a compatible group is selected before adding a dataset; honors sourceId scoping.
 */
void DataModel::ProjectModel::ensureValidGroup(int sourceId)
{
  const auto isValidGroup = [sourceId](const DataModel::Group& g) -> bool {
    if (g.groupType == DataModel::GroupType::Output)
      return false;

    if (sourceId >= 0 && g.sourceId != sourceId)
      return false;

    switch (SerialStudio::groupWidgetFromId(g.widget)) {
      case SerialStudio::MultiPlot:
      case SerialStudio::DataGrid:
      case SerialStudio::NoGroupWidget:
        return true;
      default:
        return false;
    }
  };

  const auto selId      = m_selectedGroup.groupId;
  const bool selInRange = selId >= 0 && static_cast<size_t>(selId) < m_groups.size();

  if (selInRange && isValidGroup(m_groups[selId])) {
    m_selectedGroup = m_groups[selId];
    return;
  }

  for (const auto& group : std::as_const(m_groups)) {
    if (!isValidGroup(group))
      continue;

    m_selectedGroup = group;
    return;
  }

  addGroup(tr("Group"), SerialStudio::NoGroupWidget, sourceId);
  m_selectedGroup = m_groups.back();
}

/**
 * @brief Adds a new dataset of the given type to the selected group.
 */
void DataModel::ProjectModel::addDataset(const SerialStudio::DatasetOption option, int sourceId)
{
  ensureValidGroup(sourceId);

  const auto groupId = m_selectedGroup.groupId;
  DataModel::Dataset dataset;
  dataset.groupId = groupId;

  QString title;
  switch (option) {
    case SerialStudio::DatasetGeneric:
      title = tr("New Dataset");
      break;
    case SerialStudio::DatasetPlot:
      title       = tr("New Plot");
      dataset.plt = true;
      break;
    case SerialStudio::DatasetFFT:
      title       = tr("New FFT Plot");
      dataset.fft = true;
      break;
    case SerialStudio::DatasetBar:
      title          = tr("New Level Indicator");
      dataset.widget = QStringLiteral("bar");
      break;
    case SerialStudio::DatasetGauge:
      title          = tr("New Gauge");
      dataset.widget = QStringLiteral("gauge");
      break;
    case SerialStudio::DatasetCompass:
      title          = tr("New Compass");
      dataset.wgtMin = 0;
      dataset.wgtMax = 360;
      dataset.widget = QStringLiteral("compass");
      break;
    case SerialStudio::DatasetMeter:
      title          = tr("New Meter");
      dataset.widget = QStringLiteral("meter");
      break;
    case SerialStudio::DatasetLED:
      title       = tr("New LED Indicator");
      dataset.led = true;
      break;
    case SerialStudio::DatasetWaterfall:
      title             = tr("New Waterfall");
      dataset.waterfall = true;
      break;
    default:
      break;
  }

  int count        = 1;
  QString newTitle = title;
  for (const auto& d : std::as_const(m_groups[groupId].datasets)) {
    if (d.title == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& d : std::as_const(m_groups[groupId].datasets)) {
      if (d.title != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  dataset.title     = newTitle;
  dataset.index     = nextDatasetIndex();
  dataset.datasetId = m_groups[groupId].datasets.size();
  dataset.uniqueId  = allocateUniqueId();

  m_groups[groupId].datasets.push_back(dataset);
  m_selectedDataset = dataset;

  Q_EMIT groupsChanged();
  Q_EMIT datasetAdded(groupId, static_cast<int>(m_groups[groupId].datasets.size()) - 1);
  setModified(true);
}

/**
 * @brief Appends template-defined datasets to a painter group when the group has fewer than the
 * spec demands. Existing datasets are preserved.
 */
void DataModel::ProjectModel::ensurePainterDatasets(int groupId, const QVariantList& specs)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (specs.isEmpty())
    return;

  auto& grp          = m_groups[groupId];
  const int existing = static_cast<int>(grp.datasets.size());
  bool changed       = false;

  for (int i = existing; i < specs.size(); ++i) {
    const auto map = specs.at(i).toMap();
    DataModel::Dataset ds;
    ds.groupId   = groupId;
    ds.datasetId = static_cast<int>(grp.datasets.size());
    ds.index     = nextDatasetIndex();
    ds.uniqueId  = allocateUniqueId();
    ds.title     = map.value(QStringLiteral("title"), tr("Channel %1").arg(i + 1)).toString();
    ds.units     = map.value(QStringLiteral("units")).toString();
    ds.wgtMin    = SerialStudio::toDouble(map.value(QStringLiteral("min"), 0.0));
    ds.wgtMax    = SerialStudio::toDouble(map.value(QStringLiteral("max"), 100.0));
    grp.datasets.push_back(std::move(ds));
    changed = true;
  }

  if (changed) {
    m_selectedGroup = grp;
    Q_EMIT groupsChanged();
    setModified(true);
  }
}

/**
 * @brief Toggles a dataset option flag on the currently selected dataset.
 */
void DataModel::ProjectModel::changeDatasetOption(const SerialStudio::DatasetOption option,
                                                  const bool checked)
{
  switch (option) {
    case SerialStudio::DatasetPlot:
      m_selectedDataset.plt = checked;
      break;
    case SerialStudio::DatasetFFT:
      m_selectedDataset.fft = checked;
      break;
    case SerialStudio::DatasetBar:
      m_selectedDataset.widget = checked ? QStringLiteral("bar") : "";
      break;
    case SerialStudio::DatasetGauge:
      m_selectedDataset.widget = checked ? QStringLiteral("gauge") : "";
      break;
    case SerialStudio::DatasetCompass:
      m_selectedDataset.widget = checked ? QStringLiteral("compass") : "";
      break;
    case SerialStudio::DatasetMeter:
      m_selectedDataset.widget = checked ? QStringLiteral("meter") : "";
      break;
    case SerialStudio::DatasetLED:
      m_selectedDataset.led = checked;
      break;
    case SerialStudio::DatasetWaterfall:
      m_selectedDataset.waterfall = checked;
      break;
    default:
      break;
  }

  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  m_groups[groupId].datasets[datasetId] = m_selectedDataset;

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Adds a new action with a unique title to the project.
 */
void DataModel::ProjectModel::addAction(int sourceId)
{
  int count     = 1;
  QString title = tr("New Action");
  for (const auto& action : std::as_const(m_actions)) {
    if (action.title == title) {
      count++;
      title = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& action : std::as_const(m_actions)) {
      if (action.title != title)
        continue;

      count++;
      title       = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  DataModel::Action action;
  action.title    = title;
  action.actionId = m_actions.size();
  if (sourceId >= 0)
    action.sourceId = sourceId;

  m_actions.push_back(action);
  m_selectedAction = action;

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
  setModified(true);
}

/**
 * @brief Adds a new group with a unique title and the given widget type.
 */
void DataModel::ProjectModel::addGroup(const QString& title,
                                       const SerialStudio::GroupWidget widget,
                                       int sourceId)
{
  int count        = 1;
  QString newTitle = title;
  for (const auto& group : std::as_const(m_groups)) {
    if (group.title == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& group : std::as_const(m_groups)) {
      if (group.title != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  DataModel::Group group;
  group.title    = newTitle;
  group.groupId  = m_groups.size();
  group.uniqueId = allocateUniqueId();

  if (sourceId >= 0)
    group.sourceId = sourceId;

  m_groups.push_back(group);
  setGroupWidget(static_cast<int>(m_groups.size()) - 1, widget);
  m_selectedGroup = m_groups.back();

  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Assigns a widget type to the group, replacing fixed-layout datasets.
 */
bool DataModel::ProjectModel::setGroupWidget(const int group,
                                             const SerialStudio::GroupWidget widget)
{
  if (group < 0 || group >= static_cast<int>(m_groups.size())) [[unlikely]]
    return false;

  auto& grp = m_groups[group];

  if (!confirmGroupWidgetChange(grp, widget))
    return false;

  if (!applyGroupWidget(grp, widget))
    return false;

  for (auto& d : grp.datasets)
    if (d.uniqueId < 0)
      d.uniqueId = allocateUniqueId();

  m_groups[group] = grp;

  Q_EMIT groupsChanged();
  setModified(true);
  return true;
}

/**
 * @brief Confirms a destructive group widget change and clears existing datasets if needed.
 */
bool DataModel::ProjectModel::confirmGroupWidgetChange(DataModel::Group& grp,
                                                       SerialStudio::GroupWidget widget)
{
  if (grp.datasets.empty())
    return true;

  if (widget == SerialStudio::Painter) {
    grp.widget = "painter";
    return true;
  }

  const bool compatibleTarget =
    (widget == SerialStudio::DataGrid || widget == SerialStudio::MultiPlot
     || widget == SerialStudio::NoGroupWidget);
  const bool compatibleSource = (grp.widget == "multiplot" || grp.widget == "datagrid"
                                 || grp.widget == "painter" || grp.widget == "");
  if (compatibleTarget && compatibleSource) {
    grp.widget = "";
    return true;
  }

  auto ret = Misc::Utilities::showMessageBox(tr("Are you sure you want to change the group-level "
                                                "widget?"),
                                             tr("Existing datasets for this group are deleted"),
                                             QMessageBox::Question,
                                             APP_NAME,
                                             QMessageBox::Yes | QMessageBox::No);

  if (ret == QMessageBox::No)
    return false;

  grp.datasets.clear();
  return true;
}

/**
 * @brief Assigns a group widget tag and any canonical datasets for fixed-layout types.
 */
bool DataModel::ProjectModel::applyGroupWidget(DataModel::Group& grp,
                                               SerialStudio::GroupWidget widget)
{
  if (widget == SerialStudio::NoGroupWidget) {
    grp.widget = "";
    return true;
  }

  if (widget == SerialStudio::DataGrid) {
    grp.widget = "datagrid";
    return true;
  }

  if (widget == SerialStudio::MultiPlot) {
    grp.widget = "multiplot";
    return true;
  }

  if (widget == SerialStudio::ImageView) {
    grp.widget = "image";
    return true;
  }

  if (widget == SerialStudio::Painter) {
    grp.widget = "painter";
    if (grp.datasets.empty())
      return populateFixedLayoutGroup(grp, widget);

    return true;
  }

  return populateFixedLayoutGroup(grp, widget);
}

/**
 * @brief Fills a group with the three-axis canonical datasets for sensor-style widgets.
 */
bool DataModel::ProjectModel::populateFixedLayoutGroup(DataModel::Group& grp,
                                                       SerialStudio::GroupWidget widget)
{
  const int baseIndex = nextDatasetIndex();

  if (widget == SerialStudio::Accelerometer) {
    // code-verify off
    ThreeAxisLayout layout{
      "accelerometer",
      {                            "x","y",                    "z"                                                },
      {                         "m/s²", "m/s²",                         "m/s²"},
      {tr("Accelerometer %1").arg("X"),
        tr("Accelerometer %1").arg("Y"),
        tr("Accelerometer %1").arg("Z")                                        },
      {                              0,      0,                              0},
      {                              0,      0,                              0},
      true
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Gyroscope) {
    ThreeAxisLayout layout{
      "gyro",
      {                   "x",                    "y",                    "z"},
      {               "deg/s",                "deg/s",                "deg/s"},
      {tr("Gyro %1").arg("X"), tr("Gyro %1").arg("Y"), tr("Gyro %1").arg("Z")},
      {                     0,                      0,                      0},
      {                     0,                      0,                      0},
      true
    };
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::GPS) {
    // code-verify off
    ThreeAxisLayout layout{
      "map",
      {         "lat",           "lon",          "alt"},
      {           "°",             "°",            "m"},
      {tr("Latitude"), tr("Longitude"), tr("Altitude")},
      {         -90.0,          -180.0,         -500.0},
      {          90.0,           180.0,      1000000.0},
      false
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Plot3D) {
    ThreeAxisLayout layout{
      "plot3d",
      {    "x",     "y",     "z"},
      {     "",      "",      ""},
      {tr("X"), tr("Y"), tr("Z")},
      {      0,       0,       0},
      {      0,       0,       0},
      false
    };
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Painter) {
    ThreeAxisLayout layout{
      "painter",
      {     "",      "",      ""},
      {     "",      "",      ""},
      {tr("X"), tr("Y"), tr("Z")},
      {   -100,    -100,    -100},
      {    100,     100,     100},
      false
    };
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Scalar property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the project's modification flag and emits modifiedChanged; a
 * request to dirty a truly empty project intentionally leaves the flag clean for
 * the dirty-flag UX but still emits contentTouched so the backup layer (whose
 * snapshot hash decides) gets nudged.
 */
void DataModel::ProjectModel::setModified(const bool modified)
{
  if (modified && m_groups.empty() && m_actions.empty() && m_tables.empty() && m_workspaces.empty()
      && !m_customizeWorkspaces && !m_locked && m_hiddenGroupIds.isEmpty()) {
    Q_EMIT contentTouched();
    return;
  }

  m_modified = modified;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Sets source[0].frameParserCode and emits frameParserCodeChanged.
 */
void DataModel::ProjectModel::setFrameParserCode(const QString& code)
{
  if (m_sources.empty() || code == m_sources[0].frameParserCode)
    return;

  m_sources[0].frameParserCode = code;
  setModified(true);
  Q_EMIT frameParserCodeChanged();
  Q_EMIT sourceFrameParserCodeChanged(0);
}

/**
 * @brief Sets the scripting language for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserLanguage(int language)
{
  if (m_sources.empty() || language == m_sources[0].frameParserLanguage)
    return;

  m_sources[0].frameParserLanguage = language;
  setModified(true);
  Q_EMIT frameParserLanguageChanged();
  Q_EMIT sourceFrameParserLanguageChanged(0);
}

/**
 * @brief Sets the scripting language for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserLanguage(int sourceId, int language)
{
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end())
    return;

  if (it->frameParserLanguage == language)
    return;

  it->frameParserLanguage = language;
  setModified(true);

  if (sourceId == 0)
    Q_EMIT frameParserLanguageChanged();

  Q_EMIT sourceFrameParserLanguageChanged(sourceId);
}

/**
 * @brief Sets the native parser template id for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserTemplate(const QString& templateId)
{
  if (m_sources.empty())
    return;

  updateSourceFrameParserTemplate(m_sources[0].sourceId, templateId);
}

/**
 * @brief Sets the native parser template params for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserParams(const QJsonObject& params)
{
  if (m_sources.empty())
    return;

  updateSourceFrameParserParams(m_sources[0].sourceId, params);
}

/**
 * @brief Sets the native parser template id for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserTemplate(int sourceId,
                                                              const QString& templateId)
{
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end() || it->frameParserTemplate == templateId)
    return;

  it->frameParserTemplate = templateId;
  setModified(true);

  if (sourceId == 0)
    Q_EMIT frameParserTemplateChanged();

  Q_EMIT sourceFrameParserTemplateChanged(sourceId);
}

/**
 * @brief Sets the native parser template params for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserParams(int sourceId, const QJsonObject& params)
{
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end() || it->frameParserParams == params)
    return;

  it->frameParserParams = params;
  setModified(true);

  if (sourceId == 0)
    Q_EMIT frameParserParamsChanged();

  Q_EMIT sourceFrameParserParamsChanged(sourceId);
}

/**
 * @brief Stores frame parser code without emitting signals or reloading the JS engine.
 */
void DataModel::ProjectModel::storeFrameParserCode(int sourceId, const QString& code)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  if (m_sources[sourceId].frameParserCode == code)
    return;

  m_sources[sourceId].frameParserCode = code;
  setModified(true);
}

/**
 * @brief Stages the active dashboard tab group ID.
 */
void DataModel::ProjectModel::setActiveGroupId(const int groupId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  const int current = m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
  if (current == groupId)
    return;

  if (groupId >= 0)
    m_widgetSettings.insert(Keys::kActiveGroupSubKey, groupId);
  else
    m_widgetSettings.remove(Keys::kActiveGroupSubKey);

  setModified(true);
  Q_EMIT activeGroupIdChanged();
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Stages the widget layout for a specific group.
 */
void DataModel::ProjectModel::setGroupLayout(const int groupId, const QJsonObject& layout)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  QJsonObject entry;
  entry[QStringLiteral("data")] = layout;
  m_widgetSettings.insert(Keys::layoutKey(groupId), entry);

  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

// code-verify off
//--------------------------------------------------------------------------------------------------
// Workspace CRUD
//
// State machine:
//   - Auto state (m_customizeWorkspaces == false):
//       m_workspaces is a derived view, rebuilt from m_groups on every
//       groupsChanged via regenerateAutoWorkspacesUnnotified(). Treat it as
//       read-only. m_autoSnapshot mirrors m_workspaces.
//       Persistence: neither customizeWorkspaces nor workspaces emitted in JSON.
//   - Customized state (m_customizeWorkspaces == true):
//       m_workspaces is user-owned. Structural changes invoke
//       mergeAutoWorkspaceUpdates() which adds first-appearance auto refs
//       without resurrecting user-removed entries (m_autoSnapshot is the
//       diff baseline).
//       Persistence: both keys emitted verbatim.
//
// Transitions (all via setCustomizeWorkspaces):
//   - Auto -> Customized: seed m_workspaces from buildAutoWorkspaces() so the
//     editor never opens empty. Refresh m_autoSnapshot.
//   - Customized -> Auto: discard user list, regenerate from groups.
//
// Workspace ID layout (see WorkspaceIds:: in Frame.h):
//   [1000, 5000) = auto IDs   (Overview=1000, AllData=1001, per-group=1002+gid)
//   [5000, ...)  = user IDs   (addWorkspace picks max(>=5000)+1)
// Disjoint ranges: a future group can never claim an ID a user picked.
//
// m_hiddenGroupIds removes per-group entries from buildAutoWorkspaces output;
// it is independent of customize state and survives Customize toggles.
//
// code-verify on
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new user-defined workspace with the given title.
 */
int DataModel::ProjectModel::addWorkspace(const QString& title)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  int maxId = WorkspaceIds::UserStart - 1;
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId >= WorkspaceIds::UserStart && ws.workspaceId > maxId)
      maxId = ws.workspaceId;

  DataModel::Workspace ws;
  ws.workspaceId = maxId + 1;
  ws.title       = title.simplified().isEmpty() ? tr("Workspace") : title.simplified();
  m_workspaces.push_back(ws);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return ws.workspaceId;
}

/**
 * @brief Deletes the workspace with the given ID.
 */
void DataModel::ProjectModel::deleteWorkspace(int workspaceId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  m_workspaces.erase(it);
  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Wipes every workspace, leaving an empty customised list.
 */
void DataModel::ProjectModel::clearAllWorkspaces()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  if (m_workspaces.empty())
    return;

  m_workspaces.clear();
  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Renames the workspace with the given ID.
 */
void DataModel::ProjectModel::renameWorkspace(int workspaceId, const QString& title)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      ws.title = title.simplified();
      setModified(true);
      Q_EMIT editorWorkspacesChanged();
      Q_EMIT activeWorkspacesChanged();
      return;
    }
  }
}

/**
 * @brief Patches title, icon, and/or description on the workspace with the given ID.
 */
void DataModel::ProjectModel::updateWorkspace(int workspaceId,
                                              const QString& title,
                                              const QString& icon,
                                              const QString& description,
                                              bool setTitle,
                                              bool setIcon,
                                              bool setDescription)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      if (setTitle)
        ws.title = title.simplified();

      if (setIcon)
        ws.icon = SerialStudio::normalizeIconPath(icon);

      if (setDescription)
        ws.description = description;

      setModified(true);
      Q_EMIT editorWorkspacesChanged();
      Q_EMIT activeWorkspacesChanged();
      return;
    }
  }
}

/**
 * @brief Reorders user-defined workspaces (id >= UserStart) by the given id
 * sequence, bailing out when the id set does not match the existing user
 * workspaces because a partial reorder would silently corrupt the list.
 */
void DataModel::ProjectModel::reorderWorkspaces(const QList<int>& userWorkspaceIds)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  QHash<int, DataModel::Workspace> userById;
  std::vector<DataModel::Workspace> systemSlots;
  for (auto& ws : m_workspaces)
    if (ws.workspaceId >= WorkspaceIds::UserStart)
      userById.insert(ws.workspaceId, std::move(ws));
    else
      systemSlots.push_back(std::move(ws));

  if (userWorkspaceIds.size() != userById.size())
    return;

  for (int id : userWorkspaceIds)
    if (!userById.contains(id))
      return;

  std::vector<DataModel::Workspace> rebuilt;
  rebuilt.reserve(systemSlots.size() + userById.size());
  for (auto& ws : systemSlots)
    rebuilt.push_back(std::move(ws));

  for (int id : userWorkspaceIds)
    rebuilt.push_back(std::move(userById[id]));

  m_workspaces = std::move(rebuilt);

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

//--------------------------------------------------------------------------------------------------
// Data-table CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a new empty data table with a unique name derived from @p name.
 */
QString DataModel::ProjectModel::addTable(const QString& name)
{
  QString base = name.simplified();
  if (base.isEmpty())
    base = tr("Shared Table");

  QString unique     = base;
  int suffix         = 2;
  const auto hasName = [this](const QString& n) {
    for (const auto& t : m_tables)
      if (t.name == n)
        return true;

    return false;
  };

  while (hasName(unique))
    unique = QStringLiteral("%1 %2").arg(base, QString::number(suffix++));

  DataModel::TableDef table;
  table.name = unique;
  m_tables.push_back(table);
  setModified(true);
  Q_EMIT tablesChanged();
  return unique;
}

/**
 * @brief Deletes the table with the given @p name.
 */
void DataModel::ProjectModel::deleteTable(const QString& name)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&name](const auto& t) { return t.name == name; });

  if (it == m_tables.end())
    return;

  m_tables.erase(it);
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Renames a table (no-op if target name already exists).
 */
void DataModel::ProjectModel::renameTable(const QString& oldName, const QString& newName)
{
  const QString n = newName.simplified();
  if (n.isEmpty())
    return;

  for (const auto& t : m_tables)
    if (t.name == n && t.name != oldName)
      return;

  for (auto& t : m_tables) {
    if (t.name == oldName) {
      t.name = n;
      setModified(true);
      Q_EMIT tablesChanged();
      return;
    }
  }
}

/**
 * @brief Appends a register to @p table with a unique name.
 */
void DataModel::ProjectModel::addRegister(const QString& table,
                                          const QString& registerName,
                                          bool computed,
                                          const QVariant& defaultValue)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return;

  QString base = registerName.simplified();
  if (base.isEmpty())
    base = tr("register");

  QString unique     = base;
  int suffix         = 2;
  const auto hasName = [it](const QString& n) {
    for (const auto& r : it->registers)
      if (r.name == n)
        return true;

    return false;
  };

  while (hasName(unique))
    unique = QStringLiteral("%1_%2").arg(base, QString::number(suffix++));

  DataModel::RegisterDef reg;
  reg.name         = unique;
  reg.type         = computed ? RegisterType::Computed : RegisterType::Constant;
  reg.defaultValue = defaultValue.isValid() ? defaultValue : QVariant(0.0);
  it->registers.push_back(reg);

  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Removes a register from the specified table.
 */
void DataModel::ProjectModel::deleteRegister(const QString& table, const QString& registerName)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return;

  auto rit = std::find_if(it->registers.begin(),
                          it->registers.end(),
                          [&registerName](const auto& r) { return r.name == registerName; });

  if (rit == it->registers.end())
    return;

  it->registers.erase(rit);
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Updates an existing register -- rename, retype, and/or default value.
 */
void DataModel::ProjectModel::updateRegister(const QString& table,
                                             const QString& registerName,
                                             const QString& newName,
                                             bool computed,
                                             const QVariant& defaultValue)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return;

  const QString n = newName.simplified();
  if (n.isEmpty())
    return;

  if (n != registerName) {
    for (const auto& r : it->registers)
      if (r.name == n)
        return;
  }

  for (auto& r : it->registers) {
    if (r.name == registerName) {
      r.name         = n;
      r.type         = computed ? RegisterType::Computed : RegisterType::Constant;
      r.defaultValue = defaultValue.isValid() ? defaultValue : r.defaultValue;
      setModified(true);
      Q_EMIT tablesChanged();
      return;
    }
  }
}

/**
 * @brief Returns the register list of a table as a QVariantList for QML.
 */
QVariantList DataModel::ProjectModel::registersForTable(const QString& table) const
{
  QVariantList result;
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return result;

  for (const auto& r : it->registers) {
    QVariantMap row;
    row["name"] = r.name;
    row["type"] =
      r.type == RegisterType::Computed ? QStringLiteral("computed") : QStringLiteral("constant");
    row["value"]     = r.defaultValue;
    row["valueType"] = r.defaultValue.typeId() == QMetaType::Double ? QStringLiteral("number")
                                                                    : QStringLiteral("string");
    result.append(row);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// QInputDialog wrappers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a new shared-memory table name and appends it on accept,
 * deferring the tree selection via singleShot(0) so the queued tablesChanged
 * tree rebuild lands before the new row is selected.
 */
void DataModel::ProjectModel::promptAddTable()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Shared Table"), tr("Name:"), QLineEdit::Normal, tr("Shared Table"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const QString added = addTable(name.trimmed());
  QTimer::singleShot(
    0, this, [added] { DataModel::ProjectEditor::instance().selectUserTable(added); });
}

/**
 * @brief Prompts for a new name for an existing table.
 */
void DataModel::ProjectModel::promptRenameTable(const QString& oldName)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Table"), tr("Name:"), QLineEdit::Normal, oldName, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == oldName)
    return;

  renameTable(oldName, name.trimmed());
}

/**
 * @brief Prompts for a new title and applies it to the group at @p groupId.
 */
void DataModel::ProjectModel::promptRenameGroup(int groupId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  bool ok        = false;
  const auto old = m_groups[groupId].title;
  const auto fresh =
    QInputDialog::getText(nullptr, tr("Rename Group"), tr("Name:"), QLineEdit::Normal, old, &ok)
      .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto group  = m_groups[groupId];
  group.title = fresh;
  updateGroup(groupId, group, true);
}

/**
 * @brief Prompts for a new title and applies it to the dataset at (groupId, datasetId).
 */
void DataModel::ProjectModel::promptRenameDataset(int groupId, int datasetId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  bool ok        = false;
  const auto old = m_groups[groupId].datasets[datasetId].title;
  const auto fresh =
    QInputDialog::getText(nullptr, tr("Rename Dataset"), tr("Name:"), QLineEdit::Normal, old, &ok)
      .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto dataset  = m_groups[groupId].datasets[datasetId];
  dataset.title = fresh;
  updateDataset(groupId, datasetId, dataset, true);
}

/**
 * @brief Prompts for a new title and applies it to the source at @p sourceId.
 */
void DataModel::ProjectModel::promptRenameSource(int sourceId)
{
  const Source* src = nullptr;
  for (const auto& s : m_sources)
    if (s.sourceId == sourceId) {
      src = &s;
      break;
    }
  if (!src)
    return;

  bool ok          = false;
  const auto old   = src->title;
  const auto fresh = QInputDialog::getText(
                       nullptr, tr("Rename Data Source"), tr("Name:"), QLineEdit::Normal, old, &ok)
                       .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  updateSourceTitle(sourceId, fresh);
}

/**
 * @brief Prompts for a new title and applies it to the action at @p actionId.
 */
void DataModel::ProjectModel::promptRenameAction(int actionId)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  bool ok        = false;
  const auto old = m_actions[actionId].title;
  const auto fresh =
    QInputDialog::getText(nullptr, tr("Rename Action"), tr("Name:"), QLineEdit::Normal, old, &ok)
      .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto action  = m_actions[actionId];
  action.title = fresh;
  updateAction(actionId, action);
}

/**
 * @brief Prompts for a register name and type, then appends the register with a
 *        zero default (numeric). Users can edit the value inline afterwards.
 */
void DataModel::ProjectModel::promptAddRegister(const QString& table)
{
  if (table.isEmpty())
    return;

  bool okName           = false;
  const QString regName = QInputDialog::getText(nullptr,
                                                tr("New Register"),
                                                tr("Name:"),
                                                QLineEdit::Normal,
                                                QStringLiteral("register"),
                                                &okName);

  if (!okName || regName.trimmed().isEmpty())
    return;

  addRegister(table, regName.trimmed(), true, QVariant(0.0));
}

/**
 * @brief Prompts for a new name for an existing register.
 */
void DataModel::ProjectModel::promptRenameRegister(const QString& table,
                                                   const QString& registerName)
{
  if (table.isEmpty() || registerName.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Register"), tr("Name:"), QLineEdit::Normal, registerName, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == registerName)
    return;

  for (const auto& t : m_tables) {
    if (t.name != table)
      continue;

    for (const auto& r : t.registers) {
      if (r.name == registerName) {
        updateRegister(
          table, registerName, name.trimmed(), r.type == RegisterType::Computed, r.defaultValue);
        return;
      }
    }
  }
}

/**
 * @brief Asks the user to confirm before deleting a table.
 */
void DataModel::ProjectModel::confirmDeleteTable(const QString& name)
{
  if (name.isEmpty())
    return;

  int registerCount = 0;
  for (const auto& t : m_tables) {
    if (t.name == name) {
      registerCount = static_cast<int>(t.registers.size());
      break;
    }
  }

  const QString informative =
    registerCount == 0
      ? tr("This action cannot be undone.")
      : tr("This removes %1 register(s) along with the table. This action cannot be undone.")
          .arg(registerCount);

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(name),
                                                     informative,
                                                     QMessageBox::Warning,
                                                     tr("Delete Table"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteTable(name);
}

/**
 * @brief Asks the user to confirm before deleting a register.
 */
void DataModel::ProjectModel::confirmDeleteRegister(const QString& table,
                                                    const QString& registerName)
{
  if (table.isEmpty() || registerName.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(registerName),
                                                     tr("This action cannot be undone."),
                                                     QMessageBox::Warning,
                                                     tr("Delete Register"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteRegister(table, registerName);
}

/**
 * @brief Exports a table's registers to a CSV file chosen by the user.
 */
void DataModel::ProjectModel::exportTableToCsv(const QString& tableName)
{
  const auto it = std::find_if(m_tables.begin(), m_tables.end(), [&](const DataModel::TableDef& t) {
    return t.name == tableName;
  });

  if (it == m_tables.end())
    return;

  const auto path = QFileDialog::getSaveFileName(
    nullptr,
    tr("Export Table"),
    QStringLiteral("%1/%2.csv").arg(Misc::WorkspaceManager::instance().path("CSV"), tableName),
    tr("CSV files (*.csv)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);
  out << "name,type,value\n";
  for (const auto& reg : it->registers) {
    const auto type = (reg.type == DataModel::RegisterType::Computed) ? QStringLiteral("computed")
                                                                      : QStringLiteral("constant");

    auto val = reg.defaultValue.toString();
    if (val.contains(',') || val.contains('"') || val.contains('\n'))
      val = QStringLiteral("\"%1\"").arg(val.replace('"', "\"\""));

    out << reg.name << ',' << type << ',' << val << '\n';
  }

  file.close();
}

/**
 * @brief Imports registers from a CSV file into an existing table.
 */
void DataModel::ProjectModel::importTableFromCsv(const QString& tableName)
{
  auto it = std::find_if(m_tables.begin(), m_tables.end(), [&](const DataModel::TableDef& t) {
    return t.name == tableName;
  });

  if (it == m_tables.end())
    return;

  const auto path = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Import Table"),
                                                 Misc::WorkspaceManager::instance().path("CSV"),
                                                 tr("CSV files (*.csv)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream in(&file);

  if (!in.atEnd())
    in.readLine();

  constexpr int kMaxImportRows = 1'000'000;

  int imported = 0;
  int rowsRead = 0;
  while (!in.atEnd() && rowsRead < kMaxImportRows) {
    ++rowsRead;
    const auto line = in.readLine().trimmed();
    if (line.isEmpty())
      continue;

    const auto parts = line.split(',');
    if (parts.size() < 3)
      continue;

    const auto name    = parts[0].trimmed();
    const auto typeStr = parts[1].trimmed().toLower();
    auto valStr        = parts.mid(2).join(',').trimmed();
    if (valStr.size() >= 2 && valStr.startsWith('"') && valStr.endsWith('"')) {
      valStr = valStr.mid(1, valStr.size() - 2);
      valStr.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }

    if (name.isEmpty())
      continue;

    const bool computed = (typeStr == QStringLiteral("computed"));

    bool isNumeric              = false;
    const double dval           = SerialStudio::toDouble(valStr, &isNumeric);
    const QVariant defaultValue = isNumeric ? QVariant(dval) : QVariant(valStr);

    auto regIt = std::find_if(it->registers.begin(),
                              it->registers.end(),
                              [&](const DataModel::RegisterDef& r) { return r.name == name; });

    if (regIt != it->registers.end()) {
      regIt->type =
        computed ? DataModel::RegisterType::Computed : DataModel::RegisterType::Constant;
      regIt->defaultValue = defaultValue;
    } else {
      DataModel::RegisterDef reg;
      reg.name = name;
      reg.type = computed ? DataModel::RegisterType::Computed : DataModel::RegisterType::Constant;
      reg.defaultValue = defaultValue;
      it->registers.push_back(reg);
    }

    ++imported;
  }

  file.close();

  if (imported > 0) {
    setModified(true);
    Q_EMIT tablesChanged();
  }
}

/**
 * @brief Appends a widget reference to the specified workspace.
 */
void DataModel::ProjectModel::addWidgetToWorkspace(int workspaceId,
                                                   int widgetType,
                                                   int groupUniqueId,
                                                   int relativeIndex)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (const auto& ref : ws.widgetRefs)
      if (ref.widgetType == widgetType && ref.groupUniqueId == groupUniqueId
          && ref.relativeIndex == relativeIndex)
        return;

    DataModel::WidgetRef ref;
    ref.widgetType    = widgetType;
    ref.groupUniqueId = groupUniqueId;
    ref.relativeIndex = relativeIndex;
    ws.widgetRefs.push_back(ref);

    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference from the specified workspace by index.
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId, int index)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    if (index < 0 || static_cast<size_t>(index) >= ws.widgetRefs.size())
      return;

    ws.widgetRefs.erase(ws.widgetRefs.begin() + index);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference matching (widgetType, groupId, relativeIndex).
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId,
                                                        int widgetType,
                                                        int groupUniqueId,
                                                        int relativeIndex)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    auto it = std::find_if(ws.widgetRefs.begin(), ws.widgetRefs.end(), [=](const auto& r) {
      return r.widgetType == widgetType && r.groupUniqueId == groupUniqueId
          && r.relativeIndex == relativeIndex;
    });

    if (it == ws.widgetRefs.end())
      return;

    ws.widgetRefs.erase(it);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Drops every workspace widget ref whose encoded key isn't in validKeys.
 */
int DataModel::ProjectModel::cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return 0;

  const auto encode = [](int widgetType, int groupId, int relIdx) {
    return (static_cast<qint64>(widgetType) << 40) | (static_cast<qint64>(groupId) << 20)
         | static_cast<qint64>(relIdx);
  };

  int removed = 0;
  for (auto& ws : m_workspaces) {
    auto& refs    = ws.widgetRefs;
    const auto it = std::remove_if(refs.begin(), refs.end(), [&](const auto& r) {
      return !validKeys.contains(encode(r.widgetType, r.groupUniqueId, r.relativeIndex));
    });

    const auto count = std::distance(it, refs.end());
    if (count > 0) {
      refs.erase(it, refs.end());
      removed += static_cast<int>(count);
    }
  }

  if (removed == 0)
    return 0;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return removed;
}

/**
 * @brief Returns the title of a workspace, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceTitle(int workspaceId) const
{
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId == workspaceId)
      return ws.title;

  return QString();
}

/**
 * @brief Prompts for a new workspace name and creates it. The new workspace
 *        is then selected in the project editor.
 */
void DataModel::ProjectModel::promptAddWorkspace()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Workspace"), tr("Name:"), QLineEdit::Normal, tr("Workspace"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addWorkspace(name.trimmed());
  QTimer::singleShot(
    0, this, [newId] { DataModel::ProjectEditor::instance().selectWorkspace(newId); });
}

/**
 * @brief Prompts for a new title for the given workspace.
 */
void DataModel::ProjectModel::promptRenameWorkspace(int workspaceId)
{
  const QString current = workspaceTitle(workspaceId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Workspace"), tr("Name:"), QLineEdit::Normal, current, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameWorkspace(workspaceId, name.trimmed());
}

/**
 * @brief Returns whether the user has opted in to customising workspaces.
 */
bool DataModel::ProjectModel::customizeWorkspaces() const noexcept
{
  return m_customizeWorkspaces;
}

/**
 * @brief Flips the customize switch (Off->On seeds auto layout, On->Off re-seeds it).
 */
void DataModel::ProjectModel::setCustomizeWorkspaces(const bool enabled)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_customizeWorkspaces == enabled)
    return;

  m_customizeWorkspaces = enabled;

  if (enabled) {
    m_workspaces   = buildAutoWorkspaces();
    m_autoSnapshot = m_workspaces;
  } else {
    regenerateAutoWorkspacesUnnotified();
  }

  setModified(true);
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Synthesises the default workspace layout for a project.
 */
std::vector<DataModel::Workspace> DataModel::ProjectModel::buildAutoWorkspaces() const
{
  std::vector<DataModel::Workspace> result;

  const auto mode    = AppState::instance().operationMode();
  const auto& groups = (mode == SerialStudio::QuickPlot)
                       ? DataModel::FrameBuilder::instance().quickPlotFrame().groups
                       : m_groups;

  QMap<SerialStudio::DashboardWidget, int> groupIdx;
  QMap<SerialStudio::DashboardWidget, int> datasetIdx;

  const bool pro = SerialStudio::proWidgetsEnabled();

  std::vector<DataModel::WidgetRef> allRefs;
  std::vector<DataModel::WidgetRef> overviewRefs;
  QMap<int, std::vector<DataModel::WidgetRef>> perGroupRefs;

  int eligibleGroups = 0;

  for (const auto& group : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(group))
      continue;

    auto groupRefs = buildAutoRefsForGroup(group, pro, groupIdx, datasetIdx, allRefs, overviewRefs);
    if (groupRefs.empty())
      continue;

    perGroupRefs.insert(group.groupId, std::move(groupRefs));
    ++eligibleGroups;
  }

  if (eligibleGroups == 0)
    return result;

  if (overviewRefs.size() >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = WorkspaceIds::Overview;
    ws.title       = tr("Overview");
    ws.icon        = QStringLiteral("qrc:/icons/panes/overview.svg");
    ws.widgetRefs  = overviewRefs;
    result.push_back(std::move(ws));
  }

  if (eligibleGroups >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = WorkspaceIds::AllData;
    ws.title       = tr("All Data");
    ws.icon        = QStringLiteral("qrc:/icons/panes/dashboard.svg");
    ws.widgetRefs  = allRefs;
    result.push_back(std::move(ws));
  }

  for (const auto& group : groups) {
    if (m_hiddenGroupIds.contains(group.groupId))
      continue;

    const auto it = perGroupRefs.constFind(group.groupId);
    if (it == perGroupRefs.constEnd())
      continue;

    DataModel::Workspace ws;
    ws.workspaceId = WorkspaceIds::PerGroupStart + group.groupId;
    ws.title       = group.title;
    ws.widgetRefs  = it.value();
    result.push_back(std::move(ws));
  }

  return result;
}

/**
 * @brief Refreshes m_workspaces from the project structure WITHOUT emitting signals.
 */
void DataModel::ProjectModel::regenerateAutoWorkspacesUnnotified()
{
  if (m_customizeWorkspaces)
    return;

  m_workspaces   = buildAutoWorkspaces();
  m_autoSnapshot = m_workspaces;
}

/**
 * @brief Merges newly-eligible auto refs into the user-customised workspace list.
 */
bool DataModel::ProjectModel::mergeAutoWorkspaceUpdates()
{
  if (!m_customizeWorkspaces)
    return false;

  const auto current = buildAutoWorkspaces();
  bool dirty         = false;

  const auto refsEqual = [](const WidgetRef& a, const WidgetRef& b) {
    return a.widgetType == b.widgetType && a.groupUniqueId == b.groupUniqueId
        && a.relativeIndex == b.relativeIndex;
  };

  const auto findById = [](std::vector<DataModel::Workspace>& list, int id) {
    return std::find_if(
      list.begin(), list.end(), [id](const auto& w) { return w.workspaceId == id; });
  };

  const auto findByIdConst = [](const std::vector<DataModel::Workspace>& list, int id) {
    return std::find_if(
      list.begin(), list.end(), [id](const auto& w) { return w.workspaceId == id; });
  };

  for (const auto& cur : current) {
    auto userIt = findById(m_workspaces, cur.workspaceId);
    auto snapIt = findByIdConst(m_autoSnapshot, cur.workspaceId);

    if (userIt == m_workspaces.end()) {
      if (snapIt != m_autoSnapshot.end())
        continue;

      m_workspaces.push_back(cur);
      dirty = true;
      continue;
    }

    if (cur.workspaceId >= WorkspaceIds::PerGroupStart && userIt->title != cur.title) {
      userIt->title = cur.title;
      dirty         = true;
    }

    for (const auto& r : cur.widgetRefs) {
      const bool inSnap = snapIt != m_autoSnapshot.end()
                       && std::any_of(snapIt->widgetRefs.begin(),
                                      snapIt->widgetRefs.end(),
                                      [&](const auto& s) { return refsEqual(s, r); });
      if (inSnap)
        continue;

      const bool inUser = std::any_of(userIt->widgetRefs.begin(),
                                      userIt->widgetRefs.end(),
                                      [&](const auto& s) { return refsEqual(s, r); });
      if (inUser)
        continue;

      userIt->widgetRefs.push_back(r);
      dirty = true;
    }
  }

  m_autoSnapshot = current;

  if (dirty)
    setModified(true);

  return dirty;
}

/**
 * @brief Materialises the synthetic workspace list into m_workspaces and flips the project into
 * customize mode so the user can edit it from there.
 */
int DataModel::ProjectModel::autoGenerateWorkspaces()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (m_customizeWorkspaces && !m_workspaces.empty())
    return m_workspaces.front().workspaceId;

  auto seed = buildAutoWorkspaces();
  if (seed.empty())
    return -1;

  m_workspaces           = std::move(seed);
  m_autoSnapshot         = m_workspaces;
  const bool flagChanged = !m_customizeWorkspaces;
  m_customizeWorkspaces  = true;

  Q_ASSERT(!m_workspaces.empty());
  Q_ASSERT(m_workspaces.front().workspaceId >= WorkspaceIds::AutoStart);

  setModified(true);
  if (flagChanged)
    Q_EMIT customizeWorkspacesChanged();

  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return m_workspaces.front().workspaceId;
}

/**
 * @brief Drops user customisations and returns the project to the synthetic
 *        auto-layout. Idempotent: a no-op when already in auto mode.
 */
void DataModel::ProjectModel::resetWorkspacesToAuto()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    return;

  m_customizeWorkspaces = false;
  regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Asks the user to confirm before discarding workspace customisations.
 */
void DataModel::ProjectModel::confirmResetWorkspacesToAuto()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    return;

  if (m_suppressMessageBoxes) {
    resetWorkspacesToAuto();
    return;
  }

  const int choice = Misc::Utilities::showMessageBox(
    tr("Discard workspace customisations?"),
    tr("Switching off Customize discards your edits and rebuilds the "
       "workspace list from the project's groups."),
    QMessageBox::Warning,
    tr("Customize Workspaces"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    resetWorkspacesToAuto();
}

/**
 * @brief Counts, per dashboard-widget type, how many widgets the given group
 *        contributes to Dashboard::buildWidgetGroups's running type counter.
 */
QMap<int, int> DataModel::ProjectModel::widgetTypeCountsForGroup(const Group& g) const
{
  QMap<int, int> counts;

  if (!SerialStudio::groupEligibleForWorkspace(g))
    return counts;

  auto groupKey = SerialStudio::getDashboardWidget(g);
  if (groupKey == SerialStudio::DashboardPlot3D && !SerialStudio::proWidgetsEnabled())
    groupKey = SerialStudio::DashboardMultiPlot;

  const bool isEmptyOutputPanel =
    g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();

  if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
    counts[static_cast<int>(groupKey)] += 1;

  bool groupHasLed = false;
  for (const auto& ds : g.datasets) {
    if (ds.hideOnDashboard)
      continue;

    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys) {
      if (k == SerialStudio::DashboardLED) {
        groupHasLed = true;
        continue;
      }
      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      counts[static_cast<int>(k)] += 1;
    }
  }

  if (groupHasLed)
    counts[static_cast<int>(SerialStudio::DashboardLED)] += 1;

  return counts;
}

/**
 * @brief Shifts or drops user-customised widget refs after a group delete.
 */
void DataModel::ProjectModel::shiftWorkspaceRefsAfterGroupDelete(
  int deletedGid, const QMap<int, int>& deletedTypeCounts)
{
  Q_ASSERT(deletedGid >= 0);
  Q_ASSERT(m_customizeWorkspaces);

  const int deletedAutoId = WorkspaceIds::PerGroupStart + deletedGid;

  m_workspaces.erase(
    std::remove_if(m_workspaces.begin(),
                   m_workspaces.end(),
                   [deletedAutoId](const Workspace& w) { return w.workspaceId == deletedAutoId; }),
    m_workspaces.end());

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId > deletedAutoId && ws.workspaceId < WorkspaceIds::UserStart)
      ws.workspaceId -= 1;

    for (auto it = ws.widgetRefs.begin(); it != ws.widgetRefs.end();) {
      const int newPos = groupIdForUniqueId(it->groupUniqueId);
      if (newPos < 0) {
        it = ws.widgetRefs.erase(it);
        continue;
      }

      if (newPos >= deletedGid) {
        const int lost    = deletedTypeCounts.value(it->widgetType, 0);
        it->relativeIndex = std::max(0, it->relativeIndex - lost);
      }

      ++it;
    }
  }
}

/**
 * @brief Updates m_hiddenGroupIds after a group is removed and surviving groups are renumbered down
 * by 1.
 */
void DataModel::ProjectModel::shiftHiddenGroupIdsAfterGroupDelete(int deletedGid)
{
  if (m_hiddenGroupIds.isEmpty())
    return;

  QSet<int> updated;
  for (const int id : std::as_const(m_hiddenGroupIds)) {
    if (id == deletedGid)
      continue;

    updated.insert(id > deletedGid ? id - 1 : id);
  }

  m_hiddenGroupIds = std::move(updated);
}

/**
 * @brief Updates layout:N widgetSettings entries after a group renumber.
 */
void DataModel::ProjectModel::shiftLayoutKeysAfterGroupDelete(int deletedGid)
{
  if (m_widgetSettings.isEmpty())
    return;

  const auto keys = m_widgetSettings.keys();
  bool changed    = false;

  if (m_widgetSettings.contains(Keys::layoutKey(deletedGid))) {
    m_widgetSettings.remove(Keys::layoutKey(deletedGid));
    changed = true;
  }

  const QString prefix = QStringLiteral("layout:");
  QList<QPair<int, QJsonObject>> moves;
  for (const auto& key : keys) {
    if (!key.startsWith(prefix))
      continue;

    bool ok      = false;
    const int id = key.mid(prefix.length()).toInt(&ok);
    if (!ok || id <= deletedGid)
      continue;

    moves.append({id, m_widgetSettings.value(key).toObject()});
  }

  std::sort(
    moves.begin(), moves.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  for (const auto& move : moves) {
    m_widgetSettings.remove(Keys::layoutKey(move.first));
    m_widgetSettings.insert(Keys::layoutKey(move.first - 1), move.second);
    changed = true;
  }

  if (changed)
    Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Shifts user-customised widget refs after a single dataset is deleted from a surviving
 * group.
 */
void DataModel::ProjectModel::shiftWorkspaceRefsAfterDatasetDelete(
  int groupId, const QMap<int, int>& datasetTypeCounts)
{
  Q_ASSERT(groupId >= 0);
  Q_ASSERT(m_customizeWorkspaces);

  if (datasetTypeCounts.isEmpty())
    return;

  QMap<int, int> runningAtGroup;
  for (const auto& g : m_groups) {
    if (!SerialStudio::groupEligibleForWorkspace(g))
      continue;

    if (g.groupId == groupId)
      break;

    const auto groupKey = SerialStudio::getDashboardWidget(g);

    const bool isEmptyOutputPanel =
      g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
      runningAtGroup[static_cast<int>(groupKey)] += 1;

    for (const auto& ds : g.datasets)
      tallyDatasetWidgetTypes(ds, runningAtGroup);
  }

  const int groupUid = groupUniqueIdForGroupId(groupId);

  for (auto& ws : m_workspaces) {
    ws.widgetRefs.erase(std::remove_if(ws.widgetRefs.begin(),
                                       ws.widgetRefs.end(),
                                       [&](const WidgetRef& r) {
                                         const int lost = datasetTypeCounts.value(r.widgetType, 0);
                                         if (lost == 0 || r.groupUniqueId != groupUid)
                                           return false;

                                         const int base = runningAtGroup.value(r.widgetType, 0);
                                         return r.relativeIndex >= base
                                             && r.relativeIndex < base + lost;
                                       }),
                        ws.widgetRefs.end());

    for (auto& r : ws.widgetRefs) {
      const int lost = datasetTypeCounts.value(r.widgetType, 0);
      if (lost == 0)
        continue;

      const int base = runningAtGroup.value(r.widgetType, 0);
      if (r.relativeIndex < base + lost)
        continue;

      r.relativeIndex -= lost;
      Q_ASSERT(r.relativeIndex >= 0);
    }
  }
}

/**
 * @brief Asks the user to confirm before deleting a workspace.
 */
void DataModel::ProjectModel::confirmDeleteWorkspace(int workspaceId)
{
  const QString name = workspaceTitle(workspaceId);
  if (name.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(name),
                                                     tr("This action cannot be undone."),
                                                     QMessageBox::Warning,
                                                     tr("Delete Workspace"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteWorkspace(workspaceId);
}

/**
 * @brief Hides an auto-generated group workspace from the workspace list.
 */
void DataModel::ProjectModel::hideGroup(int groupId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (groupId < 0)
    return;

  if (m_hiddenGroupIds.contains(groupId))
    return;

  m_hiddenGroupIds.insert(groupId);

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Restores a previously hidden auto-generated group workspace.
 */
void DataModel::ProjectModel::showGroup(int groupId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_hiddenGroupIds.remove(groupId))
    return;

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Returns {id, title} entries for every hidden auto-group, in group order.
 */
QVariantList DataModel::ProjectModel::hiddenGroupsSummary() const
{
  QVariantList result;
  for (const auto& g : m_groups) {
    if (!m_hiddenGroupIds.contains(g.groupId))
      continue;

    QVariantMap entry;
    entry[QStringLiteral("id")]    = g.groupId;
    entry[QStringLiteral("title")] = g.title;
    result.append(entry);
  }

  return result;
}

/**
 * @brief Restores every hidden auto-group in one shot. No-op when none hidden.
 */
void DataModel::ProjectModel::showAllHiddenGroups()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_hiddenGroupIds.isEmpty())
    return;

  m_hiddenGroupIds.clear();

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears workspaces and widget settings for QuickPlot/ConsoleOnly sessions that have no
 * backing project file.
 */
void DataModel::ProjectModel::clearTransientState()
{
  const auto opMode = AppState::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile || !m_filePath.isEmpty() || m_customizeWorkspaces)
    return;

  m_hiddenGroupIds.clear();

  if (!m_workspaces.empty()) {
    m_workspaces.clear();
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
  }

  if (!m_widgetSettings.isEmpty()) {
    m_widgetSettings = QJsonObject();
    Q_EMIT widgetSettingsChanged();
  }

  if (!m_mqttPublisher.isEmpty()) {
    m_mqttPublisher = QJsonObject();
    Q_EMIT mqttPublisherChanged();
  }

  setModified(false);
}

/**
 * @brief Returns the next available dataset frame index.
 */
int DataModel::ProjectModel::nextDatasetIndex()
{
  int maxIndex = 1;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets)
      if (dataset.index >= maxIndex)
        maxIndex = dataset.index + 1;
  }

  return maxIndex;
}

/**
 * @brief Returns a snapshot of all sources suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::sourcesForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_sources.size()));

  for (const auto& src : m_sources) {
    QVariantMap map;
    map[Keys::SourceId] = src.sourceId;
    map[Keys::BusType]  = src.busType;
    map[Keys::Title]    = src.title;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all groups (with their datasets) for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::groupsForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_groups.size()));

  for (const auto& grp : m_groups) {
    QVariantList datasets;
    datasets.reserve(static_cast<qsizetype>(grp.datasets.size()));

    for (const auto& ds : grp.datasets) {
      QVariantMap dsMap;
      dsMap[Keys::DatasetId]                = ds.datasetId;
      dsMap[Keys::Title]                    = ds.title;
      dsMap[Keys::Units]                    = ds.units;
      dsMap[Keys::Widget]                   = ds.widget;
      dsMap[QStringLiteral("hasTransform")] = !ds.transformCode.trimmed().isEmpty();
      datasets.append(dsMap);
    }

    QVariantMap map;
    map[Keys::GroupId] = grp.groupId;
    QVariantList outputWidgets;
    outputWidgets.reserve(static_cast<qsizetype>(grp.outputWidgets.size()));
    for (const auto& ow : grp.outputWidgets) {
      QVariantMap owMap;
      owMap[Keys::Title]      = ow.title;
      owMap[Keys::OutputType] = static_cast<int>(ow.type);
      outputWidgets.append(owMap);
    }

    map[Keys::SourceId]      = grp.sourceId;
    map[Keys::Title]         = grp.title;
    map[Keys::Widget]        = grp.widget;
    map[Keys::GroupType]     = static_cast<int>(grp.groupType);
    map[Keys::Datasets]      = datasets;
    map[Keys::OutputWidgets] = outputWidgets;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all actions suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::actionsForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_actions.size()));

  for (const auto& act : m_actions) {
    QVariantMap map;
    map[Keys::ActionId] = act.actionId;
    map[Keys::SourceId] = act.sourceId;
    map[Keys::Title]    = act.title;
    map[Keys::Icon]     = Misc::IconEngine::resolveActionIconSource(act.icon);
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of project data tables (name + register count) for the diagram.
 */
QVariantList DataModel::ProjectModel::tablesForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_tables.size()));

  for (const auto& tbl : m_tables) {
    QVariantMap map;
    map[Keys::Name]                      = tbl.name;
    map[QStringLiteral("registerCount")] = static_cast<int>(tbl.registers.size());
    result.append(map);
  }

  return result;
}

/**
 * @brief Silently writes the current project to disk; called from the debounce timer.
 */
void DataModel::ProjectModel::autoSave()
{
  if (m_autoSaveSuspended)
    return;

  if (m_filePath.isEmpty() || m_locked || !m_modified)
    return;

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!writeProjectFile(m_filePath)) {
    qWarning() << "[ProjectModel] Auto-save failed";
    return;
  }

  setModified(false);
}

/**
 * @brief Flushes any pending debounced autosave synchronously (called on app quit).
 */
void DataModel::ProjectModel::flushAutoSave()
{
  if (m_autoSaveTimer && m_autoSaveTimer->isActive())
    m_autoSaveTimer->stop();

  autoSave();
}

/**
 * @brief Suspends or resumes the debounced autosave (used by the API batch endpoint).
 */
void DataModel::ProjectModel::setAutoSaveSuspended(bool suspend)
{
  if (m_autoSaveSuspended == suspend)
    return;

  m_autoSaveSuspended = suspend;
  if (suspend && m_autoSaveTimer)
    m_autoSaveTimer->stop();
}

/**
 * @brief Atomically serializes the current project to @p path.
 */
bool DataModel::ProjectModel::writeProjectFile(const QString& path)
{
  Q_ASSERT(!path.isEmpty());

  QSaveFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    qWarning() << "[ProjectModel] File open error:" << file.errorString();
    return false;
  }

  const QByteArray payload = QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented);
  if (file.write(payload) != payload.size()) {
    qWarning() << "[ProjectModel] Short write:" << file.errorString();
    file.cancelWriting();
    return false;
  }

  if (!file.commit()) {
    qWarning() << "[ProjectModel] Commit failed:" << file.errorString();
    return false;
  }

  return true;
}

/**
 * @brief Writes the current project to m_filePath and reloads it.
 */
bool DataModel::ProjectModel::finalizeProjectSave()
{
  resolveDatasetTransformLanguages();
  resolveDatasetVirtualFlags();

  if (!writeProjectFile(m_filePath)) {
    if (!m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(tr("File save error"), m_filePath, QMessageBox::Critical);

    return false;
  }

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  setModified(false);
  Q_EMIT jsonFileChanged();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Stateless id-based mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the group at @p groupId; opt-in confirmation reuses deleteCurrentGroup's dialog.
 */
void DataModel::ProjectModel::deleteGroup(int groupId, bool confirm)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  const auto previousSelection = m_selectedGroup;
  setSelectedGroup(m_groups[groupId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentGroup();
  m_suppressMessageBoxes = previousSuppress;

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.groupId != groupId)
    setSelectedGroup(m_groups[previousSelection.groupId]);
}

/**
 * @brief Duplicates the group at @p groupId via the existing selection-based path.
 */
void DataModel::ProjectModel::duplicateGroup(int groupId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  const auto previousSelection = m_selectedGroup;
  setSelectedGroup(m_groups[groupId]);
  duplicateCurrentGroup();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size())
    setSelectedGroup(m_groups[previousSelection.groupId]);
}

/**
 * @brief Deletes the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectModel::deleteDataset(int groupId, int datasetId, bool confirm)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  const auto previousSelection = m_selectedDataset;
  setSelectedDataset(m_groups[groupId].datasets[datasetId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentDataset();
  m_suppressMessageBoxes = previousSuppress;

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.datasetId >= 0
      && static_cast<size_t>(previousSelection.datasetId)
           < m_groups[previousSelection.groupId].datasets.size()
      && !(previousSelection.groupId == groupId && previousSelection.datasetId == datasetId))
    setSelectedDataset(m_groups[previousSelection.groupId].datasets[previousSelection.datasetId]);
}

/**
 * @brief Duplicates the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectModel::duplicateDataset(int groupId, int datasetId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  const auto previousSelection = m_selectedDataset;
  setSelectedDataset(m_groups[groupId].datasets[datasetId]);
  duplicateCurrentDataset();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.datasetId >= 0
      && static_cast<size_t>(previousSelection.datasetId)
           < m_groups[previousSelection.groupId].datasets.size())
    setSelectedDataset(m_groups[previousSelection.groupId].datasets[previousSelection.datasetId]);
}

/**
 * @brief Deletes the action at @p actionId via the existing selection-based path.
 */
void DataModel::ProjectModel::deleteAction(int actionId, bool confirm)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  const auto previousSelection = m_selectedAction;
  setSelectedAction(m_actions[actionId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentAction();
  m_suppressMessageBoxes = previousSuppress;

  if (previousSelection.actionId >= 0
      && static_cast<size_t>(previousSelection.actionId) < m_actions.size()
      && previousSelection.actionId != actionId)
    setSelectedAction(m_actions[previousSelection.actionId]);
}

/**
 * @brief Duplicates the action at @p actionId.
 */
void DataModel::ProjectModel::duplicateAction(int actionId)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  const auto previousSelection = m_selectedAction;
  setSelectedAction(m_actions[actionId]);
  duplicateCurrentAction();

  if (previousSelection.actionId >= 0
      && static_cast<size_t>(previousSelection.actionId) < m_actions.size())
    setSelectedAction(m_actions[previousSelection.actionId]);
}

/**
 * @brief Deletes the output widget at @p groupId/@p widgetId.
 */
void DataModel::ProjectModel::deleteOutputWidget(int groupId, int widgetId, bool confirm)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= m_groups[groupId].outputWidgets.size())
    return;

  const auto previousSelection = m_selectedOutputWidget;
  setSelectedOutputWidget(m_groups[groupId].outputWidgets[widgetId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentOutputWidget();
  m_suppressMessageBoxes = previousSuppress;

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.widgetId >= 0
      && static_cast<size_t>(previousSelection.widgetId)
           < m_groups[previousSelection.groupId].outputWidgets.size()
      && !(previousSelection.groupId == groupId && previousSelection.widgetId == widgetId))
    setSelectedOutputWidget(
      m_groups[previousSelection.groupId].outputWidgets[previousSelection.widgetId]);
}

/**
 * @brief Duplicates the output widget at @p groupId/@p widgetId.
 */
void DataModel::ProjectModel::duplicateOutputWidget(int groupId, int widgetId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= m_groups[groupId].outputWidgets.size())
    return;

  const auto previousSelection = m_selectedOutputWidget;
  setSelectedOutputWidget(m_groups[groupId].outputWidgets[widgetId]);
  duplicateCurrentOutputWidget();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.widgetId >= 0
      && static_cast<size_t>(previousSelection.widgetId)
           < m_groups[previousSelection.groupId].outputWidgets.size())
    setSelectedOutputWidget(
      m_groups[previousSelection.groupId].outputWidgets[previousSelection.widgetId]);
}

//--------------------------------------------------------------------------------------------------
// Bulk multi-selection mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Duplicates every item described in @p items in declared order.
 */
void DataModel::ProjectModel::duplicateSelectedItems(const QVariantList& items)
{
  for (const auto& v : items) {
    const auto entry = v.toMap();
    const int kind   = entry.value(QStringLiteral("kind"), -1).toInt();
    const int id     = entry.value(QStringLiteral("id"), -1).toInt();
    const int parent = entry.value(QStringLiteral("parentId"), -1).toInt();

    switch (kind) {
      case ProjectEditor::KindGroup:
        duplicateGroup(id);
        break;
      case ProjectEditor::KindDataset:
        duplicateDataset(parent, id);
        break;
      case ProjectEditor::KindAction:
        duplicateAction(id);
        break;
      case ProjectEditor::KindOutputWidget:
        duplicateOutputWidget(parent, id);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Deletes every item described in @p items in dependency-safe order,
 * sorting descending by parentId/id within each kind so each removal never
 * shifts the indices of items still pending deletion.
 */
void DataModel::ProjectModel::deleteSelectedItems(const QVariantList& items)
{
  struct Entry {
    int kind;
    int id;
    int parentId;
  };

  QList<Entry> entries;
  entries.reserve(items.size());
  for (const auto& v : items) {
    const auto m = v.toMap();
    Entry e;
    e.kind     = m.value(QStringLiteral("kind"), -1).toInt();
    e.id       = m.value(QStringLiteral("id"), -1).toInt();
    e.parentId = m.value(QStringLiteral("parentId"), -1).toInt();
    entries.append(e);
  }

  std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
    if (a.kind != b.kind)
      return a.kind < b.kind;

    if (a.parentId != b.parentId)
      return a.parentId > b.parentId;

    return a.id > b.id;
  });

  for (const auto& e : entries) {
    switch (e.kind) {
      case ProjectEditor::KindGroup:
        deleteGroup(e.id, false);
        break;
      case ProjectEditor::KindDataset:
        deleteDataset(e.parentId, e.id, false);
        break;
      case ProjectEditor::KindAction:
        deleteAction(e.id, false);
        break;
      case ProjectEditor::KindOutputWidget:
        deleteOutputWidget(e.parentId, e.id, false);
        break;
      default:
        break;
    }
  }
}
