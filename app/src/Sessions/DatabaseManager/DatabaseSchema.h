/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

class QSqlQuery;

/**
 * @brief The historian's SQL schema: every CREATE and every additive migration a session archive
 *        needs, in one place because three writers (the manager, the export worker and the
 *        verifier child) create the same tables and must never disagree about them.
 *
 *        Free functions rather than a class: the schema holds no state, and DatabaseManager keeps
 *        the two public entry points its callers already use, forwarding here.
 */
namespace Sessions::DatabaseSchema {

void createAll(QSqlQuery& q, int userVersion);
void createTagTables(QSqlQuery& q);
void createBlockTable(QSqlQuery& q);
void createSampleTables(QSqlQuery& q);
void createStreamTables(QSqlQuery& q);
void createSessionTables(QSqlQuery& q);
void createVerifications(QSqlQuery& q);
void createProjectMetadata(QSqlQuery& q);
void migrateColumnsTable(QSqlQuery& q);
void migrateSessionsTable(QSqlQuery& q);

}  // namespace Sessions::DatabaseSchema

#endif
