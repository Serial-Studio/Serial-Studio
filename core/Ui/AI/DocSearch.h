/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

namespace AI {

/**
 * @brief BM25 retrieval over the bundled doc corpus.
 */
class DocSearch {
public:
  /**
   * @brief One search result row.
   */
  struct Hit {
    QString id;      // stable doc id (e.g. "skill/painter.md#3")
    QString source;  // category tag (skill, doc, template, example, script:*)
    QString title;   // human-readable title
    QString body;    // full chunk body
    double score;    // BM25 score
  };

  [[nodiscard]] static DocSearch& instance();
  [[nodiscard]] QList<Hit> search(const QString& query, int k = 5) const;
  [[nodiscard]] int corpusSize() const noexcept;

private:
  DocSearch();
  DocSearch(DocSearch&&)                 = delete;
  DocSearch(const DocSearch&)            = delete;
  DocSearch& operator=(DocSearch&&)      = delete;
  DocSearch& operator=(const DocSearch&) = delete;

  void load();

  struct Doc {
    QString id;
    QString source;
    QString title;
    QString body;
    int length;
    QHash<QString, int> tf;
  };

  bool m_loaded;
  double m_k1;
  double m_b;
  double m_avgdl;
  QHash<QString, double> m_idf;
  QList<Doc> m_docs;
};

}  // namespace AI
