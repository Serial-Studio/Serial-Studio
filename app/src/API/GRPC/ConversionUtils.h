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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <google/protobuf/struct.pb.h>

#include "DataModel/Frame.h"

namespace API {
namespace GRPC {

/**
 * @brief Conversion utilities between Qt/Serial Studio types and protobuf.
 */
namespace ConversionUtils {

/**
 * @brief Converts a Frame directly to a protobuf Struct (fast path).
 *
 * Bypasses the intermediate QJsonObject serialization for maximum
 * throughput on the hotpath. Only includes the fields that plugins
 * and streaming clients need.
 */
[[nodiscard]] google::protobuf::Struct frameToProtoStruct(
  const DataModel::Frame& frame);

/**
 * @brief Converts a QJsonObject to a google.protobuf.Struct.
 */
[[nodiscard]] google::protobuf::Struct toProtoStruct(const QJsonObject& json);

/**
 * @brief Converts a QJsonValue to a google.protobuf.Value.
 */
[[nodiscard]] google::protobuf::Value toProtoValue(const QJsonValue& json);

/**
 * @brief Converts a google.protobuf.Struct to a QJsonObject.
 */
[[nodiscard]] QJsonObject toQJsonObject(const google::protobuf::Struct& proto);

/**
 * @brief Converts a google.protobuf.Value to a QJsonValue.
 */
[[nodiscard]] QJsonValue toQJsonValue(const google::protobuf::Value& proto);

} // namespace ConversionUtils
} // namespace GRPC
} // namespace API

#endif // ENABLE_GRPC
