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

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>
#include <QToolButton>

#include "Misc/ThemeManager.h"

//--------------------------------------------------------------------------------------------------
// Placeholder-aware item delegate
//--------------------------------------------------------------------------------------------------

namespace {
/**
 * @brief Item delegate that draws a placeholder string in cells whose value is empty.
 */
class PlaceholderDelegate : public QStyledItemDelegate {
public:
  /**
   * @brief Builds the delegate, capturing a non-owning pointer to the placeholder text source.
   */
  explicit PlaceholderDelegate(const QString* text, QObject* parent = nullptr)
    : QStyledItemDelegate(parent), m_text(text)
  {}

  /**
   * @brief Paints the cell, substituting the placeholder string when the cell value is empty.
   */
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override
  {
    // Defer to base painting when the cell already has text or no placeholder is set
    const QString cell = index.data(Qt::DisplayRole).toString();
    if (!cell.isEmpty() || !m_text || m_text->isEmpty()) {
      QStyledItemDelegate::paint(painter, option, index);
      return;
    }

    // Paint background + frame using the current style
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();
    const auto* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    // Draw the placeholder text using the placeholder palette role
    painter->save();
    painter->setPen(opt.palette.color(QPalette::PlaceholderText));
    painter->drawText(opt.rect, opt.displayAlignment, *m_text);
    painter->restore();
  }

private:
  const QString* m_text;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// Constructor & setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the static table widget, installs the placeholder delegate, and applies the current
 * theme.
 */
StaticTable::StaticTable(QQuickItem* parent) : DeclarativeWidget(parent)
{
  setWidget(&m_widget);
  m_font       = m_widget.font();
  m_headerFont = m_widget.horizontalHeader()->font();

  // Configure table appearance
  m_widget.setGridStyle(Qt::DashLine);
  m_widget.verticalHeader()->setVisible(false);
  m_widget.setSelectionMode(QAbstractItemView::NoSelection);
  m_widget.setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_widget.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_widget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Install delegate that draws placeholder text in empty cells
  m_widget.setItemDelegate(new PlaceholderDelegate(&m_placeholderText, &m_widget));

  // React to theme changes
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &StaticTable::loadTheme,
          Qt::DirectConnection);

  loadTheme();
  redraw();
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * Returns the font used by the table widget to display data
 */
const QFont& StaticTable::font() const
{
  return m_font;
}

/**
 * Returns the font used by the table widget's header items
 */
const QFont& StaticTable::headerFont() const
{
  return m_headerFont;
}

/**
 * Returns the placeholder text shown in cells whose value is empty.
 */
const QString& StaticTable::placeholderText() const
{
  return m_placeholderText;
}

/**
 * Returns the data model used to generate the widget.
 */
const QList<QStringList>& StaticTable::data() const
{
  return m_data;
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the table-cell font and triggers a redraw when the value changes.
 */
void StaticTable::setFont(const QFont& font)
{
  if (m_font == font)
    return;

  m_font = font;
  m_widget.setFont(font);
  requestUpdate();
  redraw();

  Q_EMIT fontChanged();
}

/**
 * @brief Updates the header font and triggers a redraw when the value changes.
 */
void StaticTable::setHeaderFont(const QFont& font)
{
  if (m_headerFont == font)
    return;

  m_headerFont = font;
  m_widget.horizontalHeader()->setFont(font);
  requestUpdate();
  redraw();

  Q_EMIT headerFontChanged();
}

/**
 * @brief Updates the placeholder string drawn in empty cells and refreshes the rendered table.
 */
void StaticTable::setPlaceholderText(const QString& text)
{
  if (m_placeholderText == text)
    return;

  m_placeholderText = text;
  setData(m_data);

  Q_EMIT placeholderTextChanged();
}

/**
 * @brief Replaces the table model, rebuilding rows/columns and recomputing scroll-content size.
 */
void StaticTable::setData(const QList<QStringList>& data)
{
  m_data = data;
  m_widget.setRowCount(0);
  m_widget.setColumnCount(0);

  // Populate table from data model
  if (data.count() > 0) {
    m_widget.setColumnCount(data.first().count());
    m_widget.setHorizontalHeaderLabels(data.first());
    m_widget.horizontalHeader()->setMinimumSectionSize(128);
    for (int i = 0; i < m_widget.columnCount(); ++i)
      if (i < m_widget.columnCount() - 1)
        m_widget.horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
      else
        m_widget.horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);

    for (int i = 0; i < data.count() - 1; ++i) {
      m_widget.insertRow(i);
      const auto& row = data[i + 1];
      for (int j = 0; j < row.count(); ++j) {
        const QString& cell = row[j];
        auto* item          = new QTableWidgetItem(cell.isEmpty() ? QString() : " " + cell + " ");
        item->setTextAlignment(Qt::AlignCenter);
        item->setFont(m_font);
        m_widget.setItem(i, j, item);
      }

      m_widget.setRowHeight(i, 32);
    }
  }

  // Calculate minimum content dimensions for QML scrolling
  int contentWidth = 0;
  for (int i = 0; i < m_widget.columnCount(); ++i)
    contentWidth += m_widget.columnWidth(i);

  int contentHeight = m_widget.horizontalHeader()->height();
  for (int i = 0; i < m_widget.rowCount(); ++i)
    contentHeight += m_widget.rowHeight(i);

  setContentWidth(contentWidth);
  setContentHeight(contentHeight);
  requestUpdate();
  redraw();
}

//--------------------------------------------------------------------------------------------------
// Theme management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the table palette from the current ThemeManager colors and applies it.
 */
void StaticTable::loadTheme()
{
  // Build a new palette from the current theme colors
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
  redraw();
}
