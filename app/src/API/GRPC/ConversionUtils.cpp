/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#ifdef ENABLE_GRPC

#  include "API/GRPC/ConversionUtils.h"

//--------------------------------------------------------------------------------------------------
// QJsonObject -> google.protobuf.Struct
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a QJsonObject to a google.protobuf.Struct.
 *
 * Iterates over all keys in the JSON object and converts each value
 * recursively using toProtoValue(). Depth-bounded to prevent stack overflow.
 */
google::protobuf::Struct API::GRPC::ConversionUtils::toProtoStruct(const QJsonObject& json,
                                                                   int depth)
{
  google::protobuf::Struct result;

  // Depth guard against deeply nested JSON
  if (depth >= kMaxConversionDepth) [[unlikely]]
    return result;

  auto* fields = result.mutable_fields();
  for (auto it = json.begin(); it != json.end(); ++it)
    (*fields)[it.key().toStdString()] = toProtoValue(it.value(), depth + 1);

  return result;
}

/**
 * @brief Converts a QJsonValue to a google.protobuf.Value.
 *
 * Maps Qt JSON types to their protobuf equivalents:
 * - Null -> null_value
 * - Bool -> bool_value
 * - Double -> number_value
 * - String -> string_value
 * - Array -> list_value
 * - Object -> struct_value
 */
google::protobuf::Value API::GRPC::ConversionUtils::toProtoValue(const QJsonValue& json, int depth)
{
  google::protobuf::Value result;

  // Depth guard against deeply nested JSON
  if (depth >= kMaxConversionDepth) [[unlikely]] {
    result.set_null_value(google::protobuf::NULL_VALUE);
    return result;
  }

  switch (json.type()) {
    case QJsonValue::Null:
    case QJsonValue::Undefined:
      result.set_null_value(google::protobuf::NULL_VALUE);
      break;

    case QJsonValue::Bool:
      result.set_bool_value(json.toBool());
      break;

    case QJsonValue::Double:
      result.set_number_value(json.toDouble());
      break;

    case QJsonValue::String:
      result.set_string_value(json.toString().toStdString());
      break;

    case QJsonValue::Array: {
      auto* list     = result.mutable_list_value();
      const auto arr = json.toArray();
      for (const auto& elem : arr)
        *list->add_values() = toProtoValue(elem, depth + 1);
      break;
    }

    case QJsonValue::Object:
      *result.mutable_struct_value() = toProtoStruct(json.toObject(), depth + 1);
      break;
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// google.protobuf.Struct -> QJsonObject
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a google.protobuf.Struct to a QJsonObject.
 *
 * Iterates over all fields in the Struct and converts each value
 * recursively using toQJsonValue().
 */
QJsonObject API::GRPC::ConversionUtils::toQJsonObject(const google::protobuf::Struct& proto,
                                                      int depth)
{
  QJsonObject result;

  // Depth guard against deeply nested protobuf
  if (depth >= kMaxConversionDepth) [[unlikely]]
    return result;

  for (const auto& [key, value] : proto.fields())
    result.insert(QString::fromStdString(key), toQJsonValue(value, depth + 1));

  return result;
}

/**
 * @brief Converts a google.protobuf.Value to a QJsonValue.
 *
 * Maps protobuf value kinds to their Qt JSON equivalents.
 */
QJsonValue API::GRPC::ConversionUtils::toQJsonValue(const google::protobuf::Value& proto, int depth)
{
  // Depth guard against deeply nested protobuf
  if (depth >= kMaxConversionDepth) [[unlikely]]
    return QJsonValue(QJsonValue::Null);

  switch (proto.kind_case()) {
    case google::protobuf::Value::kNullValue:
      return QJsonValue(QJsonValue::Null);

    case google::protobuf::Value::kNumberValue:
      return QJsonValue(proto.number_value());

    case google::protobuf::Value::kStringValue:
      return QJsonValue(QString::fromStdString(proto.string_value()));

    case google::protobuf::Value::kBoolValue:
      return QJsonValue(proto.bool_value());

    case google::protobuf::Value::kStructValue:
      return QJsonValue(toQJsonObject(proto.struct_value(), depth + 1));

    case google::protobuf::Value::kListValue: {
      QJsonArray arr;
      for (const auto& elem : proto.list_value().values())
        arr.append(toQJsonValue(elem, depth + 1));
      return QJsonValue(arr);
    }

    default:
      return QJsonValue(QJsonValue::Null);
  }
}

//--------------------------------------------------------------------------------------------------
// Frame -> google.protobuf.Struct (fast path, no QJsonObject intermediary)
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Sets a string field on a protobuf Struct.
 */
inline void setString(google::protobuf::Struct& s, const char* key, const QString& val)
{
  (*s.mutable_fields())[key].set_string_value(val.toStdString());
}

/**
 * @brief Sets a double field on a protobuf Struct.
 */
inline void setNumber(google::protobuf::Struct& s, const char* key, double val)
{
  (*s.mutable_fields())[key].set_number_value(val);
}

}  // namespace

/**
 * @brief Converts a Frame directly to a protobuf Struct.
 *
 * This fast path avoids the QJsonObject intermediary, going straight from
 * the C++ Frame/Group/Dataset structs to protobuf Struct fields. Only the
 * fields needed by streaming clients are included.
 */
google::protobuf::Struct API::GRPC::ConversionUtils::frameToProtoStruct(
  const DataModel::Frame& frame)
{
  // Build top-level frame Struct
  google::protobuf::Struct result;
  setString(result, "title", frame.title);

  // Serialize each group and its datasets
  auto* groups_list = (*result.mutable_fields())["groups"].mutable_list_value();

  for (const auto& group : frame.groups) {
    auto* group_val = groups_list->add_values()->mutable_struct_value();
    setString(*group_val, "title", group.title);
    setString(*group_val, "widget", group.widget);

    auto* ds_list = (*group_val->mutable_fields())["datasets"].mutable_list_value();

    for (const auto& ds : group.datasets) {
      auto* ds_val = ds_list->add_values()->mutable_struct_value();
      setString(*ds_val, "title", ds.title);
      setString(*ds_val, "value", ds.value);
      setString(*ds_val, "units", ds.units);
      setNumber(*ds_val, "index", ds.index);
      setNumber(*ds_val, "widgetMin", qMin(ds.wgtMin, ds.wgtMax));
      setNumber(*ds_val, "widgetMax", qMax(ds.wgtMin, ds.wgtMax));
      setNumber(*ds_val, "plotMin", qMin(ds.pltMin, ds.pltMax));
      setNumber(*ds_val, "plotMax", qMax(ds.pltMin, ds.pltMax));
    }
  }

  return result;
}

#endif  // ENABLE_GRPC
