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

#pragma once

#include <memory>
#include <vector>

#include "IO/StreamWorker.h"

class AppState;

namespace DataModel {
class FrameBuilder;
}  // namespace DataModel

namespace IO {

class HAL_Driver;
class StreamConfigBuilder;

/**
 * @brief Owns the live dense-lane workers, one per source whose stream lane is on. Everything a
 *        worker emits goes to the FrameBuilder QUEUED (spec 0055 D8) so the pipeline thread stays
 *        the single producer for every sink; this class never routes a block to a sink itself and
 *        never caps or strides a source's rate -- an exhausted pool drops whole blocks instead.
 */
class StreamWorkerPool {
public:
  /**
   * @brief One candidate stream source: the device id its workers publish under and the live
   *        driver whose lane flag and sample rate decide the configuration.
   */
  struct Source {
    int deviceId       = 0;
    HAL_Driver* driver = nullptr;
  };

  StreamWorkerPool(DataModel::FrameBuilder& frameBuilder,
                   AppState& appState,
                   const StreamConfigBuilder& configs);
  ~StreamWorkerPool();
  StreamWorkerPool(StreamWorkerPool&&)                 = delete;
  StreamWorkerPool(const StreamWorkerPool&)            = delete;
  StreamWorkerPool& operator=(StreamWorkerPool&&)      = delete;
  StreamWorkerPool& operator=(const StreamWorkerPool&) = delete;

  void stop();
  void setPaused(bool paused);
  void refreshExportFlags() const;
  void publishTemplates() const;
  void rebuild(const std::vector<Source>& sources, bool paused, bool connected);

  [[nodiscard]] const std::vector<std::unique_ptr<StreamWorker>>& workers() const noexcept;

private:
  void wireSinks(StreamWorker& worker) const;

private:
  DataModel::FrameBuilder& m_frameBuilder;
  AppState& m_appState;
  const StreamConfigBuilder& m_configs;
  std::vector<std::unique_ptr<StreamWorker>> m_workers;
};

}  // namespace IO
