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

#include "Core/Runtime.h"

#include <atomic>

#include "Core/SSAssert.h"

static bool s_benchmarkActive = false;
static int s_sessionId        = 0;
static std::atomic<bool> s_sealed{false};
static QString s_sealedMountRoot;
static QString s_sealedProjectFolder;
static QString s_operatorProject;

/**
 * @brief Whether the headless hotpath benchmark is feeding frames; the benchmark is
 *        single-threaded by construction, so a plain bool is enough.
 */
bool Core::Runtime::benchmarkActive() noexcept
{
  return s_benchmarkActive;
}

/**
 * @brief Marks the benchmark active or idle; the benchmark root refreshes the dashboard's stream
 *        flag itself after flipping this.
 */
void Core::Runtime::setBenchmarkActive(const bool active) noexcept
{
  s_benchmarkActive = active;
}

/**
 * @brief The identifier of the session this process runs; 0 is the process-default context.
 */
int Core::Runtime::sessionId() noexcept
{
  return s_sessionId;
}

/**
 * @brief Records the session identifier; the session context sets it when it is constructed.
 */
void Core::Runtime::setSessionId(const int id) noexcept
{
  s_sessionId = id;
}

/**
 * @brief Whether this process opened a deployment package (spec 0096); never reverts to false.
 */
bool Core::Runtime::sealed() noexcept
{
  return s_sealed.load(std::memory_order_acquire);
}

/**
 * @brief The ":/ssdeploy-<id>" root the package's files are served from; empty when unsealed.
 */
const QString& Core::Runtime::sealedMountRoot() noexcept
{
  return s_sealedMountRoot;
}

/**
 * @brief The author's project folder the package was exported from, so absolute asset paths in
 *        script code can be mapped onto the mount; empty when unsealed.
 */
const QString& Core::Runtime::sealedProjectFolder() noexcept
{
  return s_sealedProjectFolder;
}

/**
 * @brief Seals the process for its whole life. There is no unseal: every exit that refuses project
 *        content relies on the answer never changing once it has been read. The first call wins;
 *        the paths are written before the release store, so a reader that sees the seal sees them.
 */
void Core::Runtime::seal(const QString& mountRoot, const QString& projectFolder) noexcept
{
  SS_ASSERT_LOG(!mountRoot.isEmpty());
  if (s_sealed.load(std::memory_order_acquire))
    return;

  s_sealedMountRoot     = mountRoot;
  s_sealedProjectFolder = projectFolder;
  s_sealed.store(true, std::memory_order_release);
}

/**
 * @brief The project a headless run named on its command line, or empty (spec 0098). GUI thread.
 */
const QString& Core::Runtime::operatorProject() noexcept
{
  return s_operatorProject;
}

/**
 * @brief Records the headless run's command-line project, before the CLI opens it.
 */
void Core::Runtime::setOperatorProject(const QString& projectPath) noexcept
{
  s_operatorProject = projectPath;
}
