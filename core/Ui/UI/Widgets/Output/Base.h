/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QElapsedTimer>
#include <QJSEngine>
#include <QJSValue>
#include <QQuickItem>
#include <QTimer>
#include <QVariant>
#include <QVariantMap>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DataModel/Scripting/JsWatchdog.h"
#include "UI/Widgets/Output/StateBinding.h"
#include "UI/Widgets/Output/TransmitTarget.h"

namespace Widgets {
namespace Output {

/**
 * @brief Base class for all interactive output widgets.
 */
class Base : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int sourceId
             READ sourceId
             CONSTANT)
  Q_PROPERTY(double minValue
             READ minValue
             CONSTANT)
  Q_PROPERTY(double maxValue
             READ maxValue
             CONSTANT)
  Q_PROPERTY(double stepSize
             READ stepSize
             CONSTANT)
  Q_PROPERTY(QString title
             READ title
             CONSTANT)
  Q_PROPERTY(QString units
             READ units
             CONSTANT)
  Q_PROPERTY(bool numeric
             READ numeric
             CONSTANT)
  Q_PROPERTY(int decimals
             READ decimals
             CONSTANT)
  Q_PROPERTY(QString rangeMessage
             READ rangeMessage
             NOTIFY rangeMessageChanged)
  Q_PROPERTY(double admittedValue
             READ admittedValue
             NOTIFY admittedValueChanged)
  Q_PROPERTY(QString onLabel
             READ onLabel
             CONSTANT)
  Q_PROPERTY(QString offLabel
             READ offLabel
             CONSTANT)
  Q_PROPERTY(bool hasTransmitFunction
             READ hasTransmitFunction
             CONSTANT)
  Q_PROPERTY(bool stateBound
             READ stateBound
             CONSTANT)
  Q_PROPERTY(bool stateKnown
             READ stateKnown
             NOTIFY stateChanged)
  Q_PROPERTY(bool statePending
             READ statePending
             NOTIFY stateChanged)
  Q_PROPERTY(bool stateOn
             READ stateOn
             NOTIFY stateChanged)
  Q_PROPERTY(double stateValue
             READ stateValue
             NOTIFY stateChanged)
  Q_PROPERTY(QString stateText
             READ stateText
             NOTIFY stateChanged)
  Q_PROPERTY(bool expectsReply
             READ expectsReply
             CONSTANT)
  Q_PROPERTY(SerialStudio::OutputOutcome outcome
             READ outcome
             NOTIFY stateChanged)
  Q_PROPERTY(QString outcomeText
             READ outcomeText
             NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap automationState
             READ automationState
             NOTIFY stateChanged)
  // clang-format on

signals:
  void stateChanged();
  void rangeMessageChanged();
  void admittedValueChanged();
  void transmitError(const QString& error);

public:
  Base(const DataModel::OutputWidget& config, TransmitTarget target, QQuickItem* parent = nullptr);
  ~Base() override;

  [[nodiscard]] int sourceId() const noexcept;
  [[nodiscard]] double minValue() const noexcept;
  [[nodiscard]] double maxValue() const noexcept;
  [[nodiscard]] double stepSize() const noexcept;
  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] const QString& units() const noexcept;
  [[nodiscard]] bool numeric() const noexcept;
  [[nodiscard]] int decimals() const noexcept;
  [[nodiscard]] const QString& rangeMessage() const noexcept;
  [[nodiscard]] double admittedValue() const noexcept;
  [[nodiscard]] Q_INVOKABLE double stepped(double from, int steps) const;
  [[nodiscard]] Q_INVOKABLE QString formatted(double value) const;
  [[nodiscard]] const QString& onLabel() const noexcept;
  [[nodiscard]] const QString& offLabel() const noexcept;
  [[nodiscard]] bool hasTransmitFunction() const noexcept;
  [[nodiscard]] bool stateBound() const noexcept;
  [[nodiscard]] bool stateKnown() const noexcept;
  [[nodiscard]] bool statePending() const;
  [[nodiscard]] bool stateOn() const noexcept;
  [[nodiscard]] double stateValue() const noexcept;
  [[nodiscard]] QString stateText() const;

  [[nodiscard]] bool expectsReply() const noexcept;
  [[nodiscard]] SerialStudio::OutputOutcome outcome() const;
  [[nodiscard]] QString outcomeText() const;
  [[nodiscard]] QVariantMap automationState() const;
  [[nodiscard]] StateBinding& stateBinding() noexcept;

  [[nodiscard]] static QVariantMap presentation(const DataModel::OutputWidget& config);

public slots:
  void sendValue(const QVariant& value);
  void beginInteraction();
  void endInteraction();
  void observeState(const double value, const QString& text, const qint64 sampleMs);
  void observeTableState(const double value, const QString& text, const quint64 writeClock);
  void forgetState();
  void refreshState();
  void observeReply(const int token, const int code, const QString& message);

protected:
  [[nodiscard]] QByteArray evaluateTransmitFunction(const QVariant& value, const int token);
  virtual void applyStateVerdict(const StateBinding::Verdict& verdict);
  void noteOperatorRequest(const QVariant& value, const int token, const bool bytesSent);
  void sendValueNow(const QVariant& value);

private slots:
  void flushPendingValue();

private:
  void armWindowTimer();
  void refreshOutcome(const qint64 nowMs);
  void submitValue(const QVariant& value, const bool paced);
  [[nodiscard]] bool admit(QVariant& value);
  void setRangeMessage(const QString& message);
  void deliverValue(const QVariant& value);
  [[nodiscard]] bool verdictDiffers(const StateBinding::Verdict& other) const noexcept;

private:
  int m_sourceId;
  double m_minValue;
  double m_maxValue;
  double m_stepSize;
  QString m_title;
  QString m_units;
  QString m_onLabel;
  QString m_offLabel;
  SerialStudio::TextEncoding m_txEncoding;

  bool m_numeric;
  bool m_quantize;
  int m_decimals;
  double m_admittedValue;
  QString m_rangeMessage;

  QJSEngine m_jsEngine;
  QJSValue m_transmitFn;
  bool m_hasFn;

  DataModel::JsWatchdog m_watchdog;
  QElapsedTimer m_rateLimiter;
  bool m_hasPending;
  QTimer m_flushTimer;
  QTimer m_windowTimer;
  QVariant m_pendingValue;
  static constexpr int kTransmitWatchdogMs = 500;
  static constexpr int kMaxPayloadBytes    = 65536;

  bool m_statePendingShown;
  StateBinding::Outcome m_shownOutcome;
  TransmitTarget m_target;
  StateBinding m_stateBinding;
  StateBinding::Verdict m_stateVerdict;
};

}  // namespace Output
}  // namespace Widgets
