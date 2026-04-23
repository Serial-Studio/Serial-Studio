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

#  include <google/protobuf/struct.pb.h>
#  include <QJsonArray>
#  include <QJsonObject>
#  include <QJsonValue>

#  include "DataModel/Frame.h"

namespace API {
namespace GRPC {

namespace ConversionUtils {

[[nodiscard]] google::protobuf::Struct frameToProtoStruct(const DataModel::Frame& frame);

constexpr int kMaxConversionDepth = 64;

[[nodiscard]] google::protobuf::Struct toProtoStruct(const QJsonObject& json, int depth = 0);
[[nodiscard]] google::protobuf::Value toProtoValue(const QJsonValue& json, int depth = 0);
[[nodiscard]] QJsonObject toQJsonObject(const google::protobuf::Struct& proto, int depth = 0);
[[nodiscard]] QJsonValue toQJsonValue(const google::protobuf::Value& proto, int depth = 0);

}  // namespace ConversionUtils
}  // namespace GRPC
}  // namespace API

#endif  // ENABLE_GRPC
