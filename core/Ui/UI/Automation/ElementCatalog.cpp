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

#include "UI/Automation/ElementCatalog.h"

#include <initializer_list>
#include <QLatin1StringView>
#include <QMetaObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <QVariant>
#include <QVarLengthArray>
#include <span>

#include "Core/SSAssert.h"

using namespace UI::Automation;

//--------------------------------------------------------------------------------------------------
// Role tables: Qt Quick Controls class names, then the repository's Item-based components. Both
// are Qt or QML implementation names pinned by tst_ui_automation; re-verify per Qt bump.
//--------------------------------------------------------------------------------------------------

namespace detail {
struct RoleEntry {
  QLatin1StringView name;
  QLatin1StringView role;
};

struct WalkFrame {
  QQuickItem* item;
  int depth;
};
}  // namespace detail

using detail::RoleEntry;
using detail::WalkFrame;

// clang-format off
static constexpr RoleEntry kQtRoles[] = {
  {QLatin1StringView("QQuickAbstractButton"), QLatin1StringView("button")},
  {QLatin1StringView("QQuickButton"),         QLatin1StringView("button")},
  {QLatin1StringView("QQuickCheckBox"),       QLatin1StringView("checkbox")},
  {QLatin1StringView("QQuickComboBox"),       QLatin1StringView("combobox")},
  {QLatin1StringView("QQuickDial"),           QLatin1StringView("slider")},
  {QLatin1StringView("QQuickItemDelegate"),   QLatin1StringView("button")},
  {QLatin1StringView("QQuickMenuItem"),       QLatin1StringView("menuitem")},
  {QLatin1StringView("QQuickRadioButton"),    QLatin1StringView("radiobutton")},
  {QLatin1StringView("QQuickRangeSlider"),    QLatin1StringView("slider")},
  {QLatin1StringView("QQuickRoundButton"),    QLatin1StringView("button")},
  {QLatin1StringView("QQuickSlider"),         QLatin1StringView("slider")},
  {QLatin1StringView("QQuickSpinBox"),        QLatin1StringView("spinbox")},
  {QLatin1StringView("QQuickSwitch"),         QLatin1StringView("switch")},
  {QLatin1StringView("QQuickTabButton"),      QLatin1StringView("pagetab")},
  {QLatin1StringView("QQuickTextArea"),       QLatin1StringView("editabletext")},
  {QLatin1StringView("QQuickTextEdit"),       QLatin1StringView("editabletext")},
  {QLatin1StringView("QQuickTextField"),      QLatin1StringView("editabletext")},
  {QLatin1StringView("QQuickTextInput"),      QLatin1StringView("editabletext")},
  {QLatin1StringView("QQuickToolButton"),     QLatin1StringView("button")},
};

static constexpr RoleEntry kComponentRoles[] = {
  {QLatin1StringView("BigButton"),        QLatin1StringView("button")},
  {QLatin1StringView("MenuButton"),       QLatin1StringView("button")},
  {QLatin1StringView("SearchField"),      QLatin1StringView("editabletext")},
  {QLatin1StringView("SegmentedControl"), QLatin1StringView("pagetab")},
  {QLatin1StringView("TaskbarButton"),    QLatin1StringView("button")},
  {QLatin1StringView("ToolbarButton"),    QLatin1StringView("button")},
  {QLatin1StringView("WidgetDelegate"),   QLatin1StringView("tile")},
  {QLatin1StringView("WidgetTitleBar"),   QLatin1StringView("tile")},
};
// clang-format on

static constexpr QLatin1StringView kQmlTypeMarker("_QMLTYPE_");
static constexpr QLatin1StringView kQmlInlineMarker("_QML_");
static constexpr QLatin1StringView kNamedItemRole("item");
static constexpr int kMaxMetaDepth = 32;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The role @p table assigns to @p name, or empty.
 */
static QString roleFromTable(const QString& name, const std::span<const RoleEntry> table)
{
  for (const auto& entry : table)
    if (entry.name == name)
      return QString(entry.role);

  return QString();
}

/**
 * @brief The first non-empty string among the object's properties named in @p names.
 */
static QString firstStringProperty(const QObject* object,
                                   const std::initializer_list<const char*> names)
{
  SS_ASSERT(object != nullptr, return QString());
  for (const char* name : names) {
    const QVariant value = object->property(name);
    if (!value.isValid() || !value.canConvert<QString>())
      continue;

    const QString text = value.toString();
    if (!text.isEmpty())
      return text;
  }

  return QString();
}

/**
 * @brief The text a control holds, by role: the field contents, the combo's current entry, or
 *        the spin box value; empty for roles with no editable or selected value.
 */
static QString textForRole(const QObject* object, const QString& role)
{
  if (role == QLatin1StringView("editabletext"))
    return object->property("text").toString();

  if (role == QLatin1StringView("combobox"))
    return firstStringProperty(object, {"currentText", "displayText"});

  if (role == QLatin1StringView("spinbox") || role == QLatin1StringView("slider"))
    return object->property("value").toString();

  return QString();
}

/**
 * @brief Whether an editable control hides what it holds (a password or PIN echo mode), in which
 *        case the catalog reports neither its text nor a label derived from it.
 */
static bool isMasked(const QObject* object)
{
  const QVariant echo = object->property("echoMode");
  return echo.isValid() && echo.toInt() != 0;
}

/**
 * @brief Whether an entry passes the substring filters.
 */
static bool matchesFilter(const Element& element, const CatalogFilter& filter)
{
  if (!filter.id.isEmpty() && !element.id.contains(filter.id, Qt::CaseInsensitive))
    return false;

  return filter.label.isEmpty() || element.label.contains(filter.label, Qt::CaseInsensitive);
}

//--------------------------------------------------------------------------------------------------
// Classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief The QML component name behind a dynamic meta-object class name, read up to the
 *        `_QMLTYPE_` (file component) or `_QML_` (inline object) marker; the whole name otherwise.
 */
QString ElementCatalog::componentName(const QMetaObject* meta)
{
  SS_ASSERT(meta != nullptr, return QString());
  const QString name = QString::fromLatin1(meta->className());
  qsizetype marker   = name.indexOf(kQmlTypeMarker);
  if (marker < 0)
    marker = name.indexOf(kQmlInlineMarker);

  return marker < 0 ? name : name.left(marker);
}

/**
 * @brief The accessibility role of @p item, or empty when it is not a control: the meta-object
 *        chain is walked from the most derived class, so a QML type built on a Qt control gets
 *        the control's role unless the repository names the component itself.
 */
QString ElementCatalog::roleOf(const QQuickItem* item)
{
  SS_ASSERT(item != nullptr, return QString());
  const QMetaObject* meta = item->metaObject();
  for (int depth = 0; depth < kMaxMetaDepth && meta != nullptr; ++depth) {
    const QString component = componentName(meta);
    QString role            = roleFromTable(component, kComponentRoles);
    if (role.isEmpty())
      role = roleFromTable(QString::fromLatin1(meta->className()), kQtRoles);

    if (!role.isEmpty())
      return role;

    meta = meta->superClass();
  }

  return QString();
}

/**
 * @brief The catalog entry for a classified item: identifier from objectName, label from the
 *        first of text, title, placeholder or tooltip, geometry mapped to window coordinates. A
 *        masked field (password echo mode) contributes no text to either.
 */
Element ElementCatalog::describe(QQuickItem* item, const QString& role)
{
  Element element;
  element.enabled = false;
  element.focused = false;
  element.item    = item;
  SS_ASSERT(item != nullptr, return element);

  element.id   = item->objectName();
  element.role = role;
  if (isMasked(item)) {
    element.label = firstStringProperty(item, {"title", "placeholderText", "toolTip"});
  } else {
    element.label = firstStringProperty(item, {"text", "title", "placeholderText", "toolTip"});
    element.text  = textForRole(item, role);
  }

  element.rect    = item->mapRectToScene(QRectF(0.0, 0.0, item->width(), item->height()));
  element.enabled = item->isEnabled();
  element.focused = item->hasActiveFocus();

  const QVariant checked = item->property("checked");
  if (checked.isValid() && checked.canConvert<bool>())
    element.checked = checked.toBool();

  return element;
}

//--------------------------------------------------------------------------------------------------
// Walk
//--------------------------------------------------------------------------------------------------

/**
 * @brief Depth-first walk with an explicit stack: invisible or transparent subtrees are skipped,
 *        every classified control passing the filter is listed in child order, an item of no
 *        known type is listed as a plain "item" only when its objectName is a dotted authored
 *        identifier (the sweep's name declares a hand-built delegate interactive). A bare
 *        objectName the author never set, such as a Qt type name on a layout container, is not a
 *        declaration and stays out of the driving surface. The depth and item caps bound the loop.
 */
QList<Element> ElementCatalog::collect(QQuickWindow* window, const CatalogFilter& filter)
{
  QList<Element> result;
  SS_ASSERT(window != nullptr, return result);
  QQuickItem* content = window->contentItem();
  SS_ASSERT(content != nullptr, return result);

  QVarLengthArray<WalkFrame, 256> stack;
  stack.append(WalkFrame{content, 0});
  for (int visited = 0; visited < kMaxItems && !stack.isEmpty(); ++visited) {
    const WalkFrame frame = stack.last();
    stack.removeLast();
    QQuickItem* item = frame.item;
    if (!item->isVisible() || item->opacity() <= 0.0)
      continue;

    QString role = roleOf(item);
    if (role.isEmpty() && item->objectName().contains(QLatin1Char('.')))
      role = QString(kNamedItemRole);

    if (!role.isEmpty()) {
      const Element element = describe(item, role);
      if (matchesFilter(element, filter))
        result.append(element);
    }

    if (frame.depth >= kMaxDepth)
      continue;

    const auto children = item->childItems();
    for (auto it = children.crbegin(); it != children.crend(); ++it)
      stack.append(WalkFrame{*it, frame.depth + 1});
  }

  return result;
}

/**
 * @brief The first control in walk order whose identifier equals @p id exactly.
 */
std::optional<Element> ElementCatalog::byId(QQuickWindow* window, const QString& id)
{
  if (id.isEmpty())
    return std::nullopt;

  const auto elements = collect(window);
  for (const Element& element : elements)
    if (element.id == id)
      return element;

  return std::nullopt;
}

/**
 * @brief The last listed control with a non-empty rect containing @p point, which is the one
 *        drawn on top in child order.
 */
std::optional<Element> ElementCatalog::atPoint(QQuickWindow* window, const QPointF& point)
{
  const auto elements = collect(window);
  for (auto it = elements.crbegin(); it != elements.crend(); ++it)
    if (!it->rect.isEmpty() && it->rect.contains(point))
      return *it;

  return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
// Serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief The reply shape of one catalog entry; text and checked appear only when they apply.
 */
QJsonObject ElementCatalog::toJson(const Element& element)
{
  QJsonObject json;
  json.insert(QStringLiteral("id"), element.id);
  json.insert(QStringLiteral("role"), element.role);
  json.insert(QStringLiteral("label"), element.label);
  if (!element.text.isEmpty())
    json.insert(QStringLiteral("text"), element.text);

  json.insert(QStringLiteral("x"), element.rect.x());
  json.insert(QStringLiteral("y"), element.rect.y());
  json.insert(QStringLiteral("width"), element.rect.width());
  json.insert(QStringLiteral("height"), element.rect.height());
  json.insert(QStringLiteral("enabled"), element.enabled);
  if (element.checked.has_value())
    json.insert(QStringLiteral("checked"), *element.checked);

  json.insert(QStringLiteral("focused"), element.focused);
  return json;
}
