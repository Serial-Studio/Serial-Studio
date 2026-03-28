/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef ENABLE_GRPC

#  include <QString>

namespace API {
namespace GRPC {

/**
 * @brief Generates typed .proto files from the CommandRegistry.
 *
 * Walks all registered commands and their JSON Schema input definitions,
 * converting them to Protocol Buffers message types and a service definition
 * with per-command RPCs.
 *
 * The generated .proto is intended as a documentation/reference file for
 * external clients that want strongly-typed gRPC stubs. The actual gRPC
 * server always uses the generic Struct-based approach.
 */
class ProtoGenerator {
public:
  /**
   * @brief Generates a complete .proto file as a string.
   */
  [[nodiscard]] static QString generateProto();

  /**
   * @brief Exports the generated .proto to a file.
   * @return true on success.
   */
  [[nodiscard]] static bool exportToFile(const QString& filePath);

private:
  [[nodiscard]] static QString jsonTypeToProtoType(const QString& jsonType);
  [[nodiscard]] static QString sanitizeName(const QString& name);
};

}  // namespace GRPC
}  // namespace API

#endif  // ENABLE_GRPC
