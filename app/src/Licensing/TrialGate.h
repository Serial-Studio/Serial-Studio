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

#pragma once

#include <QObject>

#include "Core/License.h"

namespace Licensing {
class Trial;
class LemonSqueezy;

/**
 * @brief The owner of the lazy-trial prompt policy (spec 0092), installed by the composition
 *        root as the Core::License Pro-intent handler. First run offers the 14-day trial,
 *        expiry the activate/purchase choice; an accepted trial registers through Trial and
 *        the refused gesture resumes via the retry closure. GUI thread only.
 */
class TrialGate : public QObject {
  Q_OBJECT

signals:
  void activationRequested();

public:
  explicit TrialGate(Trial& trial, LemonSqueezy& lemonSqueezy, QObject* parent = nullptr);
  ~TrialGate() override;
  TrialGate(TrialGate&&)                 = delete;
  TrialGate(const TrialGate&)            = delete;
  TrialGate& operator=(TrialGate&&)      = delete;
  TrialGate& operator=(const TrialGate&) = delete;

public slots:
  void requestProFeature(const QString& featureId);

private slots:
  void onEntitlementChanged();
  void onRegistrationSettled();

private:
  void handleIntent(const QString& featureId, Core::License::ProFeatureRetry retry);
  void offerTrial(Core::License::ProFeatureRetry retry);
  void offerActivation();

private:
  bool m_prompting;
  bool m_registrationPending;

  Trial& m_trial;
  LemonSqueezy& m_lemonSqueezy;
  Core::License::ProFeatureRetry m_pendingRetry;
};
}  // namespace Licensing
