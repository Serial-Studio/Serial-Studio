/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QResizeEvent>
#include <QRegularExpression>

#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/DataGrid.h"

#define DG_EXEC_EVENT(pointer, function, event)                                \
  if (pointer)                                                                 \
  {                                                                            \
    class PwnedWidget : public QScrollArea                                     \
    {                                                                          \
    public:                                                                    \
      using QScrollArea::function;                                             \
    };                                                                         \
    static_cast<PwnedWidget *>(pointer)->function(event);                      \
  }

/**
 * Generates the user interface elements & layout
 */
Widgets::DataGrid::DataGrid(const int index)
  : m_index(index)
{
  // Get pointers to serial studio modules
  auto dash = &UI::Dashboard::instance();

  // Invalid index, abort initialization
  if (m_index < 0 || m_index >= dash->datagridCount())
    return;

  // Get group reference
  auto group = dash->getDataGrid(m_index);

  // Configure scroll area container
  m_dataContainer = new QWidget(this);

  // Make the value label larger
  auto valueFont = dash->monoFont();
  valueFont.setPixelSize(dash->monoFont().pixelSize() * 1.3);

  // Configure grid layout
  m_units.reserve(group.datasetCount());
  m_icons.reserve(group.datasetCount());
  m_titles.reserve(group.datasetCount());
  m_values.reserve(group.datasetCount());
  m_gridLayout = new QGridLayout(m_dataContainer);
  for (int dataset = 0; dataset < group.datasetCount(); ++dataset)
  {
    // Create labels
    m_units.append(new QLabel(m_dataContainer));
    m_icons.append(new QLabel(m_dataContainer));
    m_titles.append(new ElidedLabel(m_dataContainer));
    m_values.append(new ElidedLabel(m_dataContainer));

    // Get pointers to labels
    auto dicon = m_icons.last();
    auto units = m_units.last();
    auto title = m_titles.last();
    auto value = m_values.last();

    // Set elide modes for title & value fields
    title->setType(Qt::ElideRight);
    value->setType(Qt::ElideRight);

    // Set label alignments
    units->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    title->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dicon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Set label styles & fonts
    value->setFont(valueFont);
    title->setFont(dash->monoFont());
    units->setFont(dash->monoFont());

    // Set label initial data
    auto set = group.getDataset(dataset);
    title->setText(set.title());
    if (!set.units().isEmpty())
      units->setText(QString("[%1]").arg(set.units()));

    // Set icon text
    dicon->setText("⤑");

    // Add labels to grid layout
    m_gridLayout->addWidget(title, dataset, 0);
    m_gridLayout->addWidget(dicon, dataset, 1);
    m_gridLayout->addWidget(value, dataset, 2);
    m_gridLayout->addWidget(units, dataset, 3);
  }

  // Load layout into container widget
  m_gridLayout->setColumnStretch(0, 2);
  m_gridLayout->setColumnStretch(1, 1);
  m_gridLayout->setColumnStretch(2, 2);
  m_gridLayout->setColumnStretch(3, 0);
  m_dataContainer->setLayout(m_gridLayout);

  // Configure scroll area
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setWidget(m_dataContainer);
  m_scrollArea->setFrameShape(QFrame::NoFrame);
  m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Configure main layout
  m_mainLayout = new QVBoxLayout(this);
  m_mainLayout->addWidget(m_scrollArea);
  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_mainLayout);

  // Set visual style
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::DataGrid::onThemeChanged);

  // React to dashboard events
  connect(dash, SIGNAL(updated()), this, SLOT(updateData()),
          Qt::QueuedConnection);
}

/**
 * Frees the memory allocated for each label that represents a dataset
 */
Widgets::DataGrid::~DataGrid()
{
  Q_FOREACH (auto icon, m_icons)
    delete icon;

  Q_FOREACH (auto title, m_titles)
    delete title;

  Q_FOREACH (auto value, m_values)
    delete value;

  Q_FOREACH (auto units, m_units)
    delete units;

  delete m_gridLayout;
  delete m_scrollArea;
  delete m_mainLayout;
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Widgets::DataGrid::updateData()
{
  // Widget not enabled, do nothing
  if (!isEnabled())
    return;

  // Invalid index, abort update
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->datagridCount())
    return;

  // Get group reference
  auto group = dash->getDataGrid(m_index);

  // Regular expresion handler
  static const QRegularExpression regex("^[+-]?(\\d*\\.)?\\d+$");

  // Update labels
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    // Get dataset value
    auto value = group.getDataset(i).value();

    // Check if value is a number, if so make sure that
    // we always show a fixed number of decimal places
    if (regex.match(value).hasMatch())
      value = QString::number(value.toDouble(), 'f', dash->precision());

    // Update label
    if (m_values.count() > i)
      m_values.at(i)->setText(value + " ");
  }
}

/**
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::DataGrid::onThemeChanged()
{
  // Generate widget stylesheets
  // clang-format off
  auto theme = &Misc::ThemeManager::instance();
  const auto valueQSS = QSS("color:%1", theme->getColor(QStringLiteral("widget_text")));
  const auto titleQSS = QSS("color:%1", theme->getColor(QStringLiteral("widget_text")));
  const auto unitsQSS = QSS("color:%1", theme->getColor(QStringLiteral("widget_highlight")));
  const auto iconsQSS = QSS("color:%1; font-weight:600;", theme->getColor(QStringLiteral("widget_highlight")));
  // clang-format on

  // Set palette
  // clang-format off
  QPalette palette;
  palette.setColor(QPalette::Base, theme->getColor(QStringLiteral("widget_base")));
  palette.setColor(QPalette::Window, theme->getColor(QStringLiteral("widget_window")));
  setPalette(palette);
  // clang-format on

  // Update styles for each label
  for (int i = 0; i < m_titles.count(); ++i)
  {
    auto dicon = m_icons.at(i);
    auto units = m_units.at(i);
    auto title = m_titles.at(i);
    auto value = m_values.at(i);
    title->setStyleSheet(titleQSS);
    value->setStyleSheet(valueQSS);
    units->setStyleSheet(unitsQSS);
    dicon->setStyleSheet(iconsQSS);
  }

  // Redraw widget
  update();
}

/**
 * Changes the size of the labels when the widget is resized
 */
void Widgets::DataGrid::resizeEvent(QResizeEvent *event)
{
  auto width = event->size().width();
  QFont font = UI::Dashboard::instance().monoFont();
  QFont icon = font;
  QFont valueFont = font;
  icon.setPixelSize(qMax(8, width / 16));
  font.setPixelSize(qMax(8, width / 24));
  valueFont.setPixelSize(font.pixelSize() * 1.3);

  for (int i = 0; i < m_titles.count(); ++i)
  {
    m_units.at(i)->setFont(font);
    m_icons.at(i)->setFont(icon);
    m_titles.at(i)->setFont(font);
    m_values.at(i)->setFont(valueFont);
  }

  event->accept();
}

void Widgets::DataGrid::wheelEvent(QWheelEvent *event)
{
  DG_EXEC_EVENT(m_scrollArea, wheelEvent, event);
}

void Widgets::DataGrid::mouseMoveEvent(QMouseEvent *event)
{
  DG_EXEC_EVENT(m_scrollArea, mouseMoveEvent, event);
}

void Widgets::DataGrid::mousePressEvent(QMouseEvent *event)
{
  DG_EXEC_EVENT(m_scrollArea, mousePressEvent, event);
}

void Widgets::DataGrid::mouseReleaseEvent(QMouseEvent *event)
{
  DG_EXEC_EVENT(m_scrollArea, mouseReleaseEvent, event);
}

void Widgets::DataGrid::mouseDoubleClickEvent(QMouseEvent *event)
{
  DG_EXEC_EVENT(m_scrollArea, mouseDoubleClickEvent, event);
}
