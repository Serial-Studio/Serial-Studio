/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/Panel.h"

#include <QThread>
#include <utility>

#include "API/HandlerContext.h"
#include "Core/SSAssert.h"
#include "DataModel/DataTable.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/PipelineModules.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/Output/Button.h"
#include "UI/Widgets/Output/PanelLayout.h"
#include "UI/Widgets/Output/Slider.h"
#include "UI/Widgets/Output/TextField.h"
#include "UI/Widgets/Output/Toggle.h"

//--------------------------------------------------------------------------------------------------
// Transmit destination
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the target a live control transmits through. This is the only place that resolves
 *        a payload's destination: the widget classes below hold a target, never a connection, so a
 *        preview built elsewhere has no device to reach (spec 0079).
 */
[[nodiscard]] static Widgets::Output::TransmitTarget liveTransmitTarget(int sourceId)
{
  SS_ASSERT_LOG(sourceId >= 0);
  auto* connections = &API::handlerContext().connectionManager;

  Widgets::Output::TransmitTarget target;
  target.minIntervalMs = Widgets::Output::kLiveSendIntervalMs;
  target.deliver       = [connections, sourceId](const QByteArray& data) {
    (void)connections->writeDataToDevice(sourceId, data);
  };

  return target;
}

/**
 * @brief The store's write clock, spelled the same for the live store and for a GUI mirror
 *        snapshot so the freshness read does not care which view the routing rule handed back.
 */
[[nodiscard]] static quint64 tableWriteClock(const DataModel::DataTableStore& store)
{
  return store.writeClock();
}

/**
 * @brief Write-clock overload for the published snapshot.
 */
[[nodiscard]] static quint64 tableWriteClock(const DataModel::DataTableSnapshot& snapshot)
{
  return snapshot.writeClock;
}

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the panel model from the output group at the given index.
 */
Widgets::Output::Panel::Panel(int index, QQuickItem* parent)
  : QQuickItem(parent), m_tableContext(nullptr), m_dashboard(UI::Dashboard::instance())
{
  connect(&m_dashboard, &UI::Dashboard::updated, this, &Panel::refreshStates);

  if (!VALIDATE_WIDGET(SerialStudio::DashboardOutputPanel, index))
    return;

  const auto& group = GET_GROUP(SerialStudio::DashboardOutputPanel, index);

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
    map[QStringLiteral("checkable")]    = ow.checkable;
    map[QStringLiteral("color")]        = ow.color;
    map[QStringLiteral("sizeScale")]    = DataModel::outputSizeScale(ow.size);
    map[QStringLiteral("onLabel")]      = ow.onLabel;
    map[QStringLiteral("offLabel")]     = ow.offLabel;
    m_widgets.append(map);

    const auto target = liveTransmitTarget(ow.sourceId);

    Base* model = nullptr;
    switch (ow.type) {
      case DataModel::OutputWidgetType::Button:
        model = new Button(ow, target, this);
        break;
      case DataModel::OutputWidgetType::Slider:
      case DataModel::OutputWidgetType::Knob:
        model = new Slider(ow, target, this);
        break;
      case DataModel::OutputWidgetType::Toggle:
        model = new Toggle(ow, target, this);
        break;
      case DataModel::OutputWidgetType::TextField:
        model = new TextField(ow, target, this);
        break;
      default:
        break;
    }

    m_models.append(QVariant::fromValue(model));
  }

  for (const auto& entry : std::as_const(m_models)) {
    auto* model = entry.value<Base*>();
    if (model && model->stateBinding().readsTable()) {
      m_tableContext = &DataModel::pipelineModules().frameBuilder.guiTableApiContext();
      break;
    }
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
// State feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief Feeds every bound control what its state source reports: display tick only, one lookup
 *        per bound control. A dataset binding never takes the table path, which would arm the
 *        snapshot mirror for the session (spec 0080), and a panel the operator is not looking at
 *        costs nothing, picking the state back up from itemChange().
 */
void Widgets::Output::Panel::refreshStates()
{
  if (!isEnabled() || !isVisible())
    return;

  for (const auto& entry : std::as_const(m_models)) {
    auto* model = entry.value<Base*>();
    if (!model || !model->stateBinding().bound())
      continue;

    const auto& binding = model->stateBinding();
    if (!binding.readsTable()) {
      const auto& datasets = m_dashboard.datasets();
      const auto it        = datasets.constFind(binding.datasetId());
      if (it == datasets.cend())
        model->forgetState();
      else
        model->observeState(it->numericValue, it->value, it->displaySampleMs);
    }

    else
      observeTableState(model);

    model->refreshState();
  }
}

/**
 * @brief Reads a table-variable source through the sanctioned routing, which on the GUI thread
 *        takes the mirror snapshot. The read is skipped while no snapshot has been published,
 *        because readTableView()'s remaining branch is a blocking marshal spinning a nested
 *        QEventLoop, and this runs inside the display tick that loop would re-enter.
 */
void Widgets::Output::Panel::observeTableState(Base* model)
{
  SS_ASSERT(model != nullptr, return);
  SS_ASSERT(m_tableContext != nullptr, return);

  const auto& ctx = *m_tableContext;
  SS_ASSERT(ctx.owner != nullptr, return);

  const bool ownsStore = QThread::currentThread() == ctx.owner->thread();
  if (!ownsStore && (!ctx.mirror || !*ctx.mirror))
    return;

  const auto& binding = model->stateBinding();

  bool found    = false;
  double value  = 0;
  quint64 clock = 0;
  QString text;
  DataModel::readTableView(ctx, [&](const auto& store) {
    clock           = tableWriteClock(store);
    const auto* reg = store.get(binding.table(), binding.variable());
    if (!reg)
      return;

    found = true;
    value = reg->numericValue;
    text  = reg->stringValue;
  });

  if (!found)
    model->forgetState();
  else
    model->observeTableState(value, text, clock);
}

/**
 * @brief Picks the state back up when the panel becomes visible, so a page the operator switches
 *        back to shows what the plant reports now rather than what it reported when the page was
 *        last on screen.
 */
void Widgets::Output::Panel::itemChange(ItemChange change, const ItemChangeData& value)
{
  QQuickItem::itemChange(change, value);

  if (change == ItemVisibleHasChanged && value.boolValue)
    refreshStates();
}

//--------------------------------------------------------------------------------------------------
// Layout computation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Recomputes layout geometry using the PanelLayout engine.
 */
void Widgets::Output::Panel::updateLayout(qreal width, qreal height)
{
  const qreal margin = 4.0;
  const qreal gap    = 4.0;
  const qreal w      = width - 2 * margin;
  const qreal h      = height - 2 * margin;

  if (w <= 0 || h <= 0)
    return;

  auto rects = PanelLayout::compute(m_outputWidgets, w, h, gap);

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

  if (geo != m_geometry) {
    m_geometry = geo;
    Q_EMIT geometryChanged();
  }
}
