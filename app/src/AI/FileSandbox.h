/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace AI {

/**
 * @brief Sandboxed filesystem access for AI tool calls: workspace-wide reads, AI/-only writes.
 */
class FileSandbox {
public:
  static constexpr qint64 kMaxReadSlice  = 32 * 1024;
  static constexpr qint64 kMaxWriteBytes = 4 * 1024 * 1024;
  static constexpr int kMaxListEntries   = 2000;
  static constexpr int kMaxSearchFiles   = 5000;
  static constexpr int kMaxSearchHits    = 200;
  static constexpr int kMaxRecurseDepth  = 16;
  static constexpr int kBinarySniffBytes = 8192;

  [[nodiscard]] static FileSandbox& instance();

  [[nodiscard]] QJsonObject list(const QJsonObject& args) const;
  [[nodiscard]] QJsonObject read(const QJsonObject& args) const;
  [[nodiscard]] QJsonObject search(const QJsonObject& args) const;
  [[nodiscard]] QJsonObject write(const QJsonObject& args) const;
  [[nodiscard]] QJsonObject append(const QJsonObject& args) const;
  [[nodiscard]] QJsonObject remove(const QJsonObject& args) const;

  [[nodiscard]] QString registerDroppedPath(const QString& localPath);
  void clearDroppedPaths();

  [[nodiscard]] QString workspaceRoot() const;
  [[nodiscard]] QString writeRoot() const;
  [[nodiscard]] QStringList droppedPaths() const;

private:
  FileSandbox();
  FileSandbox(FileSandbox&&)                 = delete;
  FileSandbox(const FileSandbox&)            = delete;
  FileSandbox& operator=(FileSandbox&&)      = delete;
  FileSandbox& operator=(const FileSandbox&) = delete;

  /**
   * @brief Outcome of resolving an input path against the sandbox roots.
   */
  struct Resolved {
    bool ok;
    QString path;
    QString error;
    QString hint;
  };

  [[nodiscard]] Resolved resolveRead(const QString& input) const;
  [[nodiscard]] Resolved resolveWrite(const QString& input) const;
  [[nodiscard]] QStringList readRoots() const;
  [[nodiscard]] static QString canonicalParentJoin(const QString& absolute);
  [[nodiscard]] static bool isWithinRoot(const QString& canonical, const QString& root);

  QStringList m_droppedPaths;
};

}  // namespace AI
