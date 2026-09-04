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

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace AI {

/**
 * @brief Sidecar store of small consent-gated facts the assistant carries across chats:
 *        capped, LRU-evicted, secret-scrubbed, never written into project files.
 */
class MemoryStore {
public:
  static constexpr int kMaxFactChars = 400;

  MemoryStore();

  [[nodiscard]] static QStringList categories();

  [[nodiscard]] bool isEmpty() const noexcept;
  [[nodiscard]] QVariantList list() const;
  [[nodiscard]] QString indexBlock(const QString& projectPath) const;

  [[nodiscard]] bool addFact(const QString& category,
                             const QString& text,
                             const QString& projectPath);
  [[nodiscard]] bool updateFact(const QString& id, const QString& text);
  void removeFact(const QString& id);

private:
  /**
   * @brief One remembered fact; project-category facts carry the project path they
   *        belong to, every other category applies installation-wide.
   */
  struct Fact {
    QString id;
    QString category;
    QString text;
    QString projectPath;
    qint64 createdAt  = 0;
    qint64 lastUsedAt = 0;
  };

  static constexpr int kMaxFacts      = 100;
  static constexpr int kMaxIndexBytes = 4096;
  static constexpr int kMaxFileBytes  = 256 * 1024;

  [[nodiscard]] static QString memoryPath();
  [[nodiscard]] static bool sanitizeText(QString& text);
  [[nodiscard]] int indexOf(const QString& id) const;
  void evictOverflow();
  void readStore();
  [[nodiscard]] bool writeStore() const;

private:
  QList<Fact> m_facts;
};

}  // namespace AI
