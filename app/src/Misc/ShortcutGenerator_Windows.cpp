/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QtGlobal>

#if defined(BUILD_COMMERCIAL) && defined(Q_OS_WIN)

#  include <objbase.h>
#  include <propkey.h>
#  include <propvarutil.h>
#  include <shlobj.h>
#  include <shobjidl.h>
#  include <windows.h>

#  include <QCryptographicHash>
#  include <QDir>
#  include <QFileInfo>

#  include "Misc/ShortcutGenerator.h"

namespace {

/**
 * @brief RAII helper that pairs CoInitializeEx with a matching CoUninitialize.
 */
class ComScope {
public:
  /**
   * @brief Initialises COM in apartment-threaded mode for this scope.
   */
  ComScope() : m_ok(false)
  {
    const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    m_ok             = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
  }

  /**
   * @brief Releases COM if this scope owned the initialisation.
   */
  ~ComScope()
  {
    if (m_ok)
      CoUninitialize();
  }

  /**
   * @brief Returns whether COM initialisation succeeded.
   */
  [[nodiscard]] bool ok() const { return m_ok; }

private:
  bool m_ok;
};

/**
 * @brief Joins a CLI argument list into a single Windows command-line string.
 */
QString joinForCmdLine(const QStringList& args, std::function<QString(const QString&)> quote)
{
  QStringList quoted;
  quoted.reserve(args.size());
  for (const auto& a : args)
    quoted << quote(a);

  return quoted.join(QLatin1Char(' '));
}

/**
 * @brief Computes the per-shortcut AppUserModelID that pairs with main.cpp.
 *
 * Uses SHA-1(absolute shortcut path) truncated to 16 hex chars so each .lnk
 * pins under its own taskbar group, even when several shortcuts target the
 * same project file. Must stay byte-identical to shortcutIdentityHash() in
 * main.cpp — both sides need to derive the same string from the same path.
 */
QString shortcutAumidFor(const QString& shortcutPath)
{
  const QByteArray digest =
    QCryptographicHash::hash(shortcutPath.toUtf8(), QCryptographicHash::Sha1);
  return QStringLiteral("AlexSpataru.SerialStudio.Shortcut.")
       + QString::fromLatin1(digest.toHex().left(16));
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// Windows shortcut writer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes a Windows .lnk shortcut via the IShellLink/IPersistFile pair.
 */
bool Misc::ShortcutGenerator::writeWindowsLnk(const QString& outputPath,
                                              const QString& target,
                                              const QStringList& args,
                                              const QString& title,
                                              const QString& iconPath,
                                              QString& errorOut)
{
  ComScope com;
  if (!com.ok()) {
    errorOut = tr("Could not initialise COM (required to write .lnk shortcuts).");
    return false;
  }

  IShellLinkW* link = nullptr;
  HRESULT hr        = CoCreateInstance(CLSID_ShellLink,
                                nullptr,
                                CLSCTX_INPROC_SERVER,
                                IID_IShellLinkW,
                                reinterpret_cast<void**>(&link));
  if (FAILED(hr) || link == nullptr) {
    errorOut = tr("CoCreateInstance(IShellLink) failed (HRESULT 0x%1).")
                 .arg(static_cast<quint32>(hr), 8, 16, QLatin1Char('0'));
    return false;
  }

  const auto args_joined   = joinForCmdLine(args, [this](const QString& a) { return quoteArg(a); });
  const auto target_native = QDir::toNativeSeparators(target);
  const auto workdir_native = QDir::toNativeSeparators(QFileInfo(target).absolutePath());
  const auto resolved_icon =
    iconPath.isEmpty() ? target_native : QDir::toNativeSeparators(iconPath);
  const auto output_native = QDir::toNativeSeparators(outputPath);

  link->SetPath(reinterpret_cast<LPCWSTR>(target_native.utf16()));
  link->SetArguments(reinterpret_cast<LPCWSTR>(args_joined.utf16()));
  link->SetWorkingDirectory(reinterpret_cast<LPCWSTR>(workdir_native.utf16()));
  link->SetDescription(reinterpret_cast<LPCWSTR>(title.utf16()));
  link->SetIconLocation(reinterpret_cast<LPCWSTR>(resolved_icon.utf16()), 0);

  // Per-shortcut AppUserModelID — derived from path, recomputed in main.cpp via --shortcut-path
  IPropertyStore* propStore = nullptr;
  if (SUCCEEDED(link->QueryInterface(IID_IPropertyStore, reinterpret_cast<void**>(&propStore)))
      && propStore != nullptr) {
    const QString aumid = shortcutAumidFor(outputPath);
    PROPVARIANT pv;
    if (SUCCEEDED(InitPropVariantFromString(reinterpret_cast<LPCWSTR>(aumid.utf16()), &pv))) {
      propStore->SetValue(PKEY_AppUserModel_ID, pv);
      propStore->Commit();
      PropVariantClear(&pv);
    }
    propStore->Release();
  }

  IPersistFile* persist = nullptr;
  hr = link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&persist));
  if (FAILED(hr) || persist == nullptr) {
    link->Release();
    errorOut = tr("QueryInterface(IPersistFile) failed (HRESULT 0x%1).")
                 .arg(static_cast<quint32>(hr), 8, 16, QLatin1Char('0'));
    return false;
  }

  hr = persist->Save(reinterpret_cast<LPCWSTR>(output_native.utf16()), TRUE);
  persist->Release();
  link->Release();

  if (FAILED(hr)) {
    errorOut = tr("Saving the .lnk file failed (HRESULT 0x%1).")
                 .arg(static_cast<quint32>(hr), 8, 16, QLatin1Char('0'));
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Stubs (only the platform writer is compiled in)
//--------------------------------------------------------------------------------------------------

/**
 * @brief macOS writer is unavailable on Windows builds.
 */
bool Misc::ShortcutGenerator::writeMacCommand(const QString&,
                                              const QString&,
                                              const QStringList&,
                                              const QString&,
                                              const QString&,
                                              QString& errorOut)
{
  errorOut = tr("macOS shortcut writer is not available on this platform.");
  return false;
}

/**
 * @brief Linux writer is unavailable on Windows builds.
 */
bool Misc::ShortcutGenerator::writeLinuxDesktop(const QString&,
                                                const QString&,
                                                const QStringList&,
                                                const QString&,
                                                const QString&,
                                                QString& errorOut)
{
  errorOut = tr("Linux shortcut writer is not available on this platform.");
  return false;
}

#endif  // BUILD_COMMERCIAL && Q_OS_WIN
