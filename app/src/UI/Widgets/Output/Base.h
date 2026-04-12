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

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace Widgets {
namespace Output {

/**
 * @class Widgets::Output::Base
 * @brief Base class for all interactive output widgets.
 *
 * Provides JavaScript-based value-to-bytes transformation via a user-defined
 * `transmit(value)` function. Subclasses only differ in QML UI; the C++ base
 * handles JS evaluation, rate limiting, and data transmission.
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
  Q_PROPERTY(QString onLabel
             READ onLabel
             CONSTANT)
  Q_PROPERTY(QString offLabel
             READ offLabel
             CONSTANT)
  Q_PROPERTY(bool hasTransmitFunction
             READ hasTransmitFunction
             CONSTANT)
  // clang-format on

signals:
  void transmitError(const QString& error);

public:
  explicit Base(const DataModel::OutputWidget& config, QQuickItem* parent = nullptr);
  ~Base() override;

  [[nodiscard]] int sourceId() const noexcept;
  [[nodiscard]] double minValue() const noexcept;
  [[nodiscard]] double maxValue() const noexcept;
  [[nodiscard]] double stepSize() const noexcept;
  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] const QString& units() const noexcept;
  [[nodiscard]] const QString& onLabel() const noexcept;
  [[nodiscard]] const QString& offLabel() const noexcept;
  [[nodiscard]] bool hasTransmitFunction() const noexcept;

public slots:
  void sendValue(const QVariant& value);

protected:
  [[nodiscard]] QByteArray evaluateTransmitFunction(const QVariant& value);

public:
  static void installProtocolHelpers(QJSEngine& engine);

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

  QJSEngine m_jsEngine;
  QJSValue m_transmitFn;
  bool m_hasFn;

  QElapsedTimer m_rateLimiter;
  static constexpr int kMinSendIntervalMs = 50;
};

}  // namespace Output
}  // namespace Widgets