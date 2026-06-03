/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/DocSearch.h"

#include <algorithm>
#include <cmath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>

#include "AI/Logging.h"
#include "SerialStudio.h"

/**
 * @brief English stopwords skipped during query tokenization.
 */
static const QSet<QString>& stopwords()
{
  static const QSet<QString> kStop = {
    QStringLiteral("the"),   QStringLiteral("a"),     QStringLiteral("an"),
    QStringLiteral("and"),   QStringLiteral("or"),    QStringLiteral("of"),
    QStringLiteral("to"),    QStringLiteral("for"),   QStringLiteral("in"),
    QStringLiteral("on"),    QStringLiteral("at"),    QStringLiteral("is"),
    QStringLiteral("are"),   QStringLiteral("was"),   QStringLiteral("were"),
    QStringLiteral("be"),    QStringLiteral("been"),  QStringLiteral("being"),
    QStringLiteral("this"),  QStringLiteral("that"),  QStringLiteral("these"),
    QStringLiteral("those"), QStringLiteral("it"),    QStringLiteral("its"),
    QStringLiteral("as"),    QStringLiteral("by"),    QStringLiteral("with"),
    QStringLiteral("from"),  QStringLiteral("into"),  QStringLiteral("you"),
    QStringLiteral("your"),  QStringLiteral("we"),    QStringLiteral("our"),
    QStringLiteral("they"),  QStringLiteral("their"), QStringLiteral("i"),
    QStringLiteral("if"),    QStringLiteral("then"),  QStringLiteral("do"),
    QStringLiteral("does"),  QStringLiteral("did"),   QStringLiteral("doing"),
    QStringLiteral("have"),  QStringLiteral("has"),   QStringLiteral("had"),
    QStringLiteral("but"),   QStringLiteral("not"),   QStringLiteral("no"),
    QStringLiteral("yes"),   QStringLiteral("so"),    QStringLiteral("such"),
    QStringLiteral("than"),  QStringLiteral("when"),  QStringLiteral("where"),
    QStringLiteral("what"),  QStringLiteral("which"), QStringLiteral("who"),
    QStringLiteral("how"),   QStringLiteral("why"),   QStringLiteral("can"),
    QStringLiteral("could"), QStringLiteral("would"), QStringLiteral("should"),
    QStringLiteral("will"),  QStringLiteral("may"),   QStringLiteral("might"),
    QStringLiteral("must"),  QStringLiteral("shall"),
  };
  return kStop;
}

/**
 * @brief Tokenizer mirrors the Python build script: word chars, lowercased.
 */
static QStringList tokenize(const QString& text)
{
  static const QRegularExpression re(QStringLiteral("[A-Za-z][A-Za-z0-9_]+"));
  QStringList out;
  auto it = re.globalMatch(text);
  while (it.hasNext()) {
    const auto m = it.next();
    auto t       = m.captured(0).toLower();
    if (t.size() <= 1)
      continue;

    if (stopwords().contains(t))
      continue;

    out.append(t);
  }
  return out;
}

/**
 * @brief Singleton accessor. Index loads on first call.
 */
AI::DocSearch& AI::DocSearch::instance()
{
  static DocSearch singleton;
  return singleton;
}

/**
 * @brief Constructs the searcher and triggers a one-shot index load.
 */
AI::DocSearch::DocSearch() : m_loaded(false), m_k1(1.5), m_b(0.75), m_avgdl(0.0)
{
  load();
}

/**
 * @brief Reads and decodes the rcc:/ai/search_index.json blob.
 */
void AI::DocSearch::load()
{
  QFile f(QStringLiteral(":/ai/search_index.json"));
  if (!f.open(QIODevice::ReadOnly)) {
    qCWarning(serialStudioAI) << "DocSearch: failed to open search_index.json";
    return;
  }

  QJsonParseError err;
  const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject()) {
    qCWarning(serialStudioAI) << "DocSearch: invalid index JSON:" << err.errorString();
    return;
  }

  const auto root = doc.object();
  m_k1            = SerialStudio::toDouble(root.value(QStringLiteral("k1")), 1.5);
  m_b             = SerialStudio::toDouble(root.value(QStringLiteral("b")), 0.75);
  m_avgdl         = SerialStudio::toDouble(root.value(QStringLiteral("avgdl")), 1.0);

  const auto idf = root.value(QStringLiteral("idf")).toObject();
  for (auto it = idf.constBegin(); it != idf.constEnd(); ++it)
    m_idf.insert(it.key(), SerialStudio::toDouble(it.value()));

  const auto docs = root.value(QStringLiteral("docs")).toArray();
  m_docs.reserve(docs.size());
  for (const auto& v : docs) {
    const auto obj = v.toObject();
    Doc d;
    d.id     = obj.value(QStringLiteral("id")).toString();
    d.source = obj.value(QStringLiteral("source")).toString();
    d.title  = obj.value(QStringLiteral("title")).toString();
    d.body   = obj.value(QStringLiteral("body")).toString();
    d.length = obj.value(QStringLiteral("len")).toInt(0);

    const auto tf = obj.value(QStringLiteral("tf")).toObject();
    for (auto it = tf.constBegin(); it != tf.constEnd(); ++it)
      d.tf.insert(it.key(), it.value().toInt());

    m_docs.append(std::move(d));
  }

  m_loaded = true;
  qCDebug(serialStudioAI) << "DocSearch: loaded" << m_docs.size() << "chunks";
}

/**
 * @brief Returns up to k highest-BM25-scoring chunks.
 */
QList<AI::DocSearch::Hit> AI::DocSearch::search(const QString& query, int k)
{
  if (!m_loaded || m_docs.isEmpty())
    return {};

  const auto qTokens = tokenize(query);
  if (qTokens.isEmpty())
    return {};

  // Precompute query term IDF
  QList<QPair<QString, double>> qIdf;
  qIdf.reserve(qTokens.size());
  for (const auto& t : qTokens) {
    const double w = m_idf.value(t, 0.0);
    if (w > 0.0)
      qIdf.append({t, w});
  }
  if (qIdf.isEmpty())
    return {};

  QList<QPair<int, double>> scored;
  scored.reserve(m_docs.size());
  for (int i = 0; i < m_docs.size(); ++i) {
    const auto& d   = m_docs[i];
    const double dl = d.length > 0 ? double(d.length) : 1.0;
    const double K  = m_k1 * (1.0 - m_b + m_b * dl / std::max(1.0, m_avgdl));

    double s = 0.0;
    for (const auto& [term, w] : qIdf) {
      const int tf = d.tf.value(term, 0);
      if (tf == 0)
        continue;

      s += w * (tf * (m_k1 + 1.0)) / (tf + K);
    }
    if (s > 0.0)
      scored.append({i, s});
  }

  std::sort(
    scored.begin(), scored.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

  const int kMax = std::min<int>(k > 0 ? k : 5, scored.size());
  QList<Hit> out;
  out.reserve(kMax);
  for (int i = 0; i < kMax; ++i) {
    const auto& d = m_docs[scored[i].first];
    Hit h;
    h.id     = d.id;
    h.source = d.source;
    h.title  = d.title;
    h.body   = d.body;
    h.score  = scored[i].second;
    out.append(std::move(h));
  }
  return out;
}

/**
 * @brief Reports the number of indexed chunks (0 if load failed).
 */
int AI::DocSearch::corpusSize() const noexcept
{
  return m_docs.size();
}
