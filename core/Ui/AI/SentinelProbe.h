/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>

namespace AI {

/**
 * @brief Context-health probe: sentinel instruction, streaming-safe stripping, reply
 *        validation, and per-provider+model compliance memory.
 */
class SentinelProbe {
public:
  /**
   * @brief Classification of one completed reply against the sentinel contract.
   */
  enum class Outcome : quint8 {
    Healthy = 0,
    Mutated = 1,
    Missing = 2,
    Muted   = 3,
  };

  SentinelProbe();

  [[nodiscard]] static QString sentinelLine();
  [[nodiscard]] static QString instructionBlock();
  [[nodiscard]] static QString stripForDisplay(const QString& text);

  void reset(const QString& complianceKey);
  void ensureKey(const QString& complianceKey);
  void restoreLatch(bool degraded, Outcome failure, const QString& drifted);
  [[nodiscard]] Outcome evaluateReply(const QString& completedText);

  [[nodiscard]] bool degraded() const noexcept;
  [[nodiscard]] Outcome lastFailure() const noexcept;
  [[nodiscard]] QString driftedSegment() const;

private:
  [[nodiscard]] static Outcome classify(const QString& text, QString* drifted);
  [[nodiscard]] static QString sanitizeKey(const QString& key);
  [[nodiscard]] int storedCompliance() const;
  void persistCompliance(bool compliant) const;

private:
  static constexpr int kComplianceWindow = 3;

  QString m_key;
  QString m_drifted;
  Outcome m_lastFailure;
  int m_replyCount;
  bool m_sawSentinel;
  bool m_degraded;
};

}  // namespace AI
