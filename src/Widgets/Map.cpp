/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#include "Map.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

using namespace Widgets;

/**
 * Generates the user interface elements & layout
 */
Map::Map(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->mapCount())
        return;

    // Set window palette
    QPalette palette;
    palette.setColor(QPalette::Base, theme->widgetWindowBackground());
    palette.setColor(QPalette::Window, theme->widgetWindowBackground());
    setPalette(palette);

    // Configure layout
    m_layout.setContentsMargins(12, 12, 12, 12);
    setLayout(&m_layout);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Map::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Get group pointer
    auto dash = UI::Dashboard::getInstance();
    auto group = dash->getGroups(m_index);
    if (!group)
        return;

    // Get latitiude/longitude from datasets
    qreal lat = 0;
    qreal lon = 0;
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        auto dataset = group->getDataset(i);
        if (dataset)
        {
            if (dataset->widget() == "lat")
                lat = dataset->value().toDouble();
            else if (dataset->widget() == "lon")
                lon = dataset->value().toDouble();
        }
    }

    // Update map coordinates
}
