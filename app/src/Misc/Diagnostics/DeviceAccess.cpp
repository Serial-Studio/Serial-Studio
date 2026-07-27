/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "Misc/Diagnostics/DeviceAccess.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>

#include "SSAssert.h"

#ifdef Q_OS_UNIX
#  include <grp.h>
#  include <pwd.h>
#  include <unistd.h>

#  include <sys/stat.h>
#  include <sys/types.h>
#  include <vector>
#endif

//--------------------------------------------------------------------------------------------------
// Bounds
//--------------------------------------------------------------------------------------------------

#ifdef Q_OS_UNIX
static constexpr int kMaxGroupMembers        = 4096;
static constexpr int kMaxSupplementaryGroups = 256;
#endif

//--------------------------------------------------------------------------------------------------
// POSIX helpers
//--------------------------------------------------------------------------------------------------

#ifdef Q_OS_UNIX

/**
 * @brief Reports whether the live session carries @p gid, which is what decides between "run the
 *        command" and "log out and back in" when the account is already a member on paper.
 */
[[nodiscard]] static bool sessionHasGid(gid_t gid)
{
  if (::getgid() == gid || ::getegid() == gid)
    return true;

  std::vector<gid_t> groups(kMaxSupplementaryGroups, 0);
  const int count = ::getgroups(kMaxSupplementaryGroups, groups.data());
  if (count <= 0)
    return false;

  SS_ASSERT_LOG(count <= kMaxSupplementaryGroups);

  for (int i = 0; i < count && i < kMaxSupplementaryGroups; ++i)
    if (groups[static_cast<size_t>(i)] == gid)
      return true;

  return false;
}

/**
 * @brief Fills the owning-group name and the two membership facts for @p gid.
 */
static void describeGroup(gid_t gid, Misc::Diagnostics::DeviceAccess& result)
{
  const struct group* entry = ::getgrgid(gid);
  if (entry == nullptr || entry->gr_name == nullptr)
    return;

  result.ownerGroup      = QString::fromLocal8Bit(entry->gr_name);
  result.sessionHasGroup = sessionHasGid(gid);

  const auto account = Misc::Diagnostics::currentAccountName();
  if (account.isEmpty() || entry->gr_mem == nullptr)
    return;

  for (int i = 0; i < kMaxGroupMembers && entry->gr_mem[i] != nullptr; ++i) {
    if (account == QString::fromLocal8Bit(entry->gr_mem[i])) {
      result.accountInGroup = true;
      return;
    }
  }
}

#endif

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the login name of the account this process runs as, empty when unresolvable so
 *        a remedy omits the command rather than printing a wrong account name.
 */
QString Misc::Diagnostics::currentAccountName()
{
#ifdef Q_OS_UNIX
  const struct passwd* entry = ::getpwuid(::getuid());
  if (entry != nullptr && entry->pw_name != nullptr)
    return QString::fromLocal8Bit(entry->pw_name);
#endif

  return QString();
}

/**
 * @brief Probes @p path with stat(2) for the owning group and access(2) for the ground truth of
 *        what this process may do with the node.
 */
Misc::Diagnostics::DeviceAccess Misc::Diagnostics::probeDeviceNode(const QString& path)
{
  DeviceAccess result;
  if (path.isEmpty())
    return result;

#ifdef Q_OS_UNIX
  const QByteArray native = QFile::encodeName(path);
  SS_ASSERT(!native.isEmpty(), return result);

  struct stat node = {};
  if (::stat(native.constData(), &node) != 0)
    return result;

  result.exists      = true;
  result.accessKnown = true;
  result.readable    = ::access(native.constData(), R_OK) == 0;
  result.writable    = ::access(native.constData(), W_OK) == 0;
  describeGroup(node.st_gid, result);
#else
  result.exists = QFileInfo::exists(path);
#endif

  return result;
}
