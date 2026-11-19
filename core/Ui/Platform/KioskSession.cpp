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

#include "Platform/KioskSession.h"

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include "Core/SSAssert.h"

/**
 * @brief Delay before kiosk windows take their rectangles a second time.
 */
static constexpr int kSettleMs = 250;

//--------------------------------------------------------------------------------------------------
// Screen lookup
//--------------------------------------------------------------------------------------------------

/**
 * @brief First screen reporting @p name, in system order, or nullptr. Two monitors of one model
 *        report the same name; the first one wins, an order the launcher can read as well.
 */
[[nodiscard]] static QScreen* screenNamed(const QString& name)
{
  const auto screens = QGuiApplication::screens();
  for (QScreen* screen : screens)
    if (screen && screen->name() == name)
      return screen;

  return nullptr;
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an unconfined session with no reserved strip and no control channel, which is
 *        what every run without the three options keeps. Every screen is watched from here on,
 *        since a kiosk window must take its rectangle again when its screen changes.
 */
Platform::KioskSession::KioskSession(QObject* parent)
  : QObject(parent), m_reserveLeft(0), m_sentActive(false), m_confined(false)
{
  m_settle.setSingleShot(true);
  m_settle.setInterval(kSettleMs);
  connect(&m_settle, &QTimer::timeout, this, &KioskSession::placementChanged);

  connect(&m_control, &KioskControl::linkEstablished, this, &KioskSession::greet);
  connect(&m_control, &KioskControl::raiseRequested, this, &KioskSession::raiseRequested);
  connect(&m_control, &KioskControl::closeRequested, this, &KioskSession::closeRequested);
  connect(&m_control, &KioskControl::screenRequested, this, &KioskSession::moveToScreen);
  connect(
    qGuiApp, &QGuiApplication::applicationStateChanged, this, &KioskSession::onApplicationState);
  connect(qGuiApp, &QGuiApplication::focusWindowChanged, this, &KioskSession::publishState);
  connect(qGuiApp, &QGuiApplication::screenAdded, this, &KioskSession::watchScreen);
  connect(qGuiApp, &QGuiApplication::screenAdded, this, &KioskSession::onScreensChanged);
  connect(qGuiApp, &QGuiApplication::screenRemoved, this, &KioskSession::onScreensChanged);
  connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &KioskSession::onScreensChanged);

  const auto screens = QGuiApplication::screens();
  for (QScreen* screen : screens)
    watchScreen(screen);
}

/**
 * @brief Adopts the command-line values before QML loads. An unknown screen name falls back to
 *        the primary screen with one warning, held as an empty name so the primary is resolved
 *        live; the channel opens later, once the window is shown.
 */
void Platform::KioskSession::configure(const QString& screen,
                                       int reserve_left,
                                       const QString& control_server)
{
  SS_ASSERT(reserve_left >= 0, reserve_left = 0);

  m_reserveLeft   = reserve_left;
  m_controlServer = control_server;
  if (screen.isEmpty())
    return;

  m_confined   = true;
  m_screenName = screen;
  if (screenNamed(screen))
    return;

  m_screenName.clear();
  qWarning().noquote() << "[CLI] --kiosk-screen: no screen named" << screen
                       << "; using the primary screen";
}

//--------------------------------------------------------------------------------------------------
// Placement
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true once the kiosk is held on one screen, by --kiosk-screen or a screen command.
 */
bool Platform::KioskSession::confined() const noexcept
{
  return m_confined;
}

/**
 * @brief Returns true when --kiosk-control named a server for this run.
 */
bool Platform::KioskSession::controlled() const noexcept
{
  return !m_controlServer.isEmpty();
}

/**
 * @brief Full geometry of the screen the kiosk is confined to; empty while it is unconfined.
 */
QRectF Platform::KioskSession::screenGeometry() const
{
  const QScreen* screen = confinedScreen();
  return screen ? QRectF(screen->geometry()) : QRectF();
}

/**
 * @brief The rectangle a kiosk window takes on @p screen. With no reserved strip the input comes
 *        back untouched before any screen is consulted, which keeps a plain --kiosk as it was.
 */
QRectF Platform::KioskSession::fillRect(const QRectF& screen) const
{
  if (m_reserveLeft <= 0)
    return screen;

  QList<QRect> geometries;
  int primary_index  = -1;
  const auto screens = QGuiApplication::screens();
  for (const QScreen* candidate : screens) {
    if (candidate == QGuiApplication::primaryScreen())
      primary_index = static_cast<int>(geometries.size());

    geometries.append(candidate->geometry());
  }

  return QRectF(insetRect(geometries, primary_index, screen.toRect(), m_reserveLeft));
}

/**
 * @brief Full geometry, read now, of the screen holding a point, else of the primary screen. By
 *        position and not by name, since two monitors of one model report the same name.
 */
QRectF Platform::KioskSession::screenRectAt(qreal x, qreal y) const
{
  const QPoint point(qRound(x), qRound(y));
  const auto screens = QGuiApplication::screens();
  for (const QScreen* screen : screens)
    if (screen && screen->geometry().contains(point))
      return QRectF(screen->geometry());

  const QScreen* primary = QGuiApplication::primaryScreen();
  return primary ? QRectF(primary->geometry()) : QRectF();
}

/**
 * @brief Index of the leftmost screen: the smallest left coordinate, and among equals the primary
 *        screen, else the first. Returns -1 for an empty list.
 */
int Platform::KioskSession::leftmostIndex(const QList<QRect>& screens, int primary_index)
{
  int best = -1;
  for (int i = 0; i < screens.size(); ++i) {
    const bool further_left = best < 0 || screens.at(i).x() < screens.at(best).x();
    const bool primary_tie =
      best >= 0 && screens.at(i).x() == screens.at(best).x() && i == primary_index;
    if (further_left || primary_tie)
      best = i;
  }

  return best;
}

/**
 * @brief @p target with its left edge moved right by @p reserve_left when it is the leftmost
 *        screen, and unchanged otherwise. The strip is capped at half the screen so a mistyped
 *        value cannot leave a kiosk too narrow to use.
 */
QRect Platform::KioskSession::insetRect(const QList<QRect>& screens,
                                        int primary_index,
                                        const QRect& target,
                                        int reserve_left)
{
  if (reserve_left <= 0)
    return target;

  const int leftmost = leftmostIndex(screens, primary_index);
  if (leftmost < 0 || screens.at(leftmost).topLeft() != target.topLeft())
    return target;

  return target.adjusted(qMin(reserve_left, target.width() / 2), 0, 0, 0);
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Tells every kiosk window to take its rectangle again, now and once more shortly after:
 *        macOS holds a window below the menu bar until the kiosk lock has hidden it, and reports
 *        the corrected geometry late.
 */
void Platform::KioskSession::settle()
{
  Q_EMIT placementChanged();
  m_settle.start();
}

/**
 * @brief Gives @p window the part of @p screen a kiosk window may cover, as one geometry change
 *        so the platform cannot constrain a half-moved window.
 */
void Platform::KioskSession::place(QObject* window, const QRectF& screen)
{
  auto* target = qobject_cast<QWindow*>(window);
  if (!target || screen.isEmpty())
    return;

  target->setGeometry(fillRect(screen).toRect());
}

/**
 * @brief Adopts the shown main window: its screen feeds the state events, and showing it is the
 *        moment the control channel may open.
 */
void Platform::KioskSession::setMainWindow(QObject* window)
{
  auto* main_window = qobject_cast<QWindow*>(window);
  if (!main_window || main_window == m_window)
    return;

  m_window = main_window;
  connect(main_window, &QWindow::screenChanged, this, &KioskSession::publishState);
  m_control.start(m_controlServer);
}

/**
 * @brief Gives an external dashboard a control channel of its own under @p id, so the launcher
 *        lists it beside the main window and can raise, move and close it alone. Nothing
 *        happens without a launcher: an unmanaged kiosk has nobody to switch between windows.
 */
void Platform::KioskSession::addWindow(QObject* window, const QString& id)
{
  auto* target = qobject_cast<QWindow*>(window);
  if (!target || id.isEmpty() || !controlled() || managedIndex(target) >= 0)
    return;

  auto* control = new KioskControl(this);
  control->setIdentity(id);
  connect(control, &KioskControl::linkEstablished, target, [this, target] { greetWindow(target); });
  connect(control, &KioskControl::raiseRequested, target, [this, target] {
    Q_EMIT windowRaiseRequested(target);
  });
  connect(control, &KioskControl::closeRequested, target, [this, target] {
    Q_EMIT windowCloseRequested(target);
  });
  connect(control,
          &KioskControl::screenRequested,
          target,
          [this, target](const QString& name, int reserve_left) {
            moveWindowToScreen(target, name, reserve_left);
          });
  connect(target, &QWindow::screenChanged, this, &KioskSession::publishState);
  connect(target, &QObject::destroyed, this, &KioskSession::removeWindow);

  m_managed.append(
    Managed{target, target, control, QString(), QString(), QString(), QString(), false});
  control->start(m_controlServer);
}

/**
 * @brief Names what @p window shows now, the main window or a managed one; QML passes the active
 *        workspace, which is what an operator calls the window. The launcher hears it in the
 *        next state event. An empty title falls back to the window's own, and a window that
 *        is not managed counts as the main one until that is adopted.
 */
void Platform::KioskSession::setWindowTitle(QObject* window, const QString& title)
{
  const qsizetype index = managedIndex(window);
  if (index >= 0)
    m_managed[index].title = title;
  else if (window && (!m_window || window == m_window))
    m_title = title;
  else
    return;

  reportState(false);
}

/**
 * @brief Ends the channel of a managed window, which is how the launcher learns it is gone.
 *        Reached from the window's close and from its destruction, so it compares addresses
 *        and never touches @p window; the client is deleted later because a close command
 *        arrives through that very client.
 */
void Platform::KioskSession::removeWindow(QObject* window)
{
  const qsizetype index = managedIndex(window);
  if (index < 0)
    return;

  KioskControl* control = m_managed.at(index).control;
  m_managed.removeAt(index);
  control->disconnect();
  control->deleteLater();
  reportState(false);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief A screen came, went, became primary or changed its geometry: every kiosk window takes
 *        its rectangle again, which moves a confined kiosk to the primary screen when its own was
 *        unplugged, and the launcher learns where the kiosk is.
 */
void Platform::KioskSession::onScreensChanged()
{
  settle();
  reportState(false);
}

/**
 * @brief Follows @p screen for as long as it lives; the menu bar and the Dock leaving it change
 *        its available geometry, which is one sign that the kiosk lock took effect.
 */
void Platform::KioskSession::watchScreen(QScreen* screen)
{
  if (!screen)
    return;

  connect(
    screen, &QScreen::geometryChanged, this, &KioskSession::onScreensChanged, Qt::UniqueConnection);
  connect(
    screen, &QScreen::availableGeometryChanged, this, &KioskSession::settle, Qt::UniqueConnection);
}

/**
 * @brief Reports the active flag to the launcher; on becoming active the kiosk lock applies
 *        again, so the windows take their rectangles once it has.
 */
void Platform::KioskSession::onApplicationState(Qt::ApplicationState state)
{
  reportState(false);
  if (state == Qt::ApplicationActive)
    settle();
}

/**
 * @brief Greets the launcher on every connection, then reports state so it learns whether the
 *        application is active without waiting for a change.
 */
void Platform::KioskSession::greet()
{
  m_control.sendHello(mainTitle(), currentScreenName());
  reportState(true);
}

/**
 * @brief Sends a state event when the screen or the active state differs from the last one sent.
 */
void Platform::KioskSession::publishState()
{
  reportState(false);
}

/**
 * @brief Answers the screen command: confines the kiosk to @p name and adopts @p reserve_left
 *        unless it is negative (absent). An unknown name changes and sends nothing; a known one
 *        always answers with a state event, also when the screen stayed the same.
 */
void Platform::KioskSession::moveToScreen(const QString& name, int reserve_left)
{
  if (!screenNamed(name))
    return;

  m_confined   = true;
  m_screenName = name;
  if (reserve_left >= 0)
    m_reserveLeft = reserve_left;

  Q_EMIT placementChanged();
  reportState(true);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends the state events, the main window's and every managed window's; unless @p force
 *        is set, each only when its screen or active flag differs from the last one sent. The
 *        main window is the active one whenever the application is and no managed window holds
 *        the focus, which with no managed window is the application's own state, as before.
 */
void Platform::KioskSession::reportState(bool force)
{
  for (Managed& managed : m_managed)
    reportManaged(managed, false);

  if (!m_control.isLinked())
    return;

  const QString title  = mainTitle();
  const QString screen = currentScreenName();
  const bool active =
    QGuiApplication::applicationState() == Qt::ApplicationActive && !managedHasFocus();
  if (!force && screen == m_sentScreen && active == m_sentActive && title == m_sentTitle)
    return;

  m_sentTitle  = title;
  m_sentScreen = screen;
  m_sentActive = active;
  m_control.sendState(screen, active, title);
}

/**
 * @brief What the launcher calls the main window: the title QML gave, else the window's own.
 */
QString Platform::KioskSession::mainTitle() const
{
  if (!m_title.isEmpty())
    return m_title;

  return m_window ? m_window->title() : QGuiApplication::applicationDisplayName();
}

/**
 * @brief Sends one managed window's state event: the screen it was last sent to while that
 *        screen lives, else the one it is on, and whether it holds the focus.
 */
void Platform::KioskSession::reportManaged(Managed& managed, bool force)
{
  if (!managed.window || !managed.control->isLinked())
    return;

  const QScreen* sent   = managed.screenName.isEmpty() ? nullptr : screenNamed(managed.screenName);
  const QScreen* screen = sent ? sent : managed.window->screen();
  const QString name    = screen ? screen->name() : QString();
  const bool active     = QGuiApplication::applicationState() == Qt::ApplicationActive
                       && QGuiApplication::focusWindow() == managed.window;
  const QString title   = managed.title.isEmpty() ? managed.window->title() : managed.title;
  const bool same_place = name == managed.sentScreen && active == managed.sentActive;
  if (!force && same_place && title == managed.sentTitle)
    return;

  managed.sentTitle  = title;
  managed.sentScreen = name;
  managed.sentActive = active;
  managed.control->sendState(name, active, title);
}

/**
 * @brief Greets the launcher for a managed window on every connection of its channel, then
 *        reports its state.
 */
void Platform::KioskSession::greetWindow(QObject* window)
{
  const qsizetype index = managedIndex(window);
  if (index < 0 || !m_managed.at(index).window)
    return;

  Managed& managed      = m_managed[index];
  const QScreen* screen = managed.window->screen();
  const QString title   = managed.title.isEmpty() ? managed.window->title() : managed.title;
  managed.control->sendHello(title, screen ? screen->name() : QString());
  reportManaged(managed, true);
}

/**
 * @brief Answers a managed window's screen command: that window alone goes to @p name, and the
 *        main window stays where it is. The reserved width is one value for the whole kiosk, so
 *        a present @p reserve_left makes every kiosk window take its rectangle again. An unknown
 *        name changes and sends nothing.
 */
void Platform::KioskSession::moveWindowToScreen(QObject* window,
                                                const QString& name,
                                                int reserve_left)
{
  const QScreen* screen = screenNamed(name);
  const qsizetype index = managedIndex(window);
  if (!screen || index < 0)
    return;

  m_managed[index].screenName = name;
  if (reserve_left >= 0 && reserve_left != m_reserveLeft) {
    m_reserveLeft = reserve_left;
    Q_EMIT placementChanged();
  }

  Q_EMIT windowScreenRequested(window, QRectF(screen->geometry()));
  reportManaged(m_managed[index], true);
}

/**
 * @brief Returns true while a managed window is the focus window.
 */
bool Platform::KioskSession::managedHasFocus() const
{
  const QWindow* focus = QGuiApplication::focusWindow();
  return focus && managedIndex(focus) >= 0;
}

/**
 * @brief Index of the managed entry for @p window, by address, or -1.
 */
qsizetype Platform::KioskSession::managedIndex(const QObject* window) const
{
  for (qsizetype i = 0; i < m_managed.size(); ++i)
    if (m_managed.at(i).key == window)
      return i;

  return -1;
}

/**
 * @brief The live screen the kiosk is confined to, or the primary screen once that one is gone;
 *        nullptr while unconfined.
 */
QScreen* Platform::KioskSession::confinedScreen() const
{
  if (!m_confined)
    return nullptr;

  QScreen* screen = m_screenName.isEmpty() ? nullptr : screenNamed(m_screenName);
  return screen ? screen : QGuiApplication::primaryScreen();
}

/**
 * @brief Name reported to the launcher: the screen the kiosk is confined to as resolved now, else
 *        the screen the main window is on. A platform may report a window's move late, so the
 *        confined case never asks the window.
 */
QString Platform::KioskSession::currentScreenName() const
{
  const QScreen* confined = confinedScreen();
  const QScreen* screen   = confined ? confined : (m_window ? m_window->screen() : nullptr);
  return screen ? screen->name() : QString();
}
