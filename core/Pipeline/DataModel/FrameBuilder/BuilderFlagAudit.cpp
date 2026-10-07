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

#include "DataModel/FrameBuilder/BuilderFlagAudit.h"

#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"

[[nodiscard]] static const DataModel::FrameBuilder& builderOf(const void* owner);
[[nodiscard]] static DataModel::FrameBuilder& mutableBuilderOf(void* owner);

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

const DataModel::CachedFlagSpec DataModel::BuilderFlagAudit::kOperationMode{
  .name    = "FrameBuilder::m_operationMode",
  .fresh   = &BuilderFlagAudit::freshOperationMode,
  .repair  = &BuilderFlagAudit::repairOperationMode,
  .settled = &BuilderFlagAudit::builderLive,
  .cached  = nullptr,
};

const DataModel::CachedFlagSpec DataModel::BuilderFlagAudit::kPlayerOpen{
  .name    = "FrameBuilder::m_playerOpen",
  .fresh   = &BuilderFlagAudit::freshPlayerOpen,
  .repair  = &BuilderFlagAudit::repairPlayerOpen,
  .settled = &BuilderFlagAudit::builderLive,
  .cached  = nullptr,
};

const DataModel::CachedFlagSpec DataModel::BuilderFlagAudit::kCaptureDatasetValues{
  .name    = "FrameBuilder::m_captureDatasetValues",
  .fresh   = &BuilderFlagAudit::freshCaptureDatasetValues,
  .repair  = &BuilderFlagAudit::repairDatasetFlags,
  .settled = &BuilderFlagAudit::datasetFlagsSettled,
  .cached  = nullptr,
};

const DataModel::CachedFlagSpec DataModel::BuilderFlagAudit::kCaptureLatestFrame{
  .name    = "FrameBuilder::m_captureLatestFrame",
  .fresh   = &BuilderFlagAudit::freshCaptureLatestFrame,
  .repair  = &BuilderFlagAudit::repairCaptureLatestFrame,
  .settled = &BuilderFlagAudit::builderLive,
  .cached  = nullptr,
};

const DataModel::CachedFlagSpec DataModel::BuilderFlagAudit::kChangeDriven{
  .name    = "FrameBuilder::m_changeDriven",
  .fresh   = &BuilderFlagAudit::freshChangeDriven,
  .repair  = &BuilderFlagAudit::repairDatasetFlags,
  .settled = &BuilderFlagAudit::datasetFlagsSettled,
  .cached  = nullptr,
};

const DataModel::CachedFlagSpec DataModel::BuilderFlagAudit::kAnyAsyncSink{
  .name    = "BlockPublisher::m_anyAsyncSink",
  .fresh   = &BuilderFlagAudit::freshAnyAsyncSink,
  .repair  = &BuilderFlagAudit::repairAnyAsyncSink,
  .settled = &BuilderFlagAudit::builderLive,
  .cached  = nullptr,
};

//--------------------------------------------------------------------------------------------------
// Owner access
//--------------------------------------------------------------------------------------------------

/**
 * @brief The builder behind a checker's owner pointer.
 */
[[nodiscard]] static const DataModel::FrameBuilder& builderOf(const void* owner)
{
  SS_ASSERT(owner != nullptr, qFatal("BuilderFlagAudit: null owner"));
  return *static_cast<const DataModel::FrameBuilder*>(owner);
}

/**
 * @brief Mutable twin of builderOf for the repair callbacks.
 */
[[nodiscard]] static DataModel::FrameBuilder& mutableBuilderOf(void* owner)
{
  SS_ASSERT(owner != nullptr, qFatal("BuilderFlagAudit: null owner"));
  return *static_cast<DataModel::FrameBuilder*>(owner);
}

/**
 * @brief The frame parser whose engines feed the dataset-capture derivation.
 */
[[nodiscard]] static DataModel::FrameParser& frameParser()
{
  static auto& parser = DataModel::FrameParser::instance();
  return parser;
}

//--------------------------------------------------------------------------------------------------
// Derivations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether change-driven transform execution is on (an atomic read of the project setting).
 */
bool DataModel::BuilderFlagAudit::deriveChangeDriven()
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  return projectModel.changeDrivenTransforms();
}

/**
 * @brief Whether the latest-frame capture is needed: the control script runs, or the API observer
 *        consumes (enabled with a client attached).
 */
bool DataModel::BuilderFlagAudit::deriveCaptureLatest(const FrameBuilder& builder)
{
  static auto& controlScript = DataModel::ControlScript::instance();
  const auto* server         = builder.m_publisher.sinks().server;
  return controlScript.running() || (server && server->sinkActive());
}

/**
 * @brief Whether per-dataset values must be mirrored into the table store: only scripts can read
 *        them back, and none run while a final-value player replays.
 */
bool DataModel::BuilderFlagAudit::deriveCaptureDatasetValues(const FrameBuilder& builder)
{
  const bool transforms_read = builder.m_transforms.referencesTableApi();
  const bool external_users  = builder.m_externalTableUsers > 0;
  const bool parser_reads    = frameParser().anyEngineReferencesTableApi();
  return !builder.m_playerOpen && builder.m_tableStore.isInitialized()
      && (transforms_read || external_users || parser_reads);
}

//--------------------------------------------------------------------------------------------------
// Settled predicates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every flag is audited only while the builder is live and bound to its sinks.
 */
bool DataModel::BuilderFlagAudit::builderLive(const void* owner)
{
  const auto& builder = builderOf(owner);
  return !builder.m_shuttingDown && builder.m_publisher.bound();
}

/**
 * @brief The dataset-capture pair refreshes lazily on the next dataset pass, so it is audited only
 *        once no refresh is pending and the parser engines have not moved since.
 */
bool DataModel::BuilderFlagAudit::datasetFlagsSettled(const void* owner)
{
  const auto& builder = builderOf(owner);
  return builderLive(owner) && !builder.m_captureFlagsDirty
      && frameParser().engineEpoch() == builder.m_seenEngineEpoch;
}

//--------------------------------------------------------------------------------------------------
// Spec callbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief The operation mode the pipeline mirror holds (bound pipeline: guarded by builderLive).
 */
int DataModel::BuilderFlagAudit::freshOperationMode(const void* owner)
{
  return static_cast<int>(builderOf(owner).m_publisher.sinks().pipeline->operationMode());
}

/**
 * @brief Whether any replay player is open, from the bus-fed player mask.
 */
int DataModel::BuilderFlagAudit::freshPlayerOpen(const void* owner)
{
  return static_cast<int>(builderOf(owner).anyPlayerOpen());
}

/**
 * @brief The dataset-capture verdict derived from scratch.
 */
int DataModel::BuilderFlagAudit::freshCaptureDatasetValues(const void* owner)
{
  return static_cast<int>(deriveCaptureDatasetValues(builderOf(owner)));
}

/**
 * @brief The latest-frame capture verdict derived from scratch.
 */
int DataModel::BuilderFlagAudit::freshCaptureLatestFrame(const void* owner)
{
  return static_cast<int>(deriveCaptureLatest(builderOf(owner)));
}

/**
 * @brief The change-driven setting as the project currently holds it.
 */
int DataModel::BuilderFlagAudit::freshChangeDriven(const void* owner)
{
  Q_UNUSED(owner)
  return static_cast<int>(deriveChangeDriven());
}

/**
 * @brief The any-async-sink verdict derived from every bound sink.
 */
int DataModel::BuilderFlagAudit::freshAnyAsyncSink(const void* owner)
{
  return static_cast<int>(builderOf(owner).m_publisher.deriveSinkFlag());
}

/**
 * @brief Repairs the mode cache through the builder's own mode-change path.
 */
void DataModel::BuilderFlagAudit::repairOperationMode(void* owner)
{
  mutableBuilderOf(owner).onOperationModeChanged();
}

/**
 * @brief Repairs the player cache through the builder's own player-change path.
 */
void DataModel::BuilderFlagAudit::repairPlayerOpen(void* owner)
{
  mutableBuilderOf(owner).onPlayerOpenChanged();
}

/**
 * @brief Re-derives the dataset-capture pair through the builder's own refresh.
 */
void DataModel::BuilderFlagAudit::repairDatasetFlags(void* owner)
{
  mutableBuilderOf(owner).refreshDatasetCaptureFlag();
}

/**
 * @brief Re-derives the latest-frame capture flag, dropping captures on the falling edge.
 */
void DataModel::BuilderFlagAudit::repairCaptureLatestFrame(void* owner)
{
  mutableBuilderOf(owner).refreshLatestFrameCapture();
}

/**
 * @brief Re-derives the any-async-sink flag through the publisher's own refresh.
 */
void DataModel::BuilderFlagAudit::repairAnyAsyncSink(void* owner)
{
  mutableBuilderOf(owner).m_publisher.refreshSinkFlag();
}
