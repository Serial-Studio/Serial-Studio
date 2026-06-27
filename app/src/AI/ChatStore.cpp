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

#include "AI/ChatStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QUuid>
#include <QVariantMap>

#include "AI/Logging.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Path helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the directory holding per-chat snapshot files and the index.
 */
QString AI::ChatStore::chatsDir()
{
  const auto base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  return base + QStringLiteral("/ai/chats");
}

/**
 * @brief Returns the absolute path of the chat index file.
 */
QString AI::ChatStore::indexPath()
{
  return chatsDir() + QStringLiteral("/index.json");
}

/**
 * @brief Returns the absolute path of one chat's snapshot file.
 */
QString AI::ChatStore::chatPath(const QString& id)
{
  return chatsDir() + QStringLiteral("/") + id + QStringLiteral(".json");
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the on-disk chat index into memory.
 */
AI::ChatStore::ChatStore()
{
  readIndex();
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns chat metadata rows for QML in stable creation order (newest first); chats are
 *        never reordered on activity.
 */
QVariantList AI::ChatStore::list() const
{
  QVariantList out;
  for (const auto& meta : m_chats) {
    QVariantMap row;
    row[QStringLiteral("id")]        = meta.id;
    row[QStringLiteral("title")]     = meta.title;
    row[QStringLiteral("updatedAt")] = meta.updatedAt;
    row[QStringLiteral("count")]     = meta.count;
    out.append(row);
  }
  return out;
}

/**
 * @brief Returns true when no chats exist yet.
 */
bool AI::ChatStore::isEmpty() const noexcept
{
  return m_chats.isEmpty();
}

/**
 * @brief Returns true when a chat with the given id exists.
 */
bool AI::ChatStore::contains(const QString& id) const
{
  return indexOf(id) >= 0;
}

/**
 * @brief Returns the id of the last chat marked active, or empty.
 */
QString AI::ChatStore::lastActiveId() const
{
  return m_lastActive;
}

/**
 * @brief Returns the position of a chat in m_chats, or -1.
 */
int AI::ChatStore::indexOf(const QString& id) const
{
  for (int i = 0; i < m_chats.size(); ++i)
    if (m_chats.at(i).id == id)
      return i;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Persists the active-chat pointer.
 */
void AI::ChatStore::setLastActiveId(const QString& id)
{
  if (m_lastActive == id)
    return;

  m_lastActive = id;
  writeIndex();
}

/**
 * @brief Creates a new untitled chat and returns its id.
 */
QString AI::ChatStore::createChat()
{
  Meta meta;
  meta.id        = QUuid::createUuid().toString(QUuid::WithoutBraces);
  meta.createdAt = QDateTime::currentMSecsSinceEpoch();
  meta.updatedAt = meta.createdAt;
  Q_ASSERT(!meta.id.isEmpty());

  m_chats.prepend(meta);
  writeIndex();
  return meta.id;
}

/**
 * @brief Reads one chat's snapshot, or an empty object when missing/corrupt.
 */
QJsonObject AI::ChatStore::loadChat(const QString& id) const
{
  QFile f(chatPath(id));
  if (!f.exists() || !f.open(QIODevice::ReadOnly))
    return {};

  const auto bytes = f.readAll();
  f.close();

  QJsonParseError err;
  const auto doc = QJsonDocument::fromJson(bytes, &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject()) {
    qCWarning(serialStudioAI) << "ChatStore: invalid chat file" << id << err.errorString();
    return {};
  }

  return doc.object();
}

/**
 * @brief Writes a chat's snapshot and refreshes its index metadata.
 */
void AI::ChatStore::saveChat(const QString& id,
                             const QJsonObject& snapshot,
                             const QString& title,
                             int count)
{
  const int idx = indexOf(id);
  if (idx < 0) {
    qCWarning(serialStudioAI) << "ChatStore: saveChat for unknown id" << id;
    return;
  }

  QDir().mkpath(chatsDir());
  QFile f(chatPath(id));
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCWarning(serialStudioAI) << "ChatStore: cannot write chat" << id << f.errorString();
    return;
  }

  f.write(QJsonDocument(snapshot).toJson(QJsonDocument::Compact));
  f.close();

  m_chats[idx].updatedAt = QDateTime::currentMSecsSinceEpoch();
  m_chats[idx].count     = count;
  if (!title.isEmpty() && m_chats[idx].title.isEmpty())
    m_chats[idx].title = title;

  writeIndex();
}

/**
 * @brief Deletes a chat's snapshot file and index entry.
 */
void AI::ChatStore::removeChat(const QString& id)
{
  const int idx = indexOf(id);
  if (idx < 0)
    return;

  QFile::remove(chatPath(id));
  m_chats.removeAt(idx);
  if (m_lastActive == id)
    m_lastActive.clear();

  writeIndex();
}

/**
 * @brief Overrides a chat's display title.
 */
void AI::ChatStore::renameChat(const QString& id, const QString& title)
{
  const int idx = indexOf(id);
  if (idx < 0)
    return;

  m_chats[idx].title = title;
  writeIndex();
}

//--------------------------------------------------------------------------------------------------
// Index serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the index file into m_chats / m_lastActive; tolerant of a missing file.
 */
void AI::ChatStore::readIndex()
{
  m_chats.clear();
  m_lastActive.clear();

  QFile f(indexPath());
  if (!f.exists() || !f.open(QIODevice::ReadOnly))
    return;

  const auto bytes = f.readAll();
  f.close();

  const auto doc = QJsonDocument::fromJson(bytes);
  if (!doc.isObject())
    return;

  const auto root = doc.object();
  m_lastActive    = root.value(QStringLiteral("lastActive")).toString();

  const auto arr = root.value(QStringLiteral("chats")).toArray();
  for (const auto& v : arr) {
    const auto obj     = v.toObject();
    const auto created = obj.value(QStringLiteral("createdAt"));
    const auto updated = obj.value(QStringLiteral("updatedAt"));
    Meta meta;
    meta.id        = obj.value(QStringLiteral("id")).toString();
    meta.title     = obj.value(QStringLiteral("title")).toString();
    meta.createdAt = static_cast<qint64>(SerialStudio::toDouble(created));
    meta.updatedAt = static_cast<qint64>(SerialStudio::toDouble(updated));
    meta.count     = obj.value(QStringLiteral("count")).toInt();
    if (!meta.id.isEmpty())
      m_chats.append(meta);
  }
}

/**
 * @brief Writes m_chats / m_lastActive back to the index file; best effort.
 */
void AI::ChatStore::writeIndex() const
{
  QJsonArray arr;
  for (const auto& meta : m_chats) {
    QJsonObject obj;
    obj[QStringLiteral("id")]        = meta.id;
    obj[QStringLiteral("title")]     = meta.title;
    obj[QStringLiteral("createdAt")] = static_cast<double>(meta.createdAt);
    obj[QStringLiteral("updatedAt")] = static_cast<double>(meta.updatedAt);
    obj[QStringLiteral("count")]     = meta.count;
    arr.append(obj);
  }

  QJsonObject root;
  root[QStringLiteral("schema")]     = 1;
  root[QStringLiteral("lastActive")] = m_lastActive;
  root[QStringLiteral("chats")]      = arr;

  QDir().mkpath(chatsDir());
  QFile f(indexPath());
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCWarning(serialStudioAI) << "ChatStore: cannot write index" << f.errorString();
    return;
  }

  f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
  f.close();
}
