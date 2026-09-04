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
#include <QList>
#include <QString>
#include <QVariantList>

namespace AI {

/**
 * @brief On-disk catalog of named assistant chats, each a separate snapshot file.
 */
class ChatStore {
public:
  ChatStore();

  [[nodiscard]] QVariantList list() const;
  [[nodiscard]] bool isEmpty() const noexcept;
  [[nodiscard]] bool contains(const QString& id) const;

  [[nodiscard]] QString lastActiveId() const;
  void setLastActiveId(const QString& id);

  [[nodiscard]] QString createChat();
  [[nodiscard]] QJsonObject loadChat(const QString& id) const;
  void saveChat(const QString& id, const QJsonObject& snapshot, const QString& title, int count);
  void removeChat(const QString& id);
  void renameChat(const QString& id, const QString& title);

private:
  /**
   * @brief Lightweight per-chat metadata held in the index file.
   */
  struct Meta {
    QString id;
    QString title;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;
    int count        = 0;
  };

  [[nodiscard]] static QString chatsDir();
  [[nodiscard]] static QString indexPath();
  [[nodiscard]] static QString chatPath(const QString& id);
  [[nodiscard]] int indexOf(const QString& id) const;
  void readIndex();
  void writeIndex() const;

private:
  QList<Meta> m_chats;
  QString m_lastActive;
};

}  // namespace AI
