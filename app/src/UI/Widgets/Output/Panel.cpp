/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/Panel.h"

#include "Misc/IconEngine.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/Output/Button.h"
#include "UI/Widgets/Output/PanelLayout.h"
#include "UI/Widgets/Output/Slider.h"
#include "UI/Widgets/Output/TextField.h"
#include "UI/Widgets/Output/Toggle.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the panel model from the output group at the given index.
 */
Widgets::Output::Panel::Panel(int index, QQuickItem* parent) : QQuickItem(parent)
{
  // Validate widget index and retrieve group configuration
  if (!VALIDATE_WIDGET(SerialStudio::DashboardOutputPanel, index))
    return;

  const auto& group = GET_GROUP(SerialStudio::DashboardOutputPanel, index);

  // Build QML-visible property maps for each output widget
  m_outputWidgets = group.outputWidgets;

  for (const auto& ow : m_outputWidgets) {
    QVariantMap map;
    map[QStringLiteral("icon")] =
      ow.icon.isEmpty() ? QString() : Misc::IconEngine::resolveActionIconSource(ow.icon);
    map[QStringLiteral("title")]        = ow.title;
    map[QStringLiteral("type")]         = static_cast<int>(ow.type);
    map[QStringLiteral("minValue")]     = ow.minValue;
    map[QStringLiteral("maxValue")]     = ow.maxValue;
    map[QStringLiteral("stepSize")]     = ow.stepSize;
    map[QStringLiteral("initialValue")] = ow.initialValue;
    map[QStringLiteral("monoIcon")]     = ow.monoIcon;
    m_widgets.append(map);

    Base* model = nullptr;
    switch (ow.type) {
      case DataModel::OutputWidgetType::Button:
        model = new Button(ow, this);
        break;
      case DataModel::OutputWidgetType::Slider:
      case DataModel::OutputWidgetType::Knob:
        model = new Slider(ow, this);
        break;
      case DataModel::OutputWidgetType::Toggle:
        model = new Toggle(ow, this);
        break;
      case DataModel::OutputWidgetType::TextField:
        model = new TextField(ow, this);
        break;
      default:
        break;
    }

    m_models.append(QVariant::fromValue(model));
  }
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of output widgets.
 */
int Widgets::Output::Panel::count() const noexcept
{
  return m_widgets.size();
}

/**
 * @brief Returns the list of output widget data as QVariantMaps.
 */
QVariantList Widgets::Output::Panel::widgets() const
{
  return m_widgets;
}

/**
 * @brief Returns the list of output widget model objects for transmission.
 */
QVariantList Widgets::Output::Panel::models() const
{
  return m_models;
}

/**
 * @brief Returns the computed geometry as a list of {x, y, w, h} maps.
 */
QVariantList Widgets::Output::Panel::geometry() const
{
  return m_geometry;
}

//--------------------------------------------------------------------------------------------------
// Layout computation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Recomputes layout geometry using the PanelLayout engine.
 * @param width Available width in pixels.
 * @param height Available height in pixels.
 *
 * Called from QML whenever the panel container size changes.
 */
void Widgets::Output::Panel::updateLayout(qreal width, qreal height)
{
  // Compute available area after subtracting margins
  const qreal margin = 4.0;
  const qreal gap    = 4.0;
  const qreal w      = width - 2 * margin;
  const qreal h      = height - 2 * margin;

  if (w <= 0 || h <= 0)
    return;

  // Run layout algorithm
  auto rects = PanelLayout::compute(m_outputWidgets, w, h, gap);

  // Convert layout rects to QML-visible geometry maps
  QVariantList geo;
  geo.reserve(rects.size());
  for (const auto& r : rects) {
    QVariantMap map;
    map[QStringLiteral("x")] = r.x + margin;
    map[QStringLiteral("y")] = r.y + margin;
    map[QStringLiteral("w")] = r.w;
    map[QStringLiteral("h")] = r.h;
    geo.append(map);
  }

  // Emit only if geometry actually changed
  if (geo != m_geometry) {
    m_geometry = geo;
    Q_EMIT geometryChanged();
  }
}
