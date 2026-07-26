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

#include "SSAssert.h"

#include <QLoggingCategory>

//--------------------------------------------------------------------------------------------------
// Logging category
//--------------------------------------------------------------------------------------------------

Q_LOGGING_CATEGORY(lcSoftAssert, "serialstudio.assert", QtWarningMsg)

//--------------------------------------------------------------------------------------------------
// Reporting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes one soft-assertion failure to the log. Never touches the UI: soft asserts fire on
 *        the database, loader and USB event threads as well as the GUI thread.
 */
SS_COLD SS_NEVER_INLINE void SSAssertDetail::reportSoftAssert(const char* expr,
                                                              const char* file,
                                                              int line,
                                                              const char* func)
{
  qCWarning(lcSoftAssert) << "assertion failed:" << expr << "at" << file << ':' << line << "in"
                          << func;
}

/**
 * @brief Resolves the abort policy once per process: debug builds abort so a broken invariant stays
 *        loud, SS_ASSERT_NONFATAL makes them take the recovery branch so tests can exercise it, and
 *        release builds never abort.
 */
bool SSAssertDetail::softAssertIsFatal()
{
#ifdef QT_NO_DEBUG
  return false;
#else
  static const bool fatal = !qEnvironmentVariableIsSet("SS_ASSERT_NONFATAL");
  return fatal;
#endif
}
