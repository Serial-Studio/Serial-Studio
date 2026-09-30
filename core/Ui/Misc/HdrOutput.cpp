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

#include "Misc/HdrOutput.h"

#include <algorithm>
#include <cmath>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <rhi/qrhi.h>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "Misc/GraphicsBackend.h"

static constexpr qint64 kSampleIntervalMs = 1000;
static constexpr double kSdrReferenceNits = 80.0;

Misc::GraphicsBackend* Misc::HdrOutput::s_backend = nullptr;

/**
 * @brief Binds the GraphicsBackend the QML-constructed instances report to; composition root
 *        only (ModuleManager::registerQmlTypes), same module-static pattern as spec 0040's
 *        mirror flag.
 */
void Misc::HdrOutput::bindBackend(GraphicsBackend* backend) noexcept
{
  s_backend = backend;
}

/**
 * @brief The window's effective emissive boost: the `hdrBoost` property SmartWindow and
 *        SmartDialog derive from intensity and headroom, 1 for a null window, a window
 *        without the property (raw dialogs, SDR swapchains) or a non-finite value. The one
 *        shared reader for PlotCurve and Waterfall; read at scene-graph sync time only.
 */
float Misc::HdrOutput::effectiveBoost(const QQuickWindow* window)
{
  if (window == nullptr)
    return 1.0f;

  const QVariant value = window->property("hdrBoost");
  if (!value.isValid())
    return 1.0f;

  const double boost = SerialStudio::toDouble(value);
  if (!std::isfinite(boost))
    return 1.0f;

  return static_cast<float>(std::max(1.0, boost));
}

/**
 * @brief Leaf constructor; everything interesting happens once a window is bound.
 */
Misc::HdrOutput::HdrOutput(QObject* parent)
  : QObject(parent), m_active(false), m_headroom(1.0), m_sdrWhiteScale(1.0), m_backend(s_backend)
{}

/**
 * @brief Drops this window from the active roster so hdrAnyActive cannot outlive it.
 */
Misc::HdrOutput::~HdrOutput()
{
  if (m_window != nullptr && m_backend != nullptr)
    m_backend->setWindowHdrActive(m_window, false);
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the window this helper watches.
 */
QQuickWindow* Misc::HdrOutput::window() const noexcept
{
  return m_window;
}

/**
 * @brief Returns whether the window's swapchain is actually HDR (not merely requested).
 */
bool Misc::HdrOutput::active() const noexcept
{
  return m_active;
}

/**
 * @brief Returns the display headroom as a multiple of SDR white (1 = no headroom).
 */
double Misc::HdrOutput::headroom() const noexcept
{
  return m_headroom;
}

/**
 * @brief Returns the multiplier the output transform applies to reach the OS SDR white level
 *        (1 on display-referred macOS; sdrWhiteLevel/80 on scene-referred Windows).
 */
double Misc::HdrOutput::sdrWhiteScale() const noexcept
{
  return m_sdrWhiteScale;
}

//--------------------------------------------------------------------------------------------------
// Window binding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the window: applies the swapchain-format request before first expose and hooks
 *        the render-thread sampler. `_qt_sg_hdr_format` is private Qt API (qsgrhisupport.cpp:1544
 *        in 6.11.2, re-verify per Qt bump); a Qt that drops it leaves the swapchain SDR and
 *        `active` false -- degraded, never broken.
 */
void Misc::HdrOutput::setWindow(QQuickWindow* window)
{
  if (m_window == window)
    return;

  if (m_window != nullptr) {
    disconnect(m_syncConnection);
    if (m_backend != nullptr)
      m_backend->setWindowHdrActive(m_window, false);

    publishSample(false, 1.0, 1.0);
  }

  m_window = window;
  Q_EMIT windowChanged();

  if (m_window == nullptr)
    return;

  if (m_backend != nullptr && m_backend->hdrRequested())
    m_window->setProperty("_qt_sg_hdr_format", QByteArrayLiteral("scrgb"));

  m_sampleTimer.invalidate();
  m_syncConnection = connect(m_window,
                             &QQuickWindow::beforeSynchronizing,
                             this,
                             &HdrOutput::sampleSwapChain,
                             Qt::DirectConnection);
}

//--------------------------------------------------------------------------------------------------
// Swapchain sampling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Render-thread sampler (GUI blocked during sync -- the one phase where the swapchain
 *        resource is race-free to query); throttled to ~1 Hz, publishes queued to the GUI
 *        thread so every member write stays GUI-owned. Reads m_window, never sender() -- a
 *        cross-thread Direct connection makes sender() undefined.
 */
void Misc::HdrOutput::sampleSwapChain()
{
  if (m_sampleTimer.isValid() && m_sampleTimer.elapsed() < kSampleIntervalMs)
    return;

  QQuickWindow* window = m_window.data();
  SS_ASSERT(window != nullptr, return);

  auto* iface = window->rendererInterface();
  if (iface == nullptr)
    return;

  m_sampleTimer.restart();

  auto* chain = static_cast<QRhiSwapChain*>(
    iface->getResource(window, QSGRendererInterface::RhiSwapchainResource));
  if (chain == nullptr)
    return;

  const bool active = chain->format() != QRhiSwapChain::SDR;
  double headroom   = 1.0;
  double white      = 1.0;

  if (active) {
    const QRhiSwapChainHdrInfo info = chain->hdrInfo();
    const bool scene_referred       = info.luminanceBehavior == QRhiSwapChainHdrInfo::SceneReferred;
    const double sdr_nits           = qMax(1.0, static_cast<double>(info.sdrWhiteLevel));

    if (scene_referred)
      white = sdr_nits / kSdrReferenceNits;

    if (info.limitsType == QRhiSwapChainHdrInfo::LuminanceInNits)
      headroom = static_cast<double>(info.limits.luminanceInNits.maxLuminance) / sdr_nits;
    else if (scene_referred)
      headroom = static_cast<double>(info.limits.colorComponentValue.maxColorComponentValue)
               * kSdrReferenceNits / sdr_nits;
    else
      headroom = static_cast<double>(info.limits.colorComponentValue.maxColorComponentValue);

    headroom = qMax(1.0, headroom);
  }

  QMetaObject::invokeMethod(
    this,
    [this, active, headroom, white] { publishSample(active, headroom, white); },
    Qt::QueuedConnection);
}

/**
 * @brief GUI-thread publish: updates the members, the notify signal, and the roster in
 *        GraphicsBackend; coalesces unchanged samples to nothing.
 */
void Misc::HdrOutput::publishSample(bool active, double headroom, double sdrWhiteScale)
{
  const bool changed = (m_active != active) || !qFuzzyCompare(m_headroom, headroom)
                    || !qFuzzyCompare(m_sdrWhiteScale, sdrWhiteScale);
  if (!changed)
    return;

  m_active        = active;
  m_headroom      = headroom;
  m_sdrWhiteScale = sdrWhiteScale;

  if (m_window != nullptr && m_backend != nullptr)
    m_backend->setWindowHdrActive(m_window, active);

  Q_EMIT activeChanged();
}
