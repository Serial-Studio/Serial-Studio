---
spec: 0070-concern-classes
phase: guide
status: reference
updated: 2026-08-25
---

# Concern Decomposition Guide (input to spec 0070)

> Produced from the spec-0069 mechanical sweep before it was reset. The sweep proved
> **where the seams are**; it did not create boundaries. This file records the seams so the
> real work -- turning each cohesive concern into a class -- can be done against evidence
> instead of guesswork.
>
> **How to read it.** For each god object: the concerns the sweep found, the functions in
> each, and the member-usage matrix. A member touched by exactly one concern moves out with
> that concern for free. A member touched by every concern is the irreducible core. A member
> touched by two or three is the interesting case -- usually a sub-object waiting to be named.

## Why the file split alone was not enough

Splitting a `.cpp` leaves one set of data members that every part can still reach, so nothing
stops render code mutating IO state. The sweep made that visible: 23 of 27 components needed a
`*Shared.h` to compile, because the concerns were reaching across the seam. That shared header
is the smell -- it is the coupling the split failed to break, written down.


## Partitionability at a glance

`solo` = members touched by exactly one concern (free to extract). `4+` = members most concerns touch (irreducible core).

| component | members | concerns | solo | 2 | 3 | 4+ | verdict |
|---|---:|---:|---:|---:|---:|---:|---|
| `API/Handlers/ProjectHandlerEntities.cpp` | 1 | 82 | 1 | 0 | 0 | 0 | clean split -- extract per concern |
| `DataModel/Scripting/NativeTemplates/TextTemplates.cpp` | 18 | 11 | 15 | 1 | 0 | 2 | clean split -- extract per concern |
| `DataModel/Importers/ProtoImporter.cpp` | 9 | 7 | 7 | 0 | 0 | 2 | clean split -- extract per concern |
| `UI/Widgets/PainterContext.cpp` | 19 | 10 | 14 | 3 | 0 | 2 | clean split -- extract per concern |
| `DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp` | 13 | 15 | 9 | 4 | 0 | 0 | clean split -- extract per concern |
| `API/Server.cpp` | 20 | 6 | 10 | 8 | 0 | 2 | clean split -- extract per concern |
| `MQTT/Publisher.cpp` | 69 | 6 | 23 | 7 | 17 | 22 | partial -- extract the solo clusters |
| `Sessions/DatabaseManager.cpp` | 35 | 10 | 7 | 14 | 7 | 7 | partial -- extract the solo clusters |
| `Misc/ExtensionManager.cpp` | 32 | 9 | 6 | 7 | 10 | 9 | entangled -- extract named sub-objects only |
| `UI/Widgets/Terminal.cpp` | 49 | 9 | 8 | 14 | 11 | 16 | entangled -- extract named sub-objects only |
| `IO/Drivers/USB.cpp` | 32 | 8 | 5 | 9 | 9 | 9 | entangled -- extract named sub-objects only |
| `DataModel/ProjectModel.cpp` | 146 | 33 | 21 | 50 | 22 | 53 | entangled -- extract named sub-objects only |
| `UI/Taskbar.cpp` | 23 | 8 | 3 | 3 | 6 | 11 | entangled -- extract named sub-objects only |
| `Sessions/Player.cpp` | 40 | 11 | 5 | 10 | 12 | 13 | entangled -- extract named sub-objects only |
| `IO/Drivers/Audio.cpp` | 35 | 8 | 4 | 9 | 9 | 13 | entangled -- extract named sub-objects only |
| `DataModel/FrameBuilder.cpp` | 73 | 11 | 7 | 29 | 19 | 18 | entangled -- extract named sub-objects only |
| `UI/Widgets/Waterfall.cpp` | 65 | 9 | 6 | 25 | 25 | 9 | entangled -- extract named sub-objects only |
| `UI/WindowManager.cpp` | 44 | 8 | 4 | 7 | 14 | 19 | entangled -- extract named sub-objects only |
| `UI/Dashboard.cpp` | 71 | 10 | 6 | 12 | 28 | 25 | entangled -- extract named sub-objects only |
| `IO/Drivers/OpcUa.cpp` | 59 | 9 | 4 | 12 | 19 | 24 | entangled -- extract named sub-objects only |
| `AI/Conversation.cpp` | 30 | 10 | 2 | 3 | 7 | 18 | entangled -- extract named sub-objects only |
| `IO/Drivers/Modbus.cpp` | 20 | 7 | 1 | 1 | 5 | 13 | entangled -- extract named sub-objects only |
| `CSV/Player.cpp` | 41 | 8 | 2 | 13 | 12 | 14 | entangled -- extract named sub-objects only |
| `Misc/CLI.cpp` | 2 | 6 | 0 | 0 | 0 | 2 | single responsibility -- leave as one class |
| `IO/Drivers/BluetoothLE.cpp` | 16 | 8 | 0 | 1 | 5 | 10 | entangled -- extract named sub-objects only |
| `IO/ConnectionManager.cpp` | 33 | 6 | 0 | 14 | 16 | 3 | entangled -- extract named sub-objects only |
| `AI/ToolDispatcher.cpp` | 0 | 10 | 0 | 0 | 0 | 0 | single responsibility -- leave as one class |

## Per-component detail

### `IO/Drivers/Audio.cpp`

Concern files produced by the sweep (8), with their functions:

- **Audio.cpp** (77 lines, 2 functions)
  - Audio, ~Audio
- **AudioCallback.cpp** (57 lines, 2 functions)
  - callback, handleCallback
- **AudioDeviceModels.cpp** (100 lines, 6 functions)
  - inputChannelConfigurations, inputDeviceList, inputSampleFormats, outputChannelConfigurations, outputDeviceList, outputSampleFormats
- **AudioDeviceParams.cpp** (301 lines, 18 functions)
  - configureInput, configureOutput, generateLists, sampleRates, selectedInputChannelConfiguration, selectedInputDevice, selectedInputSampleFormat, selectedOutputChannelConfiguration, selectedOutputDevice, selectedOutputSampleFormat, selectedSampleRate, setSelectedInputChannelConfiguration, setSelectedInputDevice, setSelectedInputSampleFormat, setSelectedOutputChannelConfiguration, setSelectedOutputDevice, setSelectedOutputSampleFormat, setSelectedSampleRate
- **AudioDiscovery.cpp** (150 lines, 4 functions)
  - refreshAudioDevices, setDiscoveryPaused, syncInputParameters, syncOutputParameters
- **AudioHal.cpp** (318 lines, 14 functions)
  - applyPlatformAudioConfig, close, closeDevice, configurationOk, configureCaptureFormat, configurePlaybackFormat, isOpen, isReadable, isWritable, notificationCallback, onBackendStopped, open, startInputWorker, write
- **AudioParsing.cpp** (186 lines, 5 functions)
  - isStreamCapable, processInputBuffer, publishTypedBlock, setStreamLaneActive, streamLaneActive
- **AudioPropertyModel.cpp** (368 lines, 7 functions)
  - applyConnectionSettings, deviceIdentifier, driverProperties, persistSettings, restoreSettings, selectByIdentifier, setDriverProperty

  Member clusters (candidate classes):

  - `{AudioCallback, AudioHal, CORE}` owns 5: m_outputQueue, m_rtCaptureChannels, m_rtCaptureFormat, m_rtPlaybackChannels, m_rtPlaybackFormat
  - `{AudioParsing, CORE}` owns 4: m_csvBuffer, m_csvData, m_csvStream, m_streamLaneActive
  - `{AudioDiscovery, AudioHal, CORE}` owns 3: m_context, m_init, m_isOpen
  - `{AudioDeviceModels, AudioDeviceParams}` owns 2: m_knownConfigs, m_sampleFormats
  - `{AudioHal, CORE}` owns 2: m_inputWorkerTimer, m_stopNotifyArmed
  - `{AudioHal}` owns 2: m_device, m_inputWorkerThread

### `DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`

Concern files produced by the sweep (15), with their functions:

- **BinaryTemplates.cpp** (45 lines, 1 functions)
  - binaryNativeTemplates

  Member clusters (candidate classes):

  - `{BinaryHex.h, BinaryRaw.h}` owns 3: m_bigEndian, m_bytesPerValue, m_signedValues
  - `{BinaryMessagePack.h}` owns 3: m_keyIndex, m_keys, m_mapMode
  - `{BinaryModbus.h}` owns 2: m_registerOffset, m_signedRegisters
  - `{BinaryMavlink.h}` owns 1: m_marker
  - `{BinaryOpcUaDelta.h}` owns 1: m_schema
  - `{BinaryRtcm.h}` owns 1: m_validateCrc

### `IO/Drivers/BluetoothLE.cpp`

Concern files produced by the sweep (8), with their functions:

- **BleDiscovery.cpp** (117 lines, 4 functions)
  - initializeSharedState, onDeviceDiscovered, onDiscoveryError, onHostModeStateChanged
- **BleHal.cpp** (296 lines, 14 functions)
  - adapterAvailable, adapterPoweredOn, close, configurationOk, isConnecting, isOpen, isReadable, isWritable, onControllerDisconnected, onControllerError, open, operatingSystemSupported, write, writeCharacteristic
- **BleIdentity.cpp** (67 lines, 2 functions)
  - deviceIdentifier, selectByIdentifier
- **BlePropertyModel.cpp** (70 lines, 2 functions)
  - driverProperties, setDriverProperty
- **BleSlots.cpp** (178 lines, 5 functions)
  - configureCharacteristics, onCharacteristicChanged, onServiceDiscoveryFinished, onServiceError, onServiceStateChanged
- **BleSpecifics.cpp** (349 lines, 15 functions)
  - announceGattReady, characteristicIndex, characteristicNames, deviceCount, deviceIndex, deviceNames, selectDevice, selectService, selectServiceByUuid, selectedNotifyCharacteristicUuid, selectedServiceUuid, serviceNames, setCharacteristicIndex, setNotifyCharacteristicByUuid, startDiscovery
- **BluetoothLE.cpp** (45 lines, 2 functions)
  - BluetoothLE, ~BluetoothLE

  Member clusters (candidate classes):

  - `{BleHal, BleSlots, BleSpecifics}` owns 3: m_characteristics, m_serviceNames, m_serviceUuids
  - `{BleHal, BleSlots, CORE}` owns 2: m_pendingServiceIndex, m_probeServiceIndex
  - `{BleDiscovery, BleIdentity}` owns 1: m_pendingIdentifier

### `Misc/CLI.cpp`

Concern files produced by the sweep (6), with their functions:


### `IO/ConnectionManager.cpp`

Concern files produced by the sweep (6), with their functions:

- **ConnectionAccessors.cpp** (182 lines, 15 functions)
  - activeUiDriver, audio, bluetoothLE, canBus, connectedBluetoothLE, hid, modbus, mqtt, network, opcUa, process, setUiDriverProperty, uart, uiDriverForBusType, usb
- **ConnectionLifecycle.cpp** (679 lines, 27 functions)
  - anyDeviceConnecting, beginWaitCursor, concludeConnectRequest, connectAllDevices, connectDevice, disconnectAllDevices, disconnectDevice, dropUnavailablePrimaryDevice, endWaitCursor, isConnecting, notifyConnectedStateChanged, onDriverOpenFinished, resetFrameReader, setBusType, setChecksumAlgorithm, setFinishSequence, setPaused, setStartSequence, setWriteEnabled, setupExternalConnections, shutdownDrivers, toggleConnection, wireUiDriver
- **ConnectionManager.cpp** (235 lines, 5 functions)
  - ConnectionManager, buildFrameConfig, createDriver, instance, ~ConnectionManager
- **ConnectionSlots.cpp** (704 lines, 22 functions)
  - buildDeviceForSource, buildStreamConfig, diagnosticsBusFor, onConsoleDataReceived, onDeviceOpenFinished, onProjectSourceChanged, onRawDataReceived, onUiDriverConfigurationChanged, projectConfigurationOk, publishStreamTemplates, rebuildDevices, rebuildStreamWorkers, refreshConnectedState, refreshStreamExportFlags, stopStreamWorkers, streamLaneForSource, streamWorkers, syncUiDriverFromSource0, syncUiDriverToLive, wireDevice, wireStreamLifecycle, wireStreamWorkerSinks
- **ConnectionStatus.cpp** (209 lines, 16 functions)
  - availableBuses, busType, checksumAlgorithm, configurationOk, connectedDeviceCount, driver, driverForEditing, finishSequence, isConnected, isDeviceConnected, linkState, linkStats, paused, readOnly, readWrite, startSequence
- **ConnectionTransmit.cpp** (141 lines, 7 functions)
  - disarmReplyCapture, pollReplyBuffer, processMultiSourcePayload, processPayload, writeAndArmReply, writeData, writeDataToDevice

  Member clusters (candidate classes):

  - `{CORE, ConnectionAccessors, ConnectionLifecycle}` owns 11: m_audioUi, m_bluetoothLEUi, m_canBusUi, m_hidUi, m_modbusUi, m_mqttUi, m_networkUi, m_opcUaUi
  - `{CORE, ConnectionLifecycle}` owns 6: m_connectFanOut, m_connectPending, m_lastConnectedCount, m_lastConnectedState, m_lastConnectingState, m_waitCursorActive
  - `{CORE, ConnectionLifecycle, ConnectionStatus}` owns 3: m_finishSequence, m_startSequence, m_writeEnabled
  - `{ConnectionLifecycle, ConnectionSlots}` owns 3: m_pendingDialVerdicts, m_settings, m_streamWorkers
  - `{ConnectionSlots, ConnectionTransmit}` owns 2: m_replyBuffers, m_replyMutex
  - `{CORE, ConnectionSlots}` owns 2: m_rebuildingDevices, m_uiDriverSaveTimer

### `AI/Conversation.cpp`

Concern files produced by the sweep (10), with their functions:

- **Conversation.cpp** (204 lines, 12 functions)
  - Conversation, awaitingConfirmation, busy, issueRequest, lastError, messages, setAwaitingConfirmation, setBusy, setDispatcher, setLastError, setProvider, ~Conversation
- **ConversationBudget.cpp** (66 lines, 2 functions)
  - budgetedHistory, estimateTokens
- **ConversationHandoff.cpp** (100 lines, 5 functions)
  - buildHandoffDigest, handoffSeed, probeDegraded, probeDetail, setHandoffSeed
- **ConversationHelpIndex.cpp** (231 lines, 5 functions)
  - completeHelpFetch, fetchHelpIndex, fetchHelpPage, hardenHelpReply, helpUrlAllowed
- **ConversationHistory.cpp** (378 lines, 11 functions)
  - ageHistoryToolResults, collectAssistantToolUseIds, elideAgedToolResult, firstFreshUserTurnAt, keepValidUserContent, precedingAssistantToolUseIds, pruneHistory, reconcileHistoryToolPairs, reconcileHistoryToolPairsAt, stripOrphanToolResults, synthesizeMissingResults
- **ConversationReplyHandlers.cpp** (776 lines, 24 functions)
  - dispatchByCallSafety, dispatchMetaTool, flushPendingStreamUpdate, makeToolUseBlock, onPartialText, onPartialThinking, onReplyError, onReplyFinished, onThinkingBlockFinished, onToolCallRequested, rewriteHelpLinks, runMetaDescribe, runMetaExecuteCommand, runMetaHowTo, runMetaListCategories, runMetaListCommands, runMetaLoadSkill, runMetaScriptingDocs, runMetaSearchDocs, runMetaSnapshot, scheduleTransientRetry, scheduleUiFlush, shouldRetryAfterError, skillForCommand
- **ConversationSlots.cpp** (316 lines, 11 functions)
  - approveToolCall, approveToolCallGroup, cancel, clear, denyToolCall, denyToolCallGroup, evaluateProbe, injectRoutedSkill, maybeProposeMemory, probeComplianceKey, start
- **ConversationSnapshot.cpp** (116 lines, 4 functions)
  - firstUserText, loadSnapshot, messageCount, snapshot
- **ConversationToolCalls.cpp** (479 lines, 14 functions)
  - appendToolCallCard, appendUserMessage, beginAssistantMessage, makeTruncatedResult, readBackCommandFor, recordToolResult, releaseOutstandingToolResult, resumeAfterToolBatch, runAutoVerify, runToolCall, teardownReply, toolCallCategory, updateToolCallCard, verifySourceUpdate
- **ConversationTools.cpp** (477 lines, 11 functions)
  - appendBasicMetaTools, appendCommandMetaTools, appendCoreMetaTools, appendDocMetaTools, appendReferenceMetaTools, appendSearchMetaTool, dispatcherTools, essentialToolNames, makeMetaTool, objectSchemaWithProperty, stringProp

  Member clusters (candidate classes):

  - `{CORE, ConversationReplyHandlers, ConversationSlots}` owns 4: m_streamDirty, m_streamFlushTimer, m_toolCallCount, m_uiDirty
  - `{ConversationReplyHandlers, ConversationSlots, ConversationSnapshot}` owns 2: m_pendingThinkingBlocks, m_pendingToolUseBlocks
  - `{ConversationHandoff, ConversationSlots, ConversationSnapshot}` owns 1: m_probe
  - `{ConversationHelpIndex}` owns 1: m_helpFetchNam
  - `{CORE, ConversationReplyHandlers}` owns 1: m_thinkingIsSynthetic
  - `{CORE, ConversationSlots}` owns 1: m_lastError

### `CSV/Player.cpp`

Concern files produced by the sweep (8), with their functions:

- **CsvPlayerControl.cpp** (63 lines, 4 functions)
  - frontierPause, pause, play, toggle
- **CsvPlayerFiles.cpp** (452 lines, 10 functions)
  - closeFile, nextFrame, onIndexBatch, onIndexFinished, openFile, previousFrame, runQuickPass, startIndexing, stopIndexing
- **CsvPlayerMultiSource.cpp** (294 lines, 9 functions)
  - anchorSteadyBase, backfillSparseSources, buildReplayLayout, dataColumnToFileColumn, injectFrame, injectRow, injectSourceRow, rowSecondsSinceStart, rowSteadyTimestamp
- **CsvPlayerProcessing.cpp** (351 lines, 8 functions)
  - catchUpTargetRow, processFrameBatch, promptTimestampUnitScale, promptUserForDateTimeOrInterval, recomputeMsUntilNext, sendHeaderFrame, updateData, updateTimestampDisplay
- **CsvPlayerRows.cpp** (94 lines, 3 functions)
  - quickPlotPayload, rawRow, splitDataCells
- **CsvPlayerSeeking.cpp** (189 lines, 5 functions)
  - buildSeekWindow, performSeekSettle, performSeekTick, seekWindowStartRow, setProgress
- **CsvPlayerStatus.cpp** (83 lines, 9 functions)
  - filename, frameCount, framePosition, indexProgress, indexing, isOpen, isPlaying, progress, timestamp
- **Player.cpp** (109 lines, 5 functions)
  - Player, eventFilter, handleKeyPress, instance, ~Player

  Member clusters (candidate classes):

  - `{CORE, CsvPlayerFiles}` owns 5: m_dataOffset, m_indexGeneration, m_loader, m_loaderThread, m_timeScale
  - `{CsvPlayerFiles, CsvPlayerMultiSource}` owns 5: m_bitSourceIds, m_fileColumnSourceBit, m_rowSeconds, m_rowSourceBits, m_sourceColumnsByIndex
  - `{CsvPlayerFiles, CsvPlayerProcessing, CsvPlayerRows}` owns 2: m_cells, m_splitScratch
  - `{CORE, CsvPlayerFiles, CsvPlayerProcessing}` owns 2: m_anchorMs, m_intervalSeconds
  - `{CsvPlayerControl, CsvPlayerProcessing}` owns 1: m_elapsedTimer
  - `{CORE, CsvPlayerControl, CsvPlayerFiles}` owns 1: m_pausedAtFrontier

### `UI/Dashboard.cpp`

Concern files produced by the sweep (10), with their functions:

- **Dashboard.cpp** (678 lines, 16 functions)
  - Dashboard, applyBlock, applyBlockColumn, applyBlockValues, applyStructureSnapshot, drainBlockRing, drainStructureSnapshots, feedFftFromSamples, feedMultiplotBlockSweep, feedPlotBlockSweep, growTimeRing, growTimeRings, instance, onDisplayTick, restorePersistedSettings, streamTargetsFor
- **DashboardAccess.cpp** (298 lines, 22 functions)
  - actionIndexForId, actions, datasetExtremes, datasets, fftData, getDatasetWidget, getGroupWidget, gpsSeries, groupIdForUniqueId, groupUniqueIdForGroupId, multiplotData, multiplotSweep, multiplotTimeRings, plotData, plotData3D, plotSweep, plotTimeRing, processedFrame, rawFrame, title, waterfallData, widgetMap
- **DashboardFrames.cpp** (79 lines, 2 functions)
  - advancePlotClock, resetPlotClocks
- **DashboardQueries.cpp** (381 lines, 33 functions)
  - actionCount, autoHideToolbar, available, clockEnabled, connectStreamAvailableInputs, connectViewStateResets, containsCommercialFeatures, extensionIdAt, extensionSlot, formatValue, frameValid, frozen, layoutMargin, layoutSpacing, notificationLogEnabled, plotTimeRange, points, pointsWidgetVisible, pollThinningState, relativeIndex, showActionPanel, showAlignmentGuides, stopwatchEnabled, streamAvailable ...
- **DashboardSeries.cpp** (433 lines, 12 functions)
  - configureFftSeries, configureGpsSeries, configureWaterfallSeries, registerXAxisIfNeeded, setFftAudioTap, setWaterfallAudioTap, updateDataSeries, updateFftSeries, updateGpsSeries, updateLineSeries, updatePlot3DSeries, updateWaterfallSeries
- **DashboardSession.cpp** (405 lines, 20 functions)
  - bulkLoadPlotWindow, clearViewState, fillSeekPlotMulti, fillSeekPlotSingle, globalViewState, replaySeekKey, replaySeekSeries, saveGlobalViewState, saveWidgetViewState, setAutoHideToolbar, setFrozen, setLayoutMargin, setLayoutSpacing, setPlotTimeRange, setSettingsPersistent, setShowActionPanel, setShowAlignmentGuides, setViewStateJson, viewStateJson, widgetViewState
- **DashboardSetters.cpp** (370 lines, 17 functions)
  - armMultiplotSweep, armPlotSweep, clearPlotData, clearPushTables, fftPlotRunning, multiplotRunning, plotRunning, resetData, setFFTPlotRunning, setMultiplotRunning, setMultiplotSweep, setPlotRunning, setPlotSweep, setPlotSweepRetention, setPoints, setWaterfallRunning, waterfallRunning
- **DashboardTimeRings.cpp** (486 lines, 16 functions)
  - buildLinePushes, buildMultiplotPushes, configureLineSeries, configureMultiLineSeries, configurePlot3DSeries, makeHistoryRing, replayTimeRing, restoreMultiplotSweepConfig, restoreMultiplotTimeRings, restorePlotSweepConfig, restorePlotTimeRings, ringSnapshotKey, snapshotMultiplotTimeRings, snapshotPlotTimeRings, streamSampleRate, timeRingCapacity
- **DashboardTools.cpp** (229 lines, 8 functions)
  - activateAction, applyTimerMode, configureActions, setClockEnabled, setNotificationLogEnabled, setStopwatchEnabled, setTerminalEnabled, tickRepeatTimer
- **DashboardWidgetMap.cpp** (613 lines, 18 functions)
  - addExtensionStringTargets, appendExtremePush, applyDisplayTitles, buildDatasetReferences, buildExtremePushes, buildValuePushes, buildWidgetGroups, combineSourceFrames, datasetBucketBase, foldExtremes, handleMissingDataset, makeValuePush, processDatasetIntoWidgetMaps, rebuildDatasetReferences, reconfigureDashboard, refreshDisplayTitles, registerWidgets, relabelGroupAsMultiplotFallback

  Member clusters (candidate classes):

  - `{CORE, DashboardQueries, DashboardSession}` owns 5: m_autoHideToolbar, m_layoutMargin, m_layoutSpacing, m_showActionPanel, m_showAlignmentGuides
  - `{DashboardSeries, DashboardSetters, DashboardTimeRings}` owns 5: m_multiplotPushes, m_plot3DPushes, m_timePushes, m_xLinePushes, m_yLinePushes
  - `{CORE, DashboardQueries, DashboardTools}` owns 4: m_clockEnabled, m_notificationLogEnabled, m_stopwatchEnabled, m_terminalEnabled
  - `{DashboardSeries, DashboardSetters}` owns 3: m_fftPushes, m_gpsPushes, m_waterfallPushes
  - `{CORE, DashboardSetters, DashboardWidgetMap}` owns 3: m_datasetReferences, m_updateRetryInProgress, m_valuePushes
  - `{DashboardQueries, DashboardSetters, DashboardWidgetMap}` owns 2: m_extensionDatasetIds, m_extensionGroupIds

### `Sessions/DatabaseManager.cpp`

Concern files produced by the sweep (10), with their functions:

- **DatabaseAccessors.cpp** (194 lines, 20 functions)
  - busy, canonicalDbPath, csvExportBusy, csvExportProgress, fileName, filePath, isOpen, locked, pdfExportBusy, pdfExportProgress, pdfExportStatus, selectedSessionId, selectedSessionNotes, selectedSessionTags, sessionCount, sessionList, sessionMetadata, setDbPathOverride, tagList, tagsForSession
- **DatabaseExport.cpp** (290 lines, 8 functions)
  - exportSessionToCsv, exportSessionToPdf, launchPdfExport, pickReportLogo, renderReportFromPayload, requestPdfOutputPath, requestSessionDatasets, requestStreamStats
- **DatabaseFiles.cpp** (53 lines, 3 functions)
  - closeDatabase, openDatabase
- **DatabaseLocking.cpp** (93 lines, 2 functions)
  - lockDatabase, unlockDatabase
- **DatabaseManager.cpp** (103 lines, 7 functions)
  - DatabaseManager, instance, nextToken, refreshSessionList, setBusy, setupExternalConnections, ~DatabaseManager
- **DatabaseReproducibility.cpp** (509 lines, 17 functions)
  - advanceRegressionSweep, childFailureReport, concludeRegression, concludeVerification, countSweepVerdict, finishRegressionSweep, lastRegressionReport, latestVerdicts, latestVerification, publishRegressStartFailure, regressSession, regressSessionsByTag, regressionBusy, regressionSweepStatus, verificationBusy, verifySession, writeCandidateSnapshot
- **DatabaseSchema.cpp** (258 lines, 10 functions)
  - createSchema, createSchemaBlockTable, createSchemaProjectMetadata, createSchemaSampleTables, createSchemaSessionTables, createSchemaStreamTables, createSchemaTagTables, createSchemaVerifications, migrateColumnsTable, migrateSessionsTable
- **DatabaseSessions.cpp** (208 lines, 9 functions)
  - confirmDeleteSession, deleteSession, replaySelectedSession, restoreLastDatabase, restoreProjectFromDb, runRestoreProjectFromJson, setSelectedSessionId, setSelectedSessionNotes, storeProjectMetadata
- **DatabaseTags.cpp** (92 lines, 6 functions)
  - addTag, addTagAndAssign, assignTagToSession, deleteTag, removeTagFromSession, renameTag
- **DatabaseWorker.cpp** (250 lines, 14 functions)
  - initWorker, onWorkerClosed, onWorkerCsvFinished, onWorkerCsvProgress, onWorkerGlobalProjectJsonReady, onWorkerLockStateChanged, onWorkerMutationFinished, onWorkerNotesUpdated, onWorkerOpenFailed, onWorkerOpened, onWorkerReportDataReady, onWorkerSessionListRefreshed, onWorkerTagListRefreshed, shutdown

  Member clusters (candidate classes):

  - `{DatabaseReproducibility}` owns 5: m_lastRegressionReport, m_regressCandidateTemp, m_sweepCandidate, m_sweepReports, m_sweepTag
  - `{CORE, DatabaseSessions}` owns 3: m_appState, m_player, m_projectModel
  - `{CORE, DatabaseAccessors, DatabaseExport}` owns 2: m_pdfExportBusy, m_pdfExportProgress
  - `{CORE, DatabaseExport}` owns 2: m_pendingPdfActive, m_pendingPdfSessionId
  - `{CORE, DatabaseReproducibility}` owns 2: m_regressActive, m_sweepOwnsCandidate
  - `{CORE, DatabaseReproducibility, DatabaseWorker}` owns 2: m_sweepActive, m_verifyProcess

### `Misc/ExtensionManager.cpp`

Concern files produced by the sweep (9), with their functions:

- **ExtensionAutoUpdate.cpp** (164 lines, 6 functions)
  - autoUpdateExtensions, catalogName, checkForUpdatesOnStartup, confirmAutoUpdate, setAutomaticUpdates, setUpdateCheckEnabled
- **ExtensionHelpers.cpp** (317 lines, 13 functions)
  - appendOrphanedInstalledEntries, applyFilter, buildCatalogEntryMap, catalogEntryMatchesFilters, currentPlatformKey, downloadNextFile, extensionsPath, installedManifestPath, loadPluginMetadata, rebuildInstalledPlugins, resolvePlatform, restoreSelectionByPreviousId, themesPath
- **ExtensionInstall.cpp** (116 lines, 2 functions)
  - installExtension, uninstallExtension
- **ExtensionManager.cpp** (39 lines, 2 functions)
  - ExtensionManager, instance
- **ExtensionManifest.cpp** (112 lines, 6 functions)
  - isPathSafe, loadInstalledManifest, loadLocalManifest, resolveFileUrl, saveInstalledManifest, writeExtensionFile
- **ExtensionNetwork.cpp** (160 lines, 5 functions)
  - onExtensionMetaReply, onFileDownloadReply, onManifestReply, onReadmeReply, parseManifest
- **ExtensionPlugins.cpp** (595 lines, 22 functions)
  - buildPluginEnvironment, checkLaunchPreconditions, checkPluginDependencies, ensureApiServerForLaunch, installedPlugins, isPluginRunning, launchPlugin, launchSelectedPlugin, onDashboardAvailableChanged, onPluginFinished, onWorkspacePathChanged, pluginOutput, readPluginMetadata, registerRunningPlugin, resolveAndValidateEntry, restoreRunningPlugins, runningPlugins, startPluginProcess, stopAllPlugins, stopPlugin, stopSelectedPlugin, wirePluginProcessSignals
- **ExtensionProperties.cpp** (261 lines, 24 functions)
  - automaticUpdates, count, downloadProgress, extensionTypes, extensions, filterCategory, filterType, friendlyTypeName, hasUpdate, installedVersion, isInstalled, isLocalRepo, loading, platformKey, repositories, searchFilter, selectedExtension, selectedIndex, selectedReadme, setFilterCategory, setFilterType, setSearchFilter, setSelectedIndex, updateCheckEnabled
- **ExtensionRepository.cpp** (146 lines, 5 functions)
  - addRepository, browseLocalRepo, refreshRepositories, removeRepository, resetRepositories

  Member clusters (candidate classes):

  - `{ExtensionPlugins}` owns 4: m_pluginOutput, m_plugins, m_runningPlugins, m_userClosedPlugins
  - `{ExtensionHelpers, ExtensionProperties}` owns 3: m_filterCategory, m_filterType, m_searchFilter
  - `{ExtensionAutoUpdate}` owns 2: m_autoUpdateDeclined, m_autoUpdateQueue
  - `{ExtensionHelpers, ExtensionInstall, ExtensionNetwork}` owns 2: m_downloadQueue, m_pluginMetadataCache
  - `{CORE, ExtensionInstall, ExtensionNetwork}` owns 2: m_pendingDownloads, m_totalDownloads
  - `{ExtensionInstall, ExtensionManifest, ExtensionNetwork}` owns 2: m_currentInstallId, m_currentInstallMeta

### `DataModel/FrameBuilder.cpp`

Concern files produced by the sweep (11), with their functions:

- **FrameBuilder.cpp** (214 lines, 15 functions)
  - FrameBuilder, LatestFrameInfo, frame, instance, lastTransformDataset, lastTransformError, parsedFrameCount, prepareShutdown, quickPlotFrame, resetFrameCounters, setParseBudgetEnabled, skippedFrameCount, tableStore, transformErrorCount
- **FrameBuilderBlocks.cpp** (276 lines, 11 functions)
  - bindBlockToFrame, bindSlotTemplate, claimPoolSlot, ensureStructurePublished, flushBlock, flushOpenBlocks, notePoolExhausted, openBlockFor, preparePooledSlot, refreshFramePoolBudget, stageFrameValues
- **FrameBuilderDataTables.cpp** (448 lines, 17 functions)
  - initializeTableStore, injectTableApiJS, injectTableApiLua, luaDatasetGetFinal, luaDatasetGetRaw, luaDatasetSelector, luaMqttPublish, luaOnStoreThread, luaPushRegister, luaTableContext, luaTableGet, luaTableGetH, luaTableHandle, luaTableHandleMany, luaTableSet, luaTableSetH, refreshTableStoreFromProjectModel
- **FrameBuilderHotpath.cpp** (227 lines, 8 functions)
  - captureLatestChannelSpans, captureLatestChannels, captureLatestChunk, hotpathRxFrame, hotpathRxSourceFrame, publishBlock, publishReplayValues, replayBlock
- **FrameBuilderParserBudget.cpp** (754 lines, 26 functions)
  - applyDatasetValue, applyDatasetValueSpan, applyDatasetValues, applyDatasetValuesSpans, beginDatasetPass, decodeProjectChannels, drainLatestFrameSnapshot, endDatasetPass, ensureSourceFrame, guiLatestFrame, guiParseLoads, latestFrameSnapshot, noteParseBudgetThinning, parseBudgetAccount, parseBudgetReset, parseBudgetSkipFrame, parseBudgetThinning, parseLoadSnapshot, parseQuickPlotFrame, publishLatestFrameSnapshot, publishParseLoads, refreshDatasetCaptureFlag, refreshProjectSourceSnapshot, reprocessDatasetValues ...
- **FrameBuilderParsing.cpp** (484 lines, 11 functions)
  - applyReplaySpanValue, applyReplayTypedValue, assignFormattedDouble, capturedFrameStep, parseProjectFrame, replayChannelSpans, replayChannels, replayChannelsTyped, replayColumnsFor, trySpanLane
- **FrameBuilderPool.cpp** (125 lines, 6 functions)
  - PooledFrameSlot, buildStructureSnapshot, claimBlockSlot, noteStructurePublished, refreshBlockPoolBudget, structureIsCurrent
- **FrameBuilderQuickPlot.cpp** (233 lines, 5 functions)
  - audioFormatRange, buildQuickPlotAudioFrame, buildQuickPlotFrame, makeQuickPlotSource, registerQuickPlotHeaders
- **FrameBuilderSlots.cpp** (491 lines, 20 functions)
  - claimTableSnapshotSlot, dashboardTick, drainTableSnapshot, emitRepublishedFrame, guiTableApiContext, ingestStreamBlock, ingestStreamValues, noteGuiTableApiUser, onConnectedChanged, onOperationModeChanged, onSourceRemoved, publishQuickPlotAudioTemplate, publishSourceTemplate, publishSourceTemplateFrame, publishTableSnapshot, refreshStreamDrivenFrames, reprocessFrames, republishFrames, republishOneFrame, setStreamSourceIds
- **FrameBuilderTransforms.cpp** (711 lines, 19 functions)
  - applyTransform, applyTransformExpr, applyTransformJs, applyTransformLua, collectTransformEngineGarbage, compileTransforms, compileTransformsExpr, compileTransformsJS, compileTransformsLua, compileTransformsLuaEntry, destroyTransformEngines, expressionTableValue, luaTransformAcceptsInfo, noteTransformError, openSafeLibsForTransform, rebuildTransformsForPlayback, setReplayColumnMap, transformLuaWatchdogHook
- **FrameBuilderWiring.cpp** (363 lines, 13 functions)
  - applyProjectSnapshot, buildEnabledGroups, clearLatestFrames, collectProjectSnapshot, forgetPublishedStructures, onPlayerOpenChanged, refreshAnyAsyncSink, refreshAsyncSinks, refreshLatestFrameCapture, releaseReplayPoolStorage, setupExternalConnections, syncFromProjectModel, wireDisplayTickHooks

  Member clusters (candidate classes):

  - `{CORE, FrameBuilderSlots}` owns 9: m_guiTableApiUsers, m_guiTableSnapshot, m_lastConnectedState, m_publishedTableClock, m_publishedTableGeneration, m_tableMirrorRing, m_tableSnapshotPool, m_tableSnapshotPoolHint
  - `{CORE, FrameBuilderParserBudget}` owns 8: m_changeDriven, m_guiLatestFrameUsers, m_latestFrameMirrorRing, m_latestFrameSnapshotRequested, m_parseLoadMirrorRing, m_publishedLatestFrameSeq, m_seenEngineEpoch, m_skippedFrameCount
  - `{CORE, FrameBuilderParserBudget, FrameBuilderTransforms}` owns 6: m_compileGuard, m_compilePending, m_engineCacheSourceId, m_jsEngineForSource, m_jsTransformTimedOut, m_luaEngineForSource
  - `{FrameBuilderParserBudget}` owns 4: m_channelScratch, m_datasetDeps, m_guiLatestFrameMirror, m_guiParseLoads
  - `{CORE, FrameBuilderTransforms}` owns 3: m_lastTransformDatasetUniqueId, m_lastTransformError, m_transformErrors
  - `{CORE, FrameBuilderParserBudget, FrameBuilderParsing}` owns 2: m_captureDatasetValues, m_parseBudgetEnabled

### `IO/Drivers/Modbus.cpp`

Concern files produced by the sweep (7), with their functions:

- **Modbus.cpp** (91 lines, 3 functions)
  - Modbus, wireConfigurationSignals, ~Modbus
- **ModbusGeneration.cpp** (236 lines, 3 functions)
  - buildFrameParser, buildProject, generateProject
- **ModbusHal.cpp** (279 lines, 14 functions)
  - close, configurationOk, configureRtuClient, configureTcpClient, doClose, failDial, finalizeAndConnect, isConnecting, isOpen, isReadable, isWritable, open, waitForModbusTcpEndpoint, write
- **ModbusIdentity.cpp** (105 lines, 3 functions)
  - deviceIdentifier, scorePortMatch, selectByIdentifier
- **ModbusProperties.cpp** (392 lines, 33 functions)
  - addRegisterGroup, baudRate, baudRateList, clearRegisterGroups, dataBitsIndex, dataBitsList, host, parityIndex, parityList, pollInterval, port, protocolIndex, protocolList, registerGroupCount, registerGroupInfo, registerTypeList, removeRegisterGroup, serialPortIndex, serialPortList, setBaudRate, setDataBitsIndex, setHost, setParityIndex, setPollInterval ...
- **ModbusPropertyModel.cpp** (212 lines, 4 functions)
  - appendRtuProperties, appendTcpProperties, driverProperties, setDriverProperty
- **ModbusSlots.cpp** (269 lines, 8 functions)
  - buildRtuFrame, handleDialSetback, onErrorOccurred, onReadReady, onStateChanged, pollNextGroup, pollRegisters, refreshSerialPorts

  Member clusters (candidate classes):

  - `{CORE, ModbusHal, ModbusSlots}` owns 3: m_connecting, m_device, m_lastReply
  - `{CORE, ModbusProperties, ModbusSlots}` owns 2: m_currentGroupIndex, m_settings
  - `{ModbusHal}` owns 1: m_dialTarget
  - `{ModbusHal, ModbusSlots}` owns 1: m_serialPortLocations

### `IO/Drivers/OpcUa.cpp`

Concern files produced by the sweep (9), with their functions:

- **OpcUa.cpp** (148 lines, 5 functions)
  - OpcUa, loadSettings, saveTags, setupExternalConnections, ~OpcUa
- **OpcUaBrowse.cpp** (308 lines, 11 functions)
  - buildProject, cancelBrowse, datasetFor, generateProject, loadGeneratedProject, onBrowseConnected, onBrowseFailed, startBrowse, stopBrowse, tagModel, tagModelObject
- **OpcUaCertificates.cpp** (172 lines, 12 functions)
  - certificateJson, certificateMap, certificateObject, clientCertificate, describeTrustFailure, exportCertificate, regenerateCertificate, reportTrustFailure, revokeServerCertificate, trustServerCertificate, trustedCertificates, trustedJson
- **OpcUaDiscovery.cpp** (102 lines, 3 functions)
  - continuePendingDial, discoverEndpoints, onEndpointsFinished
- **OpcUaHal.cpp** (463 lines, 32 functions)
  - applyDeferredTags, close, configurationOk, credentialsAreExposed, dialEndpoint, doClose, endpointAcceptsToken, failDial, hasSelectedEndpoint, identity, isConnecting, isOpen, isReadable, isWritable, makeSession, onBrowseTimeout, onConnectFailed, onDialTimeout, onDiscoveryTimeout, onLinkDropped, onSessionConnected, onSessionDisconnected, open, prepareClientIdentity ...
- **OpcUaProperties.cpp** (596 lines, 39 functions)
  - addTag, authMode, authModeList, browsing, clearTags, describeMode, discovering, endpointIndex, endpointList, endpointSelectable, endpointUrl, endpointsJson, negotiatedMode, negotiatedPolicy, password, pollMode, publishEndpointSelection, publishingInterval, removeTag, revisedInterval, setAuthMode, setEndpointIndex, setEndpointUrl, setPassword ...
- **OpcUaPropertyModel.cpp** (155 lines, 2 functions)
  - driverProperties, setDriverProperty
- **OpcUaSecurity.cpp** (248 lines, 18 functions)
  - credentialsExposed, endpointUsable, policyIsDeprecated, securityMode, securityModeList, securityPolicy, securityPolicyDeprecated, securityPolicyIndex, securityPolicyList, selectBestEndpoint, setSecurityMode, setSecurityPolicy, setSecurityPolicyIndex, setUserCertificatePath, setUserKeyPath, supportedPolicies, userCertificatePath, userKeyPath
- **OpcUaSubscription.cpp** (416 lines, 17 functions)
  - adoptRevisedInterval, badTags, enterPollMode, issueRead, markBad, onFrameTick, onPollTick, onReadFinished, onSubscribed, onSubscriptionLost, onValueChanged, onWatchdogTick, reserveFrame, storeValue, subscribeAll, toSteady, wireTypeFor

  Member clusters (candidate classes):

  - `{CORE, OpcUaHal, OpcUaSubscription}` owns 7: m_clockValid, m_failedMonitors, m_frameCursor, m_lastNotifyNs, m_readInFlight, m_serverOffsetMs, m_watchdog
  - `{CORE, OpcUaProperties, OpcUaSubscription}` owns 5: m_badStatusCount, m_framesPublished, m_skippedPolls, m_unstampedCount, m_valuesReceived
  - `{CORE, OpcUaHal, OpcUaProperties}` owns 3: m_connecting, m_hasDeferred, m_linkDrops
  - `{OpcUaSubscription}` owns 3: m_firstIndex, m_frame, m_slotCount
  - `{CORE, OpcUaSubscription}` owns 3: m_clockOffsetNs, m_frameBytes, m_lastStampNs
  - `{CORE, OpcUaBrowse}` owns 2: m_browseTimer, m_tagModel

### `UI/Widgets/PainterContext.cpp`

Concern files produced by the sweep (10), with their functions:

- **PainterContext.cpp** (94 lines, 6 functions)
  - PainterContext, beginFrame, endFrame, height, setProjectDirectory, width
- **PainterGradient.cpp** (141 lines, 9 functions)
  - PainterGradient, addColorStop, brush, createConicGradient, createLinearGradient, createPattern, createRadialGradient, setConic, setRadial
- **PainterHelpers.cpp** (299 lines, 15 functions)
  - active, alignTextOrigin, applyBoxBlur, applyDashToPen, applyImageSmoothing, blurReciprocalTable, boxBlurHorizontal, boxBlurVertical, parseColor, parseFontSpec, rebindFillBrush, rebindStrokeBrush, renderWithShadow, resolveImagePath, shadowActive
- **PainterImages.cpp** (41 lines, 2 functions)
  - drawImage, drawImageScaled
- **PainterPaths.cpp** (322 lines, 17 functions)
  - arc, arcTo, beginPath, bezierCurveTo, clip, closePath, ellipse, fill, isPointInPath, isPointInStroke, lineTo, moveTo, parseRoundRectRadiiArray, quadraticCurveTo, rect, roundRect, stroke
- **PainterPattern.cpp** (43 lines, 1 functions)
  - PainterPattern
- **PainterShapes.cpp** (95 lines, 5 functions)
  - clearRect, fillRect, getLineDash, setLineDash, strokeRect
- **PainterStateStack.cpp** (120 lines, 9 functions)
  - getTransform, resetTransform, restore, rotate, save, scale, setTransform, transform, translate
- **PainterStyle.cpp** (356 lines, 36 functions)
  - fillStyle, fontSpec, globalAlpha, globalCompositeOperation, imageSmoothingEnabled, imageSmoothingQuality, lineCap, lineDashOffset, lineJoin, lineWidth, miterLimit, setFillStyle, setFontSpec, setGlobalAlpha, setGlobalCompositeOperation, setImageSmoothingEnabled, setImageSmoothingQuality, setLineCap, setLineDashOffset, setLineJoin, setLineWidth, setMiterLimit, setShadowBlur, setShadowColor ...
- **PainterText.cpp** (71 lines, 4 functions)
  - fillText, measureText, measureTextWidth, strokeText

  Member clusters (candidate classes):

  - `{PainterGradient}` owns 9: m_kind, m_r0, m_r1, m_startRad, m_stops, m_x0, m_x1, m_y0
  - `{CORE}` owns 3: m_commonFonts, m_height, m_width
  - `{PainterPattern}` owns 2: m_repetition, m_tile
  - `{CORE, PainterHelpers}` owns 1: m_projectDir
  - `{CORE, PainterPaths}` owns 1: m_path
  - `{CORE, PainterStateStack}` owns 1: m_stateStack

### `API/Handlers/ProjectHandlerEntities.cpp`

Concern files produced by the sweep (82), with their functions:

- **AssistantHandler.cpp** (1259 lines, 39 functions)
  - applyAddTileDatasetOption, applyAddTileRanges, buildAddWidgetParams, buildPlanRows, checkpoint, datasetOptionBitForSlug, datasetResolve, dryRunAssistantCommandForKind, ensureCustomizeMode, extractRangeParams, findWorkspaceById, findWorkspaceByTitle, forward, groupsFromListResponse, listCheckpoints, memoryPropose, projectBulkApply, registerCheckpointCommands, registerCommands, registerEditCommands, registerMemoryCommands, registerResolverCommands, resolveAddTileDataset, resolveCheckpoint ...
- **AudioHandler.cpp** (448 lines, 14 functions)
  - getConfiguration, getInputDevices, getInputFormats, getOutputDevices, getOutputFormats, getSampleRates, registerCommands, setInputChannelConfig, setInputDevice, setInputSampleFormat, setOutputChannelConfig, setOutputDevice, setOutputSampleFormat, setSampleRate
- **BluetoothLEHandler.cpp** (495 lines, 15 functions)
  - getCharacteristicList, getConfiguration, getDeviceList, getServiceList, getStatus, registerCommands, registerQueryCommands, registerSelectionCommands, selectDevice, selectService, selectServiceByUuid, setCharacteristicIndex, setNotifyCharacteristic, startDiscovery, writeCharacteristic
- **CANBusHandler.cpp** (405 lines, 13 functions)
  - getBitrateList, getConfiguration, getInterfaceError, getInterfaceList, getPluginList, registerCommands, setBitrate, setCanFD, setDataBitrate, setInterfaceIndex, setListenOnly, setLoopback, setPluginIndex
- **CSVExportHandler.cpp** (149 lines, 5 functions)
  - close, getStatus, registerCommands, setEnabled, setInterval
- **CSVPlayerHandler.cpp** (277 lines, 9 functions)
  - close, getStatus, open, registerCommands, registerFileCommands, registerPlaybackCommands, setPaused, setProgress, step
- **ConsoleHandler.cpp** (609 lines, 23 functions)
  - clear, exportClose, exportGetStatus, exportSetEnabled, getConfiguration, registerCommands, registerDisplayCommands, registerFontAndChecksumCommands, registerIoAndExportCommands, send, setAnsiColorsEnabled, setChecksumMethod, setCollapseDuplicates, setDataMode, setDisplayMode, setEcho, setEncoding, setFontFamily, setFontSize, setLineEnding, setSearchCaseSensitive, setShowTimestamp, setVt100Emulation
- **ControlScriptHandler.cpp** (180 lines, 5 functions)
  - dryRun, getScript, getStatus, registerCommands, setScript
- **DashboardHandler.cpp** (846 lines, 21 functions)
  - canonicalTitleForUniqueId, getData, getFPS, getOperationMode, getStatus, getTimeRange, getWidgetTitles, insertTailFrameFilterUid, registerCommands, registerModeAndFpsCommands, registerQueryCommands, registerTimeRangeCommands, registerWidgetDisplayCommands, reprocess, setFPS, setOperationMode, setTimeRange, setWidgetFreezeTitle, setWidgetTitle, tailFrames, tick
- **DataTablesHandler.cpp** (931 lines, 20 functions)
  - defaultValueProp, jsonToVariant, registerAdd, registerCommands, registerDelete, registerRegisterCommands, registerTableMutationCommands, registerTableQueryCommands, registerUpdate, requireString, tableAdd, tableDelete, tableGet, tableRename, tablesList, valueGet, valueGetH, valueHandle, valueSet, valueSetH
- **DiagnosticsHandler.cpp** (178 lines, 7 functions)
  - busArray, countsFor, registerCommands, resultToJson, run, status, supportedSlugs
- **ExtensionHandler.cpp** (354 lines, 14 functions)
  - addRepository, getAddonInfo, installExtension, listAddons, listRepositories, loadPluginState, refreshRepositories, registerCatalogCommands, registerCommands, registerRepositoryCommands, registerStateCommands, removeRepository, savePluginState, uninstallExtension
- **HIDHandler.cpp** (125 lines, 4 functions)
  - getConfiguration, getDeviceList, registerCommands, setDeviceIndex
- **IOManagerHandler.cpp** (480 lines, 12 functions)
  - connect, disconnect, getAvailableBuses, getLatestFrame, getStatus, registerBusConfigCommands, registerCommands, registerConnectionCommands, registerQueryCommands, setBusType, setPaused, writeData
- **LicensingHandler.cpp** (341 lines, 11 functions)
  - activate, activateOffline, deactivate, getStatus, guardStatus, registerCommands, registerOfflineCommand, setLicense, trialEnable, trialGetStatus, validate
- **MDF4ExportHandler.cpp** (103 lines, 4 functions)
  - close, getStatus, registerCommands, setEnabled
- **MDF4PlayerHandler.cpp** (256 lines, 7 functions)
  - close, getStatus, open, registerCommands, setPaused, setProgress, step
- **MirrorHandler.cpp** (87 lines, 3 functions)
  - getInfo, getStructure, registerCommands
- **ModbusHandler.cpp** (830 lines, 28 functions)
  - addRegisterGroup, clearRegisterGroups, getBaudRateList, getConfiguration, getDataBitsList, getParityList, getProtocolList, getRegisterGroups, getRegisterTypeList, getSerialPortList, getStopBitsList, registerCommands, registerCommonCommands, registerQueryCommands, registerRegisterGroupCommands, registerRtuCommands, registerTcpCommands, removeRegisterGroup, setBaudRate, setDataBitsIndex, setHost, setParityIndex, setPollInterval, setPort ...
- **MqttHandler.cpp** (591 lines, 17 functions)
  - applyPublisherEndpoint, applyPublisherSessionAndTls, applyPublisherTopics, applySubscriberEndpoint, applySubscriberSessionAndTls, indexInRange, portIsValid, publisherGetConfig, publisherGetStatus, publisherSetConfig, registerCommands, registerPublisherCommands, registerSubscriberCommands, subscriber, subscriberGetConfig, subscriberGetStatus, subscriberSetConfig
- **NetworkHandler.cpp** (602 lines, 20 functions)
  - getConfiguration, getSocketTypes, getStatus, lookup, networkDriver, registerCommands, registerUrlTransportCommands, setHttpBody, setHttpHeaders, setHttpInterval, setHttpMethod, setHttpUrl, setIgnoreTlsErrors, setRemoteAddress, setSocketType, setTcpPort, setUdpLocalPort, setUdpMulticast, setUdpRemotePort, setWebSocketUrl
- **NotificationsHandler.cpp** (279 lines, 12 functions)
  - channels, clearAll, clearChannel, list, makeSchema, markRead, notificationStringProp, post, readEventStrings, registerCommands, resolve, unreadCount
- **OpcUaHandler.cpp** (896 lines, 37 functions)
  - addTag, browse, clearTags, discoverEndpoints, exportCertificate, generateProject, getCertificate, getConfiguration, getStatus, listEndpoints, listTags, listTrusted, missingParam, opcUaDriver, regenerateCertificate, registerCommands, registerConfigCommands, registerDiscoveryCommands, registerQueryCommands, registerSecurityCommands, registerTagCommands, removeTag, revokeTrust, setAuthMode ...
- **ProblemsHandler.cpp** (218 lines, 8 functions)
  - buildFindingsResult, findingToJson, list, listCheckers, registerCommands, run, severityFromName, severityName
- **ProcessHandler.cpp** (237 lines, 8 functions)
  - getConfiguration, getRunningProcesses, registerCommands, setArguments, setExecutable, setMode, setPipePath, setWorkingDir
- **ProjectApiSupport.h** (311 lines, 12 functions)
  - appendDatasetWidgetTypes, appendStaleProjectWarning, applyWindow, attachProjectEpoch, attachWindowInfo, buildDatasetObject, captureProjectEpoch, datasetOptionsBitflag, detectLanguageMismatch, findDatasetByAlias, findDatasetByUniqueId, resolveDatasetSelector
- **ProjectHandlerAlarmCompat.cpp** (619 lines, 11 functions)
  - applyDatasetVisualizationFlags, datasetAdd, datasetAddMany, datasetDelete, datasetDuplicate, datasetSetOption, datasetSetOptions, groupAdd, groupDelete, groupDuplicate, widgetForDatasetOptions
- **ProjectHandlerBatch.cpp** (834 lines, 17 functions)
  - actionsList, buildBatchErrorEntry, buildBatchSchemaHint, datasetGetByPath, datasetGetByTitle, datasetGetByUniqueId, datasetGetExecutionOrder, datasetMove, datasetsList, dryRunAwareCommands, ensureBatchDryRunCompatible, executeBatchOp, groupMove, groupsList, groupsListSummary, projectBatch, validateBatchOps
- **ProjectHandlerBulk.cpp** (297 lines, 4 functions)
  - actionUpdate, datasetUpdate, groupUpdate, outputWidgetUpdate
- **ProjectHandlerDatasetFields.cpp** (440 lines, 9 functions)
  - actionAdd, actionDelete, actionDuplicate, datasetGetAlarmBands, datasetGetFFTMarkers, datasetSetAlarmBands, datasetSetFFTMarkers, datasetSetTransformCode, datasetSetVirtual
- **ProjectHandlerDiscovery.cpp** (390 lines, 12 functions)
  - appendActionRows, appendDatasetRow, appendDatasetRows, appendGroupRows, appendSourceRows, appendTableRows, appendWorkspaceRows, collectSearchRows, groupGet, matchesQuery, parseTypeFilter, projectSearch
- **ProjectHandlerEntities.cpp** (150 lines, 4 functions)
  - outputWidgetAdd, outputWidgetDelete, outputWidgetDuplicate, outputWidgetGet
- **ProjectHandlerFile.cpp** (739 lines, 22 functions)
  - buildGroupExplanations, buildSnapshotExplanations, buildSnapshotGroups, buildSnapshotHint, buildSnapshotSources, buildSnapshotTables, buildSnapshotWorkspaces, buildSourceExplanations, exportJson, fileNew, fileOpen, fileSave, getStatus, loadFromJSON, loadIntoFrameBuilder, projectRedo, projectSnapshot, projectUndo, setTitle, templateApply, templateList, validate
- **ProjectHandlerPainter.cpp** (499 lines, 11 functions)
  - datasetUpdateSchema, registerBatchCommand, registerDryRunCommands, registerEndToEndDryRunCommand, registerEntityUpdateCommands, registerFrameParserDryRunCommands, registerPainterCodeCommands, registerPainterCommands, registerScriptDryRunCommands, registerTemplateCommands, registerUpdateCommands
- **ProjectHandlerParser.cpp** (962 lines, 20 functions)
  - configurePrimarySourceFrame, configureSecondarySourceFrame, endToEndDryRun, frameParserConfigure, frameParserDryCompile, frameParserDryRun, frameParserGetConfig, outputWidgetDryRun, painterDryRun, painterGetCode, painterSetCode, parserGetCode, parserGetLanguage, parserGetTemplate, parserGetTemplateSchema, parserListTemplates, parserSetCode, parserSetLanguage, parserSetTemplate, transformDryRun
- **ProjectHandlerRegistration.cpp** (1224 lines, 24 functions)
  - registerActionCommands, registerCommands, registerDatasetAlarmCommands, registerDatasetCommands, registerDatasetCreateCommands, registerDatasetCrudCommands, registerDatasetFieldCommands, registerDatasetLifecycleCommands, registerDatasetMarkerCommands, registerDatasetOptionCommands, registerDiscoveryCommands, registerFileCommands, registerFileLifecycleCommands, registerFileMetadataCommands, registerGroupCommands, registerHistoryCommands, registerListCommands, registerOutputWidgetCommands, registerParserCodeCommands, registerParserCommands, registerParserConfigCommands, registerParserTemplateCommands, registerResolverCommands, registerSnapshotAndMoveCommands
- **ScriptsHandler.cpp** (233 lines, 7 functions)
  - findKind, get, kinds, list, loadManifest, registerCommands, summarizeEntry
- **SessionsHandler.cpp** (863 lines, 26 functions)
  - addTag, assignTag, close, deleteSession, deleteTag, exportToCsv, get, getCanonicalDbPath, getRegression, getStatus, getVerification, list, listTags, openDatabase, registerBrowsingCommands, registerCommands, registerLifecycleCommands, registerRegressionCommands, registerTagCommands, regress, removeTag, renameTag, replay, setExportEnabled ...
- **SourceHandler.cpp** (482 lines, 12 functions)
  - registerCommands, registerFrameParserCommands, registerPropertyCommands, sourceAdd, sourceConfigure, sourceDelete, sourceGetConfiguration, sourceGetFrameParserCode, sourceList, sourceSetFrameParserCode, sourceSetProperty, sourceUpdate
- **StreamHandler.cpp** (122 lines, 3 functions)
  - getInfo, getSources, registerCommands
- **SystemHandler.cpp** (138 lines, 5 functions)
  - exec, kill, projectDir, registerCommands, runningProcesses
- **UARTHandler.cpp** (472 lines, 14 functions)
  - getBaudRateList, getConfiguration, getPortList, registerCommands, registerLineSettings, setAutoReconnect, setBaudRate, setDataBits, setDevice, setDtrEnabled, setFlowControl, setParity, setPortIndex, setStopBits
- **USBHandler.cpp** (273 lines, 8 functions)
  - getConfiguration, getDeviceList, registerCommands, setDeviceIndex, setInEndpointIndex, setIsoPacketSize, setOutEndpointIndex, setTransferMode
- **WindowHandler.cpp** (565 lines, 20 functions)
  - getGroups, getLayout, getStatus, getWidgetSettings, getWindowStates, loadLayout, noSession, registerCommands, registerLayoutCommands, registerStateCommands, registerStatusCommands, registerWidgetSettingCommands, saveLayout, setActiveGroup, setActiveGroupIndex, setAutoLayout, setLayout, setLayoutPattern, setWidgetSetting, setWindowState
- **WorkspacesHandler.cpp** (1393 lines, 37 functions)
  - add, autoGenerate, cleanup, clearAll, compatibleWidgetTypes, customizeGet, customizeSet, dashboardRelativeIndexFor, datasetOffsetInGroup, findWorkspace, get, groupContribution, groupHasExtensionWidget, inProjectFileMode, list, parseWidgetId, refsToJson, registerCommands, registerCustomizeCommands, registerWidgetAddCommand, registerWidgetRefCommands, registerWidgetRemoveCommand, registerWidgetValidationCommands, registerWorkspaceCrudCommands ...

  Member clusters (candidate classes):

  - `{ProjectHandlerFile}` owns 1: m_engines

### `DataModel/ProjectModel.cpp`

Concern files produced by the sweep (33), with their functions:

- **ProjectEditorCommit.cpp** (955 lines, 29 functions)
  - applyGroupBarPanelStyleEdit, applyGroupImgModeEdit, applyGroupLogAxisEdit, applyGroupSourceEdit, applyGroupTitleEdit, applyGroupWidgetEdit, applyOutputWidgetField, commitAlarmBands, commitAlarmBandsForSelection, commitDatasetFormEdit, commitFrequencyMarkers, datasetAliasInUse, datasetFormEditAccepted, handleSourceBusTypeChange, handleSourceDecoderChecksumChange, handleSourceFrameDetectionChange, handleSourceFrameStartEndChange, handleSourcePropertyChange, handleSourceTitleChange, onActionItemChanged, onDatasetItemChanged, onGroupItemChanged, onOutputWidgetItemChanged, onProjectItemChanged ...
- **ProjectEditorForms.cpp** (1169 lines, 29 functions)
  - addFFTSection, alarmBandsEqual, appendDriverPropertyRows, bandsToVariantList, buildActionGeneralRows, buildActionModel, buildActionPayloadRows, buildActionTimingRows, buildDatasetModel, buildGroupBarPanelStyleRow, buildGroupDatasetsSection, buildGroupGeneralSection, buildGroupImageSection, buildGroupModel, buildGroupSourceSection, buildGroupWebViewRow, buildGroupXAxisRow, buildOutputWidgetCommonRows, buildOutputWidgetModel, buildOutputWidgetTransmitRow, buildOutputWidgetValueRows, buildProjectModel, buildSourceCommonRows, buildSourceFrameDetectionRows ...
- **ProjectEditorMqtt.cpp** (521 lines, 9 functions)
  - appendMqttClientIdentityRows, applyMqttClientIdentityEdit, buildMqttBrokerCredentials, buildMqttBrokerSection, buildMqttPublisherModel, buildMqttPublishingSection, buildMqttSslSection, onMqttPublisherItemChanged, openMqttScriptEditor
- **ProjectEditorMultiSelect.cpp** (518 lines, 10 functions)
  - buildMultiDatasetModel, buildMultiOutputWidgetModel, buildMultiSelectionModel, changeDatasetOptionForSelection, datasetEditValues, fanOutputWidgetSelectionEdit, multiSelectionEditKeepsModel, onMultiSelectionItemChanged, outputWidgetEditValues, tryMultiSelection
- **ProjectEditorSelection.cpp** (675 lines, 31 functions)
  - canGoBack, canGoForward, captureNavEntry, clearNavHistory, navDirection, navFind, navigateBack, navigateForward, onCurrentSelectionChanged, pushNavEntry, resolveNavEntry, sameNavTarget, selectAction, selectActionItem, selectControlScriptItem, selectDataTableItem, selectDataset, selectDatasetItem, selectFrameParser, selectGroup, selectGroupFolderItem, selectGroupItem, selectMqttPublisherItem, selectOutputWidget ...
- **ProjectEditorSummaries.cpp** (924 lines, 40 functions)
  - allWidgetsSummary, canMoveCurrent, canMoveCurrentDown, canMoveCurrentUp, cleanupUnresolvedWorkspaceWidgets, confirmCleanupUnresolvedWorkspaceWidgets, groupFolderContents, groupFolderPaths, groupFolderTree, moveCurrentAction, moveCurrentDataset, moveCurrentGroup, moveCurrentOutputWidget, moveWorkspace, selectControlScript, selectGroupFolder, selectMqttPublisher, selectTableFolder, selectUserTable, selectWorkspace, selectWorkspaceFolder, selectedFolderId, selectedGroupFolderId, selectedTableFolderId ...
- **ProjectEditorTree.cpp** (1043 lines, 30 functions)
  - appendActionTreeItems, appendControlScriptTreeItem, appendDatasetChildren, appendGroupFolderItems, appendGroupTreeItems, appendMqttPublisherTreeItem, appendOutputWidgetChildren, appendSharedMemoryTreeItems, appendSourceTreeItems, appendTableFolderItems, appendWorkspaceTreeItems, buildTreeItems, buildTreeModel, buildWorkspaceFolderTree, collapseTreeToOverview, consumePendingSelection, containerSelectionItem, createWorkspaceItem, entitySelectionItem, expandAllTreeItems, expandTreeToIndex, persistTreeExpansion, restoreExpandedStateMap, restoreTreeSelection ...
- **ProjectEditorWiring.cpp** (436 lines, 8 functions)
  - wireActionSignals, wireDatasetSignals, wireEditorSelfSignals, wireExternalSignals, wireGroupSignals, wireOutputWidgetSignals, wireProjectModelRebuilds, wireSourceSignals
- **ProjectHistory.cpp** (373 lines, 27 functions)
  - ProjectHistory, ProjectUndoFrame, ProjectUndoScope, applying, canRedo, canUndo, clear, commitPending, confirmRedo, confirmUndo, dropFront, dropOverflow, enterScope, isAtSavePoint, leave, markSaved, peekRedoState, peekUndoState, pushFrame, redoText, setApplying, setEnabled, setNextHint, stageCapture ...
- **ProjectModel.cpp** (360 lines, 9 functions)
  - ProjectModel, actionsForDiagram, clearTransientState, groupsForDiagram, instance, nextDatasetIndex, setupExternalConnections, sourcesForDiagram, tablesForDiagram
- **ProjectModelBulk.cpp** (298 lines, 6 functions)
  - confirmDeleteSelectedItems, deleteSelectedItems, duplicateSelectedItems, moveSelectedItemsToFolder, setGroupsInFolderEnabled, setItemsEnabled
- **ProjectModelCrud.cpp** (174 lines, 7 functions)
  - seedDatasetAliases, setDatasetEnabled, setGroupEnabled, uniqueAliasCandidate, updateAction, updateDataset, updateGroup
- **ProjectModelDocumentInfo.cpp** (819 lines, 71 functions)
  - actions, activeGroupId, activeWorkspaces, allocateUniqueId, changeDrivenTransforms, containsCommercialFeatures, datasetCount, diagramCollapse, displayTitle, displayTitles, editorGroupFolders, editorTableFolders, editorWorkspaceFolders, editorWorkspaces, extension_scope_key, externalWindows, frameParserCode, frameParserLanguage, frameParserParams, frameParserTemplate, freezeTitleMode, frozen, groupCount, groupIdForUniqueId ...
- **ProjectModelFolders.cpp** (848 lines, 38 functions)
  - addGroupFolder, addTableFolder, addWorkspaceFolder, confirmDeleteGroupFolder, confirmDeleteTableFolder, confirmDeleteWorkspaceFolder, deleteGroupFolder, deleteTableFolder, deleteWorkspaceFolder, duplicateGroupFolderSubtree, duplicateTableFolderSubtree, groupFolderTitle, moveFolderToFolder, moveGroupFolderInParent, moveGroupFolderToFolder, moveGroupToFolder, moveTableFolderInParent, moveTableFolderToFolder, moveTableToFolder, moveWorkspaceFolderInParent, moveWorkspaceInFolder, moveWorkspaceToFolder, promptAddGroupFolder, promptAddTableFolder ...
- **ProjectModelIdMutators.cpp** (223 lines, 8 functions)
  - deleteAction, deleteDataset, deleteGroup, deleteOutputWidget, duplicateAction, duplicateDataset, duplicateGroup, duplicateOutputWidget
- **ProjectModelInit.cpp** (343 lines, 15 functions)
  - clearJsonFilePath, controlScriptCode, newJsonFile, requestLuaFastMode, setChangeDrivenTransforms, setChecksumAlgorithm, setControlScriptCode, setFrameDetection, setFrameEndSequence, setFrameStartSequence, setFrozen, setLuaFastMode, setPlotTimeRange, setPointCount, setTitle
- **ProjectModelLoading.cpp** (912 lines, 28 functions)
  - applyHistorySnapshot, applyJsonDocumentCore, deduplicateUniqueIds, emitProjectLoadedSignals, enforceGplSingleSource, importProjectFromJson, loadChangeDrivenTransforms, loadFromJsonDocument, loadFrozen, loadLuaFastMode, loadPlotTimeRange, loadPointCount, loadProjectArrays, loadProjectRootScalars, loadWidgetSettingsAndWorkspaces, migrateLegacyDashboardLayout, migrateLegacyLayoutKeys, migrateLegacySeparator, migrateLegacyWaterfallYAxisIds, migrateLegacyWorkspaceRefs, migrateLegacyXAxisIds, openJsonFile, persistLegacyMigration, resolveDatasetTransformLanguages ...
- **ProjectModelMutation.cpp** (235 lines, 5 functions)
  - deleteCurrentAction, deleteCurrentDataset, deleteCurrentGroup, duplicateCurrentAction, duplicateCurrentGroup
- **ProjectModelOutputWidgets.cpp** (814 lines, 19 functions)
  - addAction, addDataset, addGroup, addOutputControl, addOutputPanel, applyGroupWidget, changeDatasetOption, confirmGroupWidgetChange, deleteCurrentOutputWidget, duplicateCurrentDataset, duplicateCurrentOutputWidget, ensurePainterDatasets, ensureValidGroup, populateFixedLayoutGroup, setGroupWidget, setOutputWidgetIcon, setOutputWidgetType, setSelectedOutputWidget, updateOutputWidget
- **ProjectModelPersistence.cpp** (487 lines, 15 functions)
  - apiSaveJsonFile, askSave, autoSave, finalizeProjectSave, flushAutoSave, hashProjectFile, promptDiskFileReload, resolveDiskFileChange, saveJsonFile, scheduleAutoSave, serializeToJson, setAutoSaveSuspended, syncRuntime, watchProjectFile, writeProjectFile
- **ProjectModelReorder.cpp** (292 lines, 9 functions)
  - moveAction, moveDataset, moveGroup, moveOutputWidget, moveWorkspace, remapAutoWorkspaceIdsAfterReorder, remapGroupIdsAfterReorder, remapHiddenGroupIdsAfterReorder, remapLayoutKeysAfterReorder
- **ProjectModelScalarSetters.cpp** (149 lines, 7 functions)
  - promptRenameAction, promptRenameDataset, promptRenameGroup, promptRenameSource, setActiveGroupId, setGroupLayout, setModified
- **ProjectModelSelection.cpp** (61 lines, 5 functions)
  - setDecoderMethod, setHexadecimalDelimiters, setSelectedAction, setSelectedDataset, setSelectedGroup
- **ProjectModelSources.cpp** (444 lines, 20 functions)
  - addSource, captureSourceSettings, deleteSource, duplicateSource, restoreSourceSettings, setFrameParserCode, setFrameParserLanguage, setFrameParserParams, setFrameParserTemplate, setSource0BusType, setSource0ConnectionSettings, storeFrameParserCode, updateSource, updateSourceBusType, updateSourceFrameParser, updateSourceFrameParserLanguage, updateSourceFrameParserParams, updateSourceFrameParserTemplate, updateSourceStreamLane, updateSourceTitle
- **ProjectModelStatus.cpp** (150 lines, 10 functions)
  - canSave, decoderMethod, frameDetection, locked, modified, mutationEpoch, saveBlockerCode, saveBlockerDetail, saveBlockerTitle, validateProject
- **ProjectModelTables.cpp** (542 lines, 19 functions)
  - addRegister, addTable, appendTableCopyToFolder, confirmDeleteRegister, confirmDeleteTable, deleteRegister, deleteTable, duplicateTableByPath, exportTableToCsv, findTableIndexByPath, importTableFromCsv, promptAddRegister, promptAddTable, promptRenameRegister, promptRenameTable, registersForTable, renameTable, tablePathFor, updateRegister
- **ProjectModelUndo.cpp** (178 lines, 10 functions)
  - canRedo, canUndo, history, lockProject, redo, redoText, setNextUndoHint, undo, undoText, unlockProject
- **ProjectModelWorkspaces.cpp** (1080 lines, 35 functions)
  - addWidgetToWorkspace, addWorkspace, appendAutoGroupWorkspaces, autoGenerateWorkspaces, buildAutoWorkspaceFoldersFor, buildAutoWorkspaces, cleanupWorkspaceWidgetRefs, clearAllWorkspaces, confirmDeleteWorkspace, confirmResetWorkspacesToAuto, customizeWorkspaces, deleteWorkspace, hiddenGroupsSummary, hideGroup, mergeAutoWorkspaceUpdates, promptAddWorkspace, promptRenameWorkspace, regenerateAutoWorkspacesUnnotified, removeWidgetFromWorkspace, renameWorkspace, reorderWorkspaces, resetWorkspacesToAuto, setCustomizeWorkspaces, setWorkspaceIcon ...
- **PropertyHooks.cpp** (557 lines, 43 functions)
  - ExtensibleMapOptions, LiveProviderOptions, ParallelValueOptions, StaticMapOptions, TupleOptions, aliasInUseByOtherDataset, datasetIndexPlaceholder, extremeHoldApplicable, fftEnabled, fftOrWaterfallEnabled, firstForIndex, indexForPair, indexForValue, insidePainterGroup, labels, ledBandsAbsent, ledEnabled, notVirtual, onReshape, onTitleChanged, onVirtualChanged, onWidgetChanged, onXAxisChanged, plotEnabled ...
- **PropertyValidators.cpp** (38 lines, 4 functions)
  - isValidColor, isValidDatasetIndex, isValidFftWindow, isValidTransformLanguage

  Member clusters (candidate classes):

  - `{ProjectHistory, ProjectHistory.h}` owns 18: m_applying, m_clock, m_depth, m_enabled, m_hintKey, m_hintLabel, m_model, m_pendingCoalesce
  - `{ProjectEditorSelection, ProjectEditorSummaries, ProjectEditorTree}` owns 12: m_controlScriptItem, m_groupFolderItems, m_mqttPublisherItem, m_selectedFolderId, m_selectedGroupFolderId, m_selectedTableFolderId, m_selectedUserTable, m_selectedWorkspaceId
  - `{ProjectEditorForms}` owns 9: m_actionModel, m_checksumMethods, m_decoderOptions, m_deviceListConn, m_frameDetectionMethods, m_groupModel, m_imgDetectionModes, m_sourceModel
  - `{PropertyHooks, PropertyHooks.h}` owns 8: m_context, m_count, m_entries, m_labels, m_notFound, m_notFoundValue, m_provider, m_values
  - `{ProjectEditorCommit, ProjectEditorMultiSelect}` owns 6: m_batchApplying, m_datasetWidgets, m_displayFormats, m_fftSamples, m_fftWindowValues, m_plotOptions
  - `{ProjectEditorSelection}` owns 5: m_navCursor, m_navDirection, m_navHistory, m_navigatingHistory, m_suppressViewChange

### `DataModel/Importers/ProtoImporter.cpp`

Concern files produced by the sweep (7), with their functions:


  Member clusters (candidate classes):

  - `{ProtoParser}` owns 6: m_cur, m_lexer, m_line, m_packageOut, m_pos, m_src
  - `{ProtoUi}` owns 1: m_ctx

### `MQTT/Publisher.cpp`

Concern files produced by the sweep (6), with their functions:

- **Publisher.cpp** (970 lines, 39 functions)
  - Publisher, PublisherWorker, applyBrokerConfig, applyClientPropertiesUnsafe, applyTimerInterval, bootstrap, buildReconnectFlow, closeBroker, closeResources, createWorker, describeMqttError, errorString, escapeCsvField, expandBlocks, instance, isResourceOpen, licenseValid, markConfigChanged, onClientErrorChanged, onClientStateChanged, openBroker, persistCredentialsToVault, processData, processItems ...
- **PublisherConfig.cpp** (108 lines, 3 functions)
  - applyProjectConfig, resetProjectConfig, toJson
- **PublisherGetters.cpp** (314 lines, 36 functions)
  - alpnEnabled, alpnProtocol, brokerEndpoint, cleanSession, clientCertificatePath, clientId, customClientId, defaultScriptTemplate, enabled, hostname, isConnected, keepAlive, keyPassphrase, messagesSent, mode, modeLabel, modes, mqttVersion, mqttVersions, notificationTopic, password, peerVerifyDepth, peerVerifyMode, peerVerifyModes ...
- **PublisherLifecycle.cpp** (230 lines, 12 functions)
  - addCaCertificates, emitStatsIfChanged, onWorkerBrokerError, onWorkerBrokerStateChanged, onWorkerScriptError, onWorkerTestConnectionFinished, regenerateClientId, selectClientCertificate, selectPemFile, selectPrivateKey, setupExternalConnections, testConnection
- **PublisherPublish.cpp** (97 lines, 5 functions)
  - hotpathTxRawBytes, hotpathTxRawFrame, ingestBlock, mqttPublish, onNotificationPosted
- **PublisherSetters.cpp** (367 lines, 27 functions)
  - setAlpnEnabled, setAlpnProtocol, setCleanSession, setClientCertificatePath, setClientId, setCustomClientId, setEnabled, setHostname, setKeepAlive, setKeyPassphrase, setMode, setMqttVersion, setNotificationTopic, setPassword, setPeerVerifyDepth, setPeerVerifyMode, setPort, setPrivateKeyPath, setPublishFrequency, setPublishNotifications, setScriptCode, setScriptLanguage, setScriptTopic, setSslEnabled ...

  Member clusters (candidate classes):

  - `{CORE}` owns 23: m_cfg, m_client, m_compiledScriptCode, m_consumerEnabled, m_credentialVault, m_csvFrameTitle, m_csvHeaderDirty, m_csvHeaderPayload
  - `{CORE, PublisherGetters, PublisherSetters}` owns 9: m_keyPassphrase, m_mqttVersions, m_password, m_peerVerifyMode, m_peerVerifyModes, m_protocolVersion, m_sslProtocol, m_sslProtocols
  - `{CORE, PublisherConfig, PublisherLifecycle}` owns 4: m_reportConnectionErrors, m_savingToProjectModel, m_skipNextSync, m_syncTimer
  - `{CORE, PublisherSetters}` owns 3: m_bytesSent, m_workerMode, m_workerScriptLanguage
  - `{CORE, PublisherPublish}` owns 2: m_rawBytesQueue, m_rawFramesQueue
  - `{CORE, PublisherConfig, PublisherSetters}` owns 1: m_inApply

### `API/Server.cpp`

Concern files produced by the sweep (6), with their functions:

- **Server.cpp** (259 lines, 14 functions)
  - Server, allowExternalConnections, applyExternalConnections, clientCount, createWorker, enabled, externalConnections, hasStreamSubscribers, instance, maxClients, removeConnection, setEnabled, setExternalConnections, ~Server
- **ServerAuth.cpp** (276 lines, 14 functions)
  - authToken, authorizeDeviceWrite, authorizeRemoteCommand, broadcastLifecycleEvent, constantTimeEquals, ensureAuthToken, handleAuthHandshake, hotpathTxData, ingestBlock, refreshStreamSubscriberFlag, regenerateAuthToken, setAuthToken, setupExternalConnections, verifyToken
- **ServerMirror.cpp** (213 lines, 8 functions)
  - handleMirrorCommand, isMirrorCommand, mirrorPublisher, mirrorSetRate, mirrorSubscribe, mirrorUnsubscribe, sendMirrorPayload, setStreamFrames
- **ServerReception.cpp** (612 lines, 15 functions)
  - acceptConnection, disconnectClient, handleJsonMessage, onClientCountChanged, onDataReceived, onErrorOccurred, onSocketDisconnected, processBufferedJson, processJsonLine, processNoNewlineBuffer, processRawJsonCommand, processRawLine, sendResponseToSocket, validateJsonMessage, validateRateLimits
- **ServerStreamBlocks.cpp** (184 lines, 7 functions)
  - handleStreamCommand, isStreamCommand, onStreamWriteDone, pumpStreamQueue, pushStreamBlock, streamSubscribe, streamUnsubscribe
- **ServerWorker.cpp** (296 lines, 16 functions)
  - addSocket, broadcastEvent, closeResources, disconnectSocket, isResourceOpen, onSocketDisconnected, onSocketReadyRead, processItems, removeSocket, setSocketStreamFrames, setTemplateFrame, underWriteCap, writeMirrorPayload, writeRawData, writeStreamBlock, writeToSocket

  Member clusters (candidate classes):

  - `{ServerWorker}` owns 5: m_droppedBroadcasts, m_mutedSockets, m_sockets, m_templates, m_warnedSockets
  - `{CORE, ServerAuth}` owns 4: m_anyStreamSubscriber, m_authToken, m_deviceWriteConsent, m_settings
  - `{CORE}` owns 4: m_consumerEnabled, m_enabled, m_pendingQueue, m_queueSize
  - `{CORE, ServerReception}` owns 3: m_clientCount, m_externalConnections, m_server
  - `{CORE, ServerMirror}` owns 1: m_mirrorLinked
  - `{ServerReception}` owns 1: m_workerThread

### `Sessions/Player.cpp`

Concern files produced by the sweep (11), with their functions:

- **Player.cpp** (141 lines, 7 functions)
  - Player, eventFilter, formatTimestamp, handleKeyPress, instance, updateTimestampDisplay, ~Player
- **SessionAlignment.cpp** (94 lines, 2 functions)
  - alignColumnsToProject, buildMultiSourceMapping
- **SessionFiles.cpp** (216 lines, 5 functions)
  - closeFile, onLoadFinished, openFile
- **SessionLocalDb.cpp** (108 lines, 4 functions)
  - clearLocalState, detectFinalValueColumns, openLocalDb, teardownLocalDb
- **SessionProcessing.cpp** (96 lines, 2 functions)
  - processFrameBatch, updateData
- **SessionSeeking.cpp** (248 lines, 7 functions)
  - buildSeekWindow, nextFrame, performSeekSettle, performSeekTick, previousFrame, seekWindowStartRow, setProgress
- **SessionStateCapture.cpp** (136 lines, 7 functions)
  - applyBundledViewState, capturePreSessionState, performPendingRestore, restorePreSessionState, restoreProjectFromJson, schedulePreSessionRestore, viewStateDashboard
- **SessionStatus.cpp** (113 lines, 11 functions)
  - filename, frameCount, framePosition, isOpen, isPlaying, loading, pause, play, progress, timestamp, toggle
- **SessionStreamReplay.cpp** (156 lines, 4 functions)
  - fetchStreamSamples, injectStreamBlocksAt, mergeStreamBlockTimes, replayStreamGroup
- **SessionSynthesis.cpp** (248 lines, 6 functions)
  - anchorSteadyBase, buildFrameAt, fillSeekWindowFromBlocks, frameValuesFromBlocks, injectFrame, rowSteadyTimestamp
- **SessionWorker.cpp** (56 lines, 3 functions)
  - initWorker, joinWorker, shutdown

  Member clusters (candidate classes):

  - `{SessionLocalDb, SessionStreamReplay}` owns 3: m_denseBlobQuery, m_streamBlobQuery, m_streamChannelBuf
  - `{CORE, SessionFiles, SessionStatus}` owns 2: m_loading, m_playing
  - `{SessionStateCapture}` owns 2: m_preSessionProjectPath, m_preSessionViewState
  - `{CORE, SessionStateCapture}` owns 2: m_preSessionCaptured, m_preSessionOperationMode
  - `{SessionSynthesis}` owns 2: m_sourcesAtCurrentTs, m_steadyBase
  - `{SessionAlignment, SessionLocalDb, SessionSynthesis}` owns 1: m_columnToSource

### `UI/Taskbar.cpp`

Concern files produced by the sweep (8), with their functions:

- **Taskbar.cpp** (204 lines, 11 functions)
  - Taskbar, connectToRegistry, createItemFromWidgetInfo, findGroupItemByGroupId, findItemByWidgetId, findItemByWindowId, onRegistryCleared, onWidgetCreated, onWidgetDestroyed, onWidgetUpdated, ~Taskbar
- **TaskbarFullModel.cpp** (286 lines, 8 functions)
  - appendGroupChildItem, attachGroupItemToFullModel, buildOverviewGroupItem, collectGroupWidgetIds, mapMainGroupWidgetId, mapWidgetToWindow, rebuildModel, selectGroupAfterRebuild
- **TaskbarGetters.cpp** (266 lines, 18 functions)
  - activeGroupId, activeGroupIndex, activeWindow, emitWorkspaceChangeAnticipation, firstWindow, fullModel, hasMaximizedWindow, independentWorkspace, indexForGroupId, layoutContextKey, layoutScope, nextActiveWindow, saveLayout, taskbarButtons, taskbarWindowIds, windowData, windowManager, windowState
- **TaskbarModel.cpp** (30 lines, 1 functions)
  - TaskbarModel
- **TaskbarSearch.cpp** (144 lines, 5 functions)
  - allWidgets, dismissSearch, searchFilter, searchResults, setSearchFilter
- **TaskbarSelection.cpp** (242 lines, 11 functions)
  - findWindowIdByGroupAndIndex, populateTaskbarFromGroup, populateTaskbarFromWorkspace, relativeIndexForWindow, resolveWorkspaceRefWindowId, selectWorkspaceById, setActiveGroupId, setActiveGroupIndex, setDesiredGroupId, setIndependentWorkspace, setLayoutScope
- **TaskbarWindowState.cpp** (253 lines, 11 functions)
  - applySavedWindowStates, closeWindow, minimizeWindow, onFocusCycleTick, registerWindow, setActiveWindow, setWindowManager, setWindowState, showWindow, startFocusCycle, unregisterWindow
- **TaskbarWorkspaces.cpp** (395 lines, 13 functions)
  - addWidgetToActiveWorkspace, buildWorkspaceTreeLevel, createWorkspace, deleteWorkspace, navigateToWidget, removeWidgetFromActiveWorkspace, removeWorkspaceTaskbarRow, renameWorkspace, setWorkspaceWidgets, workspaceContainingWidget, workspaceModel, workspaceTree, workspaceWidgetIds

  Member clusters (candidate classes):

  - `{CORE, TaskbarFullModel}` owns 3: m_widgetIdToWindowId, m_widgetRegistry, m_windowIdToWidgetId
  - `{CORE, TaskbarFullModel, TaskbarSelection}` owns 2: m_desiredGroupId, m_rebuildInProgress
  - `{TaskbarFullModel, TaskbarSelection, TaskbarWindowState}` owns 2: m_focusCycleQueue, m_windowConnections
  - `{CORE}` owns 2: m_batchUpdateInProgress, m_uiSessionRegistry
  - `{CORE, TaskbarGetters, TaskbarWindowState}` owns 1: m_restoringLayout
  - `{TaskbarGetters, TaskbarSelection, TaskbarWindowState}` owns 1: m_layoutScope

### `UI/Widgets/Terminal.cpp`

Concern files produced by the sweep (9), with their functions:

- **Terminal.cpp** (223 lines, 6 functions)
  - Terminal, charHeight, charWidth, loadWelcomeGuide, onThemeChanged, toggleCursor
- **TerminalAnsi.cpp** (395 lines, 12 functions)
  - dispatchCsiFinal, handleCsiCursorAbsolute, handleCsiCursorMove, handleCsiDecPrivateMode, handleCsiEraseDisplay, handleCsiEraseLine, processEscape, processFormat, processIgnoreSeq, processOsc, processResetFont, processText
- **TerminalBuffer.cpp** (593 lines, 23 functions)
  - append, appendString, applyScrollbackLimit, collapseCompletedLine, cursorPosition, geometryChange, hasTimestampPrefix, initBuffer, keyPressEvent, lineContentView, lineCount, lineHasRtlChar, linesPerPage, maxCharsPerLine, removeStringFromCursor, scanPrintableRun, scrollOffsetY, terminalColumns, terminalRows, translateEnterKey, translateKeyToVt100, translateSpecialKey, trimExcessLines
- **TerminalColor.cpp** (307 lines, 10 functions)
  - applyAnsiColor, applyAnsiSgrCode, formatDebugMessage, getColor256, getColor256Static, replaceData, setCursorPosition, shouldEndSelection, updateAnsiColorPalette
- **TerminalInput.cpp** (153 lines, 6 functions)
  - mouseDoubleClickEvent, mouseMoveEvent, mousePressEvent, mouseReleaseEvent, mouseUngrabEvent, wheelEvent
- **TerminalRender.cpp** (578 lines, 18 functions)
  - applyScrollbarOffset, drawCursor, drawRepeatBadge, drawSegmentMatch, drawSegmentSelection, handleScrollbarPress, isOverScrollbar, paint, paintScrollbar, paintSearchHighlights, paintSegment, paintSelectionHighlights, paintTextContent, renderAnsiSegment, renderFastSegment, scrollOffsetForThumbY, scrollbarThumbRect, scrollbarTrackRect
- **TerminalSearch.cpp** (152 lines, 9 functions)
  - clearSearch, refreshSearchMatches, scrollToCurrentMatch, searchActive, searchCurrentMatch, searchMatchCount, searchNext, searchPrevious, setSearchQuery
- **TerminalSelection.cpp** (152 lines, 6 functions)
  - calcCursorPixelX, clear, copy, findCharAtPixelX, positionToCursor, selectAll
- **TerminalStyle.cpp** (146 lines, 14 functions)
  - ansiColors, autoscroll, colorPalette, copyAvailable, font, paused, setAnsiColors, setAutoscroll, setColorPalette, setFont, setPaused, setScrollOffsetY, setVt100Emulation, vt100emulation

  Member clusters (candidate classes):

  - `{CORE, TerminalBuffer}` owns 3: m_collapseDuplicates, m_connectionManager, m_consoleHandler
  - `{TerminalColor}` owns 3: m_ansiBrightColors, m_ansiStandardColors, m_currentBgColor
  - `{CORE, TerminalStyle}` owns 3: m_ansiColors, m_autoscroll, m_emulateVt100
  - `{CORE}` owns 3: m_cursorTimer, m_lemonSqueezy, m_timerEvents
  - `{CORE, TerminalAnsi}` owns 2: m_currentFormatValue, m_privateMode
  - `{TerminalAnsi}` owns 2: m_formatValues, m_savedCursorPosition

### `DataModel/Scripting/NativeTemplates/TextTemplates.cpp`

Concern files produced by the sweep (11), with their functions:

- **TextTemplates.cpp** (47 lines, 3 functions)
  - singleFrame, textNativeTemplates, trNative

  Member clusters (candidate classes):

  - `{TextDelimited.h}` owns 4: m_quote, m_sepUtf8, m_separator, m_skipEmpty
  - `{TextKeyValue.h}` owns 3: m_kvSeparator, m_numericOnly, m_pairSeparator
  - `{TextXml.h}` owns 3: m_closeTags, m_openTags, m_tags
  - `{TextNmea0183.h}` owns 2: m_talker, m_validateChecksum
  - `{TextAtCommand.h}` owns 1: m_commands
  - `{TextDelimited.h, TextFixedWidth.h}` owns 1: m_trim

### `AI/ToolDispatcher.cpp`

Concern files produced by the sweep (10), with their functions:

- **ToolAssistantTools.h** (412 lines, 19 functions)
  - assistantToolDefs, bulkInputSchema, checkpointInputSchema, datasetInputSchema, isAssistantTool, listCheckpointsInputSchema, makeArrayProperty, makeMultiTypeProperty, makeObjectSchema, makeProperty, memoryProposeInputSchema, planInputSchema, restoreInputSchema, scriptPropsBag, snapshotInputSchema, stringEnum, stringEnumProperty, tileInputSchema, workspaceInputSchema
- **ToolBulkTools.h** (157 lines, 4 functions)
  - compactBatchRowResult, executeBulkApply, innerOpAllowed, makeInnerOpRejection
- **ToolCatalog.h** (440 lines, 8 functions)
  - availableTools, canonicalToolName, describeCommand, listCategories, listCommands, metaToolRoster, scopeDescriptions, searchCommands
- **ToolDispatch.h** (107 lines, 3 functions)
  - executeCommand, makeBlockedReply, withDefaultListLimit
- **ToolDispatcher.cpp** (106 lines, 4 functions)
  - ToolDispatcher, getProjectState, getSnapshot, runSafeCommand
- **ToolFilesystemTools.h** (167 lines, 9 functions)
  - executeFsTool, fsDeleteInputSchema, fsListInputSchema, fsReadInputSchema, fsSearchInputSchema, fsToolDefs, fsToolDescription, fsWriteInputSchema, isFsTool
- **ToolResolve.h** (266 lines, 6 functions)
  - compactDatasets, compactProjectSnapshotResult, optionSlugForWidget, resolveDataset, resolveGroup, resolveWorkspace
- **ToolScriptTools.h** (380 lines, 14 functions)
  - applyFrameParserScript, applyPainterScript, applyTransformScript, dryRunCommandForKind, dryRunResultOk, executeScriptApply, executeScriptDryRun, frameConfigArgsFor, frameParserDryRunCommand, maybeMarkDatasetVirtual, scriptTargetDataset, scriptingDocKindForScriptKind, seedEndToEndDryArgs, seedTransformDryArgs
- **ToolSupport.h** (166 lines, 5 functions)
  - assistantToolDescription, attachRepairHint, executeAssistantTool, makeRepairHint, runCommand
- **ToolTileTools.h** (257 lines, 7 functions)
  - applyTileRangeUpdates, createTileWorkspace, enableCustomizeMode, enableTileWidgetOption, executeAddTile, resolveOrCreateTileWorkspace, resolveTileDataset

### `IO/Drivers/USB.cpp`

Concern files produced by the sweep (8), with their functions:

- **USB.cpp** (94 lines, 2 functions)
  - USB, ~USB
- **UsbControlTransfers.cpp** (211 lines, 6 functions)
  - controlStatusText, controlTransferCallback, isHexChar, parseHexBytes, parseHexUInt, sendControlRequest
- **UsbHal.cpp** (163 lines, 7 functions)
  - close, configurationOk, isOpen, isReadable, isWritable, open, write
- **UsbHelpers.cpp** (600 lines, 23 functions)
  - activateSelectedEndpoints, allocateIsoTransfers, altSettingHasOtherMode, buildEndpointLists, cancelAndDrainTransfers, claimInterface, clearEndpointLists, collectEndpoint, configHasOtherMode, endpointErrorMessage, enrichDeviceLabel, eventLoop, freeTransfers, hotplugCallback, isoReadLoop, isoTransferCallback, notifyDrainWaiter, readConfigDescriptor, readLoop, releaseInterfaces, stopEventThread, stopReadThread, typeUsableInMode
- **UsbIdentity.cpp** (109 lines, 3 functions)
  - deviceIdentifier, deviceSerialMatches, selectByIdentifier
- **UsbProperties.cpp** (90 lines, 10 functions)
  - advancedModeEnabled, deviceIndex, deviceList, inEndpointIndex, inEndpointList, isoModeEnabled, isoPacketSize, outEndpointIndex, outEndpointList, transferMode
- **UsbPropertyModel.cpp** (82 lines, 2 functions)
  - driverProperties, setDriverProperty
- **UsbSlots.cpp** (250 lines, 8 functions)
  - enumerateDevices, onReadError, setDeviceIndex, setInEndpointIndex, setIsoPacketSize, setOutEndpointIndex, setTransferMode, setupExternalConnections

  Member clusters (candidate classes):

  - `{CORE, UsbHal, UsbHelpers}` owns 5: m_activeInEp, m_activeInEpType, m_activeOutEp, m_activeOutEpType, m_running
  - `{UsbHelpers}` owns 4: m_claimedInterfaces, m_drainCv, m_drainMutex, m_isoTransfers
  - `{CORE, UsbHelpers}` owns 3: m_drainWaiting, m_eventLoopRunning, m_isoInFlight
  - `{CORE, UsbControlTransfers, UsbHelpers}` owns 2: m_controlInFlight, m_controlTransfer
  - `{UsbHelpers, UsbProperties}` owns 2: m_inEndpointLabels, m_outEndpointLabels
  - `{CORE, UsbSlots}` owns 2: m_hotplugHandle, m_settings

### `UI/Widgets/Waterfall.cpp`

Concern files produced by the sweep (9), with their functions:

- **Waterfall.cpp** (255 lines, 6 functions)
  - Waterfall, interpolateLut, onFontsChanged, onThemeChanged, sampleColorMap, ~Waterfall
- **WaterfallAxes.cpp** (586 lines, 15 functions)
  - collectFreqTicks, computePlotRect, computeSourceRect, cursorReadoutValues, drawCursor, drawCursorTooltip, drawHistoryImage, drawMarkerChip, drawMarkers, drawXAxis, drawYAxis, markAxisDirty, markerChipAt, renderAxisLayer, visibleFreqWindow
- **WaterfallFft.cpp** (114 lines, 6 functions)
  - allocateFftPlan, freqFromWorld, imageRow, rebuildLogColumnTable, releaseFftPlan, worldFromFreq
- **WaterfallHotpath.cpp** (131 lines, 2 functions)
  - computeSmoothedRow, updateData
- **WaterfallImage.cpp** (82 lines, 4 functions)
  - paintRowInto, rebuildHistoryImage, writeRow, writeRowAt
- **WaterfallInput.cpp** (140 lines, 8 functions)
  - geometryChange, hoverEnterEvent, hoverLeaveEvent, hoverMoveEvent, mouseMoveEvent, mousePressEvent, mouseReleaseEvent, wheelEvent
- **WaterfallPaint.cpp** (44 lines, 1 functions)
  - paint
- **WaterfallTicks.cpp** (70 lines, 4 functions)
  - computeFreqTicks, computeTimeTicks, formatFreqTick, formatTimeTick
- **WaterfallViewState.cpp** (496 lines, 40 functions)
  - atDefaultView, audioRecordingEnabled, axisVisible, clearHistory, colorAt, colorMap, colorMapCount, colorMapName, colorbarVisible, cursorEnabled, fftSize, historySize, loadMarkers, markersVisible, maxDb, maxFreq, minDb, minFreq, panBy, recordingsFolder, resetView, running, samplingRate, setAudioRecordingEnabled ...

  Member clusters (candidate classes):

  - `{CORE, WaterfallAxes}` owns 8: m_accentColor, m_alarmColor, m_borderColor, m_commonFonts, m_gridColor, m_textColor, m_warningColor, m_yAxisTitle
  - `{CORE, WaterfallAxes, WaterfallViewState}` owns 5: m_axisVisible, m_xPan, m_xZoom, m_yPan, m_yZoom
  - `{CORE, WaterfallHotpath, WaterfallViewState}` owns 5: m_center, m_dashboard, m_halfRange, m_index, m_scaleIsValid
  - `{CORE, WaterfallImage, WaterfallViewState}` owns 5: m_colorMap, m_filledOnce, m_maxDb, m_minDb, m_writeRow
  - `{WaterfallFft, WaterfallHotpath}` owns 4: m_dbCache, m_fftOutput, m_samples, m_window
  - `{CORE, WaterfallAxes, WaterfallFft}` owns 3: m_logActive, m_logMax, m_logMin

### `UI/WindowManager.cpp`

Concern files produced by the sweep (8), with their functions:

- **WindowGeometry.cpp** (300 lines, 11 functions)
  - applyResizeCursor, computeResizedGeometry, detectResizeEdge, extractGeometry, findOverlapTarget, focusWindowUnderCursor, getIdForWindow, manualResizeTargetAt, sortedByVisualStacking, topmostWindowAt, updateHoverCursor
- **WindowInteraction.cpp** (436 lines, 12 functions)
  - childMouseEventFilter, commitManualGeometry, handleDragMove, handleResizeMove, hoverLeaveEvent, hoverMoveEvent, mouseDoubleClickEvent, mouseMoveEvent, mousePressEvent, mouseReleaseEvent, startManualPress, tryReorderDraggedWindow
- **WindowLayout.cpp** (509 lines, 14 functions)
  - applyManualLayout, autoLayout, autoLayoutEnabled, cascadeLayout, computeMergedEdges, constrainWindows, layoutRatioStops, patternHasPrimary, patternPreview, refreshLayoutChoice, selectLayoutPattern, storeManualGeometry, storeManualLayout, triggerLayoutUpdate
- **WindowManager.cpp** (60 lines, 2 functions)
  - WindowManager, ~WindowManager
- **WindowPersistence.cpp** (391 lines, 13 functions)
  - applySavedGeometries, clear, clearBackgroundImage, clearManualGesture, clearSnapGuides, loadLayout, parseSavedGeometries, preloadPendingGeometries, reconcileWindowOrder, resolveSavedOrder, restoreLayout, savedWindowStates, serializeLayout
- **WindowQueries.cpp** (157 lines, 21 functions)
  - alignmentGuides, backgroundImage, firstTileWindowId, fractionPreviewLabel, fractionPreviewRect, frozen, gridEnabled, gridSize, layoutPattern, layoutRatio, manualGestureActive, manualGestureGeometry, mergedEdges, sizeMatchRect, sizeMatchVisible, snapIndicator, snapIndicatorVisible, spacingIndicators, windowOrder, zCounter, zOrder
- **WindowRegistry.cpp** (220 lines, 11 functions)
  - bringToFront, registerWindow, selectBackgroundImage, setAutoLayoutEnabled, setBackgroundImage, setFrozen, setGridEnabled, setGridSize, setLayoutContext, setTaskbar, unregisterWindow
- **WindowSnap.cpp** (86 lines, 4 functions)
  - cacheSnapSiblings, publishFractionPreview, publishManualGesture, publishSnapGuides

  Member clusters (candidate classes):

  - `{WindowPersistence, WindowQueries, WindowSnap}` owns 5: m_alignmentGuides, m_fractionPreviewLabel, m_fractionPreviewRect, m_sizeMatchRect, m_spacingIndicators
  - `{CORE, WindowLayout, WindowPersistence}` owns 3: m_lastCanvasHeight, m_lastCanvasWidth, m_suppressGeometrySignal
  - `{WindowLayout, WindowQueries}` owns 2: m_layoutPattern, m_mergedEdges
  - `{WindowRegistry}` owns 2: m_layoutContextKey, m_workspaceConnection
  - `{WindowGeometry, WindowInteraction}` owns 1: m_initialGeometry
  - `{WindowInteraction, WindowQueries}` owns 1: m_snapIndicator
