/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QtGlobal>

#include "SessionContext.h"

/**
 * @file session_context_stub.cpp
 * @brief Link-only stand-in for SessionContext::current() in unit suites (spec 0032/0039).
 *
 * Production instance() forwarders route through SessionContext::current(), so any suite that
 * compiles one of their TUs needs the symbol at link time even when no test ever executes it.
 * Linking the real SessionContext.cpp instead would drag the dtor closure of all eight core
 * modules (the tst_proto_importer trap in CMakeLists.txt). Reaching this stub at runtime means
 * the test walked onto a composition-root path, which is the same named-fatal contract the
 * production accessor enforces before adoption.
 */
SessionContext& SessionContext::current()
{
  qFatal("SessionContext::current() reached from a unit test without a composition root");
}
