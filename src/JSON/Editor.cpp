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
#include "IO/Manager.h"
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

/**
 * Constructor function. Initializes internal members and configures the signals/slots so
 * that the editor can know if the user modified the JSON document. Finally, the
 * constructor configures signals/slots with the JSON Generator to share the same JSON
 * document file.
 */
Editor::Editor()
    : m_title("")
    , m_separator("")
    , m_frameEndSequence("")
    , m_frameStartSequence("")
    , m_modified(false)
    , m_filePath("")
{
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

/**
 * Returns a pointer to the only instance of the editor class.
 */
Editor *Editor::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Editor();

    return INSTANCE;
}

//--------------------------------------------------------------------------------------------------
// Member access functions
//--------------------------------------------------------------------------------------------------

/**
 * Returns a list with the available group-level widgets. This list is used by the user
 * interface to allow the user to build accelerometer, gyro & map widgets directly from
 * the UI.
 */
QStringList Editor::availableGroupLevelWidgets()
{
    return QStringList { tr("Dataset widgets"), tr("Accelerometer"), tr("Gyroscope"),
                         tr("Map") };
}

/**
 * Returns a list with the available dataset-level widgets. This list is used by the user
 * interface to allow the user to build gauge, bar & compass widgets directly from the UI.
 */
QStringList Editor::availableDatasetLevelWidgets()
{
    return QStringList { tr("None"), tr("Gauge"), tr("Bar/level"), tr("Compass") };
}

/**
 * Returns the title of the current project
 */
QString Editor::title() const
{
    return m_title;
}

/**
 * Returns the data separator sequence for the current project.
 */
QString Editor::separator() const
{
    return m_separator;
}

/**
 * Returns the frame end sequence for the current project.
 */
QString Editor::frameEndSequence() const
{
    return m_frameEndSequence;
}

/**
 * Returns the frame start sequence for the current project.
 */
QString Editor::frameStartSequence() const
{
    return m_frameStartSequence;
}

/**
 * Returns @c true if the user modified the current project. This is
 * used to know if Serial Studio shall prompt the user to save his/her
 * modifications before closing the editor window.
 */
bool Editor::modified() const
{
    return m_modified;
}

/**
 * Returns the number of groups contained in the current JSON project.
 */
int Editor::groupCount() const
{
    return m_groups.count();
}

/**
 * Returns the full path of the current JSON project file.
 */
QString Editor::jsonFilePath() const
{
    return m_filePath;
}

/**
 * Returns the simplified file name of the current JSON project file.
 * This is used to change the title of the Editor window.
 */
QString Editor::jsonFileName() const
{
    if (!jsonFilePath().isEmpty())
    {
        auto fileInfo = QFileInfo(m_filePath);
        return fileInfo.fileName();
    }

    return tr("New Project");
}

/**
 * Checks if the current project has been modified and prompts the
 * user to save his/her changes.
 */
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

/**
 * Validates the configuration of the current JSON project and saves the JSON
 * document on the hard disk.
 */
bool Editor::saveJsonFile()
{
    // Validate project title
    if (title().isEmpty())
    {
        Misc::Utilities::showMessageBox(tr("Project error"),
                                        tr("Project title cannot be empty!"));
        return false;
    }

    // Validate group titles
    for (int i = 0; i < groupCount(); ++i)
    {
        if (groupTitle(i).isEmpty())
        {
            Misc::Utilities::showMessageBox(tr("Project error - Group %1").arg(i + 1),
                                            tr("Group title cannot be empty!"));
            return false;
        }
    }

    // Validate dataset titles
    for (int i = 0; i < groupCount(); ++i)
    {
        for (int j = 0; j < datasetCount(i); ++j)
        {
            if (datasetTitle(i, j).isEmpty())
            {
                Misc::Utilities::showMessageBox(
                    tr("Project error - Group %1, Dataset %2").arg(i + 1).arg(j + 1),
                    tr("Dataset title cannot be empty!"));
                return false;
            }
        }
    }

    // Validate dataset indexes
    QList<int> indexes;
    for (int i = 0; i < groupCount(); ++i)
    {
        for (int j = 0; j < datasetCount(i); ++j)
        {
            if (!indexes.contains(datasetIndex(i, j)))
                indexes.append(datasetIndex(i, j));

            else
            {
                auto ret = Misc::Utilities::showMessageBox(
                    tr("Warning - Group %1, Dataset %2").arg(i + 1).arg(j + 1),
                    tr("Dataset contains duplicate frame index position! Continue?"),
                    APP_NAME, QMessageBox::Yes | QMessageBox::No);

                if (ret == QMessageBox::No)
                    return false;
            }
        }
    }

    // Get file save path
    if (jsonFilePath().isEmpty())
    {
        auto path = QFileDialog::getSaveFileName(nullptr, tr("Save JSON project"),
                                                 QDir::homePath(), "*.json");
        if (path.isEmpty())
            return false;

        m_filePath = path;
    }

    // Open file for writing
    QFile file(m_filePath);
    if (!file.open(QFile::WriteOnly))
    {
        Misc::Utilities::showMessageBox(tr("File open error"), file.errorString());
        return false;
    }

    // Create JSON document & add properties
    QJsonObject json;
    json.insert("t", title());
    json.insert("s", separator());
    json.insert("fe", frameEndSequence());
    json.insert("fs", frameStartSequence());

    // Create group array
    QJsonArray groups;
    for (int i = 0; i < groupCount(); ++i)
    {
        // Create group
        QJsonObject group;
        group.insert("t", groupTitle(i));
        group.insert("w", groupWidget(i));

        // Create dataset array
        QJsonArray datasets;
        for (int j = 0; j < datasetCount(i); ++j)
        {
            // Create dataset
            QJsonObject dataset;
            dataset.insert("t", datasetTitle(i, j));
            dataset.insert("u", datasetUnits(i, j));
            dataset.insert("g", datasetGraph(i, j));
            dataset.insert("w", datasetWidget(i, j));
            dataset.insert("v", "%" + QString::number(datasetIndex(i, j)));
            dataset.insert("min", datasetWidgetMin(i, j).toDouble());
            dataset.insert("max", datasetWidgetMax(i, j).toDouble());
            dataset.insert("alarm", datasetWidgetAlarm(i, j).toDouble());

            // Add dataset to array
            datasets.append(dataset);
        }

        // Add datasets to group
        group.insert("d", datasets);
        groups.append(group);
    }

    // Add groups array to JSON
    json.insert("g", groups);

    // Write JSON data to file
    file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    file.close();

    // Load JSON file to Serial Studio
    openJsonFile(file.fileName());
    Generator::getInstance()->loadJsonMap(file.fileName());
    return true;
}

/**
 * Returns the number of datasets contained by the given @a group index.
 */
int Editor::datasetCount(const int group) const
{
    if (group < groupCount())
        return m_groups.at(group)->m_datasets.count();

    return 0;
}

/**
 * Returns a pointer to the group object positioned at the given @a index
 */
Group *Editor::getGroup(const int index)
{
    if (index < groupCount())
        return m_groups.at(index);

    return nullptr;
}

/**
 * Returns a pointer to the dataset object contained by the @a group at
 * the given @a index
 */
Dataset *Editor::getDataset(const int group, const int index)
{
    if (index < datasetCount(group))
        return getGroup(group)->m_datasets.at(index);

    return nullptr;
}

/**
 * Returns the title of the given @a group.
 */
QString Editor::groupTitle(const int group)
{
    auto grp = getGroup(group);
    if (grp)
        return grp->m_title;

    return "";
}

/**
 * Returns the widget of the given @a group.
 */
QString Editor::groupWidget(const int group)
{
    auto grp = getGroup(group);
    if (grp)
        return grp->m_widget;

    return "";
}

/**
 * Returns the widget ID of the given @a group. The widget ID is a number
 * that represents a group-level widget. The ID depends on the widget type
 * and the order of the widgets returned by the @c availableGroupLevelWidgets()
 * function.
 */
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

/**
 * Returns the position in the frame that holds the value for the given
 * @a dataset (which is contained by the specified @a group).
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
int Editor::datasetIndex(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_index;

    return 0;
}

/**
 * Returns @c true if Serial Studio should graph the data of the given
 * @a dataset (which is contained by the specified @a group).
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
bool Editor::datasetGraph(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_graph;

    return 0;
}

/**
 * Returns the title of the specified dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
QString Editor::datasetTitle(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_title;

    return 0;
}

/**
 * Returns the measurement units of the specified dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
QString Editor::datasetUnits(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_units;

    return 0;
}

/**
 * Returns the widget string of the specified dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
QString Editor::datasetWidget(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_widget;

    return 0;
}

/**
 * Returns the widget ID of the specified dataset. The widget ID
 * corresponds to the list returned by the
 * @c availableDatasetLevelWidgets() function.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
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

/**
 * Returns the minimum widget value of the specified dataset.
 * This option is used by the bar & gauge widgets.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
QString Editor::datasetWidgetMin(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_min;

    return 0;
}

/**
 * Returns the maximum widget value of the specified dataset.
 * This option is used by the bar & gauge widgets.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
QString Editor::datasetWidgetMax(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
        return set->m_max;

    return 0;
}

/**
 * Returns the widget alarm value of the specified dataset.
 * This option is used by the bar & gauge widgets.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
QString Editor::datasetWidgetAlarm(const int group, const int dataset)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        if (set->m_alarm.isEmpty())
            return set->m_max;
        else
            return set->m_alarm;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * Resets the C++ model used to represent the JSON project file.
 */
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

/**
 * Prompts the user to select a JSON project file & generates the appropiate C++
 * model that represents the JSON document.
 */
void Editor::openJsonFile()
{
    // clang-format off
    auto path = QFileDialog::getOpenFileName(nullptr,
                                             tr("Select JSON file"),
                                             QDir::homePath(),
                                             "*.json");
    // clang-format on

    // Invalid path, abort
    if (path.isEmpty())
        return;

    // Open the JSON file
    openJsonFile(path);
}

/**
 * Opens the JSON document at the given @a path & generates the appropiate C++
 * model that represents the JSON document.
 */
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

    // Modify IO manager settings
    auto manager = IO::Manager::getInstance();
    manager->setSeparatorSequence(separator());
    manager->setFinishSequence(frameEndSequence());
    manager->setStartSequence(frameStartSequence());

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
            auto alarm = jsonDataset.value("alarm").toDouble();
            setDatasetWidgetMin(group, dataset, QString::number(min));
            setDatasetWidgetMax(group, dataset, QString::number(max));
            setDatasetWidgetAlarm(group, dataset, QString::number(alarm));

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

/**
 * Changes the title of the JSON project file.
 */
void Editor::setTitle(const QString &title)
{
    if (title != m_title)
    {
        m_title = title;
        emit titleChanged();
    }
}

/**
 * Changes the data separator sequence of the JSON project file.
 */
void Editor::setSeparator(const QString &separator)
{
    if (separator != m_separator)
    {
        m_separator = separator;
        emit separatorChanged();
    }
}

/**
 * Changes the frame end sequence of the JSON project file.
 */
void Editor::setFrameEndSequence(const QString &sequence)
{
    if (sequence != m_frameEndSequence)
    {
        m_frameEndSequence = sequence;
        emit frameEndSequenceChanged();
    }
}

/**
 * Changes the frame start sequence of the JSON project file.
 */
void Editor::setFrameStartSequence(const QString &sequence)
{
    if (sequence != m_frameStartSequence)
    {
        m_frameStartSequence = sequence;
        emit frameStartSequenceChanged();
    }
}

/**
 * Adds a new group to the C++ model that represents the JSON project file.
 */
void Editor::addGroup()
{
    m_groups.append(new Group);
    setGroupTitle(m_groups.count() - 1, tr("New Group"));

    emit groupCountChanged();
}

/**
 * Removes the given @a group from the C++ model that represents the JSON
 * project file.
 */
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

/**
 * Changes the position of the given @a group in the C++ model.
 */
void Editor::moveGroupUp(const int group)
{
    if (group > 0)
    {
        m_groups.move(group, group - 1);
        emit groupOrderChanged();
    }
}

/**
 * Changes the position of the given @a group in the C++ model.
 */
void Editor::moveGroupDown(const int group)
{
    if (group < groupCount() - 1)
    {
        m_groups.move(group, group + 1);
        emit groupOrderChanged();
    }
}

/**
 * Changes the group-level widget for the specified @a group.
 * If necessary, this function shall generate the appropiate datasets
 * needed to implement the widget (e.g. x,y,z for accelerometer widgets).
 */
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
            grp->m_title = tr("Accelerometer");

            // Create datasets
            auto x = new Dataset;
            auto y = new Dataset;
            auto z = new Dataset;

            // Set dataset indexes
            x->m_index = nextDatasetIndex();
            y->m_index = nextDatasetIndex() + 1;
            z->m_index = nextDatasetIndex() + 2;

            // Set measurement units
            x->m_units = "m/s²";
            y->m_units = "m/s²";
            z->m_units = "m/s²";

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
            grp->m_title = tr("Gyroscope");

            // Create datasets
            auto x = new Dataset;
            auto y = new Dataset;
            auto z = new Dataset;

            // Set dataset indexes
            x->m_index = nextDatasetIndex();
            y->m_index = nextDatasetIndex() + 1;
            z->m_index = nextDatasetIndex() + 2;

            // Set measurement units
            x->m_units = "°";
            y->m_units = "°";
            z->m_units = "°";

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
            grp->m_title = tr("GPS Map");

            // Create datasets
            auto lat = new Dataset;
            auto lon = new Dataset;

            // Set dataset indexes
            lat->m_index = nextDatasetIndex();
            lon->m_index = nextDatasetIndex() + 1;

            // Set measurement units
            lat->m_units = "°";
            lon->m_units = "°";

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

/**
 * Changes the @a title for the given @a group in the C++ model
 */
void Editor::setGroupTitle(const int group, const QString &title)
{
    auto grp = getGroup(group);
    if (grp)
    {
        grp->m_title = title;
        emit groupChanged(group);
    }
}

/**
 * Changes the @a widget for the given @a group in the C++ model
 */
void Editor::setGroupWidgetData(const int group, const QString &widget)
{
    auto grp = getGroup(group);
    if (grp)
    {
        grp->m_widget = widget;
        emit groupChanged(group);
    }
}

/**
 * Adds a new dataset to the given @a group & increments the frame
 * index counter (to avoid having datasets that pull data from the
 * same position in the MCU frame).
 */
void Editor::addDataset(const int group)
{
    auto grp = getGroup(group);
    if (grp)
    {
        grp->m_datasets.append(new Dataset);
        setDatasetIndex(group, grp->m_datasets.count() - 1, nextDatasetIndex());
        setDatasetTitle(group, grp->m_datasets.count() - 1, tr("New dataset"));
        emit groupChanged(group);
    }
}

/**
 * Removes the given @a dataset from the specified @a group.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
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

/**
 * Updates the @a title of the given @a dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
void Editor::setDatasetTitle(const int group, const int dataset, const QString &title)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_title = title;
        emit datasetChanged(group, dataset);
    }
}

/**
 * Updates the measurement @a units of the given @a dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
void Editor::setDatasetUnits(const int group, const int dataset, const QString &units)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_units = units;
        emit datasetChanged(group, dataset);
    }
}

/**
 * Updates the @a frameIndex of the given @a dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
void Editor::setDatasetIndex(const int group, const int dataset, const int frameIndex)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_index = frameIndex;
        emit datasetChanged(group, dataset);
    }
}

/**
 * Updates the @a generateGraph flag of the given @a dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
void Editor::setDatasetGraph(const int group, const int dataset, const bool generateGraph)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_graph = generateGraph;
        emit datasetChanged(group, dataset);
    }
}

/**
 * Updates the @a widgetId of the given @a dataset. The widget ID is dependent on
 * the order of the widgets returned by the @c availableDatasetLevelWidgets() function.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
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

/**
 * Updates the @a minimum value used by the bar & gauge widgets for the given
 * @a dataset. The value is specified in a @c QString to facilitate integration
 * with the QML user interface.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
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

/**
 * Updates the @a maximum value used by the bar & gauge widgets for the given
 * @a dataset. The value is specified in a @c QString to facilitate integration
 * with the QML user interface.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
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

/**
 * Updates the @a widget string of the given @a dataset.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
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

/**
 * Updates the @a alarm value used by the bar & gauge widgets for the given
 * @a dataset. The value is specified in a @c QString to facilitate integration
 * with the QML user interface.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
void Editor::setDatasetWidgetAlarm(const int group, const int dataset,
                                   const QString &alarm)
{
    auto set = getDataset(group, dataset);
    if (set)
    {
        set->m_alarm = alarm;
        emit datasetChanged(group, dataset);
    }
}

/**
 * Updates the @a modified flag of the current JSON project.
 * This flag is used to know if we should ask the user to save
 * his/her modifications to the project file.
 */
void Editor::setModified(const bool modified)
{
    if (m_modified != modified)
    {
        m_modified = modified;
        emit modifiedChanged();
    }
}

/**
 * Ensures that the JSON project file is the same as the one used by
 * the JSON Generator class.
 */
void Editor::onJsonLoaded()
{
    if (jsonFilePath() != Generator::getInstance()->jsonMapFilepath())
        openJsonFile(Generator::getInstance()->jsonMapFilepath());
}

/**
 * Sets the modified flag to @c true when the user adds/removes/moves
 * one of the groups contained in the JSON project.
 */
void Editor::onModelChanged()
{
    setModified(true);
}

/**
 * Sets the modified flag to @c true when the user changes the title
 * or the widget of one of the groups contained in the JSON project.
 */
void Editor::onGroupChanged(const int group)
{
    (void)group;
    setModified(true);
}

/**
 * Sets the modified flag to @c true when the user modifies the
 * properties of one of the datasets contained in the JSON project.
 *
 * @param group   index of the group in which the dataset belongs
 * @param dataset index of the dataset
 */
void Editor::onDatasetChanged(const int group, const int dataset)
{
    (void)group;
    (void)dataset;
    setModified(true);
}

/**
 * Gets the number of datasets registered in the projects and
 * adds 1 to the result.
 *
 * This function is used when registering new datasets, so that
 * the user does not need to manually specify dataset indexes.
 */
int Editor::nextDatasetIndex()
{
    int maxIndex = 1;
    for (int i = 0; i < groupCount(); ++i)
    {
        for (int j = 0; j < datasetCount(i); ++j)
        {
            auto dataset = getDataset(i, j);
            if (dataset)
            {
                if (dataset->m_index >= maxIndex)
                    maxIndex = dataset->m_index + 1;
            }
        }
    }

    return maxIndex;
}
