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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

// clang-format off
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <shlobj.h>
#endif
// clang-format on

#include "Platform/AppPlatform.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFileOpenEvent>
#include <QWheelEvent>

#ifdef SERIAL_STUDIO_WITH_WEBENGINE
#  include <QtWebEngineQuick>
#endif

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

namespace Platform {

//---------------------------------------------------------------------------------------------------
// FileOpenEventFilter
//---------------------------------------------------------------------------------------------------

/**
 * @brief Intercepts QFileOpenEvent and loads the referenced .ssproj into the project model.
 */
bool FileOpenEventFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::FileOpen) {
    auto* fileEvent    = static_cast<QFileOpenEvent*>(event);
    const QString path = fileEvent->file();
    if (path.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive)) {
      AppState::instance().setOperationMode(SerialStudio::ProjectFile);
      DataModel::ProjectModel::instance().openJsonFile(path);
      return true;
    }
  }

  return QObject::eventFilter(obj, event);
}

//---------------------------------------------------------------------------------------------------
// TrackpadScrollFilter
//---------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the trackpad scroll filter with reentry guard cleared.
 */
TrackpadScrollFilter::TrackpadScrollFilter(QObject* parent) : QObject(parent), m_reentry(false) {}

/**
 * @brief Multiplies trackpad pixelDelta so QML Flickables scroll usably; angleDelta untouched.
 */
bool TrackpadScrollFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() != QEvent::Wheel || m_reentry)
    return QObject::eventFilter(obj, event);

  auto* wheel = static_cast<QWheelEvent*>(event);

  // Amplify pixel-precise trackpad scrolls only; mouse wheels use angleDelta.
  const QPoint pixel = wheel->pixelDelta();
  if (pixel.isNull())
    return QObject::eventFilter(obj, event);

  // Skip synthesized gesture events.
  if (wheel->source() != Qt::MouseEventNotSynthesized)
    return QObject::eventFilter(obj, event);

  // Scale pixelDelta only; angleDelta-driven zoom widgets keep their tuned factors.
  constexpr qreal kScale = 1.0;
  const QPoint scaledPixel(static_cast<int>(pixel.x() * kScale),
                           static_cast<int>(pixel.y() * kScale));

  QWheelEvent amplified(wheel->position(),
                        wheel->globalPosition(),
                        scaledPixel,
                        wheel->angleDelta(),
                        wheel->buttons(),
                        wheel->modifiers(),
                        wheel->phase(),
                        wheel->inverted(),
                        wheel->source(),
                        wheel->pointingDevice());

  m_reentry            = true;
  const bool delivered = QCoreApplication::sendEvent(obj, &amplified);
  m_reentry            = false;

  event->setAccepted(amplified.isAccepted());
  return delivered && amplified.isAccepted();
}

//---------------------------------------------------------------------------------------------------
// Windows-only helpers
//---------------------------------------------------------------------------------------------------

#ifdef Q_OS_WIN

/**
 * @brief Attaches the application to the parent console and redirects stdout/stderr.
 */
static void attachToConsole()
{
  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    FILE* fp = nullptr;
    (void)freopen_s(&fp, "CONOUT$", "w", stdout);
    (void)freopen_s(&fp, "CONOUT$", "w", stderr);
    // code-verify off  -- raw stdio is the only path before Qt is up
    printf("\n");
    // code-verify on
  }
}

/**
 * @brief Pins the process to a stable Windows AppUserModelID.
 */
static void setWindowsAppUserModelId(const QString& shortcutPath)
{
  QString aumid = QStringLiteral("AlexSpataru.SerialStudio");
  if (!shortcutPath.isEmpty())
    aumid += QStringLiteral(".Shortcut.") + AppPlatform::shortcutIdentityHash(shortcutPath);

  SetCurrentProcessExplicitAppUserModelID(reinterpret_cast<LPCWSTR>(aumid.utf16()));
}

/**
 * @brief Opts the process out of EcoQoS throttling and prevents idle sleep.
 */
static void enableWindowsPerformanceMode()
{
#  if defined(PROCESS_POWER_THROTTLING_CURRENT_VERSION)
  PROCESS_POWER_THROTTLING_STATE state = {};
  state.Version                        = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
  state.ControlMask                    = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
  state.StateMask                      = 0;
  SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &state, sizeof(state));
#  endif

  SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
}

// code-verify off  (cold argv setup before QApplication exists; C malloc is intentional)
/**
 * @brief Forces the Qt windows platform plugin to use FreeType font rendering.
 */
static char** adjustArgumentsForFreeType(int& argc, char** argv)
{
  const char* platformArgument = "-platform";
  const char* platformOption   = "windows:fontengine=freetype";

  char** newArgv = static_cast<char**>(malloc(sizeof(char*) * (argc + 2)));
  if (!newArgv)
    return argv;

  for (int i = 0; i < argc; ++i)
    newArgv[i] = _strdup(argv[i]);

  newArgv[argc]     = _strdup(platformArgument);
  newArgv[argc + 1] = _strdup(platformOption);

  argc += 2;
  return newArgv;
}

// code-verify on
#endif

namespace AppPlatform {

/**
 * @brief Computes a stable 16-char SHA-1 prefix identifying a shortcut path.
 */
QString shortcutIdentityHash(const QString& shortcutPath)
{
  if (shortcutPath.isEmpty())
    return QString();

  const QByteArray digest =
    QCryptographicHash::hash(shortcutPath.toUtf8(), QCryptographicHash::Sha1);
  return QString::fromLatin1(digest.toHex().left(16));
}

// code-verify off  (cold argv setup before QApplication exists; C malloc is intentional)
/**
 * @brief Injects "-platform <platform>" into argv before Qt parses it.
 */
char** injectPlatformArg(int& argc, char** argv, const char* platform)
{
  char** newArgv = static_cast<char**>(malloc(sizeof(char*) * (argc + 3)));
  if (!newArgv)
    return argv;

  newArgv[0] = argv[0];
  newArgv[1] = const_cast<char*>("-platform");
  newArgv[2] = const_cast<char*>(platform);

  for (int i = 1; i < argc; ++i)
    newArgv[i + 2] = argv[i];

  newArgv[argc + 2]  = nullptr;
  argc              += 2;
  return newArgv;
}

// code-verify on

/**
 * @brief Performs platform fixups, fractional scaling, and WebEngine init.
 */
void prepareEnvironment(int& argc, char**& argv, const QString& shortcutPath)
{
#if defined(Q_OS_WIN)
  attachToConsole();
  setWindowsAppUserModelId(shortcutPath);
  enableWindowsPerformanceMode();
  argv = adjustArgumentsForFreeType(argc, argv);
#else
  Q_UNUSED(argc);
  Q_UNUSED(argv);
  Q_UNUSED(shortcutPath);
#endif

  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);

#ifdef SERIAL_STUDIO_WITH_WEBENGINE
#  if defined(Q_OS_LINUX)
  // QtWebEngineProcess sandbox can't read AppImage FUSE squashfs; disable before initialize()
  if (!qEnvironmentVariableIsSet("QTWEBENGINE_DISABLE_SANDBOX")
      && !qEnvironmentVariableIsSet("QTWEBENGINE_CHROMIUM_FLAGS"))
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-namespace-sandbox");
#  endif
  QtWebEngineQuick::initialize();
#endif
}

/**
 * @brief Registers .ssproj with this executable in the Windows registry (HKCU).
 */
void registerFileAssociation()
{
#ifdef Q_OS_WIN
  const QString exePath = QCoreApplication::applicationFilePath().replace('/', '\\');
  const QString progId  = QStringLiteral("SerialStudio.ssproj");
  const QString openCmd =
    QStringLiteral("\"%1\" --project \"%2\"").arg(exePath, QStringLiteral("%1"));

  auto writeKey = [](const QString& path, const QString& value) {
    HKEY hKey   = nullptr;
    auto status = RegCreateKeyExW(HKEY_CURRENT_USER,
                                  reinterpret_cast<LPCWSTR>(path.utf16()),
                                  0,
                                  nullptr,
                                  0,
                                  KEY_WRITE,
                                  nullptr,
                                  &hKey,
                                  nullptr);

    if (status == ERROR_SUCCESS) {
      RegSetValueExW(hKey,
                     nullptr,
                     0,
                     REG_SZ,
                     reinterpret_cast<const BYTE*>(value.utf16()),
                     static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
      RegCloseKey(hKey);
    }
  };

  const QString progIdPath = QStringLiteral("Software\\Classes\\%1").arg(progId);
  writeKey(progIdPath, QStringLiteral("Serial Studio Project"));
  writeKey(progIdPath + QStringLiteral("\\DefaultIcon"), QStringLiteral("\"%1\",0").arg(exePath));
  writeKey(progIdPath + QStringLiteral("\\shell\\open\\command"), openCmd);
  writeKey(QStringLiteral("Software\\Classes\\.ssproj"), progId);

  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
#endif
}

// code-verify off  (mirrors malloc above; pair with adjustArgumentsForFreeType / injectPlatformArg)
/**
 * @brief Frees the heap-duplicated argv produced by adjustArgumentsForFreeType.
 */
void releaseAdjustedArgv(int argc, char** argv)
{
#ifdef Q_OS_WIN
  for (int i = 0; i < argc; ++i)
    free(argv[i]);

  free(argv);
#else
  Q_UNUSED(argc);
  Q_UNUSED(argv);
#endif
}

// code-verify on

}  // namespace AppPlatform

}  // namespace Platform
