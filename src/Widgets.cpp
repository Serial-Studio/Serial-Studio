/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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

#include "Group.h"
#include "Dataset.h"
#include "Widgets.h"
#include "QmlBridge.h"

#include <cfloat>
#include <climits>

static Widgets *INSTANCE = Q_NULLPTR;

Widgets::Widgets()
{
   auto bridge = QmlBridge::getInstance();
   connect(bridge, SIGNAL(updated()), this, SLOT(updateModels()));
}

Widgets *Widgets::getInstance()
{
   if (!INSTANCE)
      INSTANCE = new Widgets;

   return INSTANCE;
}

QList<Group *> Widgets::barGroup() const
{
   return m_barGroups;
}

QList<Group *> Widgets::mapGroup() const
{
   return m_mapGroups;
}

QList<Group *> Widgets::gyroGroup() const
{
   return m_gyroGroups;
}

QList<Group *> Widgets::tankGroup() const
{
   return m_tankGroups;
}

QList<Group *> Widgets::gaugeGroup() const
{
   return m_gaugeGroups;
}

QList<Group *> Widgets::accelerometerGroup() const
{
   return m_accelerometerGroups;
}

int Widgets::barGroupCount() const
{
   return barGroup().count();
}

int Widgets::mapGroupCount() const
{
   return mapGroup().count();
}

int Widgets::gyroGroupCount() const
{
   return gyroGroup().count();
}

int Widgets::tankGroupCount() const
{
   return tankGroup().count();
}

int Widgets::gaugeGroupCount() const
{
   return gaugeGroup().count();
}

int Widgets::accelerometerGroupCount() const
{
   return accelerometerGroup().count();
}

Group *Widgets::barGroupAt(const int index)
{
   if (barGroup().count() > index)
      return barGroup().at(index);

   return Q_NULLPTR;
}

Group *Widgets::mapGroupAt(const int index)
{
   if (mapGroup().count() > index)
      return mapGroup().at(index);

   return Q_NULLPTR;
}

Group *Widgets::gyroGroupAt(const int index)
{
   if (gyroGroup().count() > index)
      return gyroGroup().at(index);

   return Q_NULLPTR;
}

Group *Widgets::tankGroupAt(const int index)
{
   if (tankGroup().count() > index)
      return tankGroup().at(index);

   return Q_NULLPTR;
}

Group *Widgets::gaugeGroupAt(const int index)
{
   if (gaugeGroup().count() > index)
      return gaugeGroup().at(index);

   return Q_NULLPTR;
}

Group *Widgets::accelerometerGroupAt(const int index)
{
   if (accelerometerGroup().count() > index)
      return accelerometerGroup().at(index);

   return Q_NULLPTR;
}

double Widgets::gyroX(const int index)
{
   auto gyro = gyroGroupAt(index);

   if (gyro)
   {
      foreach (auto dataset, gyro->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "x")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::gyroY(const int index)
{
   auto gyro = gyroGroupAt(index);

   if (gyro)
   {
      foreach (auto dataset, gyro->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "y")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::gyroZ(const int index)
{
   auto gyro = gyroGroupAt(index);

   if (gyro)
   {
      foreach (auto dataset, gyro->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "z")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::accelerometerX(const int index)
{
   auto accelerometer = accelerometerGroupAt(index);

   if (accelerometer)
   {
      foreach (auto dataset, accelerometer->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "x")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::accelerometerY(const int index)
{
   auto accelerometer = accelerometerGroupAt(index);

   if (accelerometer)
   {
      foreach (auto dataset, accelerometer->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "y")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::accelerometerZ(const int index)
{
   auto accelerometer = accelerometerGroupAt(index);

   if (accelerometer)
   {
      foreach (auto dataset, accelerometer->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "z")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::bar(const int index)
{
   auto bar = barGroupAt(index);

   if (bar)
   {
      foreach (auto dataset, bar->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "value")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::tank(const int index)
{
   auto tank = tankGroupAt(index);

   if (tank)
   {
      foreach (auto dataset, tank->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "value")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::gauge(const int index)
{
   auto gauge = gaugeGroupAt(index);

   if (gauge)
   {
      foreach (auto dataset, gauge->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "value")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::barMin(const int index)
{
   auto bar = barGroupAt(index);

   if (bar)
   {
      foreach (auto dataset, bar->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "min")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::barMax(const int index)
{
   auto bar = barGroupAt(index);

   if (bar)
   {
      foreach (auto dataset, bar->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "max")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::tankMin(const int index)
{
   auto tank = tankGroupAt(index);

   if (tank)
   {
      foreach (auto dataset, tank->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "min")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::tankMax(const int index)
{
   auto tank = tankGroupAt(index);

   if (tank)
   {
      foreach (auto dataset, tank->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "max")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::gaugeMin(const int index)
{
   auto gauge = gaugeGroupAt(index);

   if (gauge)
   {
      foreach (auto dataset, gauge->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "min")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::gaugeMax(const int index)
{
   auto gauge = gaugeGroupAt(index);

   if (gauge)
   {
      foreach (auto dataset, gauge->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "max")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::mapLatitude(const int index)
{
   auto map = mapGroupAt(index);

   if (map)
   {
      foreach (auto dataset, map->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "lat")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

double Widgets::mapLongitude(const int index)
{
   auto map = mapGroupAt(index);

   if (map)
   {
      foreach (auto dataset, map->datasets())
      {
         auto widget = dataset->widget();
         if (widget.toLower() == "lon")
            return dataset->value().toDouble();
      }
   }

   return DBL_MAX;
}

void Widgets::updateModels()
{
   // Clear current groups
   m_barGroups.clear();
   m_mapGroups.clear();
   m_gyroGroups.clear();
   m_tankGroups.clear();
   m_gaugeGroups.clear();
   m_accelerometerGroups.clear();

   // Update groups
   m_barGroups = getWidgetGroup("bar");
   m_mapGroups = getWidgetGroup("map");
   m_gyroGroups = getWidgetGroup("gyro");
   m_tankGroups = getWidgetGroup("tank");
   m_gaugeGroups = getWidgetGroup("gauge");
   m_accelerometerGroups = getWidgetGroup("accelerometer");

   // Update UI
   emit dataChanged();
}

QList<Group *> Widgets::getWidgetGroup(const QString &handle)
{
   QList<Group *> widgetGroup;

   foreach (auto group, QmlBridge::getInstance()->groups())
   {
      if (group->widget().toLower() == handle)
         widgetGroup.append(group);
   }

   return widgetGroup;
}
