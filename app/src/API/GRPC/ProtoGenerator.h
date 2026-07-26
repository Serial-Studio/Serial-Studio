/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef ENABLE_GRPC

#  include <QJsonObject>
#  include <QJsonValue>
#  include <QMap>
#  include <QString>
#  include <QStringList>

class QTextStream;

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
  [[nodiscard]] static const QJsonObject& ledger();
  [[nodiscard]] static QString jsonScalar(const QJsonValue& value);
  [[nodiscard]] static QString commentBlock(const QString& text);
  [[nodiscard]] static QString protoFieldName(const QString& param);
  [[nodiscard]] static QString sanitizeName(const QString& name);
  [[nodiscard]] static QString jsonTypeToProtoType(const QString& jsonType);
  [[nodiscard]] static QString fieldComment(const QString& param, const QJsonObject& schema);
  [[nodiscard]] static QMap<int, QString> numberedFields(const QJsonObject& props,
                                                         const QJsonObject& entry);
  [[nodiscard]] static QString buildMessage(const QString& name,
                                            const QJsonObject& schema,
                                            const QJsonObject& entry);

  static void writeProtoHeader(QTextStream& out);
  static void writeSharedMessages(QTextStream& out);
  static void buildCommandMessages(QStringList& message_defs, QStringList& rpc_lines);
  static void writeService(QTextStream& out,
                           const QStringList& message_defs,
                           const QStringList& rpc_lines);
};

}  // namespace GRPC
}  // namespace API

#endif  // ENABLE_GRPC
