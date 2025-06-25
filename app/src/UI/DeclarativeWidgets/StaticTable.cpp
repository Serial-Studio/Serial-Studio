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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "StaticTable.h"
#include "Misc/ThemeManager.h"

#include <QHeaderView>
#include <QLabel>
#include <QScrollBar>
#include <QToolButton>

/**
 * Constructor function
 */
StaticTable::StaticTable(QQuickItem *parent)
  : DeclarativeWidget(parent)
{
  // Set widget to render
  setWidget(&m_widget);

  // Get fonts
  m_font = m_widget.font();
  m_headerFont = m_widget.horizontalHeader()->font();

  // Configure widget style
  m_widget.setGridStyle(Qt::DashLine);
  m_widget.verticalHeader()->setVisible(false);
  m_widget.setSelectionMode(QAbstractItemView::NoSelection);
  m_widget.setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Disable scrollbars
  m_widget.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_widget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // React to theme changes
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &StaticTable::loadTheme, Qt::DirectConnection);

  // Redraw UI
  loadTheme();
  redraw();
}

/**
 * Returns the font used by the table widget to display data
 */
const QFont &StaticTable::font() const
{
  return m_font;
}

/**
 * Returns the font used by the table widget's header items
 */
const QFont &StaticTable::headerFont() const
{
  return m_headerFont;
}

/**
 * Returns the data model used to generate the widget.
 */
const QList<QStringList> &StaticTable::data() const
{
  return m_data;
}

/**
 * Updates the font used for the table widget to display data
 */
void StaticTable::setFont(const QFont &font)
{
  m_font = font;
  m_widget.setFont(font);

  Q_EMIT fontChanged();
}

/**
 * Updates the font used for the table widget's column headers
 */
void StaticTable::setHeaderFont(const QFont &font)
{
  m_headerFont = font;
  m_widget.horizontalHeader()->setFont(font);
  Q_EMIT headerFontChanged();
}

/**
 * Updates the data visualized by the widget.
 */
void StaticTable::setData(const QList<QStringList> &data)
{
  // Clear data
  m_data = data;
  m_widget.setRowCount(0);
  m_widget.setColumnCount(0);

  // Import data into table
  if (data.count() > 0)
  {
    m_widget.setColumnCount(data.first().count());
    m_widget.setHorizontalHeaderLabels(data.first());
    m_widget.horizontalHeader()->setMinimumSectionSize(128);
    for (int i = 0; i < m_widget.columnCount(); ++i)
    {
      if (i < m_widget.columnCount() - 1)
        m_widget.horizontalHeader()->setSectionResizeMode(
            i, QHeaderView::ResizeToContents);
      else
        m_widget.horizontalHeader()->setSectionResizeMode(i,
                                                          QHeaderView::Stretch);
    }

    for (int i = 0; i < data.count() - 1; ++i)
    {
      m_widget.insertRow(i);
      const auto &row = data[i + 1];
      for (int j = 0; j < row.count(); ++j)
      {
        auto *item = new QTableWidgetItem(" " + row[j] + " ");
        item->setTextAlignment(Qt::AlignCenter);
        item->setFont(m_font);
        m_widget.setItem(i, j, item);
      }

      m_widget.setRowHeight(i, 32);
    }
  }

  // Compute minimum width
  int contentWidth = 0;
  for (int i = 0; i < m_widget.columnCount(); ++i)
    contentWidth += m_widget.columnWidth(i);

  // Compute minimum height
  int contentHeight = m_widget.horizontalHeader()->height();
  for (int i = 0; i < m_widget.rowCount(); ++i)
    contentHeight += m_widget.rowHeight(i);

  // Tell QML interface about content size
  setContentWidth(contentWidth);
  setContentHeight(contentHeight);

  // Update user interface
  requestUpdate();
  redraw();
}

/**
 * Set's the widgets theme based on the current theme manager values
 */
void StaticTable::loadTheme()
{
  QPalette p;
  auto c = Misc::ThemeManager::instance().colors();

  // clang-format off
  p.setColor(QPalette::Base, QColor(c.value("base").toString()));
  p.setColor(QPalette::Text, QColor(c.value("text").toString()));
  p.setColor(QPalette::Button, QColor(c.value("button").toString()));
  p.setColor(QPalette::Window, QColor(c.value("window").toString()));
  p.setColor(QPalette::WindowText, QColor(c.value("text").toString()));
  p.setColor(QPalette::AlternateBase, QColor(c.value("base").toString()));
  p.setColor(QPalette::ButtonText, QColor(c.value("button_text").toString()));
  p.setColor(QPalette::Highlight, QColor(c.value("table_highlight").toString()));
  p.setColor(QPalette::PlaceholderText, QColor(c.value("placeholder_text").toString()));
  p.setColor(QPalette::HighlightedText, QColor(c.value("table_highlighted_text").toString()));
  // clang-format on

  m_widget.setPalette(p);
  requestUpdate();
}
