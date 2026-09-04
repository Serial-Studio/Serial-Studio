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

#include "Core/HotpathOptimization.h"
#include "DataModel/DataBlock.h"

namespace IO {
class PipelineHost;
}  // namespace IO

namespace CSV {
class Export;
}  // namespace CSV

namespace MDF4 {
class Export;
}  // namespace MDF4

namespace API {
class Server;

namespace GRPC {
class GRPCServer;
}  // namespace GRPC
}  // namespace API

#ifdef BUILD_COMMERCIAL
namespace Sessions {
class Export;
}  // namespace Sessions

namespace MQTT {
class Publisher;
}  // namespace MQTT

namespace Widgets {
class AudioExport;
}  // namespace Widgets

namespace InfluxDB {
class Export;
}  // namespace InfluxDB
#endif

namespace DataModel {

/**
 * @brief The publish fan-out of the frame builder (spec 0075, A6/R12.8): the dashboard hop, the
 *        cached any-async-sink flag and the ONE trimmed copy every recording sink shares. Runs on
 *        the pipeline thread only, which is what makes it the single producer for every sink.
 */
class BlockPublisher {
public:
  /**
   * @brief Every sink a finished block reaches. The composition root resolves them once, in
   *        FrameBuilder::setupExternalConnections(), and binds them here.
   */
  struct Sinks {
    IO::PipelineHost* pipeline = nullptr;
    API::Server* server        = nullptr;
    CSV::Export* csv           = nullptr;
    MDF4::Export* mdf4         = nullptr;
#ifdef BUILD_COMMERCIAL
    Sessions::Export* sessions  = nullptr;
    MQTT::Publisher* mqtt       = nullptr;
    Widgets::AudioExport* audio = nullptr;
    InfluxDB::Export* influx    = nullptr;
#endif
#ifdef ENABLE_GRPC
    API::GRPC::GRPCServer* grpc = nullptr;
#endif
  };

  explicit BlockPublisher(const bool& maskSinks);

  BlockPublisher(BlockPublisher&&)                 = delete;
  BlockPublisher(const BlockPublisher&)            = delete;
  BlockPublisher& operator=(BlockPublisher&&)      = delete;
  BlockPublisher& operator=(const BlockPublisher&) = delete;

  void bind(const Sinks& sinks);
  void refreshSinkFlag();
  void publish(const DataBlockPtr& block);

  [[nodiscard]] bool anyAsyncSink() const noexcept;

private:
  [[nodiscard]] bool observedByReadOnly() const;
  void fanOutToObservers(const DataBlockPtr& block);

private:
  // Binds FrameBuilder::m_maskSinks, whose address never moves; BlockStager binds the same bool
  const bool& m_maskSinks;
  bool m_anyAsyncSink;
  Sinks m_sinks;
};

}  // namespace DataModel
