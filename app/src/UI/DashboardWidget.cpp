/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "UI/DashboardWidget.h"

#include <QHash>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>

#include "Misc/ThemeManager.h"
#include "UI/Dashboard.h"
#include "UI/WidgetExtensions.h"
#include "UI/WidgetRegistry.h"
#include "UI/Widgets/Accelerometer.h"
#include "UI/Widgets/Bar.h"
#include "UI/Widgets/BarPanel.h"
#include "UI/Widgets/Compass.h"
#include "UI/Widgets/DataGrid.h"
#include "UI/Widgets/ExtensionData.h"
#include "UI/Widgets/FFTPlot.h"
#include "UI/Widgets/Gauge.h"
#include "UI/Widgets/GPS.h"
#include "UI/Widgets/Gyroscope.h"
#include "UI/Widgets/LEDPanel.h"
#include "UI/Widgets/Meter.h"
#include "UI/Widgets/MultiPlot.h"
#include "UI/Widgets/Plot.h"
#include "UI/Widgets/WebView.h"

/**
 * @brief Maps a widget type to the project widget string a bundled package may declare it
 *        replaces; empty for types no package can ship as (tools, sentinels, output panels).
 */
[[nodiscard]] static QString builtinWidgetId(SerialStudio::DashboardWidget type)
{
  static const QHash<SerialStudio::DashboardWidget, QString> kIds = {
    {     SerialStudio::DashboardDataGrid,      QStringLiteral("datagrid")},
    {    SerialStudio::DashboardMultiPlot,     QStringLiteral("multiplot")},
    {SerialStudio::DashboardAccelerometer, QStringLiteral("accelerometer")},
    {    SerialStudio::DashboardGyroscope,          QStringLiteral("gyro")},
    {          SerialStudio::DashboardGPS,           QStringLiteral("gps")},
    {      SerialStudio::DashboardWebView,       QStringLiteral("webview")},
    {          SerialStudio::DashboardLED,     QStringLiteral("led-panel")},
    {          SerialStudio::DashboardBar,           QStringLiteral("bar")},
    {        SerialStudio::DashboardGauge,         QStringLiteral("gauge")},
    {      SerialStudio::DashboardCompass,       QStringLiteral("compass")},
    {        SerialStudio::DashboardMeter,         QStringLiteral("meter")},
  };
  return kIds.value(type);
}

/**
 * @brief Builds a web-view widget for slot @p index, seeding its URL from the project.
 */
[[nodiscard]] static QQuickItem* makeWebViewWidget(int index, QQuickItem* parent)
{
  static auto& dashboard = UI::Dashboard::instance();
  auto* w                = new Widgets::WebView(index, parent);
  const auto& group      = dashboard.getGroupWidget(SerialStudio::DashboardWebView, index);
  w->setUrl(group.webViewUrl);
  return w;
}

#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/ImageView.h"
#  include "UI/Widgets/Output/Panel.h"
#  include "UI/Widgets/Painter.h"
#  include "UI/Widgets/Plot3D.h"
#  include "UI/Widgets/Waterfall.h"

/**
 * @brief Builds a painter widget for slot @p index, seeding its user code from the project.
 */
[[nodiscard]] static QQuickItem* makePainterWidget(int index, QQuickItem* parent)
{
  static auto& dashboard = UI::Dashboard::instance();
  auto* p                = new Widgets::Painter(index, parent);
  const auto& group      = dashboard.getGroupWidget(SerialStudio::DashboardPainter, index);
  p->setUserCode(group.painterCode);
  return p;
}
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a DashboardWidget with default state and theme bindings.
 */
UI::DashboardWidget::DashboardWidget(QQuickItem* parent)
  : QQuickItem(parent)
  , m_dashboard(UI::Dashboard::instance())
  , m_themeManager(Misc::ThemeManager::instance())
  , m_widgetRegistry(UI::WidgetRegistry::instance())
  , m_index(-1)
  , m_relativeIndex(-1)
  , m_widgetType(SerialStudio::DashboardNoWidget)
  , m_qmlPath("")
  , m_extensionId("")
  , m_extensionError("")
  , m_dbWidget(nullptr)
{
  connect(
    this, &UI::DashboardWidget::widgetIndexChanged, this, &UI::DashboardWidget::widgetColorChanged);
  connect(
    this, &UI::DashboardWidget::widgetIndexChanged, this, &UI::DashboardWidget::widgetTitleChanged);
  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &UI::DashboardWidget::widgetColorChanged);
  connect(&m_dashboard,
          &UI::Dashboard::displayTitlesChanged,
          this,
          &UI::DashboardWidget::widgetTitleChanged);
}

/**
 * @brief Destroys the widget and schedules its inner model for deletion.
 */
UI::DashboardWidget::~DashboardWidget()
{
  if (m_dbWidget)
    m_dbWidget->deleteLater();
}

//--------------------------------------------------------------------------------------------------
// Widget properties
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the global index of the widget.
 */
int UI::DashboardWidget::widgetIndex() const
{
  return m_index;
}

/**
 * @brief Returns the relative index of the widget within its type.
 */
int UI::DashboardWidget::relativeIndex() const
{
  return m_relativeIndex;
}

//--------------------------------------------------------------------------------------------------
// Color & style
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when a dataset widget belongs to the plot family, the only widgets that
 *        keep the per-index multicolor cycle (spec 0052).
 */
[[nodiscard]] static bool plotClassDatasetWidget(const SerialStudio::DashboardWidget w)
{
  if (w == SerialStudio::DashboardPlot || w == SerialStudio::DashboardFFT)
    return true;

#ifdef BUILD_COMMERCIAL
  if (w == SerialStudio::DashboardWaterfall)
    return true;
#endif

  return false;
}

/**
 * @brief Returns the color of the widget based on the current theme: plot-family widgets cycle
 *        the theme palette, every other dataset widget shares the single accent (overrides win).
 */
QColor UI::DashboardWidget::widgetColor() const
{
  if (VALIDATE_WIDGET(m_widgetType, m_relativeIndex)) {
    const auto slot = m_dashboard.widgetSlot(m_widgetType, m_relativeIndex);
    if (slot.valid && !slot.group) {
      const auto& dataset = GET_DATASET(m_widgetType, slot.bucketIndex);
      if (plotClassDatasetWidget(m_widgetType))
        return SerialStudio::getDatasetColor(dataset);

      return SerialStudio::getDatasetAccentColor(dataset);
    }
  }

  return QColor::fromRgba(qRgba(0, 0, 0, 0));
}

/**
 * @brief Returns the window title for the given widget.
 */
QString UI::DashboardWidget::widgetTitle() const
{
  if (VALIDATE_WIDGET(m_widgetType, m_relativeIndex)) {
    const auto slot = m_dashboard.widgetSlot(m_widgetType, m_relativeIndex);
    if (slot.valid && slot.group)
      return GET_GROUP(m_widgetType, slot.bucketIndex).title;

    if (slot.valid)
      return GET_DATASET(m_widgetType, slot.bucketIndex).title;
  }

  return tr("Invalid");
}

/**
 * @brief Returns the type of the current widget.
 */
SerialStudio::DashboardWidget UI::DashboardWidget::widgetType() const
{
  return m_widgetType;
}

/**
 * @brief Returns a stable string key identifying this specific widget instance.
 */
QString UI::DashboardWidget::widgetId() const
{
  auto id   = m_widgetRegistry.widgetIdByTypeAndIndex(m_widgetType, m_relativeIndex);
  auto info = m_widgetRegistry.widgetInfo(id);
  return QStringLiteral("%1:%2:%3")
    .arg(static_cast<int>(m_widgetType))
    .arg(info.groupId)
    .arg(info.datasetIndex);
}

/**
 * @brief Returns the stable uniqueId of the dataset or group backing this widget (-1 if none).
 */
int UI::DashboardWidget::widgetUniqueId() const
{
  if (!VALIDATE_WIDGET(m_widgetType, m_relativeIndex))
    return -1;

  const auto slot = m_dashboard.widgetSlot(m_widgetType, m_relativeIndex);
  if (!slot.valid)
    return -1;

  if (slot.group)
    return GET_GROUP(m_widgetType, slot.bucketIndex).uniqueId;

  return GET_DATASET(m_widgetType, slot.bucketIndex).uniqueId;
}

/**
 * @brief Returns the source/device index of the group this widget belongs to.
 */
int UI::DashboardWidget::widgetSourceId() const
{
  if (!VALIDATE_WIDGET(m_widgetType, m_relativeIndex))
    return 0;

  const auto slot = m_dashboard.widgetSlot(m_widgetType, m_relativeIndex);
  if (!slot.valid)
    return 0;

  if (slot.group)
    return GET_GROUP(m_widgetType, slot.bucketIndex).sourceId;

  return GET_DATASET(m_widgetType, slot.bucketIndex).sourceId;
}

/**
 * @brief Returns the QML path of the current widget.
 */
QString UI::DashboardWidget::widgetQmlPath() const
{
  return m_qmlPath;
}

/**
 * @brief Returns whether this slot is rendered by an installed widget-extension package.
 */
bool UI::DashboardWidget::widgetIsExtension() const
{
  return !m_extensionId.isEmpty();
}

/**
 * @brief Returns the id of the package rendering this slot, empty for a compiled-in widget.
 */
const QString& UI::DashboardWidget::widgetExtensionId() const
{
  return m_extensionId;
}

/**
 * @brief Returns why the package could not be created, empty while nothing has failed.
 */
const QString& UI::DashboardWidget::widgetExtensionError() const
{
  return m_extensionError;
}

/**
 * @brief Returns the model item of the current widget.
 */
QQuickItem* UI::DashboardWidget::widgetModel() const
{
  return m_dbWidget;
}

/**
 * @brief Selects and configures the appropriate widget for the given @a index.
 */
void UI::DashboardWidget::setWidgetIndex(const int index)
{
  if (index < 0 || index >= m_dashboard.totalWidgetCount())
    return;

  m_index         = index;
  m_widgetType    = m_dashboard.widgetType(index);
  m_relativeIndex = m_dashboard.relativeIndex(index);

  if (m_dbWidget) {
    m_dbWidget->deleteLater();
    m_dbWidget = nullptr;
  }

  m_extensionId.clear();
  m_extensionError.clear();

  if (!buildExtensionModel())
    buildWidgetForType();

  if (m_dbWidget)
    m_dbWidget->setParentItem(this);

  Q_EMIT widgetIndexChanged();
}

/**
 * @brief Creates the package's root item in its own QML context, with the host's Cpp_* names
 *        shadowed to undefined for every package the user installed; a bundled package ships
 *        inside the application and is exempt, exactly as it is exempt from consent. That shadowing
 *        is a speed bump, not a boundary: the QML engine is shared, so a package that means to
 *        reach the host still can, and the consent flow is what makes the trust model honest.
 *        Returns nullptr and records the cause on failure.
 */
QQuickItem* UI::DashboardWidget::createExtensionItem(QQuickItem* parent,
                                                     const QVariantMap& properties)
{
  m_extensionError.clear();
  if (m_extensionId.isEmpty())
    return nullptr;

  auto* engine = qmlEngine(this);
  if (!engine || m_qmlPath.isEmpty()) {
    failExtension(tr("The package is not installed, or has not been allowed to run."));
    return nullptr;
  }

  QQmlComponent component(engine, QUrl(m_qmlPath), QQmlComponent::PreferSynchronous);
  if (component.isError()) {
    failExtension(component.errorString().trimmed());
    return nullptr;
  }

  static auto& catalog = UI::WidgetExtensions::instance();

  auto* context = new QQmlContext(qmlContext(this), this);
  if (!catalog.descriptor(m_extensionId).bundled) {
    const auto names = UI::WidgetExtensions::hostContextNames();
    for (const auto& name : names)
      context->setContextProperty(name, QVariant());
  }

  auto* object = component.createWithInitialProperties(properties, context);
  auto* item   = qobject_cast<QQuickItem*>(object);
  if (!item) {
    delete object;
    failExtension(component.errorString().trimmed());
    return nullptr;
  }

  item->setParentItem(parent);
  QQmlEngine::setObjectOwnership(item, QQmlEngine::JavaScriptOwnership);
  return item;
}

/**
 * @brief Records a package failure for the placeholder tile and the problem center.
 */
void UI::DashboardWidget::failExtension(const QString& error)
{
  static auto& catalog = UI::WidgetExtensions::instance();

  m_extensionError = error;
  catalog.reportLoadFailure(m_extensionId, error);
  Q_EMIT widgetExtensionErrorChanged();
}

/**
 * @brief Points this slot at a widget-extension package when one serves it: a third-party widget,
 *        or a built-in whose implementation now ships bundled. Returns false when the built-in
 *        keeps its compiled implementation, which is every built-in until a package replaces one.
 */
bool UI::DashboardWidget::buildExtensionModel()
{
  static auto& catalog = UI::WidgetExtensions::instance();

  if (widgetType() == SerialStudio::DashboardExtension)
    m_extensionId = m_dashboard.extensionSlot(relativeIndex()).extensionId;

  else
    m_extensionId = catalog.builtinReplacement(builtinWidgetId(widgetType()));

  if (m_extensionId.isEmpty())
    return false;

  m_qmlPath = catalog.qmlUrl(m_extensionId);
  if (m_qmlPath.isEmpty())
    catalog.requestConsent(m_extensionId);

  m_dbWidget = new Widgets::ExtensionData(m_extensionId, widgetType(), relativeIndex(), this);
  return true;
}

/**
 * @brief Re-resolves this slot after the catalog changed, so a package that was just allowed to
 *        run (or updated) replaces its placeholder without reloading the project.
 */
void UI::DashboardWidget::reloadWidget()
{
  setWidgetIndex(m_index);
}

/**
 * @brief Constructs the QQuickItem and selects the QML path for the current widget type.
 */
void UI::DashboardWidget::buildWidgetForType()
{
#ifdef BUILD_COMMERCIAL
  if (buildCommercialWidgetForType())
    return;
#endif

  switch (widgetType()) {
    case SerialStudio::DashboardDataGrid:
      m_dbWidget = new Widgets::DataGrid(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/DataGrid.qml";
      break;
    case SerialStudio::DashboardMultiPlot:
      m_dbWidget = new Widgets::MultiPlot(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/MultiPlot.qml";
      break;
    case SerialStudio::DashboardFFT:
      m_dbWidget = new Widgets::FFTPlot(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/FFTPlot.qml";
      break;
    case SerialStudio::DashboardPlot:
      m_dbWidget = new Widgets::Plot(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Plot.qml";
      break;
    case SerialStudio::DashboardBar:
      m_dbWidget = new Widgets::Bar(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Bar.qml";
      break;
    case SerialStudio::DashboardGauge:
      m_dbWidget = new Widgets::Gauge(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Gauge.qml";
      break;
    case SerialStudio::DashboardMeter:
      m_dbWidget = new Widgets::Meter(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Meter.qml";
      break;
    case SerialStudio::DashboardCompass:
      m_dbWidget = new Widgets::Compass(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Compass.qml";
      break;
    case SerialStudio::DashboardGyroscope:
      m_dbWidget = new Widgets::Gyroscope(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Gyroscope.qml";
      break;
    case SerialStudio::DashboardAccelerometer:
      m_dbWidget = new Widgets::Accelerometer(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Accelerometer.qml";
      break;
    case SerialStudio::DashboardTerminal:
      m_dbWidget = nullptr;
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Terminal.qml";
      break;
    case SerialStudio::DashboardClock:
      m_dbWidget = nullptr;
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Clock.qml";
      break;
    case SerialStudio::DashboardStopwatch:
      m_dbWidget = nullptr;
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Stopwatch.qml";
      break;
    case SerialStudio::DashboardGPS:
      m_dbWidget = new Widgets::GPS(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/GPS.qml";
      break;
    case SerialStudio::DashboardLED:
      m_dbWidget = new Widgets::LEDPanel(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/LEDPanel.qml";
      break;
    case SerialStudio::DashboardBarPanel:
      m_dbWidget = new Widgets::BarPanel(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/BarPanel.qml";
      break;
    case SerialStudio::DashboardWebView:
      m_dbWidget = makeWebViewWidget(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/WebView.qml";
      break;
    default:
      break;
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Builds the widget when the type is a commercial one; returns false for free types.
 */
bool UI::DashboardWidget::buildCommercialWidgetForType()
{
  switch (widgetType()) {
    case SerialStudio::DashboardPlot3D:
      m_dbWidget = new Widgets::Plot3D(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Plot3D.qml";
      return true;
    case SerialStudio::DashboardImageView:
      m_dbWidget = new Widgets::ImageView(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/ImageView.qml";
      return true;
    case SerialStudio::DashboardOutputPanel:
      m_dbWidget = new Widgets::Output::Panel(relativeIndex(), this);
      m_qmlPath =
        "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml";
      return true;
    case SerialStudio::DashboardNotificationLog:
      m_dbWidget = nullptr;
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/NotificationLog.qml";
      return true;
    case SerialStudio::DashboardWaterfall:
      m_dbWidget = new Widgets::Waterfall(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Waterfall.qml";
      return true;
    case SerialStudio::DashboardPainter:
      m_dbWidget = makePainterWidget(relativeIndex(), this);
      m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Painter.qml";
      return true;
    default:
      return false;
  }
}
#endif
