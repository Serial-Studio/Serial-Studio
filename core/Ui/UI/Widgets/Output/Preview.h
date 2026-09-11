/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>
#include <QTimer>
#include <QVariantMap>

#include "Core/DataModel/Frame.h"
#include "UI/Widgets/Output/Base.h"

namespace Widgets {
namespace Output {

/**
 * @brief Drives one real output control against the script under edit and reports the bytes it
 *        produces, without a device anywhere in reach. It builds the same concrete model the
 *        dashboard builds, so the payload shown is the payload that would be sent -- but with a
 *        capture target, so nothing can be sent (spec 0079).
 */
class Preview : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QObject* model
             READ model
             NOTIFY previewChanged)
  Q_PROPERTY(QString widgetId
             READ widgetId
             NOTIFY previewChanged)
  Q_PROPERTY(QVariantMap widget
             READ widget
             NOTIFY previewChanged)
  Q_PROPERTY(bool hasPayload
             READ hasPayload
             NOTIFY payloadChanged)
  Q_PROPERTY(bool payloadStale
             READ payloadStale
             NOTIFY payloadChanged)
  Q_PROPERTY(int payloadSize
             READ payloadSize
             NOTIFY payloadChanged)
  Q_PROPERTY(QString payloadHex
             READ payloadHex
             NOTIFY payloadChanged)
  Q_PROPERTY(QString payloadText
             READ payloadText
             NOTIFY payloadChanged)
  Q_PROPERTY(QString payloadAscii
             READ payloadAscii
             NOTIFY payloadChanged)
  // clang-format on

signals:
  void previewChanged();
  void payloadChanged();

public:
  explicit Preview(QQuickItem* parent = nullptr);

  [[nodiscard]] QObject* model() const;
  [[nodiscard]] QString widgetId() const;
  [[nodiscard]] QVariantMap widget() const;
  [[nodiscard]] bool hasPayload() const noexcept;
  [[nodiscard]] bool payloadStale() const noexcept;
  [[nodiscard]] int payloadSize() const noexcept;
  [[nodiscard]] QString payloadHex() const;
  [[nodiscard]] QString payloadText() const;
  [[nodiscard]] QString payloadAscii() const;

public slots:
  void setConfig(const DataModel::OutputWidget& config);
  void applyScript(const QString& code);
  void setPayloadStale(const bool stale);

private:
  void rebuildModel();
  void onPayload(const QByteArray& payload);

private:
  bool m_stale;
  bool m_rebuilding;
  Base* m_model;
  QTimer m_payloadTimer;
  QByteArray m_payload;
  QVariantMap m_widget;
  DataModel::OutputWidget m_config;
};

}  // namespace Output
}  // namespace Widgets
