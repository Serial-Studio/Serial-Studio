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

#pragma once

#include <QObject>
#include <DataTypes.h>
#include <JSON/Group.h>
#include <JSON/Dataset.h>

namespace JSON
{
/**
 * @brief The Editor class
 *
 * The editor class allows the representation of a Serial Studio JSON
 * project file in the graphical user interface.
 *
 * Additionaly, the class facilitates the modificiation of a project
 * file or the creation of new JSON project files.
 */
class Editor : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(int groupCount
               READ groupCount
               NOTIFY groupCountChanged)
    Q_PROPERTY(bool modified
               READ modified
               NOTIFY modifiedChanged)
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    Q_PROPERTY(QString separator
               READ separator
               WRITE setSeparator
               NOTIFY separatorChanged)
    Q_PROPERTY(QString frameEndSequence
               READ frameEndSequence
               WRITE setFrameEndSequence
               NOTIFY frameEndSequenceChanged)
    Q_PROPERTY(QString frameStartSequence
               READ frameStartSequence
               WRITE setFrameStartSequence
               NOTIFY frameStartSequenceChanged)
    Q_PROPERTY(QString jsonFilePath
               READ jsonFilePath
               NOTIFY jsonFileChanged)
    Q_PROPERTY(QString jsonFileName
               READ jsonFileName
               NOTIFY jsonFileChanged)
    // clang-format on

Q_SIGNALS:
    void titleChanged();
    void jsonFileChanged();
    void modifiedChanged();
    void separatorChanged();
    void groupCountChanged();
    void groupOrderChanged();
    void frameEndSequenceChanged();
    void frameStartSequenceChanged();
    void groupChanged(const int group);
    void datasetChanged(const int group, const int dataset);

private:
    explicit Editor();
    Editor(Editor &&) = delete;
    Editor(const Editor &) = delete;
    Editor &operator=(Editor &&) = delete;
    Editor &operator=(const Editor &) = delete;

public:
    static Editor &instance();

    Q_INVOKABLE StringList availableGroupLevelWidgets();
    Q_INVOKABLE StringList availableDatasetLevelWidgets();

    QString jsonProjectsPath() const;

    QString title() const;
    QString separator() const;
    QString frameEndSequence() const;
    QString frameStartSequence() const;

    bool modified() const;
    int groupCount() const;
    QString jsonFilePath() const;
    QString jsonFileName() const;

    Q_INVOKABLE bool askSave();
    Q_INVOKABLE bool saveJsonFile();
    Q_INVOKABLE int datasetCount(const int group) const;

    Q_INVOKABLE const Group &getGroup(const int index) const;
    Q_INVOKABLE const JSON::Dataset &getDataset(const int group, const int index) const;

    Q_INVOKABLE QString groupTitle(const int group) const;
    Q_INVOKABLE QString groupWidget(const int group) const;
    Q_INVOKABLE int groupWidgetIndex(const int group) const;
    Q_INVOKABLE int datasetIndex(const int group, const int dataset) const;
    Q_INVOKABLE bool datasetLED(const int group, const int dataset) const;
    Q_INVOKABLE bool datasetGraph(const int group, const int dataset) const;
    Q_INVOKABLE bool datasetFftPlot(const int group, const int dataset) const;
    Q_INVOKABLE bool datasetLogPlot(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetTitle(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetUnits(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetWidget(const int group, const int dataset) const;
    Q_INVOKABLE int datasetWidgetIndex(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetWidgetMin(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetWidgetMax(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetFFTSamples(const int group, const int dataset) const;
    Q_INVOKABLE QString datasetWidgetAlarm(const int group, const int dataset) const;

    Q_INVOKABLE bool setGroupWidget(const int group, const int widgetId);

public Q_SLOTS:
    void newJsonFile();
    void openJsonFile();
    void openJsonFile(const QString &path);

    void setTitle(const QString &title);
    void setSeparator(const QString &separator);
    void setFrameEndSequence(const QString &sequence);
    void setFrameStartSequence(const QString &sequence);

    void addGroup();
    void deleteGroup(const int group);
    void moveGroupUp(const int group);
    void moveGroupDown(const int group);
    void setGroupTitle(const int group, const QString &title);
    void setGroupWidgetData(const int group, const QString &widget);

    void addDataset(const int group);
    void deleteDataset(const int group, const int dataset);
    void setDatasetWidget(const int group, const int dataset, const int widgetId);
    void setDatasetTitle(const int group, const int dataset, const QString &title);
    void setDatasetUnits(const int group, const int dataset, const QString &units);
    void setDatasetIndex(const int group, const int dataset, const int frameIndex);
    void setDatasetLED(const int group, const int dataset, const bool generateLED);
    void setDatasetGraph(const int group, const int dataset, const bool generateGraph);
    void setDatasetFftPlot(const int group, const int dataset, const bool generateFft);
    void setDatasetLogPlot(const int group, const int dataset, const bool generateLog);
    void setDatasetWidgetMin(const int group, const int dataset, const QString &minimum);
    void setDatasetWidgetMax(const int group, const int dataset, const QString &maximum);
    void setDatasetWidgetData(const int group, const int dataset, const QString &widget);
    void setDatasetWidgetAlarm(const int group, const int dataset, const QString &alarm);
    void setDatasetFFTSamples(const int group, const int dataset, const QString &samples);

private Q_SLOTS:
    void onJsonLoaded();
    void onModelChanged();
    void onGroupChanged(const int group);
    void setModified(const bool modified);
    void onDatasetChanged(const int group, const int dataset);

private:
    int nextDatasetIndex();

private:
    QString m_title;
    QString m_separator;
    QString m_frameEndSequence;
    QString m_frameStartSequence;

    bool m_modified;
    QString m_filePath;

    QVector<Group> m_groups;
};
}
