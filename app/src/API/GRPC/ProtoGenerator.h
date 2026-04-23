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
 */
class ProtoGenerator {
public:
  [[nodiscard]] static QString generateProto();
  [[nodiscard]] static bool exportToFile(const QString& filePath);

private:
  [[nodiscard]] static QString jsonTypeToProtoType(const QString& jsonType);
  [[nodiscard]] static QString sanitizeName(const QString& name);
};

}  // namespace GRPC
}  // namespace API

#endif  // ENABLE_GRPC
