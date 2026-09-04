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

#include "AI/MemoryStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>
#include <QVariantMap>

#include "AI/Logging.h"
#include "AI/Redactor.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Path helper & construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the absolute path of the sidecar memory file.
 */
QString AI::MemoryStore::memoryPath()
{
  const auto base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  return base + QStringLiteral("/ai/memory.json");
}

/**
 * @brief Loads the on-disk fact list into memory.
 */
AI::MemoryStore::MemoryStore()
{
  readStore();
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the valid fact categories.
 */
QStringList AI::MemoryStore::categories()
{
  return {
    QStringLiteral("user"),
    QStringLiteral("feedback"),
    QStringLiteral("project"),
    QStringLiteral("reference"),
  };
}

/**
 * @brief Returns true when no facts are stored.
 */
bool AI::MemoryStore::isEmpty() const noexcept
{
  return m_facts.isEmpty();
}

/**
 * @brief Returns fact rows for the QML memory manager in insertion order (newest-created
 *        first); updates do not reorder facts.
 */
QVariantList AI::MemoryStore::list() const
{
  QVariantList out;
  for (const auto& fact : m_facts) {
    QVariantMap row;
    row[QStringLiteral("id")]          = fact.id;
    row[QStringLiteral("category")]    = fact.category;
    row[QStringLiteral("text")]        = fact.text;
    row[QStringLiteral("projectPath")] = fact.projectPath;
    row[QStringLiteral("createdAt")]   = fact.createdAt;
    out.append(row);
  }
  return out;
}

/**
 * @brief Returns the capped plain-text index of applicable facts (installation-wide plus
 *        those scoped to the given project) in insertion order, newest-created first;
 *        empty when nothing applies.
 */
QString AI::MemoryStore::indexBlock(const QString& projectPath) const
{
  QString out;
  for (const auto& fact : m_facts) {
    if (!fact.projectPath.isEmpty() && fact.projectPath != projectPath)
      continue;

    const auto line = QStringLiteral("- [%1] %2\n").arg(fact.category, fact.text);
    if (out.size() + line.size() > kMaxIndexBytes)
      break;

    out += line;
  }
  return out;
}

/**
 * @brief Returns the position of a fact in m_facts, or -1.
 */
int AI::MemoryStore::indexOf(const QString& id) const
{
  for (int i = 0; i < m_facts.size(); ++i)
    if (m_facts.at(i).id == id)
      return i;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scrubs secrets and enforces the length cap; returns false when nothing meaningful
 *        survives redaction (a fully-redacted fact must be refused, not stored).
 */
bool AI::MemoryStore::sanitizeText(QString& text)
{
  text = text.trimmed();
  if (text.isEmpty())
    return false;

  (void)Redactor::scrub(text);

  auto probe = text;
  static const QRegularExpression kRedacted(QStringLiteral("\\[REDACTED:[^\\]]*\\]"));
  probe.remove(kRedacted);
  if (probe.trimmed().isEmpty())
    return false;

  if (text.size() > kMaxFactChars) {
    text.truncate(kMaxFactChars - 3);
    text += QStringLiteral("...");
  }

  return true;
}

/**
 * @brief Adds a consent-confirmed fact; project-category facts bind to the given project
 *        path. Returns false on invalid category or refused (secret-only/empty) text.
 */
bool AI::MemoryStore::addFact(const QString& category,
                              const QString& text,
                              const QString& projectPath)
{
  if (!categories().contains(category))
    return false;

  auto clean = text;
  if (!sanitizeText(clean))
    return false;

  Fact fact;
  fact.id          = QUuid::createUuid().toString(QUuid::WithoutBraces);
  fact.category    = category;
  fact.text        = clean;
  fact.projectPath = category == QStringLiteral("project") ? projectPath : QString();
  fact.createdAt   = QDateTime::currentMSecsSinceEpoch();
  fact.lastUsedAt  = fact.createdAt;

  m_facts.prepend(fact);
  evictOverflow();
  return writeStore();
}

/**
 * @brief Rewrites an existing fact's text through the same sanitize pass.
 */
bool AI::MemoryStore::updateFact(const QString& id, const QString& text)
{
  const int idx = indexOf(id);
  if (idx < 0)
    return false;

  auto clean = text;
  if (!sanitizeText(clean))
    return false;

  m_facts[idx].text       = clean;
  m_facts[idx].lastUsedAt = QDateTime::currentMSecsSinceEpoch();
  return writeStore();
}

/**
 * @brief Deletes a fact; it never appears in a subsequent index build.
 */
void AI::MemoryStore::removeFact(const QString& id)
{
  const int idx = indexOf(id);
  if (idx < 0)
    return;

  m_facts.removeAt(idx);
  (void)writeStore();
}

/**
 * @brief Evicts the facts with the oldest lastUsedAt once the hard cap is exceeded. Only
 *        updateFact refreshes lastUsedAt, so never-updated facts evict oldest-created first.
 */
void AI::MemoryStore::evictOverflow()
{
  while (m_facts.size() > kMaxFacts) {
    int oldest = 0;
    for (int i = 1; i < m_facts.size(); ++i)
      if (m_facts.at(i).lastUsedAt < m_facts.at(oldest).lastUsedAt)
        oldest = i;

    qCDebug(serialStudioAI) << "MemoryStore: evicting" << m_facts.at(oldest).id;
    m_facts.removeAt(oldest);
  }
}

//--------------------------------------------------------------------------------------------------
// Serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the memory file into m_facts; tolerant of a missing or corrupt file. The
 *        file is a user-writable input, so every stored invariant is re-imposed on load:
 *        size cap before reading, schema check, per-fact sanitize/truncate, count cap.
 */
void AI::MemoryStore::readStore()
{
  m_facts.clear();

  QFile f(memoryPath());
  if (!f.exists() || !f.open(QIODevice::ReadOnly))
    return;

  if (f.size() > kMaxFileBytes) {
    qCWarning(serialStudioAI) << "MemoryStore: refusing oversized memory file (" << f.size()
                              << "bytes )";
    f.close();
    return;
  }

  const auto bytes = f.readAll();
  f.close();

  const auto doc = QJsonDocument::fromJson(bytes);
  if (!doc.isObject())
    return;

  if (doc.object().value(QStringLiteral("schema")).toInt() != 1) {
    qCWarning(serialStudioAI) << "MemoryStore: unknown schema, ignoring file";
    return;
  }

  const auto arr = doc.object().value(QStringLiteral("facts")).toArray();
  for (const auto& v : arr) {
    if (m_facts.size() >= kMaxFacts)
      break;

    const auto obj = v.toObject();
    Fact fact;
    fact.id          = obj.value(QStringLiteral("id")).toString();
    fact.category    = obj.value(QStringLiteral("category")).toString();
    fact.text        = obj.value(QStringLiteral("text")).toString();
    fact.projectPath = obj.value(QStringLiteral("projectPath")).toString();
    fact.createdAt =
      static_cast<qint64>(SerialStudio::toDouble(obj.value(QStringLiteral("createdAt"))));
    fact.lastUsedAt =
      static_cast<qint64>(SerialStudio::toDouble(obj.value(QStringLiteral("lastUsedAt"))));
    if (!fact.id.isEmpty() && categories().contains(fact.category) && sanitizeText(fact.text))
      m_facts.append(fact);
  }
}

/**
 * @brief Writes m_facts back to the memory file; returns disk truth so callers can tell
 *        the user when a consented fact failed to persist.
 */
bool AI::MemoryStore::writeStore() const
{
  QJsonArray arr;
  for (const auto& fact : m_facts) {
    QJsonObject obj;
    obj[QStringLiteral("id")]          = fact.id;
    obj[QStringLiteral("category")]    = fact.category;
    obj[QStringLiteral("text")]        = fact.text;
    obj[QStringLiteral("projectPath")] = fact.projectPath;
    obj[QStringLiteral("createdAt")]   = static_cast<double>(fact.createdAt);
    obj[QStringLiteral("lastUsedAt")]  = static_cast<double>(fact.lastUsedAt);
    arr.append(obj);
  }

  QJsonObject root;
  root[QStringLiteral("schema")] = 1;
  root[QStringLiteral("facts")]  = arr;

  QDir().mkpath(QFileInfo(memoryPath()).absolutePath());
  QFile f(memoryPath());
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCWarning(serialStudioAI) << "MemoryStore: cannot write" << f.errorString();
    return false;
  }

  const auto bytes   = QJsonDocument(root).toJson(QJsonDocument::Compact);
  const auto written = f.write(bytes);
  f.close();
  if (written != bytes.size()) {
    qCWarning(serialStudioAI) << "MemoryStore: short write" << written << "of" << bytes.size();
    return false;
  }

  return true;
}
