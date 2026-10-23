/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "ApiHandlers/AutomationHandler.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <QJsonArray>
#include <QKeySequence>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "Core/SerialStudio.h"
#include "UI/Automation/ElementCatalog.h"
#include "UI/Automation/InputSynthesizer.h"
#include "UI/Automation/WindowCapture.h"
#include "UI/Automation/WindowInventory.h"

using namespace UI::Automation;

static constexpr qsizetype kMaxModifiers     = 8;
static constexpr const char* kBackendContext = "Cpp_Misc_GraphicsBackend";
static constexpr const char* kReduceMotion   = "reduceMotion";

namespace API::Handlers::detail {
struct ModifierName {
  QLatin1StringView name;
  Qt::KeyboardModifier flag;
};
}  // namespace API::Handlers::detail

using API::Handlers::detail::ModifierName;

static constexpr ModifierName kModifierNames[] = {
  { QLatin1StringView("ctrl"), Qt::ControlModifier},
  {QLatin1StringView("shift"),   Qt::ShiftModifier},
  {  QLatin1StringView("alt"),     Qt::AltModifier},
  { QLatin1StringView("meta"),    Qt::MetaModifier},
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The refusal for a window that does not exist: an execution error naming the headless
 *        case when nothing is open at all, an invalid parameter when a name missed.
 */
static API::CommandResponse noWindow(const QString& id, const QString& name)
{
  if (WindowInventory::windows().isEmpty())
    return API::CommandResponse::makeError(
      id,
      API::ErrorCode::ExecutionError,
      QStringLiteral("No window: the application has no visible user interface (headless run)"));

  return API::CommandResponse::makeError(
    id, API::ErrorCode::InvalidParam, QStringLiteral("No visible window named '%1'").arg(name));
}

/**
 * @brief The window a request addresses (`window`, default main), and its name for the refusal.
 */
static QQuickWindow* resolveWindow(const QJsonObject& params, QString& name)
{
  name = params.value(QStringLiteral("window")).toString();
  return WindowInventory::resolve(name);
}

/**
 * @brief The mouse button a request names; left when absent.
 */
static std::optional<Qt::MouseButton> parseButton(const QString& text)
{
  if (text.isEmpty() || text == QLatin1StringView("left"))
    return Qt::LeftButton;

  if (text == QLatin1StringView("right"))
    return Qt::RightButton;

  if (text == QLatin1StringView("middle"))
    return Qt::MiddleButton;

  return std::nullopt;
}

/**
 * @brief The flag a modifier name stands for; nothing on an unknown name.
 */
static std::optional<Qt::KeyboardModifier> modifierFor(const QString& name)
{
  for (const auto& entry : kModifierNames)
    if (entry.name == name)
      return entry.flag;

  return std::nullopt;
}

/**
 * @brief The modifier set a request lists (ctrl, shift, alt, meta): absent means none; anything
 *        but an array of known names, or more names than the cap, is refused.
 */
static std::optional<Qt::KeyboardModifiers> parseModifiers(const QJsonValue& value)
{
  Qt::KeyboardModifiers modifiers;
  if (value.isUndefined() || value.isNull())
    return modifiers;

  if (!value.isArray() || value.toArray().size() > kMaxModifiers)
    return std::nullopt;

  const QJsonArray names = value.toArray();
  for (qsizetype i = 0; i < names.size() && i < kMaxModifiers; ++i) {
    const auto flag = names.at(i).isString() ? modifierFor(names.at(i).toString()) : std::nullopt;
    if (!flag.has_value())
      return std::nullopt;

    modifiers |= *flag;
  }

  return modifiers;
}

/**
 * @brief A finite number from a parameter, or nothing when it is absent, not a number or NaN.
 */
static std::optional<double> finiteNumber(const QJsonValue& value)
{
  if (!value.isDouble())
    return std::nullopt;

  bool ok             = false;
  const double number = SerialStudio::toDouble(value, &ok);
  if (!ok || !std::isfinite(number))
    return std::nullopt;

  return number;
}

/**
 * @brief The graphics backend the window's QML engine publishes as a root context property: the
 *        reduce-motion switch is a preference of that interface, reached the way QML reaches it
 *        rather than through a new singleton edge in the census.
 */
static QObject* graphicsBackendOf(const QQuickWindow* window)
{
  const QQmlContext* context = QQmlEngine::contextForObject(window);
  if (context == nullptr)
    return nullptr;

  return context->contextProperty(QString::fromLatin1(kBackendContext)).value<QObject*>();
}

/**
 * @brief The point a click lands on: the centre of the named element (visible and enabled), or
 *        the given coordinates inside the window. Returns the refusal when neither resolves.
 */
static std::optional<API::CommandResponse> resolveClickPoint(const QString& id,
                                                             QQuickWindow* window,
                                                             const QJsonObject& params,
                                                             QPointF& point)
{
  if (params.contains(QStringLiteral("id"))) {
    const QString target = params.value(QStringLiteral("id")).toString();
    const auto element   = ElementCatalog::byId(window, target);
    if (!element.has_value())
      return API::CommandResponse::makeError(
        id,
        API::ErrorCode::InvalidParam,
        QStringLiteral("No visible element with id '%1'").arg(target));

    if (!element->enabled)
      return API::CommandResponse::makeError(
        id, API::ErrorCode::ExecutionError, QStringLiteral("Element '%1' is disabled").arg(target));

    if (element->rect.isEmpty())
      return API::CommandResponse::makeError(
        id, API::ErrorCode::ExecutionError, QStringLiteral("Element '%1' has no size").arg(target));

    point = element->rect.center();
    return std::nullopt;
  }

  if (!params.contains(QStringLiteral("x")) || !params.contains(QStringLiteral("y")))
    return API::CommandResponse::makeError(
      id, API::ErrorCode::MissingParam, QStringLiteral("Provide an element id or both x and y"));

  const auto x = finiteNumber(params.value(QStringLiteral("x")));
  const auto y = finiteNumber(params.value(QStringLiteral("y")));
  if (!x.has_value() || !y.has_value())
    return API::CommandResponse::makeError(
      id, API::ErrorCode::InvalidParam, QStringLiteral("x and y must be finite numbers"));

  point = QPointF(*x, *y);
  const QRectF bounds(0.0, 0.0, window->width(), window->height());
  if (!bounds.contains(point))
    return API::CommandResponse::makeError(
      id, API::ErrorCode::InvalidParam, QStringLiteral("Point lies outside the window"));

  return std::nullopt;
}

/**
 * @brief A string-typed schema property.
 */
static API::SchemaProp stringProp(const QString& name, const QString& description)
{
  API::SchemaProp prop;
  prop.name        = name;
  prop.type        = QStringLiteral("string");
  prop.description = description;
  return prop;
}

/**
 * @brief A number-typed schema property.
 */
static API::SchemaProp numberProp(const QString& name, const QString& description)
{
  API::SchemaProp prop;
  prop.name        = name;
  prop.type        = QStringLiteral("number");
  prop.description = description;
  return prop;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all ui.automation.* commands with the command registry.
 */
void API::Handlers::AutomationHandler::registerCommands()
{
  registerReadCommands();
  registerInputCommands();
}

/**
 * @brief Register the read-only commands: windows, elements, screenshot, reduce motion.
 */
void API::Handlers::AutomationHandler::registerReadCommands()
{
  auto& registry = API::handlerContext().registry;
  const auto windowProp =
    stringProp(QStringLiteral("window"), QStringLiteral("Window id (default: main)"));

  registry.registerCommand(
    QStringLiteral("ui.automation.listWindows"),
    QStringLiteral("List the visible windows: [{id, title, x, y, width, height, scale, active, "
                   "modal, focusedId}]. Empty in a headless run."),
    API::emptySchema(),
    &listWindows);

  registry.registerCommand(
    QStringLiteral("ui.automation.listElements"),
    QStringLiteral("List the interactive controls of a window: [{id, role, label, text, x, y, "
                   "width, height, enabled, checked, focused}] in window coordinates. Roles: "
                   "button, checkbox, radiobutton, switch, pagetab, menuitem, combobox, "
                   "editabletext, spinbox, slider, tile (params: window, id substring, label "
                   "substring)"),
    API::makeSchema(
      {},
      {windowProp,
       stringProp(QStringLiteral("id"), QStringLiteral("Substring of the id")),
       stringProp(QStringLiteral("label"), QStringLiteral("Substring of the label"))}),
    &listElements);

  API::SchemaProp scale = numberProp(QStringLiteral("scale"),
                                     QStringLiteral("Downscale factor from 0.1 to 1 (default 1)"));
  scale.minimum         = WindowCapture::kMinScale;
  scale.maximum         = 1.0;
  scale.defaultValue    = 1.0;
  registry.registerCommand(
    QStringLiteral("ui.automation.screenshot"),
    QStringLiteral("Capture a window (or one element of it) as base64 PNG: {png, width, height, "
                   "scale}. The scale halves until the reply fits 4 MiB (params: window, id, "
                   "scale)"),
    API::makeSchema({},
                    {windowProp,
                     stringProp(QStringLiteral("id"), QStringLiteral("Element id to crop to")),
                     scale}),
    &screenshot);

  API::SchemaProp enabled;
  enabled.name        = QStringLiteral("enabled");
  enabled.type        = QStringLiteral("boolean");
  enabled.description = QStringLiteral("Whether chrome motion is reduced");
  registry.registerCommand(
    QStringLiteral("ui.automation.setReduceMotion"),
    QStringLiteral("Switch the reduced-motion preference and return the previous value: "
                   "{enabled, previous}. A preference, persisted like the Preferences toggle "
                   "(params: enabled)"),
    API::makeSchema({enabled}),
    &setReduceMotion);
}

/**
 * @brief Register the two driving commands, whose descriptions state the consent and the
 *        native-dialog caveat.
 */
void API::Handlers::AutomationHandler::registerInputCommands()
{
  auto& registry = API::handlerContext().registry;
  const auto windowProp =
    stringProp(QStringLiteral("window"), QStringLiteral("Window id (default: main)"));

  API::SchemaProp button =
    stringProp(QStringLiteral("button"), QStringLiteral("Mouse button (default left)"));
  button.enumValues =
    QJsonArray{QStringLiteral("left"), QStringLiteral("right"), QStringLiteral("middle")};
  button.defaultValue = QStringLiteral("left");

  API::SchemaProp doubleClick;
  doubleClick.name        = QStringLiteral("double");
  doubleClick.type        = QStringLiteral("boolean");
  doubleClick.description = QStringLiteral("Double-click instead of a single click");

  registry.registerCommand(
    QStringLiteral("ui.automation.click"),
    QStringLiteral("Click a control by id or a point in window coordinates, as the user would; "
                   "the reply is {hit, id, x, y, delivered}. Needs the interface-driving consent "
                   "from a remote client. A control that opens a native file dialog blocks until "
                   "a person dismisses it (params: window, id or x and y, button, modifiers "
                   "[ctrl, shift, alt, meta], double)"),
    API::makeSchema({},
                    {windowProp,
                     stringProp(QStringLiteral("id"), QStringLiteral("Element id to click")),
                     numberProp(QStringLiteral("x"), QStringLiteral("X in window coordinates")),
                     numberProp(QStringLiteral("y"), QStringLiteral("Y in window coordinates")),
                     button,
                     API::typedArrayProp(QStringLiteral("modifiers"),
                                         QStringLiteral("Held modifiers: ctrl, shift, alt, meta"),
                                         QStringLiteral("string")),
                     doubleClick}),
    &click);

  registry.registerCommand(
    QStringLiteral("ui.automation.key"),
    QStringLiteral("Type text or press a key sequence (QKeySequence notation such as Ctrl+S) in a "
                   "window, after focusing the optional target id; the reply is {delivered, "
                   "focusedId}. Needs the interface-driving consent from a remote client "
                   "(params: window, target, text, keys)"),
    API::makeSchema({},
                    {windowProp,
                     stringProp(QStringLiteral("target"), QStringLiteral("Element id to focus")),
                     stringProp(QStringLiteral("text"), QStringLiteral("Text to type")),
                     stringProp(QStringLiteral("keys"), QStringLiteral("Key sequence to press"))}),
    &key);
}

//--------------------------------------------------------------------------------------------------
// Read commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief The visible windows; empty when the root loaded no interface.
 */
API::CommandResponse API::Handlers::AutomationHandler::listWindows(const QString& id,
                                                                   const QJsonObject& params)
{
  (void)params;
  QJsonArray windows;
  const auto list = WindowInventory::windows();
  for (QQuickWindow* window : list) {
    const WindowInfo info = WindowInventory::describe(window);
    QJsonObject json;
    json.insert(QStringLiteral("id"), info.id);
    json.insert(QStringLiteral("title"), info.title);
    json.insert(QStringLiteral("x"), info.frame.x());
    json.insert(QStringLiteral("y"), info.frame.y());
    json.insert(QStringLiteral("width"), info.frame.width());
    json.insert(QStringLiteral("height"), info.frame.height());
    json.insert(QStringLiteral("scale"), info.scale);
    json.insert(QStringLiteral("active"), info.active);
    json.insert(QStringLiteral("modal"), info.modal);
    json.insert(QStringLiteral("focusedId"), info.focusedId);
    windows.append(json);
  }

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("windows"), windows}
  });
}

/**
 * @brief The filtered element catalog of one window.
 */
API::CommandResponse API::Handlers::AutomationHandler::listElements(const QString& id,
                                                                    const QJsonObject& params)
{
  QString name;
  QQuickWindow* window = resolveWindow(params, name);
  if (window == nullptr)
    return noWindow(id, name);

  CatalogFilter filter;
  filter.id    = params.value(QStringLiteral("id")).toString();
  filter.label = params.value(QStringLiteral("label")).toString();

  QJsonArray elements;
  const auto list = ElementCatalog::collect(window, filter);
  for (const Element& element : list)
    elements.append(ElementCatalog::toJson(element));

  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {  QStringLiteral("window"), window->objectName()},
      {QStringLiteral("elements"),             elements}
  });
}

/**
 * @brief A window or element capture as base64 PNG.
 */
API::CommandResponse API::Handlers::AutomationHandler::screenshot(const QString& id,
                                                                  const QJsonObject& params)
{
  QString name;
  QQuickWindow* window = resolveWindow(params, name);
  if (window == nullptr)
    return noWindow(id, name);

  QRectF crop;
  if (params.contains(QStringLiteral("id"))) {
    const QString target = params.value(QStringLiteral("id")).toString();
    const auto element   = ElementCatalog::byId(window, target);
    if (!element.has_value())
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("No visible element with id '%1'").arg(target));

    crop = element->rect;
  }

  double scale = 1.0;
  if (params.contains(QStringLiteral("scale"))) {
    const auto requested = finiteNumber(params.value(QStringLiteral("scale")));
    if (!requested.has_value() || *requested < WindowCapture::kMinScale || *requested > 1.0)
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("scale must be a number from 0.1 to 1"));

    scale = *requested;
  }

  const auto capture = WindowCapture::capture(window, crop, scale);
  if (!capture.has_value())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("The window could not be captured"));

  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {   QStringLiteral("png"), QString::fromLatin1(capture->png.toBase64())},
      { QStringLiteral("width"),                               capture->width},
      {QStringLiteral("height"),                              capture->height},
      { QStringLiteral("scale"),                               capture->scale}
  });
}

/**
 * @brief Switches the persisted reduced-motion preference and reports the previous value, so a
 *        test can restore it.
 */
API::CommandResponse API::Handlers::AutomationHandler::setReduceMotion(const QString& id,
                                                                       const QJsonObject& params)
{
  const QJsonValue enabled = params.value(QStringLiteral("enabled"));
  if (enabled.isUndefined())
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  if (!enabled.isBool())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("enabled must be a boolean"));

  QString name;
  const QQuickWindow* window = resolveWindow(QJsonObject(), name);
  if (window == nullptr)
    return noWindow(id, name);

  QObject* backend = graphicsBackendOf(window);
  if (backend == nullptr)
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("The main window exposes no graphics backend to switch"));

  const bool previous = backend->property(kReduceMotion).toBool();
  if (!backend->setProperty(kReduceMotion, enabled.toBool()))
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("The reduce-motion preference is read-only"));

  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      { QStringLiteral("enabled"), backend->property(kReduceMotion).toBool()},
      {QStringLiteral("previous"),                                  previous}
  });
}

//--------------------------------------------------------------------------------------------------
// Driving commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief Delivers a click at an element's centre or at a point, then reports the control under
 *        that point and the coordinates used.
 */
API::CommandResponse API::Handlers::AutomationHandler::click(const QString& id,
                                                             const QJsonObject& params)
{
  QString name;
  QQuickWindow* window = resolveWindow(params, name);
  if (window == nullptr)
    return noWindow(id, name);

  const QJsonValue buttonValue = params.value(QStringLiteral("button"));
  const auto button            = buttonValue.isUndefined() || buttonValue.isString()
                                 ? parseButton(buttonValue.toString())
                                 : std::nullopt;
  if (!button.has_value())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("button must be left, right or middle"));

  const auto modifiers = parseModifiers(params.value(QStringLiteral("modifiers")));
  if (!modifiers.has_value())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("modifiers must be an array of up to 8 names among ctrl, shift, alt, meta"));

  const QJsonValue doubleValue = params.value(QStringLiteral("double"));
  if (!doubleValue.isUndefined() && !doubleValue.isBool())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("double must be a boolean"));

  QPointF point;
  const auto refused = resolveClickPoint(id, window, params, point);
  if (refused.has_value())
    return *refused;

  const bool doubleClick = doubleValue.toBool();
  const bool delivered   = InputSynthesizer::click(window, point, *button, *modifiers, doubleClick);
  const auto hit         = ElementCatalog::atPoint(window, point);

  QJsonObject result;
  result.insert(QStringLiteral("hit"), hit.has_value());
  if (hit.has_value())
    result.insert(QStringLiteral("id"), hit->id);

  result.insert(QStringLiteral("x"), point.x());
  result.insert(QStringLiteral("y"), point.y());
  result.insert(QStringLiteral("delivered"), delivered);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Focuses the optional target, then types the text and presses the key sequence; the
 *        window is asked to activate first because Qt Quick routes keys through active focus.
 */
API::CommandResponse API::Handlers::AutomationHandler::key(const QString& id,
                                                           const QJsonObject& params)
{
  QString name;
  QQuickWindow* window = resolveWindow(params, name);
  if (window == nullptr)
    return noWindow(id, name);

  const QString text = params.value(QStringLiteral("text")).toString();
  const QString keys = params.value(QStringLiteral("keys")).toString();
  if (text.isEmpty() && keys.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Provide text or keys"));

  if (text.size() > InputSynthesizer::kMaxTextLength)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("text exceeds 4096 characters"));

  const QKeySequence sequence(keys, QKeySequence::PortableText);
  if (!keys.isEmpty() && sequence.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("keys is not a key sequence"));

  const QString target = params.value(QStringLiteral("target")).toString();
  if (!target.isEmpty()) {
    const auto element = ElementCatalog::byId(window, target);
    if (!element.has_value())
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("No visible element with id '%1'").arg(target));

    InputSynthesizer::focus(element->item);
  }

  if (!window->isActive())
    window->requestActivate();

  bool delivered = false;
  if (!text.isEmpty())
    delivered = InputSynthesizer::text(window, text);

  if (!keys.isEmpty())
    delivered = InputSynthesizer::keys(window, sequence) || delivered;

  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {QStringLiteral("delivered"),                          delivered},
      {QStringLiteral("focusedId"), WindowInventory::focusedId(window)}
  });
}
