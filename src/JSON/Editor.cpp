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

#include "Editor.h"
#include "Generator.h"
#include "Misc/Utilities.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

using namespace JSON;
static Editor *INSTANCE = nullptr;

//--------------------------------------------------------------------------------------------------
// Constructor/deconstructor & singleton
//--------------------------------------------------------------------------------------------------

Editor::Editor()
{
    // Set default values
    m_title = "";
    m_filePath = "";
    m_separator = "";
    m_frameEndSequence = "";
    m_frameStartSequence = "";
    m_modified = false;

    // Connect signals/slots
    connect(this, &Editor::groupChanged, this, &Editor::onGroupChanged);
    connect(this, &Editor::titleChanged, this, &Editor::onModelChanged);
    connect(this, &Editor::datasetChanged, this, &Editor::onDatasetChanged);
    connect(this, &Editor::separatorChanged, this, &Editor::onModelChanged);
    connect(this, &Editor::groupCountChanged, this, &Editor::onModelChanged);
    connect(this, &Editor::groupOrderChanged, this, &Editor::onModelChanged);
    connect(this, &Editor::frameEndSequenceChanged, this, &Editor::onModelChanged);
    connect(this, &Editor::frameStartSequenceChanged, this, &Editor::onModelChanged);

    // Load current JSON map file into C++ model
    auto generator = Generator::getInstance();
    connect(generator, &Generator::jsonFileMapChanged, this, &Editor::onJsonLoaded);
}

Editor::~Editor() { }

Editor *Editor::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Editor();

    return INSTANCE;
}

//--------------------------------------------------------------------------------------------------
// Member access functions
//--------------------------------------------------------------------------------------------------

QStringList Editor::availableGroupLevelWidgets()
{
    return QStringList { tr("Dataset widgets"), tr("Accelerometer"), tr("Gyroscope"),
                         tr("Map") };
}

QStringList Editor::availableDatasetLevelWidgets()
{
    return QStringList { tr("None"), tr("Gauge"), tr("Bar/level"), tr("Compass") };
}

QString Editor::title() const
{
    return m_title;
}

QString Editor::separator() const
{
    return m_separator;
}

QString Editor::frameEndSequence() const
{
    return m_frameEndSequence;
}

QString Editor::frameStartSequence() const
{
    return m_frameStartSequence;
}

bool Editor::modified() const
{
    return m_modified;
}

int Editor::groupCount() const
{
    return m_groups.count();
}

QString Editor::jsonFilePath() const
{
    return m_filePath;
}

QString Editor::jsonFileName() const
{
    if (!jsonFilePath().isEmpty())
    {
        auto fileInfo = QFileInfo(m_filePath);
        return fileInfo.fileName();
    }

    return tr("New Project");
}

bool Editor::askSave()
{
    if (!modified())
        return true;

    auto ret = Misc::Utilities::showMessageBox(
        tr("Do you want to save your changes?"),
        tr("You have unsaved modifications in this project!"), APP_NAME,
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return false;

    if (ret == QMessageBox::Discard)
        return true;

    return saveJsonFile();
}

bool Editor::saveJsonFile()
{
    return true;
}

int Editor::datasetCount(const int group) const
{
    if (group < groupCount())
        return m_groups.at(group)->m_datasets.count();

    return 0;
}

Group *Editor::getGroup(const int index)
{
    if (index < groupCount())
        return m_groups.at(index);

    return nullptr;
}

Dataset *Editor::getDataset(const int group, const int index)
{
    if (index < datasetCount(group))
        return getGroup(group)->m_datasets.at(index);

    return nullptr;
}

QString Editor::groupTitle(const int group)
{
    auto grp = getGroup(group);
    if (grp)
        return grp->m_title;

    return "";
}

int Editor::groupWidgetIndex(const int group)
{
    auto grp = getGroup(group);
    if (grp)
    {
        auto widget = grp->m_widget;

        if (widget == "accelerometer")
            return 1;

        if (widget == "gyro")
            return 2;

        if (widget == "map")
            return 3;
    }

    return 0;
}

int Editor::datasetIndex(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_index;

    return 0;
}

bool Editor::datasetGraph(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_graph;

    return 0;
}

QString Editor::datasetTitle(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_title;

    return 0;
}

QString Editor::datasetUnits(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_units;

    return 0;
}

QString Editor::datasetWidget(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_widget;

    return 0;
}

int Editor::datasetWidgetIndex(const int group, const int dataset)
{
    auto widget = datasetWidget(group, dataset);
    if (widget == "gauge")
        return 1;
    if (widget == "bar")
        return 2;
    if (widget == "compass")
        return 3;

    return 0;
}

QString Editor::datasetWidgetMin(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_min;

    return 0;
}

QString Editor::datasetWidgetMax(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_max;

    return 0;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

void Editor::newJsonFile()
{
    // Clear groups list
    m_groups.clear();

    // Reset project properties
    setTitle("");
    setSeparator("");
    setFrameEndSequence("");
    setFrameStartSequence("");

    // Update file path
    m_filePath = "";
    emit jsonFileChanged();

    // Update UI
    emit groupCountChanged();
    setModified(false);
}

void Editor::openJsonFile()
{
    // Let user select the file
    auto path
        = QFileDialog::getOpenFileName(nullptr, tr("Select JSON file"), "", "*.json");
    if (path.isEmpty())
        return;

    // Open the JSON file
    openJsonFile(path);
}

void Editor::openJsonFile(const QString &path)
{
    // Open file
    QFile file(path);
    QJsonDocument document;
    if (file.open(QFile::ReadOnly))
    {
        document = QJsonDocument::fromJson(file.readAll());
        file.close();
    }

    // Validate JSON document
    if (document.isEmpty())
        return;

    // Let the generator use the given JSON file
    auto generator = Generator::getInstance();
    if (generator->jsonMapFilepath() != path)
        generator->loadJsonMap(path);

    // Reset C++ model
    newJsonFile();

    // Update current JSON document
    m_filePath = path;
    emit jsonFileChanged();

    // Read data from JSON document
    auto json = document.object();
    setTitle(json.value("t").toString());
    setSeparator(json.value("s").toString());
    setFrameEndSequence(json.value("fe").toString());
    setFrameStartSequence(json.value("fs").toString());

    // Read groups from JSON document
    auto groups = json.value("g").toArray();
    for (int group = 0; group < groups.count(); ++group)
    {
        // Get JSON group data
        auto jsonGroup = groups.at(group).toObject();

        // Register group with C++ model
        addGroup();
        setGroupTitle(group, jsonGroup.value("t").toString());
        setGroupWidgetData(group, jsonGroup.value("w").toString());

        // Get JSON group datasets
        auto jsonDatasets = jsonGroup.value("d").toArray();
        for (int dataset = 0; dataset < jsonDatasets.count(); ++dataset)
        {
            // Get dataset JSON data
            auto jsonDataset = jsonDatasets.at(dataset).toObject();

            // Register dataset with C++ model
            addDataset(group);
            setDatasetGraph(group, dataset, jsonDataset.value("g").toBool());
            setDatasetTitle(group, dataset, jsonDataset.value("t").toString());
            setDatasetUnits(group, dataset, jsonDataset.value("u").toString());
            setDatasetWidgetData(group, dataset, jsonDataset.value("w").toString());

            // Get max/min texts
            auto min = jsonDataset.value("min").toDouble();
            auto max = jsonDataset.value("max").toDouble();
            setDatasetWidgetMin(group, dataset, QString::number(min));
            setDatasetWidgetMax(group, dataset, QString::number(max));

            // Calculate dataset index
            auto index = jsonDataset.value("v").toString();
            index.replace("%", "");
            setDatasetIndex(group, dataset, index.toInt());
        }
    }

    // Update UI
    emit groupCountChanged();
    setModified(false);
}

void Editor::setTitle(const QString &title)
{
    if (title != m_title)
    {
        m_title = title;
        emit titleChanged();
    }
}

void Editor::setSeparator(const QString &separator)
{
    if (separator != m_separator)
    {
        m_separator = separator;
        emit separatorChanged();
    }
}

void Editor::setFrameEndSequence(const QString &sequence)
{
    if (sequence != m_frameEndSequence)
    {
        m_frameEndSequence = sequence;
        emit frameEndSequenceChanged();
    }
}

void Editor::setFrameStartSequence(const QString &sequence)
{
    if (sequence != m_frameStartSequence)
    {
        m_frameStartSequence = sequence;
        emit frameStartSequenceChanged();
    }
}

void Editor::addGroup()
{
    m_groups.append(new Group);
    setGroupTitle(m_groups.count() - 1, tr("New Group"));

    emit groupCountChanged();
}

void Editor::deleteGroup(const int group)
{
    auto grp = getGroup(group);
    if (grp)
    {
        auto ret = Misc::Utilities::showMessageBox(
            tr("Delete group \"%1\"").arg(grp->title()),
            tr("Are you sure you want to delete this group?"), APP_NAME,
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            m_groups.removeAt(group);
            emit groupCountChanged();
        }
    }
}

void Editor::moveGroupUp(const int group)
{
    if (group > 0)
    {
        m_groups.move(group, group - 1);
        emit groupOrderChanged();
    }
}

void Editor::moveGroupDown(const int group)
{
    if (group < groupCount() - 1)
    {
        m_groups.move(group, group + 1);
        emit groupOrderChanged();
    }
}

bool Editor::setGroupWidget(const int group, const int widgetId)
{
    auto grp = getGroup(group);
    if (grp)
    {
        // Warn user if group contains existing datasets
        if (!(grp->m_datasets.isEmpty()))
        {
            auto ret = Misc::Utilities::showMessageBox(
                tr("Are you sure you want to change the group-level widget?"),
                tr("Existing datasets for this group will be deleted"), APP_NAME,
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::No)
                return false;
            else
                grp->m_datasets.clear();
        }

        // Accelerometer widget
        if (widgetId == 1)
        {
            // Set widget title
            grp->m_widget = "accelerometer";

            // Create datasets
            auto x = new Dataset;
            auto y = new Dataset;
            auto z = new Dataset;

            // Set dataset properties
            x->m_widget = "x";
            y->m_widget = "y";
            z->m_widget = "z";
            x->m_title = tr("Accelerometer %1").arg("X");
            y->m_title = tr("Accelerometer %1").arg("Y");
            z->m_title = tr("Accelerometer %1").arg("Z");

            // Add datasets to group
            grp->m_datasets.append(x);
            grp->m_datasets.append(y);
            grp->m_datasets.append(z);
        }

        // Gyroscope widget
        else if (widgetId == 2)
        {
            // Set widget title
            grp->m_widget = "gyro";

            // Create datasets
            auto x = new Dataset;
            auto y = new Dataset;
            auto z = new Dataset;

            // Set dataset properties
            x->m_widget = "x";
            y->m_widget = "y";
            z->m_widget = "z";
            x->m_title = tr("Gyro %1").arg("X");
            y->m_title = tr("Gyro %1").arg("Y");
            z->m_title = tr("Gyro %1").arg("Z");

            // Add datasets to group
            grp->m_datasets.append(x);
            grp->m_datasets.append(y);
            grp->m_datasets.append(z);
        }

        // Map widget
        else if (widgetId == 3)
        {
            // Set widget title
            grp->m_widget = "map";

            // Create datasets
            auto lat = new Dataset;
            auto lon = new Dataset;

            // Set dataset properties
            lat->m_widget = "lat";
            lon->m_widget = "lon";
            lat->m_title = tr("Latitude");
            lon->m_title = tr("Longitude");

            // Add datasets to group
            grp->m_datasets.append(lat);
            grp->m_datasets.append(lon);
        }

        // Update UI
        emit groupChanged(group);
        return true;
    }

    return false;
}

void Editor::setGroupTitle(const int group, const QString &title)
{
    auto grp = getGroup(group);
    if (grp)
    {
        grp->m_title = title;
        emit groupChanged(group);
    }
}

void Editor::setGroupWidgetData(const int group, const QString &widget)
{
    auto grp = getGroup(group);
    if (grp)
    {
        grp->m_widget = widget;
        emit groupChanged(group);
    }
}

void Editor::addDataset(const int group)
{
    auto grp = getGroup(group);
    if (grp)
    {
        grp->m_datasets.append(new Dataset);
        setDatasetTitle(group, grp->m_datasets.count() - 1, tr("New dataset"));
        emit groupChanged(group);
    }
}

void Editor::deleteDataset(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        auto ret = Misc::Utilities::showMessageBox(
            tr("Delete dataset \"%1\"").arg(set->title()),
            tr("Are you sure you want to delete this dataset?"), APP_NAME,
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            getGroup(group)->m_datasets.removeAt(dataset);
            emit groupChanged(group);
        }
    }
}

void Editor::moveDatasetUp(const int group, const int dataset) { }

void Editor::moveDatasetDown(const int group, const int dataset) { }

void Editor::setDatasetTitle(const int group, const int dataset, const QString &title)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_title = title;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetUnits(const int group, const int dataset, const QString &units)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_units = units;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetIndex(const int group, const int dataset, const int frameIndex)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_index = frameIndex;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetGraph(const int group, const int dataset, const bool generateGraph)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_graph = generateGraph;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetWidget(const int group, const int dataset, const int widgetId)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        QString widget;
        if (widgetId == 1)
            widget = "gauge";
        else if (widgetId == 2)
            widget = "bar";
        else if (widgetId == 3)
            widget = "compass";

        set->m_widget = widget;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetWidgetMin(const int group, const int dataset,
                                 const QString &minimum)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_min = minimum;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetWidgetMax(const int group, const int dataset,
                                 const QString &maximum)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_max = maximum;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setDatasetWidgetData(const int group, const int dataset,
                                  const QString &widget)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_widget = widget;
        emit datasetChanged(group, dataset);
    }
}

void Editor::setModified(const bool modified)
{
    if (m_modified != modified)
    {
        m_modified = modified;
        emit modifiedChanged();
    }
}

void Editor::onJsonLoaded()
{
    if (jsonFilePath() != Generator::getInstance()->jsonMapFilepath())
        openJsonFile(Generator::getInstance()->jsonMapFilepath());
}

void Editor::onModelChanged()
{
    setModified(true);
}

void Editor::onGroupChanged(const int group)
{
    (void)group;
    setModified(true);
}

void Editor::onDatasetChanged(const int group, const int dataset)
{
    (void)group;
    (void)dataset;
    setModified(true);
}
