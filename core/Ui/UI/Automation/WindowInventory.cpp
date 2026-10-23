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

#include "UI/Automation/WindowInventory.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <QVariant>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Qt internals the inventory leans on (pinned by tst_ui_automation; re-verify per Qt bump)
//--------------------------------------------------------------------------------------------------

static constexpr const char* kDefaultWindowId = "main";
static constexpr const char* kOverlayClass    = "QQuickOverlay";
static constexpr const char* kPopupItemClass  = "QQuickPopupItem";
static constexpr int kMaxAncestors            = 64;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether @p popupItem is a visible popup whose owning popup object reports itself modal.
 *        The popup item is parented to its popup, which is how the modal flag is reached without
 *        the private Templates headers.
 */
static bool isModalPopupItem(const QQuickItem* popupItem)
{
  SS_ASSERT(popupItem != nullptr, return false);
  if (!popupItem->isVisible() || !popupItem->inherits(kPopupItemClass))
    return false;

  const QObject* popup = popupItem->parent();
  return popup != nullptr && popup->property("modal").toBool();
}

//--------------------------------------------------------------------------------------------------
// Window list
//--------------------------------------------------------------------------------------------------

/**
 * @brief The visible Qt Quick windows in top-level order; a headless root has none.
 */
QList<QQuickWindow*> UI::Automation::WindowInventory::windows()
{
  QList<QQuickWindow*> result;
  const auto all = QGuiApplication::topLevelWindows();
  for (QWindow* window : all) {
    auto* quick = qobject_cast<QQuickWindow*>(window);
    if (quick != nullptr && quick->isVisible())
      result.append(quick);
  }

  return result;
}

/**
 * @brief The visible window named @p id, "main" for an empty id; an empty id also falls back to
 *        the first visible window, so a single-window host needs no name.
 */
QQuickWindow* UI::Automation::WindowInventory::resolve(const QString& id)
{
  const auto list      = windows();
  const QString wanted = id.isEmpty() ? QString::fromLatin1(kDefaultWindowId) : id;
  for (QQuickWindow* window : list)
    if (window->objectName() == wanted)
      return window;

  if (id.isEmpty() && !list.isEmpty())
    return list.first();

  return nullptr;
}

//--------------------------------------------------------------------------------------------------
// Window description
//--------------------------------------------------------------------------------------------------

/**
 * @brief Geometry, scale, activation, modality and focus of @p window, as the API reports them.
 */
UI::Automation::WindowInfo UI::Automation::WindowInventory::describe(QQuickWindow* window)
{
  WindowInfo info;
  info.scale  = 1.0;
  info.active = false;
  info.modal  = false;
  SS_ASSERT(window != nullptr, return info);

  info.id        = window->objectName();
  info.title     = window->title();
  info.frame     = window->frameGeometry();
  info.scale     = window->devicePixelRatio();
  info.active    = window->isActive();
  info.modal     = hasModalPopup(window);
  info.focusedId = focusedId(window);
  return info;
}

/**
 * @brief Whether a modal popup is open: a visible popup item under the window's overlay whose
 *        popup object reports modal.
 */
bool UI::Automation::WindowInventory::hasModalPopup(QQuickWindow* window)
{
  SS_ASSERT(window != nullptr, return false);
  const QQuickItem* content = window->contentItem();
  SS_ASSERT(content != nullptr, return false);

  const auto roots = content->childItems();
  for (const QQuickItem* root : roots) {
    if (!root->inherits(kOverlayClass))
      continue;

    const auto popups = root->childItems();
    for (const QQuickItem* popupItem : popups)
      if (isModalPopupItem(popupItem))
        return true;
  }

  return false;
}

/**
 * @brief The identifier of the active focus item or of its nearest named ancestor.
 */
QString UI::Automation::WindowInventory::focusedId(QQuickWindow* window)
{
  SS_ASSERT(window != nullptr, return QString());
  return nearestId(window->activeFocusItem());
}

/**
 * @brief The objectName of @p item or of its nearest ancestor that has one; empty past the
 *        ancestor cap or at the root.
 */
QString UI::Automation::WindowInventory::nearestId(const QQuickItem* item)
{
  const QQuickItem* current = item;
  for (int depth = 0; depth < kMaxAncestors && current != nullptr; ++depth) {
    if (!current->objectName().isEmpty())
      return current->objectName();

    current = current->parentItem();
  }

  return QString();
}
