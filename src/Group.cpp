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

Group::Group(QObject *parent)
   : QObject(parent)
{
   m_widget = "";
   m_title = tr("Invalid");
}

/**
 * Destructor function
 */
Group::~Group()
{
   for (int i = 0; i < count(); ++i)
      m_datasets.at(i)->deleteLater();

   m_datasets.clear();
}

/**
 * @return The number of datasets inside this group
 */
int Group::count() const
{
   return datasets().count();
}

/**
 * @return The title/description of this group
 */
QString Group::title() const
{
   return m_title;
}

/**
 * @return The widget type of this group (if any)
 */
QString Group::widget() const
{
    return m_widget;
}

/**
 * @return A list with all the dataset objects contained in this group
 */
QList<Dataset *> Group::datasets() const
{
   return m_datasets;
}

/**
 * @return The dataset object at the given @a index,
 *         returns @c Q_NULLPTR on invalid index
 */
Dataset *Group::getDataset(const int index)
{
   if (index < count() && index >= 0)
      return m_datasets.at(index);

   return Q_NULLPTR;
}

/**
 * Reads the group information and all its asociated datasets
 * from the given JSON @c object.
 *
 * @return @c true on success, @c false on failure
 */
bool Group::read(const QJsonObject &object)
{
   if (!object.isEmpty())
   {
      auto array = object.value("d").toArray();
      auto title = object.value("t").toVariant().toString();
      auto widget = object.value("w").toVariant().toString();

      if (!title.isEmpty() && !array.isEmpty())
      {
         m_title = title;
         m_widget = widget;
         m_datasets.clear();

         for (auto i = 0; i < array.count(); ++i)
         {
            auto object = array.at(i).toObject();
            if (!object.isEmpty())
            {
               auto dataset = new Dataset(this);
               if (dataset->read(object))
                  m_datasets.append(dataset);
               else
                  dataset->deleteLater();
            }
         }

         return count() > 0;
      }
   }

   return false;
}
