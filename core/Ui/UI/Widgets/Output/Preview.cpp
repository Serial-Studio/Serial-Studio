/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/Preview.h"

#include <QScopedValueRollback>
#include <utility>

#include "Core/SSAssert.h"
#include "Misc/IconEngine.h"
#include "UI/Widgets/Output/Button.h"
#include "UI/Widgets/Output/Slider.h"
#include "UI/Widgets/Output/TextField.h"
#include "UI/Widgets/Output/Toggle.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an unconfigured preview; nothing exists until setConfig() names a widget.
 */
Widgets::Output::Preview::Preview(QQuickItem* parent)
  : QQuickItem(parent), m_stale(false), m_rebuilding(false), m_model(nullptr)
{
  m_payloadTimer.setSingleShot(true);
  m_payloadTimer.setInterval(0);
  connect(&m_payloadTimer, &QTimer::timeout, this, &Preview::payloadChanged);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief The control model the preview QML binds to, or null while unbound.
 */
QObject* Widgets::Output::Preview::model() const
{
  return m_model;
}

/**
 * @brief Identity of the previewed widget; the dashboard controls require one.
 */
QString Widgets::Output::Preview::widgetId() const
{
  return QString::number(m_config.widgetId);
}

/**
 * @brief The widget's visual configuration, in the same shape the dashboard panel publishes.
 */
QVariantMap Widgets::Output::Preview::widget() const
{
  return m_widget;
}

/**
 * @brief Whether the preview has produced a payload since it was configured.
 */
bool Widgets::Output::Preview::hasPayload() const noexcept
{
  return !m_payload.isEmpty();
}

/**
 * @brief Whether the shown payload predates the edit currently in the editor.
 */
bool Widgets::Output::Preview::payloadStale() const noexcept
{
  return m_stale;
}

/**
 * @brief Byte count of the shown payload.
 */
int Widgets::Output::Preview::payloadSize() const noexcept
{
  return static_cast<int>(m_payload.size());
}

/**
 * @brief The shown payload as spaced uppercase hex, matching the sample-value test's rendering.
 */
QString Widgets::Output::Preview::payloadHex() const
{
  return QString::fromLatin1(m_payload.toHex(' ')).toUpper();
}

/**
 * @brief The shown payload decoded as UTF-8, for scripts that transmit text commands.
 */
QString Widgets::Output::Preview::payloadText() const
{
  return QString::fromUtf8(m_payload);
}

/**
 * @brief The payload rendered as printable ASCII, with a dot for every byte that has no printable
 *        form. Shown beside the hex so a text command reads as itself rather than as codepoints.
 */
QString Widgets::Output::Preview::payloadAscii() const
{
  QString out;
  out.reserve(m_payload.size());
  for (const char byte : m_payload) {
    const auto value = static_cast<unsigned char>(byte);
    out.append((value >= 0x20 && value < 0x7F) ? QChar(value) : QChar('.'));
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts the widget configuration to preview. The owner pushes it rather than the preview
 *        reaching for the project: this object knows about controls and payloads, nothing else,
 *        so it holds no project singleton and no connection.
 */
void Widgets::Output::Preview::setConfig(const DataModel::OutputWidget& config)
{
  m_payload.clear();
  m_stale  = false;
  m_config = config;

  rebuildModel();
  Q_EMIT payloadChanged();
}

/**
 * @brief Adopts a new transmit script and rebuilds the control around it. The caller only sends
 *        scripts that passed validation, so a rebuild always yields a model that can transmit.
 */
void Widgets::Output::Preview::applyScript(const QString& code)
{
  setPayloadStale(false);
  if (m_config.transmitFunction == code)
    return;

  m_config.transmitFunction = code;
  rebuildModel();
}

/**
 * @brief Marks the shown payload as predating the current edit, which is what the preview does
 *        while the script does not compile: the control stays usable and its last good output
 *        stays on screen rather than the panel blanking on every half-typed keystroke.
 */
void Widgets::Output::Preview::setPayloadStale(const bool stale)
{
  if (m_stale == stale)
    return;

  m_stale = stale;
  Q_EMIT payloadChanged();
}

//--------------------------------------------------------------------------------------------------
// Model construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the concrete control with a capture target: delivery lands here, the interval is
 *        zero, and nothing resolves a connection. The copy also clears the state source -- a
 *        preview that followed live equipment while you edit a script would be a surprise, and the
 *        editor is about what the script sends, not what the plant is doing (spec 0080).
 */
void Widgets::Output::Preview::rebuildModel()
{
  if (m_rebuilding)
    return;

  QScopedValueRollback<bool> guard(m_rebuilding, true);
  Base* replaced = m_model;

  TransmitTarget target;
  target.minIntervalMs = 0;
  target.surface       = DataModel::TransmitScriptSurface::Judging;
  target.deliver       = [this](const QByteArray& data) {
    onPayload(data);
  };

  DataModel::OutputWidget config = m_config;
  config.stateSource             = DataModel::OutputStateSource::None;

  Base* built = nullptr;
  switch (config.type) {
    case DataModel::OutputWidgetType::Button:
      built = new Button(config, std::move(target), this);
      break;
    case DataModel::OutputWidgetType::Slider:
    case DataModel::OutputWidgetType::Knob:
      built = new Slider(config, std::move(target), this);
      break;
    case DataModel::OutputWidgetType::Toggle:
      built = new Toggle(config, std::move(target), this);
      break;
    case DataModel::OutputWidgetType::TextField:
      built = new TextField(config, std::move(target), this);
      break;
  }

  SS_ASSERT_LOG(built != nullptr);
  m_model = built;
  if (replaced)
    replaced->deleteLater();

  m_widget = QVariantMap();
  m_widget[QStringLiteral("icon")] =
    m_config.icon.isEmpty() ? QString() : Misc::IconEngine::resolveActionIconSource(m_config.icon);
  m_widget[QStringLiteral("title")]        = m_config.title;
  m_widget[QStringLiteral("type")]         = static_cast<int>(m_config.type);
  m_widget[QStringLiteral("minValue")]     = m_config.minValue;
  m_widget[QStringLiteral("maxValue")]     = m_config.maxValue;
  m_widget[QStringLiteral("stepSize")]     = m_config.stepSize;
  m_widget[QStringLiteral("initialValue")] = m_config.initialValue;
  m_widget[QStringLiteral("monoIcon")]     = m_config.monoIcon;
  m_widget[QStringLiteral("checkable")]    = m_config.checkable;
  m_widget[QStringLiteral("color")]        = m_config.color;
  m_widget[QStringLiteral("sizeScale")]    = DataModel::outputSizeScale(m_config.size);
  m_widget[QStringLiteral("onLabel")]      = m_config.onLabel;
  m_widget[QStringLiteral("offLabel")]     = m_config.offLabel;

  Q_EMIT previewChanged();
}

/**
 * @brief Records a payload the control just produced. Republishing is collapsed to one emission
 *        per event-loop turn, so dragging a slider reports the value it settles on rather than
 *        re-evaluating the QML byte view once per mouse step.
 */
void Widgets::Output::Preview::onPayload(const QByteArray& payload)
{
  SS_ASSERT_LOG(!payload.isEmpty());

  m_payload = payload;
  m_stale   = false;
  m_payloadTimer.start();
}
