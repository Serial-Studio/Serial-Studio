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

#include <algorithm>
#include <QApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "UI/Dashboard.h"

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

  deduplicateUniqueIds();
}

/**
 * @brief Enforces globally unique dataset+group uniqueIds across all sources. Downstream
 *        dashboard/export maps are keyed by uid alone, so a uid reused across sources (possible on
 * a hand-edited/merged/stale project, since persisted uids are read verbatim) bleeds one source's
 * data into the other's. Runs after the allocator is bumped past every uid.
 */
void DataModel::ProjectModel::deduplicateUniqueIds()
{
  QSet<int> seenGroups;
  QSet<int> seenDatasets;
  QMap<int, int> datasetRemap;
  for (auto& group : m_groups) {
    if (seenGroups.contains(group.uniqueId)) {
      qWarning() << "ProjectModel: duplicate group uniqueId" << group.uniqueId
                 << "- reassigning to keep per-source maps unambiguous";
      group.uniqueId = m_nextUniqueId++;
    }
    seenGroups.insert(group.uniqueId);

    for (auto& dataset : group.datasets) {
      dataset.sourceId = group.sourceId;
      if (seenDatasets.contains(dataset.uniqueId)) {
        qWarning() << "ProjectModel: duplicate dataset uniqueId" << dataset.uniqueId
                   << "- reassigning to keep per-source maps unambiguous";
        const int newId = m_nextUniqueId++;
        datasetRemap.insert(dataset.uniqueId, newId);
        dataset.uniqueId = newId;
      }
      seenDatasets.insert(dataset.uniqueId);
    }
  }

  if (datasetRemap.isEmpty())
    return;

  for (auto& group : m_groups)
    for (auto& dataset : group.datasets) {
      if (dataset.xAxisId > 0)
        dataset.xAxisId = datasetRemap.value(dataset.xAxisId, dataset.xAxisId);

      if (dataset.waterfallYAxis > 0)
        dataset.waterfallYAxis = datasetRemap.value(dataset.waterfallYAxis, dataset.waterfallYAxis);
    }
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
      if (dataset.xAxisId == kXAxisTime || dataset.xAxisId == kXAxisSamples)
        continue;

      if (dataset.xAxisId <= 0)
        dataset.xAxisId = kXAxisTime;
      else
        dataset.xAxisId = indexMap.value(dataset.xAxisId, kXAxisTime);
    }
  }
}

namespace DataModel {

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

}  // namespace DataModel

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
    static auto& workspaceManager = Misc::WorkspaceManager::instance();
    const QString remapped        = workspaceManager.remapLegacyPath(path);
    if (remapped != path && QFileInfo::exists(remapped))
      resolved = remapped;
  }

  if (m_filePath == resolved && !m_groups.empty())
    return true;

  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);

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

  m_filePath = sourcePath;

  const auto json            = document.object();
  const auto flags           = applyJsonDocumentCore(json);
  const int loadedSchema     = flags.loadedSchema;
  const bool olderSchema     = flags.olderSchema;
  const bool legacyUniqueIds = flags.legacyUniqueIds;

  m_history.clear();
  Q_EMIT projectHistoryChanged();

  setModified(false);
  watchProjectFile();

  const bool separatorMigrated = migrateLegacySeparator(json);

  m_autoSnapshot = buildAutoWorkspaces();
  emitProjectLoadedSignals();

  if (!separatorMigrated && legacyUniqueIds && !m_filePath.isEmpty())
    persistLegacyMigration();

  m_autoSaveSuspended = false;
  m_runtimeDirty      = false;

  if (olderSchema && !sourcePath.isEmpty()) {
    const QString title = tr("Project upgraded from an earlier file format");
    const QString body  = tr("This project was saved with schema version %1; the current version "
                             "is %2. Defaults have been applied to any new fields. Save the "
                             "project to lock in the upgrade.")
                           .arg(loadedSchema)
                           .arg(DataModel::kSchemaVersion);
    QTimer::singleShot(0, this, [title, body] {
      static auto& nc = DataModel::NotificationCenter::instance();
      nc.postInfo(QStringLiteral("ProjectModel"), title, body);
    });
  }

  return true;
}

/**
 * @brief Replaces the in-memory document from a parsed project JSON object: clears the
 *        replaced containers, loads scalars/arrays, runs the sanitizers and legacy
 *        migrations, and returns the schema flags the caller needs for follow-up work.
 */
DataModel::ProjectModel::DocumentLoadFlags DataModel::ProjectModel::applyJsonDocumentCore(
  const QJsonObject& json)
{
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_widgetSettings  = QJsonObject();
  m_treeExpansion   = QJsonObject();
  m_diagramCollapse = QJsonObject();

  const QString legacyParserCode = json.value(QLatin1StringView("frameParser")).toString();

  DocumentLoadFlags flags;
  flags.legacyUniqueIds = !json.contains(Keys::NextUniqueId);
  flags.loadedSchema    = ss_jsr(json, Keys::SchemaVersion, 0).toInt();
  flags.olderSchema     = flags.loadedSchema < DataModel::kSchemaVersion;

  m_controlScriptCode        = ss_jsr(json, Keys::ControlScriptCode, "").toString();
  static auto& controlScript = DataModel::ControlScript::instance();
  controlScript.setCode(m_controlScriptCode);

  loadProjectRootScalars(json);
  loadProjectArrays(json, legacyParserCode);
  enforceGplSingleSource();
  resolveDatasetTransformLanguages();
  resolveDatasetVirtualFlags();

  seedNextUniqueIdFromGroups();
  loadWidgetSettingsAndWorkspaces(json);

  if (flags.olderSchema) {
    m_customizeWorkspaces = false;
    m_workspaces.clear();
  }

  if (flags.legacyUniqueIds) {
    migrateLegacyWorkspaceRefs();
    migrateLegacyXAxisIds();
  }

  migrateLegacyWaterfallYAxisIds();

  loadPointCount(json);
  loadPlotTimeRange(json);
  loadFrozen(json);
  loadChangeDrivenTransforms(json);
  loadLuaFastMode(json);
  migrateLegacyLayoutKeys();
  migrateLegacyDashboardLayout(json);

  return flags;
}

/**
 * @brief Restores an undo/redo snapshot in place: same document rebuild as a load, but the
 *        file path, watcher, and history survive, and jsonFileChanged never fires (the
 *        frameDetectionChanged hop drives the AppState/FrameBuilder resync instead).
 */
bool DataModel::ProjectModel::applyHistorySnapshot(const QByteArray& state)
{
  const auto document = QJsonDocument::fromJson(state);
  if (document.isEmpty() || !document.isObject())
    return false;

  m_history.setApplying(true);
  m_autoSaveSuspended = true;
  if (m_autoSaveTimer)
    m_autoSaveTimer->stop();

  (void)applyJsonDocumentCore(document.object());

  m_autoSnapshot = buildAutoWorkspaces();
  emitProjectLoadedSignals(false);

  m_autoSaveSuspended = false;
  m_runtimeDirty      = false;
  scheduleAutoSave();

  if (m_widgetSettings.isEmpty())
    Q_EMIT widgetSettingsChanged();

  m_history.setApplying(false);
  Q_EMIT historySnapshotApplied();
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

            static auto& appState = AppState::instance();
            appState.setOperationMode(SerialStudio::ProjectFile);

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

  if (m_sources[0].frameParserCode.isEmpty()
      && m_sources[0].frameParserLanguage != SerialStudio::Native)
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
  static auto& cm                     = IO::ConnectionManager::instance();
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
  for (auto& g : m_groups) {
    if (g.sourceId > 0)
      g.sourceId = 0;

    for (auto& widget : g.outputWidgets)
      if (widget.sourceId > 0)
        widget.sourceId = 0;
  }

  for (auto& action : m_actions)
    if (action.sourceId > 0)
      action.sourceId = 0;

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

namespace DataModel {

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

}  // namespace DataModel

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
  m_widgetSettings  = json.value(Keys::WidgetSettings).toObject();
  m_widgetDisplay   = json.value(Keys::WidgetDisplay).toObject();
  m_treeExpansion   = json.value(Keys::TreeExpansion).toObject();
  m_diagramCollapse = json.value(Keys::DiagramCollapse).toObject();

  m_workspaces.clear();
  m_workspaceFolders.clear();
  m_groupFolders.clear();
  m_tableFolders.clear();
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
        static auto& nc = DataModel::NotificationCenter::instance();
        nc.postWarning(
          QStringLiteral("ProjectModel"),
          tr("Workspace IDs remapped on load"),
          tr("%1 custom workspace ID(s) overlapped the new reserved auto range and were "
             "moved into the user range. Save the project to make the remap permanent.")
            .arg(n));
      });
    }
  }

  if (m_customizeWorkspaces && json.contains(Keys::WorkspaceFolders)) {
    const auto folderArray = json.value(Keys::WorkspaceFolders).toArray();
    for (const auto& val : folderArray) {
      DataModel::WorkspaceFolder folder;
      if (DataModel::read(folder, val.toObject()))
        m_workspaceFolders.push_back(folder);
    }
  }

  sanitizeWorkspaceFolders();

  if (json.contains(Keys::GroupFolders)) {
    const auto folderArray = json.value(Keys::GroupFolders).toArray();
    for (const auto& val : folderArray) {
      DataModel::GroupFolder folder;
      if (DataModel::read(folder, val.toObject()))
        m_groupFolders.push_back(folder);
    }
  }

  sanitizeGroupFolders();

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

  if (json.contains(Keys::TableFolders)) {
    const auto folderArray = json.value(Keys::TableFolders).toArray();
    for (const auto& val : folderArray) {
      DataModel::TableFolder folder;
      if (DataModel::read(folder, val.toObject()))
        m_tableFolders.push_back(folder);
    }
  }

  sanitizeTableFolders();

  m_mqttPublisher = json.value(Keys::MqttPublisher).toObject();
  Q_EMIT mqttPublisherChanged();
}

/**
 * @brief Resolves the project point-count from JSON or legacy widgetSettings, syncing dashboard.
 */
void DataModel::ProjectModel::loadPointCount(const QJsonObject& json)
{
  static auto& dashboard = UI::Dashboard::instance();

  m_pointCount = dashboard.points();
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

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile)
    dashboard.setPoints(m_pointCount);
}

/**
 * @brief Resolves the project plot time range (seconds) from JSON, syncing the dashboard.
 */
void DataModel::ProjectModel::loadPlotTimeRange(const QJsonObject& json)
{
  static auto& dashboard = UI::Dashboard::instance();

  m_plotTimeRange = dashboard.plotTimeRange();
  if (json.contains(Keys::PlotTimeRange)) {
    const double secs = SerialStudio::toDouble(json.value(Keys::PlotTimeRange));
    if (secs > 0)
      m_plotTimeRange = secs;
  }

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile)
    dashboard.setPlotTimeRange(m_plotTimeRange);
}

/**
 * @brief Resolves the dashboard freeze flag from JSON; absent means false. Writes the member
 *        directly (never setFrozen) so an unlicensed load/save cycle preserves the flag.
 */
void DataModel::ProjectModel::loadFrozen(const QJsonObject& json)
{
  m_frozen = json.value(Keys::Frozen).toBool(false);
}

/**
 * @brief Resolves the change-driven-transforms flag from JSON; absent means false (opt-in).
 */
void DataModel::ProjectModel::loadChangeDrivenTransforms(const QJsonObject& json)
{
  m_changeDrivenTransforms = json.value(Keys::ChangeDrivenTransforms).toBool(false);
}

/**
 * @brief Reads the Fast-Lua-execution flag from the project document (absent key = Safe).
 */
void DataModel::ProjectModel::loadLuaFastMode(const QJsonObject& json)
{
  m_luaFastMode = json.value(Keys::LuaFastMode).toBool(false);
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
 *        Migrated projects save to disk inside this call, so the caller skips
 *        persistLegacyMigration() but must still run the normal load tail (snapshot, loaded
 *        signals, autosave re-enable).
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
 * @brief Emits the standard burst of "project loaded" signals for downstream views; history
 *        restores skip jsonFileChanged so backups and file-path handlers stay quiet.
 */
void DataModel::ProjectModel::emitProjectLoadedSignals(const bool includeJsonFileChanged)
{
  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();

  if (includeJsonFileChanged)
    Q_EMIT jsonFileChanged();

  Q_EMIT tablesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();
  Q_EMIT controlScriptChanged();
  Q_EMIT pointCountChanged();
  Q_EMIT plotTimeRangeChanged();
  Q_EMIT frozenChanged();
  Q_EMIT changeDrivenTransformsChanged();
  Q_EMIT luaFastModeChanged();

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
