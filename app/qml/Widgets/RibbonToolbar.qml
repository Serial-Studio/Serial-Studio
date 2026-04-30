/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

/**
 * @brief A Microsoft Office-style ribbon toolbar with collapsible sections
 *        and horizontal scrolling.
 *
 * Sections with `collapsible: true` will automatically collapse to a single
 * icon+dropdown button when the toolbar is too narrow. Collapse order is
 * determined by each section's `collapsePriority` (lowest collapses first).
 *
 * When all collapsible sections are folded and content still doesn't fit,
 * horizontal scrolling is enabled.
 */
Item {
  id: root

  //
  // Default content items go into the RowLayout
  //
  default property alias sections: sectionLayout.data

  //
  // Flickable wrapping the section row
  //
  Flickable {
    id: flickable

    clip: true
    anchors.fill: parent
    contentHeight: height
    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.HorizontalFlick
    contentWidth: sectionLayout.implicitWidth + 16

    ScrollBar.horizontal: ScrollBar {
      height: 3
      policy: flickable.contentWidth > flickable.width
              ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    }

    RowLayout {
      id: sectionLayout

      width: implicitWidth
      anchors.verticalCenter: parent.verticalCenter

      Item {
        implicitWidth: 4
      }
    }
  }

  //
  // Collapse logic — runs whenever width changes
  //
  property bool _updating: false

  onWidthChanged: Qt.callLater(updateCollapse)
  Component.onCompleted: Qt.callLater(updateCollapse)

  function updateCollapse() {
    if (width <= 0 || _updating)
      return

    _updating = true

    var available = width

    // Collect all collapsible sections
    var sections = []
    for (var i = 0; i < sectionLayout.children.length; ++i) {
      var child = sectionLayout.children[i]
      if (child.collapsible !== undefined && child.collapsible)
        sections.push(child)
    }

    // Calculate total width if everything were expanded
    var totalExpanded = 16  // margins
    for (var a = 0; a < sectionLayout.children.length; ++a) {
      var item = sectionLayout.children[a]
      if (item.collapsible !== undefined && item.collapsible)
        totalExpanded += item.expandedWidth
      else if (item.implicitWidth !== undefined)
        totalExpanded += item.implicitWidth + 4
    }

    // If everything fits expanded, expand all
    if (totalExpanded <= available) {
      for (var j = 0; j < sections.length; ++j)
        sections[j].collapsed = false

      _updating = false
      return
    }

    // Sort by collapsePriority ascending (lowest collapses first).
    sections.sort(function(a, b) {
      return a.collapsePriority - b.collapsePriority
    })

    // Start with everything expanded, then collapse one by one
    var currentWidth = totalExpanded
    for (var k = 0; k < sections.length; ++k)
      sections[k].collapsed = false

    for (var c = 0; c < sections.length; ++c) {
      if (currentWidth <= available)
        break

      // Collapsing saves: expandedWidth - collapsedWidth
      var savings = sections[c].expandedWidth - sections[c].collapsedWidth
      sections[c].collapsed = true
      currentWidth -= savings
    }

    _updating = false
  }

  //
  // Left fade indicator (shows when scrolled right)
  //
  Rectangle {
    z: 10
    width: 16
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    visible: flickable.contentX > 4

    gradient: Gradient {
      orientation: Gradient.Horizontal

      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["toolbar_top"]
      }

      GradientStop {
        position: 1
        color: "transparent"
      }
    }
  }

  //
  // Right fade indicator (shows when more content to the right)
  //
  Rectangle {
    z: 10
    width: 16
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    visible: flickable.contentX + flickable.width < flickable.contentWidth - 4

    gradient: Gradient {
      orientation: Gradient.Horizontal

      GradientStop {
        position: 0
        color: "transparent"
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["toolbar_top"]
      }
    }
  }
}
