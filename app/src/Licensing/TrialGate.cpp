/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "TrialGate.h"

#include <QTimer>

#include "API/CommandRegistry.h"
#include "Core/Licensing/CommercialToken.h"
#include "LemonSqueezy.h"
#include "Misc/Utilities.h"
#include "Trial.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs this object as the process-wide Pro-intent handler and watches the entitlement
 *        funnel. Constructed by the composition root right after the licensing block, GUI builds
 *        only: a headless root never constructs one, so gate refusals stay silent there.
 */
Licensing::TrialGate::TrialGate(Trial& trial, LemonSqueezy& lemonSqueezy, QObject* parent)
  : QObject(parent)
  , m_prompting(false)
  , m_registrationPending(false)
  , m_trial(trial)
  , m_lemonSqueezy(lemonSqueezy)
{
  connect(&m_lemonSqueezy,
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          &Licensing::TrialGate::onEntitlementChanged);
  connect(&m_trial,
          &Licensing::Trial::registrationSettled,
          this,
          &Licensing::TrialGate::onRegistrationSettled);

  Core::License::setProFeatureHandler(
    [this](const QString& featureId, Core::License::ProFeatureRetry retry) {
      handleIntent(featureId, std::move(retry));
    });
}

/**
 * @brief Uninstalls the Pro-intent handler so a stale pointer can never be invoked.
 */
Licensing::TrialGate::~TrialGate()
{
  Core::License::setProFeatureHandler({});
}

//--------------------------------------------------------------------------------------------------
// Intent handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief QML-facing entry for gate sites that live in the UI (freeze fallbacks, file
 *        transmission): same policy as the C++ seam, no retry closure.
 */
void Licensing::TrialGate::requestProFeature(const QString& featureId)
{
  handleIntent(featureId, {});
}

/**
 * @brief The one prompt policy. Remote-origin dispatches keep the silent refusal; while a prompt
 *        is up or a registration is in flight new intents coalesce (latest retry wins, never a
 *        second box); first-run machines get the trial offer, expired ones the activation choice.
 */
void Licensing::TrialGate::handleIntent(const QString& featureId,
                                        Core::License::ProFeatureRetry retry)
{
  Q_UNUSED(featureId);

  if (API::RemoteDispatchScope::active() != nullptr)
    return;

  if (m_prompting || m_registrationPending) {
    if (retry)
      m_pendingRetry = std::move(retry);

    return;
  }

  if (Core::License::activated()) {
    if (retry)
      retry();

    return;
  }

  if (m_trial.firstRun())
    offerTrial(std::move(retry));
  else if (m_trial.trialExpired())
    offerActivation();
}

/**
 * @brief Asks the one trial question (spec 0092 R2). Accepting keeps the newest retry (a
 *        coalesced one wins over this gesture's), short-circuits if activation landed while the
 *        box was up, and arms the pending flag only when a fetch actually started, so a no-op
 *        enableTrial() can never wedge the gate. Blocking box sanctioned: gesture stack only.
 */
void Licensing::TrialGate::offerTrial(Core::License::ProFeatureRetry retry)
{
  m_prompting      = true;
  const int answer = Misc::Utilities::showMessageBox(
    tr("Start your free 14-day Serial Studio Pro trial?"),
    tr("This feature is part of Serial Studio Pro. The trial unlocks every Pro feature "
       "for 14 days, with no account and no payment. An internet connection is required "
       "to register the trial on this machine."),
    QMessageBox::Question,
    tr("Serial Studio Pro Trial"),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::Yes);
  m_prompting = false;

  if (answer != QMessageBox::Yes) {
    m_pendingRetry = {};
    return;
  }

  if (!m_pendingRetry)
    m_pendingRetry = std::move(retry);

  if (Core::License::activated()) {
    onEntitlementChanged();
    return;
  }

  m_trial.enableTrial();
  m_registrationPending = m_trial.busy();
  if (!m_registrationPending)
    m_pendingRetry = {};
}

/**
 * @brief The post-expiry choice (spec 0092 R5): activate an existing license, open the store
 *        page, or dismiss. Never offers the trial again.
 */
void Licensing::TrialGate::offerActivation()
{
  ButtonTextMap labels;
  labels[QMessageBox::Yes] = tr("Activate License");
  labels[QMessageBox::No]  = tr("Get Serial Studio Pro");

  m_prompting      = true;
  const int answer = Misc::Utilities::showMessageBox(
    tr("This feature requires Serial Studio Pro"),
    tr("Your trial has ended. All free features remain fully functional; activate a "
       "license or purchase one to use Pro features again."),
    QMessageBox::Information,
    tr("Serial Studio Pro"),
    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
    QMessageBox::Yes,
    labels);
  m_prompting = false;

  if (answer == QMessageBox::Yes)
    Q_EMIT activationRequested();
  else if (answer == QMessageBox::No)
    m_lemonSqueezy.buy();

  if (!m_registrationPending)
    m_pendingRetry = {};
}

//--------------------------------------------------------------------------------------------------
// Entitlement transitions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resumes the refused gesture once the token lands. Queued single-shot on purpose: every
 *        directly-connected activatedChanged consumer (Core::License publish, device rebuilds,
 *        widget re-resolution) must settle before the gesture replays.
 */
void Licensing::TrialGate::onEntitlementChanged()
{
  if (!Licensing::CommercialToken::current().isValid())
    return;

  m_registrationPending = false;
  if (!m_pendingRetry)
    return;

  QTimer::singleShot(0, this, [this, retry = std::move(m_pendingRetry)] {
    if (retry && Core::License::activated())
      retry();
  });
  m_pendingRetry = {};
}

/**
 * @brief Settles the pending gesture when a registration reply finishes, success or failure:
 *        the signal fires AFTER any token install, so a valid token means onEntitlementChanged
 *        already scheduled the retry; anything else is a failure, the feature stays locked and
 *        nothing is persisted (spec 0092 R9; Trial's reply path reported the error).
 */
void Licensing::TrialGate::onRegistrationSettled()
{
  if (!m_registrationPending)
    return;

  m_registrationPending = false;
  if (!Licensing::CommercialToken::current().isValid())
    m_pendingRetry = {};
}
