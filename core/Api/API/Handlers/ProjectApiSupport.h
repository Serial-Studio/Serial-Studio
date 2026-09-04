/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace DataModel {
struct Dataset;
struct Group;
}  // namespace DataModel

/**
 * @brief Helpers shared by more than one project command class. Single-caller helpers live in
 *        their caller's translation unit, not here.
 */
namespace API::Handlers::ProjectApiSupport {

/**
 * @brief A resolved dataset and its owning group; both null when resolution failed. The pointers
 *        alias ProjectModel::groups() and are valid only until the next project mutation.
 */
struct DatasetMatch {
  const DataModel::Group* group     = nullptr;
  const DataModel::Dataset* dataset = nullptr;
};

/**
 * @brief A resolved offset/limit window over a list of @c total items; nextOffset is -1 when
 *        the window reaches the end.
 */
struct ListWindow {
  int start      = 0;
  int count      = 0;
  int nextOffset = -1;
};

/**
 * @brief Appends a dataset's compatible DashboardWidget enums to compat (deduped).
 */
void appendDatasetWidgetTypes(const DataModel::Dataset& ds, QJsonArray& compat);

/**
 * @brief Returns the DatasetOption bitflag value of @a ds (1=Plot, 2=FFT, ...).
 */
[[nodiscard]] int datasetOptionsBitflag(const DataModel::Dataset& ds);

/**
 * @brief Builds the canonical dataset response object used by list / get* / snapshot.
 */
[[nodiscard]] QJsonObject buildDatasetObject(const DataModel::Dataset& dataset,
                                             const DataModel::Group& group);

/**
 * @brief Resolves a dataset selector -- a numeric uniqueId or a string alias -- against the live
 *        project. A string is always an alias and a number always a uniqueId. On failure @p error
 *        names the unresolved selector; on success both match pointers are set.
 */
[[nodiscard]] DatasetMatch resolveDatasetSelector(const QJsonValue& selector, QString& error);

/**
 * @brief Resolves offset/limit paging over @p total items; limit <= 0 means "everything after
 *        offset" (the wire back-compat default shared with project.snapshot).
 */
[[nodiscard]] ListWindow applyWindow(int total, int offset, int limit);

/**
 * @brief Adds a window self-identification block so a paged reply cannot be mistaken for the
 *        full set (offset/count/total; matches the applyWindow result that produced it).
 */
void attachWindowInfo(QJsonObject& result, const ListWindow& window, int total);

/**
 * @brief Adds projectEpoch (monotonic mutation counter) to @p result.
 */
void attachProjectEpoch(QJsonObject& result);

/**
 * @brief Snapshot of the project epoch before a mutating handler runs.
 */
[[nodiscard]] qint64 captureProjectEpoch();

/**
 * @brief Append a stale_project warning when the caller's expectedProjectEpoch is stale.
 */
void appendStaleProjectWarning(QJsonObject& result,
                               const QJsonObject& params,
                               qint64 preMutationEpoch);

/**
 * @brief Flags obvious language/syntax mismatches; returns a short warning or empty.
 */
[[nodiscard]] QString detectLanguageMismatch(const QString& code, int language);

}  // namespace API::Handlers::ProjectApiSupport
