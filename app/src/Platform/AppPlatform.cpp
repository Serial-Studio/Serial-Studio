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
#  include <avrt.h>
#  include <shlobj.h>
#  include <io.h>
#  include <fcntl.h>
#endif

#ifdef __linux__
#  include <cstdint>
#  include <sys/syscall.h>
#  include <unistd.h>
#endif

#ifdef __APPLE__
#  include <pthread.h>
#  include <pthread/qos.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#  include <sys/mman.h>
#endif
// clang-format on

#include "Platform/AppPlatform.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
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
 * @brief Posts a synthetic Enter so the shell redraws its prompt after the process exits.
 */
static void redrawConsolePromptAtExit()
{
  const HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
  if (in == nullptr || in == INVALID_HANDLE_VALUE)
    return;

  INPUT_RECORD records[2]                     = {};
  records[0].EventType                        = KEY_EVENT;
  records[0].Event.KeyEvent.bKeyDown          = TRUE;
  records[0].Event.KeyEvent.wRepeatCount      = 1;
  records[0].Event.KeyEvent.wVirtualKeyCode   = VK_RETURN;
  records[0].Event.KeyEvent.uChar.UnicodeChar = L'\r';
  records[1]                                  = records[0];
  records[1].Event.KeyEvent.bKeyDown          = FALSE;

  DWORD written = 0;
  (void)WriteConsoleInput(in, records, 2, &written);
}

/**
 * @brief True when a std handle is wired to a pipe or disk file (redirection, not a console).
 */
static bool handleIsRedirected(DWORD stdHandle)
{
  const HANDLE h = GetStdHandle(stdHandle);
  if (h == nullptr || h == INVALID_HANDLE_VALUE)
    return false;

  const DWORD type = GetFileType(h) & ~static_cast<DWORD>(FILE_TYPE_REMOTE);
  return type == FILE_TYPE_DISK || type == FILE_TYPE_PIPE;
}

/**
 * @brief Wires a CRT stream to the inherited std handle so a GUI-subsystem build honors
 * redirection.
 */
static void bindStreamToStdHandle(DWORD stdHandle, FILE* stream)
{
  Q_ASSERT(stream != nullptr);

  const HANDLE src = GetStdHandle(stdHandle);
  if (src == nullptr || src == INVALID_HANDLE_VALUE)
    return;

  // Duplicate first: the CRT fd takes ownership, and closing it must not close the process handle.
  const HANDLE self = GetCurrentProcess();
  HANDLE dup        = nullptr;
  if (!DuplicateHandle(self, src, self, &dup, 0, FALSE, DUPLICATE_SAME_ACCESS))
    return;

  const int fd = _open_osfhandle(reinterpret_cast<intptr_t>(dup), _O_TEXT);
  if (fd == -1) {
    CloseHandle(dup);
    return;
  }

  if (_dup2(fd, _fileno(stream)) == 0) {
    clearerr(stream);
    (void)setvbuf(stream, nullptr, _IONBF, 0);
  }

  (void)_close(fd);
}

/**
 * @brief Attaches the application to the parent console and redirects stdout/stderr.
 */
static void attachToConsole()
{
  // Redirected stdio: a GUI-subsystem CRT leaves streams unbound, so wire them, not CONOUT$.
  if (handleIsRedirected(STD_OUTPUT_HANDLE)) {
    bindStreamToStdHandle(STD_OUTPUT_HANDLE, stdout);
    bindStreamToStdHandle(STD_ERROR_HANDLE, stderr);
    return;
  }

  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    FILE* fp = nullptr;
    (void)freopen_s(&fp, "CONOUT$", "w", stdout);
    (void)freopen_s(&fp, "CONOUT$", "w", stderr);
    // code-verify off  -- raw stdio is the only path before Qt is up
    printf("\n");
    // code-verify on

    // GUI-subsystem apps hand the prompt back before finishing; nudge it on every exit path.
    std::atexit(redrawConsolePromptAtExit);
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
 * @brief Enables SeIncreaseWorkingSetPrivilege (present in every user token) so the minimum
 *        working set can grow enough for VirtualLock to pin the acquisition buffers.
 */
static void enableWorkingSetPrivilege()
{
  HANDLE token = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
    return;

  TOKEN_PRIVILEGES tp         = {};
  tp.PrivilegeCount           = 1;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (LookupPrivilegeValueW(nullptr, SE_INC_WORKING_SET_NAME, &tp.Privileges[0].Luid))
    (void)AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr);

  CloseHandle(token);
}

/**
 * @brief Opts the process out of EcoQoS throttling and prevents idle sleep.
 */
static void enableWindowsPerformanceMode()
{
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

#  if defined(PROCESS_POWER_THROTTLING_CURRENT_VERSION)
  PROCESS_POWER_THROTTLING_STATE state = {};
  state.Version                        = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
  state.ControlMask                    = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
  state.StateMask                      = 0;
  SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &state, sizeof(state));
#  endif

  SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
  enableWorkingSetPrivilege();

  // MMCSS is deferred to registerIngestThreadWithMmcss(): its warning filter must install first
}

// code-verify off  (cold argv setup before QApplication exists; C malloc is intentional)
/**
 * @brief Forces the Qt windows platform plugin to use FreeType font rendering.
 */
static char** adjustArgumentsForFreeType(int& argc, char** argv)
{
  // Skip the override if a -platform is already set (offscreen for --headless/--benchmark, or an
  // explicit choice): Qt takes the last -platform, so appending here would defeat it.
  bool hasPlatform = false;
  for (int i = 0; i < argc; ++i)
    if (std::strcmp(argv[i], "-platform") == 0) {
      hasPlatform = true;
      break;
    }

  const int extra = hasPlatform ? 0 : 2;
  char** newArgv  = static_cast<char**>(malloc(sizeof(char*) * (argc + extra)));
  if (!newArgv)
    return argv;

  for (int i = 0; i < argc; ++i)
    newArgv[i] = _strdup(argv[i]);

  if (!hasPlatform) {
    newArgv[argc]      = _strdup("-platform");
    newArgv[argc + 1]  = _strdup("windows:fontengine=freetype");
    argc              += 2;
  }

  return newArgv;
}

// code-verify on
#endif

//---------------------------------------------------------------------------------------------------
// Linux-only helpers
//---------------------------------------------------------------------------------------------------

#ifdef Q_OS_LINUX

/**
 * @brief Linux analog of the Windows EcoQoS opt-out: raises the scheduler utilization clamp so
 *        EAS places the main thread on a big core. Unprivileged (>= 5.3); failures ignored.
 */
static void enableLinuxPerformanceMode()
{
#  ifdef SYS_sched_setattr
  // Mirrors the kernel's struct sched_attr through the raw syscall (no glibc wrapper exists)
  struct SchedAttr {
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    int32_t sched_nice;
    uint32_t sched_priority;
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
    uint32_t sched_util_min;
    uint32_t sched_util_max;
  };

  constexpr uint64_t kKeepPolicy   = 0x08;  // SCHED_FLAG_KEEP_POLICY
  constexpr uint64_t kKeepParams   = 0x10;  // SCHED_FLAG_KEEP_PARAMS
  constexpr uint64_t kUtilClampMin = 0x20;  // SCHED_FLAG_UTIL_CLAMP_MIN

  SchedAttr attr      = {};
  attr.size           = sizeof(attr);
  attr.sched_flags    = kKeepPolicy | kKeepParams | kUtilClampMin;
  attr.sched_util_min = 1024;  // SCHED_CAPACITY_SCALE: full-capacity placement hint

  // Child threads inherit the clamp, so setting it on main covers the workers too
  (void)syscall(SYS_sched_setattr, 0, &attr, 0);
#  endif
}

#endif

//---------------------------------------------------------------------------------------------------
// macOS-only helpers
//---------------------------------------------------------------------------------------------------

#ifdef Q_OS_MACOS

/**
 * @brief macOS analog of the Windows EcoQoS opt-out: pins the main thread's QoS class to
 *        user-interactive so the scheduler prefers performance cores on Apple silicon.
 */
static void enableMacPerformanceMode()
{
  (void)pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
}

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
#  if defined(Q_OS_LINUX)
  enableLinuxPerformanceMode();
#  elif defined(Q_OS_MACOS)
  enableMacPerformanceMode();
#  endif
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

/**
 * @brief Pins @p len bytes at @p ptr into physical memory (best effort): page-fault stalls on
 *        the frame-scan buffers become latency spikes at rate. Returns true when pinned.
 */
bool lockMemoryResident(const void* ptr, size_t len)
{
  Q_ASSERT(ptr != nullptr);
  Q_ASSERT(len > 0);

  if (!ptr || len == 0) [[unlikely]]
    return false;

#if defined(Q_OS_WIN)
  void* base = const_cast<void*>(ptr);
  if (VirtualLock(base, len))
    return true;

  // Default minimum working set is too small for MiB-scale locks; grow once and retry
  SIZE_T minWs = 0;
  SIZE_T maxWs = 0;
  if (!GetProcessWorkingSetSize(GetCurrentProcess(), &minWs, &maxWs))
    return false;

  const SIZE_T slack  = 2u * 1024u * 1024u;
  const SIZE_T newMin = minWs + len + slack;
  const SIZE_T newMax = (maxWs > newMin + slack) ? maxWs : (newMin + slack);
  if (!SetProcessWorkingSetSize(GetCurrentProcess(), newMin, newMax))
    return false;

  return VirtualLock(base, len) != 0;
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
  // Unprivileged RLIMIT_MEMLOCK (8 MiB on modern systemd) covers the 1 MiB scan buffers
  return mlock(ptr, len) == 0;
#else
  Q_UNUSED(ptr);
  Q_UNUSED(len);
  return false;
#endif
}

/**
 * @brief Registers the calling (ingest/main) thread with MMCSS "Pro Audio"; call only after the
 *        Qt message handler is installed (common-mistakes.md: benign InheritPriority warning).
 */
void registerIngestThreadWithMmcss()
{
#if defined(Q_OS_WIN)
  static bool registered = false;
  if (registered)
    return;

  registered = true;

  using SetCharacteristicsFn = HANDLE(WINAPI*)(LPCWSTR, LPDWORD);
  using SetPriorityFn        = BOOL(WINAPI*)(HANDLE, AVRT_PRIORITY);

  const HMODULE avrt = LoadLibraryW(L"avrt.dll");
  if (!avrt)
    return;

  const auto setCharacteristics =
    reinterpret_cast<SetCharacteristicsFn>(GetProcAddress(avrt, "AvSetMmThreadCharacteristicsW"));
  const auto setPriority =
    reinterpret_cast<SetPriorityFn>(GetProcAddress(avrt, "AvSetMmThreadPriority"));
  if (!setCharacteristics)
    return;

  DWORD taskIndex    = 0;
  const HANDLE mmcss = setCharacteristics(L"Pro Audio", &taskIndex);
  if (mmcss && setPriority)
    (void)setPriority(mmcss, AVRT_PRIORITY_HIGH);
#endif
}

/**
 * @brief Releases a lockMemoryResident() pin so the lock quota is returned before the
 *        underlying allocation is freed or recycled.
 */
void unlockMemoryResident(const void* ptr, size_t len)
{
  Q_ASSERT(ptr != nullptr);
  Q_ASSERT(len > 0);

  if (!ptr || len == 0) [[unlikely]]
    return;

#if defined(Q_OS_WIN)
  (void)VirtualUnlock(const_cast<void*>(ptr), len);
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
  (void)munlock(ptr, len);
#else
  Q_UNUSED(ptr);
  Q_UNUSED(len);
#endif
}

}  // namespace AppPlatform

}  // namespace Platform
