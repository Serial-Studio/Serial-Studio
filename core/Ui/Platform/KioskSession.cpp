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
 *        what every run without the three options keeps.
 */
Platform::KioskSession::KioskSession(QObject* parent)
  : QObject(parent), m_reserveLeft(0), m_sentActive(false), m_confined(false)
{
  connect(&m_control, &KioskControl::linkEstablished, this, &KioskSession::greet);
  connect(&m_control, &KioskControl::raiseRequested, this, &KioskSession::raiseRequested);
  connect(&m_control, &KioskControl::closeRequested, this, &KioskSession::closeRequested);
  connect(&m_control, &KioskControl::screenRequested, this, &KioskSession::moveToScreen);
  connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &KioskSession::publishState);
  connect(qGuiApp, &QGuiApplication::screenAdded, this, &KioskSession::onScreensChanged);
  connect(qGuiApp, &QGuiApplication::screenRemoved, this, &KioskSession::onScreensChanged);
  connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &KioskSession::onScreensChanged);
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

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief A screen came, went or became primary: a confined kiosk takes its rectangle again, which
 *        moves it to the primary screen when its own was unplugged, and reports where it is.
 */
void Platform::KioskSession::onScreensChanged()
{
  if (!m_confined)
    return;

  Q_EMIT placementChanged();
  reportState(false);
}

/**
 * @brief Greets the launcher on every connection, then reports state so it learns whether the
 *        application is active without waiting for a change.
 */
void Platform::KioskSession::greet()
{
  const QString title = m_window ? m_window->title() : QGuiApplication::applicationDisplayName();
  m_control.sendHello(title, currentScreenName());
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
 * @brief Sends the state event; unless @p force is set, only when the screen or the active flag
 *        differs from the last one sent on this run.
 */
void Platform::KioskSession::reportState(bool force)
{
  if (!m_control.isLinked())
    return;

  const QString screen = currentScreenName();
  const bool active    = QGuiApplication::applicationState() == Qt::ApplicationActive;
  if (!force && screen == m_sentScreen && active == m_sentActive)
    return;

  m_sentScreen = screen;
  m_sentActive = active;
  m_control.sendState(screen, active);
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
