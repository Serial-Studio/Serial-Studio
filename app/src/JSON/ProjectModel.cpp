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

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QFileDialog>
#include <QJsonObject>
#include <QDirIterator>
#include <QJsonDocument>

#include "AppInfo.h"
#include "Misc/Utilities.h"
#include "Misc/Translator.h"

#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"
#include "JSON/FrameBuilder.h"

//------------------------------------------------------------------------------
// Private enums to keep track of which item the user selected/modified
//------------------------------------------------------------------------------

/**
 * @brief Enum representing top-level items in the project structure.
 */
// clang-format off
typedef enum
{
  kRootItem,     /**< Represents the root item of the project. */
  kFrameParser,  /**< Represents the frame parser function item. */
} TopLevelItem;
// clang-format on

/**
 * @brief Enum representing items in the project view.
 */
// clang-format off
typedef enum
{
  kProjectView_Title,               /**< Represents the project title item. */
  kProjectView_ItemSeparator,       /**< Represents the item separator. */
  kProjectView_FrameStartSequence,  /**< Represents the frame start sequence. */
  kProjectView_FrameEndSequence,    /**< Represents the frame end sequence. */
  kProjectView_FrameDecoder,        /**< Represents the frame decoder item. */
  kProjectView_FrameDetection,      /**< Represents the frame detection item. */
  kProjectView_ThunderforestApiKey, /**< Represents the Thunderforest API key. */
  kProjectView_MapTilerApiKey       /**< Represents the MapTiler API key. */
} ProjectItem;
// clang-format on

/**
 * @brief Enum representing items in the dataset view.
 */
// clang-format off
typedef enum
{
  kDatasetView_Title,         /**< Represents the dataset title item. */
  kDatasetView_Index,         /**< Represents the dataset frame index item. */
  kDatasetView_Units,         /**< Represents the dataset units item. */
  kDatasetView_Widget,        /**< Represents the dataset widget item. */
  kDatasetView_FFT,           /**< Represents the FFT plot checkbox item. */
  kDatasetView_LED,           /**< Represents the LED panel checkbox item. */
  kDatasetView_LED_High,      /**< Represents the LED high (on) value item. */
  kDatasetView_Plot,          /**< Represents the dataset plot mode item. */
  kDatasetView_Min,           /**< Represents the dataset minimum value item. */
  kDatasetView_Max,           /**< Represents the dataset maximum value item. */
  kDatasetView_Alarm,         /**< Represents the dataset alarm value item. */
  kDatasetView_FFT_Samples,    /**< Represents the FFT window size item. */
  kDatasetView_FFT_SamplingRate, /**< Represents the FFT sampling rate item. */
} DatasetItem;
// clang-format on

/**
 * @brief Enum representing items in the action view.
 */
// clang-format off
typedef enum
{
  kActionView_Title, /**< Represents the action title item. */
  kActionView_Icon,  /**< Represents the icon item. */
  kActionView_EOL,   /**< Represents the EOL (end of line) item. */
  kActionView_Data   /**< Represents the TX data item. */
} ActionItem;
// clang-format on

/**
 * @brief Enum representing items in the group view.
 */
// clang-format off
typedef enum
{
  kGroupView_Title,  /**< Represents the group title item. */
  kGroupView_Widget  /**< Represents the group widget item. */
} GroupItem;
// clang-format on

//------------------------------------------------------------------------------
// Define ports for the OSM server
//------------------------------------------------------------------------------

static quint16 SERVER_PORT = 9727;
static quint16 TILE_SERVER_PORT = 2701;

//------------------------------------------------------------------------------
// Constructor/deconstructor & singleton instance access function
//------------------------------------------------------------------------------

/**
 * @brief Constructor for the JSON::ProjectModel class.
 *
 * Initializes the ProjectModel class by setting default values for member
 * variables, generating the necessary combo box models, and connecting signals
 * to handle J SON file changes.
 *
 * This constructor also loads the current JSON map file into the model or
 * creates a new project if no file is present.
 */
JSON::ProjectModel::ProjectModel()
  : m_title("")
  , m_separator("")
  , m_frameParserCode("")
  , m_frameEndSequence("")
  , m_frameStartSequence("")
  , m_currentView(ProjectView)
  , m_frameDecoder(WC::PlainText)
  , m_frameDetection(WC::EndDelimiterOnly)
  , m_modified(false)
  , m_filePath("")
  , m_treeModel(nullptr)
  , m_selectionModel(nullptr)
  , m_groupModel(nullptr)
  , m_projectModel(nullptr)
  , m_datasetModel(nullptr)
  , m_server("", "", TILE_SERVER_PORT, SERVER_PORT, nullptr)
{
  // Generate data sources for project model
  generateComboBoxModels();

  // Clear selection model when JSON file is changed
  connect(this, &JSON::ProjectModel::jsonFileChanged, this, [=] {
    if (m_selectionModel)
    {
      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index,
                                        QItemSelectionModel::ClearAndSelect);
    }
  });

  // Ensure toolbar actions are synched with models
  connect(this, &JSON::ProjectModel::groupModelChanged, this,
          &JSON::ProjectModel::editableOptionsChanged);
  connect(this, &JSON::ProjectModel::datasetModelChanged, this,
          &JSON::ProjectModel::editableOptionsChanged);
  connect(this, &JSON::ProjectModel::datasetModelChanged, this,
          &JSON::ProjectModel::datasetOptionsChanged);

  // Re-load JSON map file into C++ model
  connect(&JSON::FrameBuilder::instance(),
          &JSON::FrameBuilder::jsonFileMapChanged, this,
          &JSON::ProjectModel::onJsonLoaded);

  // Apply map provider API keys
  connect(this, &JSON::ProjectModel::gpsApiKeysChanged, this,
          &JSON::ProjectModel::onGpsApiKeysChanged);

  // Generate combo-boxes again when app is translated
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, [=] {
            generateComboBoxModels();
            buildTreeModel();

            switch (currentView())
            {
              case ProjectView:
                buildProjectModel();
                break;
              case GroupView:
                buildGroupModel(m_selectedGroup);
                break;
              case ActionView:
                buildActionModel(m_selectedAction);
                break;
              case DatasetView:
                buildDatasetModel(m_selectedDataset);
                break;
              default:
                break;
            }
          });

  // Load current JSON map file into C++ model
  if (!JSON::FrameBuilder::instance().jsonMapFilepath().isEmpty())
    onJsonLoaded();

  // Create a new project
  else
    newJsonFile();
}

/**
 * @brief Retrieves the singleton instance of the JSON::ProjectModel class.
 *
 * @return Reference to the singleton instance of JSON::ProjectModel.
 */
JSON::ProjectModel &JSON::ProjectModel::instance()
{
  static ProjectModel singleton;
  return singleton;
}

//------------------------------------------------------------------------------
// Document status access functions
//------------------------------------------------------------------------------

/**
 * @brief Checks if the document has been modified.
 *
 * This function returns the current modification status of the project.
 *
 * @return True if the document is modified, false otherwise.
 */
bool JSON::ProjectModel::modified() const
{
  return m_modified;
}

/**
 * @brief Retrieves the current view of the project.
 *
 * This function returns the current view mode (e.g., ProjectView, GroupView,
 * DatasetView, etc.) that is active.
 *
 * @return The current view as a value from the CurrentView enum.
 */
JSON::ProjectModel::CurrentView JSON::ProjectModel::currentView() const
{
  return m_currentView;
}

/**
 * @brief Retrieves the current method used for decoding data frames.
 *
 * This function returns the decoder method currently set for parsing data
 * frames. The decoder method determines how incoming data is interpreted
 * (e.g., as normal UTF-8, hexadecimal, or Base64).
 *
 * @return The current decoder method as a value from the `DecoderMethod` enum.
 */
WC::DecoderMethod JSON::ProjectModel::decoderMethod() const
{
  return m_frameDecoder;
}

/**
 * @brief Retrieves the current strategy used for detecting data frames.
 *
 * This function returns the detection method currently set for identifying data
 * frames.
 *
 * @return The current decoder method as a value from the `FrameDetection` enum.
 */
WC::FrameDetection JSON::ProjectModel::frameDetection() const
{
  return m_frameDetection;
}

//------------------------------------------------------------------------------
// Document information functions
//------------------------------------------------------------------------------

/**
 * @brief Retrieves the name of the JSON file associated with the project.
 *
 * If the file path is not empty, it extracts and returns the file name.
 * If no file path is set, it returns "New JSON".
 *
 * @return The name of the JSON file or "New JSON" if no file is present.
 */
QString JSON::ProjectModel::jsonFileName() const
{
  if (!jsonFilePath().isEmpty())
  {
    auto fileInfo = QFileInfo(m_filePath);
    return fileInfo.fileName();
  }

  return tr("New Project");
}

/**
 * @brief Retrieves the default path for JSON project files.
 *
 * Returns the path where JSON project files are stored, creating the directory
 * if it does not exist.
 *
 * @return The default file path for JSON project files.
 */
QString JSON::ProjectModel::jsonProjectsPath() const
{
  // Get file name and path
  static QString path
      = QString("%1/Documents/%2/JSON Projects/")
            .arg(QDir::homePath(), qApp->applicationDisplayName());

  // Generate file path if required
  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(".");

  return path;
}

/**
 * @brief Returns the server address of the OSM provider from SdrAngel
 */
QString JSON::ProjectModel::osmAddress() const
{
  return QStringLiteral("http://localhost:%1").arg(m_server.serverPort());
}

/**
 * @brief Retrieves the currently selected item's text.
 *
 * This function returns the text of the currently selected item in the tree
 * model. If no item is selected, an empty string is returned.
 *
 * @return The selected item's text, or an empty string if nothing is selected.
 */
QString JSON::ProjectModel::selectedText() const
{
  if (!m_selectionModel || !m_treeModel)
    return "";

  const auto index = m_selectionModel->currentIndex();
  const auto data = m_treeModel->data(index, TreeViewText);
  return data.toString();
}

/**
 * @brief Retrieves the currently selected item's icon.
 *
 * This function returns the icon associated with the currently selected item
 * in the tree model. If no item is selected, an empty string is returned.
 *
 * @return The selected item's icon, or an empty string if nothing is selected.
 */
QString JSON::ProjectModel::selectedIcon() const
{
  if (!m_selectionModel || !m_treeModel)
    return "";

  const auto index = m_selectionModel->currentIndex();
  const auto data = m_treeModel->data(index, TreeViewIcon);
  return data.toString();
}

/**
 * @brief Retrieves the project title.
 *
 * This function returns the title of the current project.
 *
 * @return A reference to the project title.
 */
const QString &JSON::ProjectModel::title() const
{
  return m_title;
}

/**
 * @brief Retrieves the current icon of the selected action.
 */
const QString JSON::ProjectModel::actionIcon() const
{
  return m_selectedAction.icon();
}

/**
 * @brief Retrieves the list of available action icons.
 *
 * This function searches for all SVG files in a predefined resource path,
 * strips their paths and file extensions, and returns the base names as a list.
 * The icons are cached the first time this function is called to improve
 * performance on subsequent calls.
 *
 * @return A QStringList containing the base names of all available action
 * icons.
 */
const QStringList &JSON::ProjectModel::availableActionIcons() const
{
  static QStringList icons;

  if (icons.isEmpty())
  {
    const auto path = QStringLiteral(":/rcc/actions/");
    QDirIterator it(path, QStringList() << "*.svg", QDir::Files);
    while (it.hasNext())
    {
      const auto filePath = it.next();
      const QFileInfo fileInfo(filePath);
      icons.append(fileInfo.baseName());
    }
  }

  return icons;
}
/**
 * @brief Retrieves the file path of the JSON file.
 *
 * This function returns the full path of the current JSON file associated with
 * the project.
 *
 * @return A reference to the file path of the JSON file.
 */
const QString &JSON::ProjectModel::jsonFilePath() const
{
  return m_filePath;
}

/**
 * @brief Retrieves the frame parser code.
 *
 * This function returns the code used for parsing frames in the project.
 *
 * @return A reference to the frame parser code.
 */
const QString &JSON::ProjectModel::frameParserCode() const
{
  return m_frameParserCode;
}

/**
 * @brief Retrieves the MapTiler API key used by the project.
 *
 * This function returns the API key used for accessing MapTiler mapping
 * services.
 *
 * @return A reference to the Thunderforest API key.
 */
const QString &JSON::ProjectModel::mapTilerApiKey() const
{
  return m_mapTilerApiKey;
}

/**
 * @brief Retrieves the Thunderforest API key used by the project.
 *
 * This function returns the API key used for accessing Thunderforest mapping
 * services.
 *
 * @return A reference to the Thunderforest API key.
 */
const QString &JSON::ProjectModel::thunderforestApiKey() const
{
  return m_thunderforestApiKey;
}

/**
 * @brief Determines whether the currently selected group is editable.
 *
 * This function checks if the currently selected group is editable.
 *
 * Groups with automatically generateddatasets (such as those with specific
 * widgets like "accelerometer" or "gyroscope") are not editable.
 *
 * Only custom groups or groups with the "multiplot" widget are editable.
 *
 * @return True if the group is editable, false otherwise.
 */
bool JSON::ProjectModel::currentGroupIsEditable() const
{
  if (m_currentView == GroupView)
  {
    const auto widget = m_selectedGroup.widget();
    if (widget != "" && widget != "multiplot" && widget != "datagrid")
      return false;
  }

  return true;
}

/**
 * @brief Determines whether the currently selected dataset is editable.
 *
 * This function checks if the currently selected dataset is editable.
 * Datasets within groups that have automatically generated datasets
 * (such as those associated with widgets like "accelerometer" or "gyroscope")
 * are not editable.
 *
 * Only datasets in custom groups or groups with the "multiplot" widget are
 * editable.
 *
 * @return True if the dataset is editable, false otherwise.
 */
bool JSON::ProjectModel::currentDatasetIsEditable() const
{
  if (m_currentView == DatasetView)
  {
    const auto groupId = m_selectedDataset.groupId();
    if (m_groups.count() > groupId)
    {
      const auto widget = m_groups[groupId].widget();
      if (widget != "" && widget != "multiplot" && widget != "datagrid")
        return false;
    }
  }

  return true;
}

/**
 * @brief Retrieves the number of groups in the project.
 *
 * This function returns the total count of groups currently present in the
 * project.
 *
 * @return The number of groups.
 */
int JSON::ProjectModel::groupCount() const
{
  return groups().count();
}

/**
 * @brief Retrieves the dataset options for the selected dataset.
 *
 * This function returns a bitmask representing the options enabled for the
 * selected dataset, such as whether it supports plotting, FFT, or specific
 * widgets (bar, gauge, compass).
 *
 * @return A bitmask of dataset options as a quint8 value.
 */
quint8 JSON::ProjectModel::datasetOptions() const
{
  quint8 option = WC::DatasetGeneric;

  if (m_selectedDataset.graph())
    option |= WC::DatasetPlot;

  if (m_selectedDataset.fft())
    option |= WC::DatasetFFT;

  if (m_selectedDataset.led())
    option |= WC::DatasetLED;

  if (m_selectedDataset.widget() == QStringLiteral("bar"))
    option |= WC::DatasetBar;

  else if (m_selectedDataset.widget() == QStringLiteral("gauge"))
    option |= WC::DatasetGauge;

  else if (m_selectedDataset.widget() == QStringLiteral("compass"))
    option |= WC::DatasetCompass;

  return option;
}

/**
 * @brief Retrieves the list of groups in the project.
 *
 * This function returns a reference to the vector of JSON::Group objects,
 * representing all the groups present in the project.
 *
 * @return A reference to the vector of groups.
 */
const QVector<JSON::Group> &JSON::ProjectModel::groups() const
{
  return m_groups;
}

//------------------------------------------------------------------------------
// Model access functions
//------------------------------------------------------------------------------

/**
 * @brief Retrieves the tree model used in the project.
 *
 * This function returns the @c CustomModel that represents the tree structure
 * of the project.
 *
 * @return A pointer to the tree model.
 */
JSON::CustomModel *JSON::ProjectModel::treeModel() const
{
  return m_treeModel;
}

/**
 * @brief Retrieves the selection model.
 *
 * This function returns the @c QItemSelectionModel used to manage and track
 * item selections in the tree model.
 *
 * @return A pointer to the selection model.
 */
QItemSelectionModel *JSON::ProjectModel::selectionModel() const
{
  return m_selectionModel;
}

/**
 * @brief Retrieves the model for the current group.
 *
 * This function returns the @c CustomModel that represents the data of the
 * currently selected group in the project.
 *
 * @return A pointer to the group model.
 */
JSON::CustomModel *JSON::ProjectModel::groupModel() const
{
  return m_groupModel;
}

/**
 * @brief Retrieves the model for the current action.
 *
 * This function returns the @c CustomModel that represents the data of the
 * currently selected action in the project.
 *
 * @return A pointer to the action model.
 */
JSON::CustomModel *JSON::ProjectModel::actionModel() const
{
  return m_actionModel;
}

/**
 * @brief Retrieves the project model.
 *
 * This function returns the @c CustomModel that represents the basic parameters
 * of the project, such as it's title, the start/end sequences and data
 * conversion method to use.
 *
 * @return A pointer to the project model.
 */
JSON::CustomModel *JSON::ProjectModel::projectModel() const
{
  return m_projectModel;
}

/**
 * @brief Retrieves the model for the current dataset.
 *
 * This function returns the @c CustomModel that represents the data of the
 * currently selected dataset in the project.
 *
 * @return A pointer to the dataset model.
 */
JSON::CustomModel *JSON::ProjectModel::datasetModel() const
{
  return m_datasetModel;
}

//------------------------------------------------------------------------------
// Document saving/export
//------------------------------------------------------------------------------

/**
 * @brief Prompts the user to save unsaved changes in the project.
 *
 * If the project has been modified, this function asks the user whether they
 * want to save, discard, or cancel the operation.
 *
 * It will save the project if the user selects "Save" and return a
 * corresponding boolean value based on the user's choice.
 *
 * @return True if the user chooses to save or discard changes, false if the
 *         user cancels the operation.
 */
bool JSON::ProjectModel::askSave()
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
 * @brief Saves the current project as a JSON file.
 *
 * This function saves the current state of the project, including its title,
 * decoder settings, and groups, into a JSON file. If the file path is not
 * specified, the user is prompted to provide one. It writes the JSON data to
 * disk and loads the saved file into the application.
 *
 * @return True if the project was saved successfully, false otherwise.
 */
bool JSON::ProjectModel::saveJsonFile()
{
  // Validate project title
  if (m_title.isEmpty())
  {
    Misc::Utilities::showMessageBox(tr("Project error"),
                                    tr("Project title cannot be empty!"));
    return false;
  }

  // Get file save path
  if (jsonFilePath().isEmpty())
  {
    auto path = QFileDialog::getSaveFileName(nullptr, tr("Save JSON project"),
                                             jsonProjectsPath(), "*.json");
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
  json.insert("title", m_title);
  json.insert("separator", m_separator);
  json.insert("decoder", m_frameDecoder);
  json.insert("frameEnd", m_frameEndSequence);
  json.insert("frameParser", m_frameParserCode);
  json.insert("frameDetection", m_frameDetection);
  json.insert("frameStart", m_frameStartSequence);
  json.insert("mapTilerApiKey", m_mapTilerApiKey);
  json.insert("thunderforestApiKey", m_thunderforestApiKey);

  // Create group array
  QJsonArray groupArray;
  for (const auto &group : m_groups)
    groupArray.append(group.serialize());

  // Add groups array to JSON
  json.insert("groups", groupArray);

  // Create actions array
  QJsonArray actionsArray;
  for (const auto &action : m_actions)
    actionsArray.append(action.serialize());

  // Insert actions array to JSON
  json.insert("actions", actionsArray);

  // Write JSON data to file
  file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
  file.close();

  // Load JSON file to Serial Studio
  openJsonFile(file.fileName());
  JSON::FrameBuilder::instance().loadJsonMap(file.fileName());
  return true;
}

//------------------------------------------------------------------------------
// Document initialization
//------------------------------------------------------------------------------

/**
 * @brief Initializes a new JSON project.
 *
 * This function clears the current groups, resets project properties
 * (such as the title, separator, frame decoder, and sequences), and sets
 * default values for the project.
 *
 * It also updates the internal models, removes the modified state, and
 * switches the view to the project view.
 *
 * Relevant signals are emitted to notify the UI of these changes.
 */
void JSON::ProjectModel::newJsonFile()
{
  // Clear groups list
  m_groups.clear();

  // Clear actions list
  m_actions.clear();

  // Reset project properties
  m_separator = ",";
  m_frameDecoder = WC::PlainText;
  m_frameDetection = WC::EndDelimiterOnly;
  m_frameEndSequence = "\\n";
  m_mapTilerApiKey = "";
  m_thunderforestApiKey = "";
  m_frameStartSequence = "$";
  m_title = tr("Untitled Project");
  m_frameParserCode = JSON::FrameParser::defaultCode();

  // Update file path
  m_filePath = "";

  // Update the models
  buildTreeModel();
  buildProjectModel();

  // Set project view
  setCurrentView(ProjectView);

  // Emit signals
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT gpsApiKeysChanged();
  Q_EMIT frameParserCodeChanged();

  // Reset modified flag
  setModified(false);
}

//------------------------------------------------------------------------------
// Document loading/import
//------------------------------------------------------------------------------

/**
 * @brief Opens a JSON file by prompting the user to select a file.
 *
 * This function opens a file dialog for the user to select a JSON project file.
 * If a valid file is selected, the project is loaded from the file.
 *
 * If the file path is invalid or no file is selected, the operation is aborted.
 */
void JSON::ProjectModel::openJsonFile()
{
  // Let user select a file
  const auto path = QFileDialog::getOpenFileName(
      nullptr, tr("Select JSON file"), jsonProjectsPath(), "*.json");

  // Invalid path, abort
  if (path.isEmpty())
    return;

  // Open the JSON file
  openJsonFile(path);
}

/**
 * @brief Opens and loads a JSON project file from the given path.
 *
 * This function opens a JSON file from the specified file path, validates the
 * content, and loads the project data into the application.
 *
 * It reads the project settings, groups, and updates the models accordingly.
 *
 * It also modifies the @c IO::Manager settings based on the loaded data and
 * emitssignals to update the UI.
 *
 * @param path The file path of the JSON project to load.
 */
void JSON::ProjectModel::openJsonFile(const QString &path)
{
  // Validate path
  if (path.isEmpty())
    return;

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
  if (JSON::FrameBuilder::instance().jsonMapFilepath() != path)
    JSON::FrameBuilder::instance().loadJsonMap(path);

  // Reset C++ model
  newJsonFile();

  // Update current JSON document
  m_filePath = path;

  // Read data from JSON document
  auto json = document.object();
  m_title = json.value("title").toString();
  m_separator = json.value("separator").toString();
  m_frameEndSequence = json.value("frameEnd").toString();
  m_frameParserCode = json.value("frameParser").toString();
  m_frameStartSequence = json.value("frameStart").toString();
  m_mapTilerApiKey = json.value("mapTilerApiKey").toString();
  m_thunderforestApiKey = json.value("thunderforestApiKey").toString();
  m_frameDecoder
      = static_cast<WC::DecoderMethod>(json.value("decoder").toInt());
  m_frameDetection
      = static_cast<WC::FrameDetection>(json.value("frameDetection").toInt());

  // Preserve compatibility with previous projects
  if (!json.contains("frameDetection"))
    m_frameDetection = WC::StartAndEndDelimiter;

  // Read groups from JSON document
  auto groups = json.value("groups").toArray();
  for (int g = 0; g < groups.count(); ++g)
  {
    JSON::Group group(g);
    if (group.read(groups.at(g).toObject()))
      m_groups.append(group);
  }

  // Read actions from JSON document
  auto actions = json.value("actions").toArray();
  for (int a = 0; a < actions.count(); ++a)
  {
    JSON::Action action(a);
    if (action.read(actions.at(a).toObject()))
      m_actions.append(action);
  }

  // Regenerate the tree model
  buildProjectModel();
  buildTreeModel();

  // Show project view
  setCurrentView(ProjectView);

  // Update UI
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT gpsApiKeysChanged();
  Q_EMIT frameParserCodeChanged();

  // Reset modified flag
  setModified(false);
}

//------------------------------------------------------------------------------
// Group/dataset operations
//------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected group.
 *
 * This function prompts the user for confirmation before deleting the currently
 * selected group. If the user confirms, the group is removed from the project,
 * and group and dataset IDs areregenerated. The tree model is rebuilt,
 * the modified flag is set, and the project item is selected in the UI.
 */
void JSON::ProjectModel::deleteCurrentGroup()
{
  // Ask the user for confirmation
  const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete group \"%1\"?").arg(m_selectedGroup.title()),
      tr("This action cannot be undone. Do you wish to proceed?"), APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

  // Validate the user input
  if (ret != QMessageBox::Yes)
    return;

  // Delete the group
  m_groups.removeAt(m_selectedGroup.groupId());

  // Regenerate group IDs
  int id = 0;
  for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id)
  {
    g->m_groupId = id;
    for (auto d = g->m_datasets.begin(); d != g->m_datasets.end(); ++d)
      d->m_groupId = id;
  }

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select project item
  const auto index = m_treeModel->index(0, 0);
  m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Deletes the currently action group.
 *
 * This function prompts the user for confirmation before deleting the currently
 * selected action. If the user confirms, the action is removed from the
 * project, and action IDs areregenerated. The tree model is rebuilt, the
 * modified flag is set, and the project item is selected in the UI.
 */
void JSON::ProjectModel::deleteCurrentAction()
{
  // Ask the user for confirmation
  const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete action \"%1\"?").arg(m_selectedAction.title()),
      tr("This action cannot be undone. Do you wish to proceed?"), APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

  // Validate the user input
  if (ret != QMessageBox::Yes)
    return;

  // Delete the action
  m_actions.removeAt(m_selectedAction.actionId());

  // Regenerate action IDs
  int id = 0;
  for (auto a = m_actions.begin(); a != m_actions.end(); ++a, ++id)
    a->m_actionId = id;

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select project item
  const auto index = m_treeModel->index(0, 0);
  m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Deletes the currently selected dataset.
 *
 * This function prompts the user for confirmation before deleting the currently
 * selected dataset. If the user confirms, the dataset is removed from the
 * associated group, and dataset IDs within the group are reassigned.
 * The tree model is rebuilt, the modified flag is set, and the parent group is
 * selected in the UI.
 */
void JSON::ProjectModel::deleteCurrentDataset()
{
  // Ask the user for confirmation
  const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete dataset \"%1\"?")
          .arg(m_selectedDataset.title()),
      tr("This action cannot be undone. Do you wish to proceed?"), APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

  // Validate the user input
  if (ret != QMessageBox::Yes)
    return;

  // Get group ID & dataset ID
  const auto groupId = m_selectedDataset.groupId();
  const auto datasetId = m_selectedDataset.datasetId();

  // Remove dataset
  m_groups[groupId].m_datasets.removeAt(datasetId);

  // Reassign dataset IDs
  int id = 0;
  auto begin = m_groups[groupId].m_datasets.begin();
  auto end = m_groups[groupId].m_datasets.end();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->m_datasetId = id;

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select parent group
  for (auto i = m_groupItems.constBegin(); i != m_groupItems.constEnd(); ++i)
  {
    if (i.value().groupId() == groupId)
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Duplicates the currently selected group.
 *
 * This function creates a copy of the currently selected group, including all
 * its datasets. The new group is registered, the tree model is updated, and
 * the modified flag is set. The duplicated group is then selected in the UI.
 */
void JSON::ProjectModel::duplicateCurrentGroup()
{
  // Initialize a new group
  auto group = JSON::Group(m_groups.count());
  group.m_widget = m_selectedGroup.widget();
  group.m_title = tr("%1 (Copy)").arg(m_selectedGroup.title());
  for (auto i = 0; i < m_selectedGroup.m_datasets.count(); ++i)
  {
    auto dataset = m_selectedGroup.m_datasets.at(i);
    dataset.m_groupId = group.groupId();
    dataset.m_index = nextDatasetIndex() + i;
    group.m_datasets.append(dataset);
  }

  // Register the group
  m_groups.append(group);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select the group
  for (auto i = m_groupItems.constBegin(); i != m_groupItems.constEnd(); ++i)
  {
    if (i.value().groupId() == group.groupId())
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Duplicates the currently selected action.
 *
 * This function creates a copy of the currently selected action.
 * The new action is registered, the tree model is updated, and
 * the modified flag is set. The duplicated action is then selected in the UI.
 */
void JSON::ProjectModel::duplicateCurrentAction()
{
  // Initialize a new group
  auto action = JSON::Action(m_actions.count());
  action.m_title = tr("%1 (Copy)").arg(m_selectedAction.title());
  action.m_eolSequence = m_selectedAction.eolSequence();
  action.m_txData = m_selectedAction.txData();
  action.m_icon = m_selectedAction.icon();

  // Register the group
  m_actions.append(action);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select the action
  for (auto i = m_actionItems.constBegin(); i != m_actionItems.constEnd(); ++i)
  {
    if (i.value().actionId() == action.actionId())
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Duplicates the currently selected dataset.
 *
 * This function creates a copy of the currently selected dataset.
 * The new dataset is registered under the same group, the tree model is
 * updated, and the modified flag is set. The duplicated dataset is then
 * selected in the UI.
 */
void JSON::ProjectModel::duplicateCurrentDataset()
{
  // Initialize a new dataset
  auto dataset = m_selectedDataset;
  dataset.m_title = tr("%1 (Copy)").arg(dataset.title());
  dataset.m_index = nextDatasetIndex();
  dataset.m_datasetId = m_groups[dataset.groupId()].datasetCount();

  // Register the dataset to the group
  m_groups[dataset.groupId()].m_datasets.append(dataset);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select dataset
  for (auto i = m_datasetItems.begin(); i != m_datasetItems.end(); ++i)
  {
    if (i.value().groupId() == dataset.groupId()
        && i.value().datasetId() == dataset.datasetId())
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Adds a new dataset to the currently selected group.
 *
 * This function creates a new dataset based on the provided option, assigns it
 * a unique title, ID, and frame index, and adds it to the selected group.
 * The tree model is rebuilt, the modification flag is set, and the group is
 * reselected to rebuild the dataset model.
 *
 * @param option The dataset option that defines the type of dataset to add
 *               (e.g., Plot, FFT, Bar, Gauge, Compass).
 */
void JSON::ProjectModel::addDataset(const WC::DatasetOption option)
{
  // Initialize a new dataset
  const auto groupId = m_selectedGroup.groupId();
  JSON::Dataset dataset(groupId);

  // Configure dataset options
  QString title;
  switch (option)
  {
    case WC::DatasetGeneric:
      title = tr("New Dataset");
      break;
    case WC::DatasetPlot:
      title = tr("New Plot");
      dataset.m_graph = true;
      break;
    case WC::DatasetFFT:
      title = tr("New FFT Plot");
      dataset.m_fft = true;
      break;
    case WC::DatasetBar:
      title = tr("New Bar Widget");
      dataset.m_widget = QStringLiteral("bar");
      break;
    case WC::DatasetGauge:
      title = tr("New Gauge");
      dataset.m_widget = QStringLiteral("gauge");
      break;
    case WC::DatasetCompass:
      title = tr("New Compass");
      dataset.m_widget = QStringLiteral("compass");
      break;
    case WC::DatasetLED:
      title = tr("New LED Indicator");
      dataset.m_led = true;
      break;
    default:
      break;
  }

  // Check if any existing dataset has the same title
  int count = 1;
  QString newTitle = title;
  for (const auto &d : m_groups[groupId].m_datasets)
  {
    if (d.m_title == newTitle)
    {
      count++;
      newTitle = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  // If the title was modified, ensure it is unique
  while (count > 1)
  {
    bool titleExists = false;
    for (const auto &d : m_groups[groupId].m_datasets)
    {
      if (d.m_title == newTitle)
      {
        count++;
        newTitle = QString("%1 (%2)").arg(title).arg(count);
        titleExists = true;
        break;
      }
    }

    if (!titleExists)
      break;
  }

  // Assign dataset title, ID & frame index
  dataset.m_title = newTitle;
  dataset.m_index = nextDatasetIndex();
  dataset.m_datasetId = m_groups[groupId].m_datasets.count();

  // Add dataset to group
  m_groups[groupId].m_datasets.append(dataset);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select group item again to rebuild dataset model
  for (auto i = m_groupItems.begin(); i != m_groupItems.end(); ++i)
  {
    if (i.value().groupId() == groupId)
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Changes the selected dataset's options.
 *
 * This function modifies the currently selected dataset's options (e.g.,
 * enabling/disabling plots, FFT, or widgets) based on the provided option and
 * checked state. The dataset is updated, the tree model is rebuilt, and the
 * dataset is reselected to rebuild the dataset model.
 *
 * @param option The dataset option to modify (e.g., Plot, Bar, Gauge, etc).
 * @param checked A boolean indicating whether the option should be enabled
 *                (true) or disabled (false).
 */
void JSON::ProjectModel::changeDatasetOption(const WC::DatasetOption option,
                                             const bool checked)
{
  // Modify dataset options
  switch (option)
  {
    case WC::DatasetPlot:
      m_selectedDataset.m_graph = checked;
      break;
    case WC::DatasetFFT:
      m_selectedDataset.m_fft = checked;
      break;
    case WC::DatasetBar:
      m_selectedDataset.m_widget = checked ? QStringLiteral("bar") : "";
      break;
    case WC::DatasetGauge:
      m_selectedDataset.m_widget = checked ? QStringLiteral("gauge") : "";
      break;
    case WC::DatasetCompass:
      m_selectedDataset.m_widget = checked ? QStringLiteral("compass") : "";
      break;
    case WC::DatasetLED:
      m_selectedDataset.m_led = checked;
      break;
    default:
      break;
  }

  // Replace dataset
  const auto groupId = m_selectedDataset.groupId();
  const auto datasetId = m_selectedDataset.datasetId();
  m_groups[groupId].m_datasets.replace(datasetId, m_selectedDataset);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select dataset item again to rebuild dataset model
  for (auto i = m_datasetItems.begin(); i != m_datasetItems.end(); ++i)
  {
    if (i.value().datasetId() == datasetId && i.value().groupId() == groupId)
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Adds a new action to the project.
 *
 * This function creates a new action, ensuring the title is unique by appending
 * a number if necessary. It then registers the action, updates the tree model,
 * and sets the modified flag. The newly added action is selected in the user
 * interface.
 */
void JSON::ProjectModel::addAction()
{
  // Check if any existing group has the same title
  int count = 1;
  QString title = tr("New Action");
  for (const auto &action : m_actions)
  {
    if (action.m_title == title)
    {
      count++;
      title = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  // If the title was modified, ensure it is unique
  while (count > 1)
  {
    bool titleExists = false;
    for (const auto &action : m_actions)
    {
      if (action.m_title == title)
      {
        count++;
        title = QString("%1 (%2)").arg(title).arg(count);
        titleExists = true;
        break;
      }
    }

    if (!titleExists)
      break;
  }

  // Create a new action
  JSON::Action action(m_actions.count());
  action.m_title = title;

  // Register the action
  m_actions.append(action);

  // Update the user interface
  buildTreeModel();
  setModified(true);

  // Select action
  for (auto i = m_actionItems.constBegin(); i != m_actionItems.constEnd(); ++i)
  {
    if (i.value().actionId() == action.actionId())
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Adds a new group to the project with the specified title and widget.
 *
 * This function creates a new group, ensuring the title is unique by appending
 * a number if necessary. It then registers the group, sets its widget type,
 * updates the tree model, and sets the modified flag. The newly added group is
 * selected in the user interface.
 *
 * @param title The desired title for the new group.
 * @param widget The widget type associated with the group.
 */
void JSON::ProjectModel::addGroup(const QString &title,
                                  const WC::GroupWidget widget)
{
  // Check if any existing group has the same title
  int count = 1;
  QString newTitle = title;
  for (const auto &group : m_groups)
  {
    if (group.m_title == newTitle)
    {
      count++;
      newTitle = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  // If the title was modified, ensure it is unique
  while (count > 1)
  {
    bool titleExists = false;
    for (const auto &group : m_groups)
    {
      if (group.m_title == newTitle)
      {
        count++;
        newTitle = QString("%1 (%2)").arg(title).arg(count);
        titleExists = true;
        break;
      }
    }

    if (!titleExists)
      break;
  }

  // Create a new group
  JSON::Group group(m_groups.count());
  group.m_title = newTitle;

  // Register the group & add the widget
  m_groups.append(group);
  setGroupWidget(m_groups.count() - 1, widget);

  // Update the user interface
  buildTreeModel();
  setModified(true);

  // Select group
  for (auto i = m_groupItems.constBegin(); i != m_groupItems.constEnd(); ++i)
  {
    if (i.value().groupId() == group.groupId())
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Sets the widget type for the specified group and optionally creates
 *        associated datasets.
 *
 * This function assigns a widget type to the specified group based on the
 * provided `GroupWidget` value. If the group contains existing datasets and a
 * new widget type is selected, the user is prompted for confirmation as
 * existing datasets may be deleted.
 *
 * Depending on the selected widget (e.g., Accelerometer, Gyroscope, GPS), the
 * function may create default datasets with specific properties. The group is
 * updated in the internal list and the UI is refreshed.
 *
 * @param group The index of the group to modify.
 * @param widget The type of widget to assign to the group.
 * @return True if the widget was successfully assigned, false otherwise.
 */
bool JSON::ProjectModel::setGroupWidget(const int group,
                                        const WC::GroupWidget widget)
{
  // Get group data
  auto grp = m_groups.at(group);
  const auto groupId = grp.groupId();

  // Warn user if group contains existing datasets
  if (!(grp.m_datasets.isEmpty()))
  {
    if ((widget == WC::DataGrid || widget == WC::MultiPlot
         || widget == WC::NoGroupWidget)
        && (grp.widget() == "multiplot" || grp.widget() == "datagrid"
            || grp.widget() == ""))
      grp.m_widget = "";

    else
    {
      auto ret = Misc::Utilities::showMessageBox(
          tr("Are you sure you want to change the group-level widget?"),
          tr("Existing datasets for this group will be deleted"), APP_NAME,
          QMessageBox::Yes | QMessageBox::No);
      if (ret == QMessageBox::No)
        return false;
      else
        grp.m_datasets.clear();
    }
  }

  // No widget
  if (widget == WC::NoGroupWidget)
    grp.m_widget = "";

  // Data grid widget
  if (widget == WC::DataGrid)
    grp.m_widget = "datagrid";

  // Multiplot widget
  else if (widget == WC::MultiPlot)
    grp.m_widget = "multiplot";

  // Accelerometer widget
  else if (widget == WC::Accelerometer)
  {
    // Set widget type
    grp.m_widget = "accelerometer";

    // Create datasets
    JSON::Dataset x, y, z;

    // Set dataset IDs
    x.m_datasetId = 0;
    y.m_datasetId = 1;
    z.m_datasetId = 2;

    // Register parent group for the datasets
    x.m_groupId = groupId;
    y.m_groupId = groupId;
    z.m_groupId = groupId;

    // Set dataset indexes
    x.m_index = nextDatasetIndex();
    y.m_index = nextDatasetIndex() + 1;
    z.m_index = nextDatasetIndex() + 2;

    // Set measurement units
    x.m_units = "m/s²";
    y.m_units = "m/s²";
    z.m_units = "m/s²";

    // Set dataset properties
    x.m_widget = "x";
    y.m_widget = "y";
    z.m_widget = "z";
    x.m_graph = true;
    y.m_graph = true;
    z.m_graph = true;
    x.m_title = tr("Accelerometer %1").arg("X");
    y.m_title = tr("Accelerometer %1").arg("Y");
    z.m_title = tr("Accelerometer %1").arg("Z");

    // Add datasets to group
    grp.m_datasets.append(x);
    grp.m_datasets.append(y);
    grp.m_datasets.append(z);
  }

  // Gyroscope widget
  else if (widget == WC::Gyroscope)
  {
    // Set widget type
    grp.m_widget = "gyro";

    // Create datasets
    JSON::Dataset x, y, z;

    // Set dataset IDs
    x.m_datasetId = 0;
    y.m_datasetId = 1;
    z.m_datasetId = 2;

    // Register parent group for the datasets
    x.m_groupId = groupId;
    y.m_groupId = groupId;
    z.m_groupId = groupId;

    // Set dataset indexes
    x.m_index = nextDatasetIndex();
    y.m_index = nextDatasetIndex() + 1;
    z.m_index = nextDatasetIndex() + 2;

    // Set measurement units
    x.m_units = "deg/s";
    y.m_units = "deg/s";
    z.m_units = "deg/s";

    // Set dataset properties
    x.m_widget = "x";
    y.m_widget = "y";
    z.m_widget = "z";
    x.m_graph = true;
    y.m_graph = true;
    z.m_graph = true;
    x.m_title = tr("Gyro %1").arg("X");
    y.m_title = tr("Gyro %1").arg("Y");
    z.m_title = tr("Gyro %1").arg("Z");

    // Add datasets to group
    grp.m_datasets.append(x);
    grp.m_datasets.append(y);
    grp.m_datasets.append(z);
  }

  // Map widget
  else if (widget == WC::GPS)
  {
    // Set widget type
    grp.m_widget = "map";

    // Create datasets
    JSON::Dataset lat, lon, alt;

    // Set dataset IDs
    lat.m_datasetId = 0;
    lon.m_datasetId = 1;
    alt.m_datasetId = 2;

    // Register parent group for the datasets
    lat.m_groupId = groupId;
    lon.m_groupId = groupId;
    alt.m_groupId = groupId;

    // Set dataset indexes
    lat.m_index = nextDatasetIndex();
    lon.m_index = nextDatasetIndex() + 1;
    alt.m_index = nextDatasetIndex() + 2;

    // Set measurement units
    lat.m_units = "°";
    lon.m_units = "°";
    alt.m_units = "m";

    // Set dataset properties
    lat.m_widget = "lat";
    lon.m_widget = "lon";
    alt.m_widget = "alt";
    lat.m_title = tr("Latitude");
    lon.m_title = tr("Longitude");
    alt.m_title = tr("Altitude");

    // Add datasets to group
    grp.m_datasets.append(lat);
    grp.m_datasets.append(lon);
    grp.m_datasets.append(alt);
  }

  // Replace previous group with new group
  m_groups.replace(group, grp);

  // Update UI
  return true;
}

//------------------------------------------------------------------------------
// Frame parser code modification
//------------------------------------------------------------------------------

/**
 * @brief Sets the frame parser code for the project.
 *
 * This function updates the frame parser code with the provided string if it
 * differs from the current value. It also sets the project as modified and
 * emits the `frameParserCodeChanged()` signal to notify the UI of the change.
 *
 * @param code The new frame parser code to set.
 */
void JSON::ProjectModel::setFrameParserCode(const QString &code)
{
  if (code != m_frameParserCode)
  {
    m_frameParserCode = code;
    setModified(true);

    Q_EMIT frameParserCodeChanged();
  }
}

//------------------------------------------------------------------------------
// Model generation code
//------------------------------------------------------------------------------

/**
 * @brief Builds the tree model that represents the hierarchical structure of
 *        the project.
 *
 * This function clears the current tree model and its associated selection
 * model, then creates a new `CustomModel` for the project hierarchy.
 *
 * It populates the tree with the root project information, the frame parser
 * function, and groups with their respective datasets.
 *
 * The function also restores the expanded state of the tree items from the
 * previous model and sets up the selection model for interaction.
 *
 * Finally, it emits the `treeModelChanged()` signal to update the UI.
 */
void JSON::ProjectModel::buildTreeModel()
{
  // Clear model/pointer maps
  m_rootItems.clear();
  m_groupItems.clear();
  m_actionItems.clear();
  m_datasetItems.clear();

  // Store previous tree model's expanded states in a hash
  QHash<QString, bool> expandedStates;
  if (m_treeModel)
    saveExpandedStateMap(m_treeModel->invisibleRootItem(), expandedStates, "");

  // Delete the existing selection model
  if (m_selectionModel)
  {
    disconnect(m_selectionModel);
    m_selectionModel->deleteLater();
    m_selectionModel = nullptr;
  }

  // Delete the existing tree model
  if (m_treeModel)
  {
    disconnect(m_treeModel);
    m_treeModel->deleteLater();
    m_treeModel = nullptr;
  }

  // Create a new CustomModel for the tree
  m_treeModel = new CustomModel(this);

  // Add project information section
  auto *root = new QStandardItem(title());
  root->setData(root->text(), TreeViewText);

  // Add "root" project items
  auto *frameParsingCode = new QStandardItem(tr("Frame Parser Function"));

  // Set display roles for root project items
  frameParsingCode->setData(frameParsingCode->text(), TreeViewText);

  // Set decoration roles (icons) for root items section
  // clang-format off
  root->setData("qrc:/rcc/icons/project-editor/treeview/project-setup.svg", TreeViewIcon);
  frameParsingCode->setData("qrc:/rcc/icons/project-editor/treeview/code.svg", TreeViewIcon);
  // clang-format on

  // Expand project root item
  root->setData(true, TreeViewExpanded);

  // Generate tree hierarchy for root items
  root->appendRow(frameParsingCode);
  m_treeModel->appendRow(root);

  // Create relationship with root items & indices
  m_rootItems.insert(root, kRootItem);
  m_rootItems.insert(frameParsingCode, kFrameParser);

  // Iterare through the actions and add them to the model
  for (int aIndex = 0; aIndex < m_actions.size(); ++aIndex)
  {
    // Create action item
    const auto action = m_actions[aIndex];
    auto *actionItem = new QStandardItem(action.title());

    // Configure action item
    const auto icon = "qrc:/rcc/icons/project-editor/treeview/action.svg";
    actionItem->setData(-1, TreeViewFrameIndex);
    actionItem->setData(icon, TreeViewIcon);
    actionItem->setData(action.title(), TreeViewText);

    // Register action item
    root->appendRow(actionItem);
    m_actionItems.insert(actionItem, action);
  }

  // Iterate through the groups and add them to the model
  for (int gIndex = 0; gIndex < m_groups.size(); ++gIndex)
  {
    // Create group item
    const auto group = m_groups[gIndex];
    auto *groupItem = new QStandardItem(group.title());

    // Decide which icon to use for the group
    QString icon;
    if (group.widget() == "map")
      icon = "qrc:/rcc/icons/project-editor/treeview/gps.svg";
    else if (group.widget() == "accelerometer")
      icon = "qrc:/rcc/icons/project-editor/treeview/accelerometer.svg";
    else if (group.widget() == "gyro")
      icon = "qrc:/rcc/icons/project-editor/treeview/gyroscope.svg";
    else if (group.widget() == "multiplot")
      icon = "qrc:/rcc/icons/project-editor/treeview/multiplot.svg";
    else if (group.widget() == "datagrid")
      icon = "qrc:/rcc/icons/project-editor/treeview/datagrid.svg";
    else
      icon = "qrc:/rcc/icons/project-editor/treeview/group.svg";

    // Set metadata for the group item
    groupItem->setData(icon, TreeViewIcon);
    groupItem->setData(-1, TreeViewFrameIndex);
    groupItem->setData(group.title(), TreeViewText);

    // Iterate through the datasets within this group and add them as children
    for (int dIndex = 0; dIndex < group.datasets().size(); ++dIndex)
    {
      // Create dataset item
      const auto dataset = group.datasets()[dIndex];
      auto *datasetItem = new QStandardItem(dataset.title());
      const auto icon = "qrc:/rcc/icons/project-editor/treeview/dataset.svg";
      datasetItem->setData(icon, TreeViewIcon);
      datasetItem->setData(dataset.title(), TreeViewText);
      datasetItem->setData(dataset.index(), TreeViewFrameIndex);

      // Add dataset item as child for group item
      groupItem->appendRow(datasetItem);

      // Create relationship with dataset item & actual dataset
      m_datasetItems.insert(datasetItem, dataset);
    }

    // Restore expanded states for the group
    restoreExpandedStateMap(groupItem, expandedStates,
                            root->text() + "/" + group.title());

    // Add the group item to the root
    root->appendRow(groupItem);

    // Create relationship with group item & actual group
    m_groupItems.insert(groupItem, group);
  }

  // Construct selection model
  m_selectionModel = new QItemSelectionModel(m_treeModel);
  connect(m_selectionModel, &QItemSelectionModel::currentChanged, this,
          &JSON::ProjectModel::onCurrentSelectionChanged);

  // Update user interface
  Q_EMIT treeModelChanged();
}

/**
 * @brief Builds the project model that contains project configuration settings.
 *
 * This function creates a new `CustomModel` for managing and displaying
 * project-level settings such as the title, separator sequence, frame start
 * and end delimiters, data conversion method, and the Thunderforest API key.
 *
 * Each item in the model is editable and has associated metadata like
 * placeholders and descriptions. The function also sets up a signal to handle
 * changes made to the model items and emits the `projectModelChanged()` signal
 * to update the user interface.
 */
void JSON::ProjectModel::buildProjectModel()
{
  // Clear the existing model
  if (m_projectModel)
  {
    disconnect(m_projectModel);
    m_projectModel->deleteLater();
  }

  // Create a new model
  m_projectModel = new CustomModel(this);

  // Add project title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(TextField, WidgetType);
  title->setData(m_title, EditableValue);
  title->setData(tr("Title"), ParameterName);
  title->setData(kProjectView_Title, ParameterType);
  title->setData(tr("Untitled Project"), PlaceholderValue);
  title->setData(tr("Project name/description"), ParameterDescription);
  m_projectModel->appendRow(title);

  // Add decoding
  auto decoding = new QStandardItem();
  decoding->setEditable(true);
  decoding->setData(ComboBox, WidgetType);
  decoding->setData(m_decoderOptions, ComboBoxData);
  decoding->setData(m_frameDecoder, EditableValue);
  decoding->setData(tr("Data Conversion Method"), ParameterName);
  decoding->setData(kProjectView_FrameDecoder, ParameterType);
  decoding->setData(tr("Input data format for frame parser"),
                    ParameterDescription);
  m_projectModel->appendRow(decoding);

  // Add frame detection method
  auto frameDetection = new QStandardItem();
  frameDetection->setEditable(true);
  frameDetection->setData(ComboBox, WidgetType);
  frameDetection->setData(m_frameDetectionMethods, ComboBoxData);
  frameDetection->setData(m_frameDetection, EditableValue);
  frameDetection->setData(tr("Frame Detection"), ParameterName);
  frameDetection->setData(kProjectView_FrameDetection, ParameterType);
  frameDetection->setData(tr("Strategy used for identifying frame data"),
                          ParameterDescription);
  m_projectModel->appendRow(frameDetection);

  // Add frame start sequence
  if (m_frameDetection == WC::StartAndEndDelimiter)
  {
    auto frameStart = new QStandardItem();
    frameStart->setEditable(true);
    frameStart->setData(TextField, WidgetType);
    frameStart->setData(m_frameStartSequence, EditableValue);
    frameStart->setData(tr("Frame Start Delimeter"), ParameterName);
    frameStart->setData(kProjectView_FrameStartSequence, ParameterType);
    frameStart->setData(QStringLiteral("/*"), PlaceholderValue);
    frameStart->setData(tr("String marking the start of a frame"),
                        ParameterDescription);
    m_projectModel->appendRow(frameStart);
  }

  // Add frame end sequence
  auto frameEnd = new QStandardItem();
  frameEnd->setEditable(true);
  frameEnd->setData(TextField, WidgetType);
  frameEnd->setData(m_frameEndSequence, EditableValue);
  frameEnd->setData(tr("Frame End Delimeter"), ParameterName);
  frameEnd->setData(kProjectView_FrameEndSequence, ParameterType);
  frameEnd->setData(QStringLiteral("*/"), PlaceholderValue);
  frameEnd->setData(tr("String marking the end of a frame"),
                    ParameterDescription);
  m_projectModel->appendRow(frameEnd);

  // Add separator sequence
  auto separator = new QStandardItem();
  separator->setEditable(true);
  separator->setData(TextField, WidgetType);
  separator->setData(m_separator, EditableValue);
  separator->setData(tr("Separator Sequence"), ParameterName);
  separator->setData(kProjectView_ItemSeparator, ParameterType);
  separator->setData(QStringLiteral(","), PlaceholderValue);
  separator->setData(tr("String used to split items in a frame"),
                     ParameterDescription);
  m_projectModel->appendRow(separator);

  // Add Thunderforest API Key
  auto thunderforest = new QStandardItem();
  thunderforest->setEditable(true);
  thunderforest->setData(TextField, WidgetType);
  thunderforest->setData(m_thunderforestApiKey, EditableValue);
  thunderforest->setData(tr("Thunderforest API Key"), ParameterName);
  thunderforest->setData(kProjectView_ThunderforestApiKey, ParameterType);
  thunderforest->setData(tr("None"), PlaceholderValue);
  thunderforest->setData(tr("Required for Thunderforest maps"),
                         ParameterDescription);
  m_projectModel->appendRow(thunderforest);

  // Add MapTiler API Key
  auto mapTiler = new QStandardItem();
  mapTiler->setEditable(true);
  mapTiler->setData(TextField, WidgetType);
  mapTiler->setData(m_mapTilerApiKey, EditableValue);
  mapTiler->setData(tr("MapTiler API Key"), ParameterName);
  mapTiler->setData(kProjectView_MapTilerApiKey, ParameterType);
  mapTiler->setData(tr("None"), PlaceholderValue);
  mapTiler->setData(tr("Required for satellite maps"), ParameterDescription);
  m_projectModel->appendRow(mapTiler);

  // Handle edits
  connect(m_projectModel, &CustomModel::itemChanged, this,
          &JSON::ProjectModel::onProjectItemChanged);

  // Update user interface
  Q_EMIT projectModelChanged();
}

/**
 * @brief Builds the group model for managing the settings of a specific group.
 *
 * This function creates a new `CustomModel` to represent the settings of the
 * provided group. It includes editable fields such as the group's title and
 * associated widget. The widget selection is pre-populated with available
 * options, and the current widget is set based on the group's data.
 *
 * The function also sets up a signal to handle item changes in the model and
 * emits the `groupModelChanged()` signal to update the user interface.
 *
 * @param group The group for which the model is being built.
 */
void JSON::ProjectModel::buildGroupModel(const JSON::Group &group)
{
  // Clear the existing model
  if (m_groupModel)
  {
    disconnect(m_groupModel);
    m_groupModel->deleteLater();
  }

  // Create a new model
  m_selectedGroup = group;
  m_groupModel = new CustomModel(this);

  // Add group title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(TextField, WidgetType);
  title->setData(group.title(), EditableValue);
  title->setData(tr("Title"), ParameterName);
  title->setData(kGroupView_Title, ParameterType);
  title->setData(tr("Untitled Group"), PlaceholderValue);
  title->setData(tr("Name or description of the group"), ParameterDescription);
  m_groupModel->appendRow(title);

  // Get appropiate widget index for current group
  int index = 0;
  bool found = false;
  for (auto it = m_groupWidgets.begin(); it != m_groupWidgets.end();
       ++it, ++index)
  {
    if (it.key() == group.widget())
    {
      found = true;
      break;
    }
  }

  // If not found, reset the index to 0
  if (!found)
    index = 0;

  // Add widgets
  auto widget = new QStandardItem();
  widget->setEditable(true);
  widget->setData(ComboBox, WidgetType);
  widget->setData(m_groupWidgets.values(), ComboBoxData);
  widget->setData(index, EditableValue);
  widget->setData(tr("Widget"), ParameterName);
  widget->setData(kGroupView_Widget, ParameterType);
  widget->setData(tr("Group display widget (optional)"), ParameterDescription);
  m_groupModel->appendRow(widget);

  // Handle edits
  connect(m_groupModel, &CustomModel::itemChanged, this,
          &JSON::ProjectModel::onGroupItemChanged);

  // Update user interface
  emit groupModelChanged();
}

void JSON::ProjectModel::buildActionModel(const JSON::Action &action)
{
  // Clear the existing model
  if (m_actionModel)
  {
    disconnect(m_actionModel);
    m_actionModel->deleteLater();
  }

  // Create a new model
  m_selectedAction = action;
  m_actionModel = new CustomModel(this);

  // Add action title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(TextField, WidgetType);
  title->setData(action.title(), EditableValue);
  title->setData(tr("Title"), ParameterName);
  title->setData(kActionView_Title, ParameterType);
  title->setData(tr("Untitled Action"), PlaceholderValue);
  title->setData(tr("Name or description of the action"), ParameterDescription);
  m_actionModel->appendRow(title);

  // Add action icon
  auto icon = new QStandardItem();
  icon->setEditable(true);
  icon->setData(IconPicker, WidgetType);
  icon->setData(action.icon(), EditableValue);
  icon->setData(tr("Icon"), ParameterName);
  icon->setData(kActionView_Icon, ParameterType);
  icon->setData(tr("Default Icon"), PlaceholderValue);
  icon->setData(tr("Icon to display in the dashboard"), ParameterDescription);
  m_actionModel->appendRow(icon);

  // Add action data
  auto data = new QStandardItem();
  data->setEditable(true);
  data->setData(TextField, WidgetType);
  data->setData(action.txData(), EditableValue);
  data->setData(tr("TX Data"), ParameterName);
  data->setData(kActionView_Data, ParameterType);
  data->setData(tr("Command"), PlaceholderValue);
  data->setData(tr("Data to transmit when the action is triggered."),
                ParameterDescription);
  m_actionModel->appendRow(data);

  // Get appropiate end of line index for current action
  int eolIndex = 0;
  bool found = false;
  for (auto it = m_eolSequences.begin(); it != m_eolSequences.end();
       ++it, ++eolIndex)
  {
    if (it.key() == action.eolSequence())
    {
      found = true;
      break;
    }
  }

  // If not found, reset the index to 0
  if (!found)
    eolIndex = 0;

  // Add EOL combobox
  auto eol = new QStandardItem();
  eol->setEditable(true);
  eol->setData(ComboBox, WidgetType);
  eol->setData(m_eolSequences.values(), ComboBoxData);
  eol->setData(eolIndex, EditableValue);
  eol->setData(tr("EOL Sequence"), ParameterName);
  eol->setData(kActionView_EOL, ParameterType);
  eol->setData(tr("End-of-line (EOL) sequence to use"), ParameterDescription);
  m_actionModel->appendRow(eol);

  // Handle edits
  connect(m_actionModel, &CustomModel::itemChanged, this,
          &JSON::ProjectModel::onActionItemChanged);

  // Update user interface
  emit actionModelChanged();
}

/**
 * @brief Builds the dataset model for managing the settings of a dataset.
 *
 * This function creates a new `CustomModel` to represent the settings of the
 * provided dataset.
 *
 * It includes editable fields such as the dataset title, frame index,
 * measurement units, widget type, minimum and maximum values, alarm values,
 * plotting modes, FFT settings, and LED panel options.
 *
 * The appropriate widget and plot mode indices are calculated based on the
 * dataset's current configuration.
 *
 * The function also sets up a signal to handle item changes in the model and
 * emits the `datasetModelChanged()` signal to update the user interface.
 *
 * @param dataset The dataset for which the model is being built.
 */
void JSON::ProjectModel::buildDatasetModel(const JSON::Dataset &dataset)
{
  // Clear the existing model
  if (m_datasetModel)
  {
    disconnect(m_datasetModel);
    m_datasetModel->deleteLater();
  }

  // Create a new model
  m_selectedDataset = dataset;
  m_datasetModel = new CustomModel(this);

  // Add dataset title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(TextField, WidgetType);
  title->setData(dataset.title(), EditableValue);
  title->setData(tr("Title"), ParameterName);
  title->setData(kDatasetView_Title, ParameterType);
  title->setData(tr("Untitled Dataset"), PlaceholderValue);
  title->setData(tr("Name or description of the dataset"),
                 ParameterDescription);
  m_datasetModel->appendRow(title);

  // Add dataset index
  auto index = new QStandardItem();
  index->setEditable(true);
  index->setData(IntField, WidgetType);
  index->setData(dataset.index(), EditableValue);
  index->setData(tr("Frame Index"), ParameterName);
  index->setData(kDatasetView_Index, ParameterType);
  index->setData(nextDatasetIndex(), PlaceholderValue);
  index->setData(tr("Position in the frame"), ParameterDescription);
  m_datasetModel->appendRow(index);

  // Add units
  auto units = new QStandardItem();
  units->setEditable(true);
  units->setData(TextField, WidgetType);
  units->setData(dataset.units(), EditableValue);
  units->setData(tr("Measurement Unit"), ParameterName);
  units->setData(kDatasetView_Units, ParameterType);
  units->setData(tr("Volts, Amps, etc."), PlaceholderValue);
  units->setData(tr("Unit of measurement (optional)"), ParameterDescription);
  m_datasetModel->appendRow(units);

  // Add widget combobox item
  if (currentDatasetIsEditable())
  {
    // Get appropiate widget index for current dataset
    int widgetIndex = 0;
    bool found = false;
    for (auto it = m_datasetWidgets.begin(); it != m_datasetWidgets.end();
         ++it, ++widgetIndex)
    {
      if (it.key() == dataset.widget())
      {
        found = true;
        break;
      }
    }

    // If not found, reset the index to 0
    if (!found)
      widgetIndex = 0;

    // Add widget combobox
    auto widget = new QStandardItem();
    widget->setEditable(true);
    widget->setData(ComboBox, WidgetType);
    widget->setData(m_datasetWidgets.values(), ComboBoxData);
    widget->setData(widgetIndex, EditableValue);
    widget->setData(tr("Widget"), ParameterName);
    widget->setData(kDatasetView_Widget, ParameterType);
    widget->setData(tr("Display widget (optional)"), ParameterDescription);
    m_datasetModel->appendRow(widget);
  }

  // Add minimum value
  auto min = new QStandardItem();
  min->setEditable(true);
  min->setData(FloatField, WidgetType);
  min->setData(dataset.min(), EditableValue);
  min->setData(tr("Minimum Value"), ParameterName);
  min->setData(kDatasetView_Min, ParameterType);
  min->setData(0, PlaceholderValue);
  min->setData(tr("Required for bar/gauge widgets"), ParameterDescription);
  m_datasetModel->appendRow(min);

  // Add maximum value
  auto max = new QStandardItem();
  max->setEditable(true);
  max->setData(FloatField, WidgetType);
  max->setData(dataset.max(), EditableValue);
  max->setData(tr("Maximum Value"), ParameterName);
  max->setData(kDatasetView_Max, ParameterType);
  max->setData(0, PlaceholderValue);
  max->setData(tr("Required for bar/gauge widgets"), ParameterDescription);
  m_datasetModel->appendRow(max);

  // Add alarm value
  auto alarm = new QStandardItem();
  alarm->setEditable(true);
  alarm->setData(FloatField, WidgetType);
  alarm->setData(dataset.alarm(), EditableValue);
  alarm->setData(tr("Alarm Value"), ParameterName);
  alarm->setData(kDatasetView_Alarm, ParameterType);
  alarm->setData(0, PlaceholderValue);
  alarm->setData(tr("Triggers alarm in bar widgets and LED panels"),
                 ParameterDescription);
  m_datasetModel->appendRow(alarm);

  // Get appropiate plotting mode index for current dataset
  int plotIndex = 0;
  bool found = false;
  const auto currentPair = qMakePair(dataset.graph(), dataset.log());
  for (auto it = m_plotOptions.begin(); it != m_plotOptions.end();
       ++it, ++plotIndex)
  {
    if (it.key() == currentPair)
    {
      found = true;
      break;
    }
  }

  // If not found, reset the index to 0
  if (!found)
    plotIndex = 0;

  // Add plotting mode
  auto plot = new QStandardItem();
  plot->setEditable(true);
  plot->setData(ComboBox, WidgetType);
  plot->setData(m_plotOptions.values(), ComboBoxData);
  plot->setData(plotIndex, EditableValue);
  plot->setData(tr("Oscilloscope Plot"), ParameterName);
  plot->setData(kDatasetView_Plot, ParameterType);
  plot->setData(tr("Plot data in real-time"), ParameterDescription);
  m_datasetModel->appendRow(plot);

  // Add FFT checkbox
  auto fft = new QStandardItem();
  fft->setEditable(true);
  fft->setData(CheckBox, WidgetType);
  fft->setData(dataset.fft(), EditableValue);
  fft->setData(tr("FFT Plot"), ParameterName);
  fft->setData(kDatasetView_FFT, ParameterType);
  fft->setData(0, PlaceholderValue);
  fft->setData(tr("Plot frequency-domain data"), ParameterDescription);
  m_datasetModel->appendRow(fft);

  // Get FFT window size index
  const auto windowSize = QString::number(dataset.fftSamples());
  int windowIndex = m_fftSamples.indexOf(windowSize);
  if (windowIndex < 0)
    windowIndex = 7;

  // Add FFT window size
  auto fftWindow = new QStandardItem();
  fftWindow->setEditable(true);
  fftWindow->setData(ComboBox, WidgetType);
  fftWindow->setData(m_fftSamples, ComboBoxData);
  fftWindow->setData(windowIndex, EditableValue);
  fftWindow->setData(tr("FFT Window Size"), ParameterName);
  fftWindow->setData(kDatasetView_FFT_Samples, ParameterType);
  fftWindow->setData(tr("Samples for FFT calculation"), ParameterDescription);
  m_datasetModel->appendRow(fftWindow);

  // Add FFT sampling rate
  auto fftSamplingRate = new QStandardItem();
  fftSamplingRate->setEditable(true);
  fftSamplingRate->setData(IntField, WidgetType);
  fftSamplingRate->setData(100, PlaceholderValue);
  fftSamplingRate->setData(dataset.fftSamplingRate(), EditableValue);
  fftSamplingRate->setData(tr("FFT Sampling Rate"), ParameterName);
  fftSamplingRate->setData(kDatasetView_FFT_SamplingRate, ParameterType);
  fftSamplingRate->setData(tr("Sampling rate (Hz) for FFT calculation"),
                           ParameterDescription);
  m_datasetModel->appendRow(fftSamplingRate);

  // Add LED panel checkbox
  auto led = new QStandardItem();
  led->setEditable(true);
  led->setData(CheckBox, WidgetType);
  led->setData(dataset.led(), EditableValue);
  led->setData(tr("Show in LED Panel"), ParameterName);
  led->setData(kDatasetView_LED, ParameterType);
  led->setData(0, PlaceholderValue);
  led->setData(tr("Quick status monitoring"), ParameterDescription);
  m_datasetModel->appendRow(led);

  // Add LED High value
  auto ledHigh = new QStandardItem();
  ledHigh->setEditable(true);
  ledHigh->setData(FloatField, WidgetType);
  ledHigh->setData(dataset.ledHigh(), EditableValue);
  ledHigh->setData(tr("LED High (On) Value"), ParameterName);
  ledHigh->setData(kDatasetView_LED_High, ParameterType);
  ledHigh->setData(0, PlaceholderValue);
  ledHigh->setData(tr("Threshold for LED on"), ParameterDescription);
  m_datasetModel->appendRow(ledHigh);

  // Handle edits
  connect(m_datasetModel, &CustomModel::itemChanged, this,
          &JSON::ProjectModel::onDatasetItemChanged);

  // Update user interface
  emit datasetModelChanged();
}

//------------------------------------------------------------------------------
// Re-generate project model when user opens another JSON file
//------------------------------------------------------------------------------

/**
 * @brief Reloads the project model when a new JSON file is loaded.
 *
 * This function checks if the current JSON file path differs from the one
 * loaded by `JSON::FrameBuilder`. If they differ, it opens the new JSON file
 * and reloads the project model.
 */
void JSON::ProjectModel::onJsonLoaded()
{
  openJsonFile(JSON::FrameBuilder::instance().jsonMapFilepath());
}

/**
 * @brief Sets the API keys for the map providers.
 */
void JSON::ProjectModel::onGpsApiKeysChanged()
{
  m_server.setMaptilerAPIKey(m_mapTilerApiKey);
  m_server.setThunderforestAPIKey(m_thunderforestApiKey);
}

//------------------------------------------------------------------------------
// Re-generate combobox data sources when the language is changed
//------------------------------------------------------------------------------

/**
 * @brief Generates data sources for combo boxes used in the project.
 *
 * This function initializes the data lists for various combo box models used
 * in the project. It includes options for FFT window sizes, decoder methods,
 * group-level widgets, dataset-level widgets, and plotting options.
 *
 * These sources are re-generated when the language is changed to ensure proper
 * localization.
 */
void JSON::ProjectModel::generateComboBoxModels()
{
  // Initialize FFT window sizes list
  m_fftSamples.clear();
  m_fftSamples.append("8");
  m_fftSamples.append("16");
  m_fftSamples.append("32");
  m_fftSamples.append("64");
  m_fftSamples.append("128");
  m_fftSamples.append("256");
  m_fftSamples.append("512");
  m_fftSamples.append("1024");
  m_fftSamples.append("2048");
  m_fftSamples.append("4096");
  m_fftSamples.append("8192");
  m_fftSamples.append("16384");

  // Initialize decoder options
  m_decoderOptions.clear();
  m_decoderOptions.append(tr("Plain Text (UTF8)"));
  m_decoderOptions.append(tr("Hexadecimal"));
  m_decoderOptions.append(tr("Base64"));

  // Initialize frame detection methods
  m_frameDetectionMethods.clear();
  m_frameDetectionMethods.append(tr("End Delimiter Only"));
  m_frameDetectionMethods.append(tr("Start + End Delimiter"));

  // Initialize group-level widgets
  m_groupWidgets.clear();
  m_groupWidgets.insert(QStringLiteral("datagrid"), tr("Data Grid"));
  m_groupWidgets.insert(QStringLiteral("map"), tr("GPS Map"));
  m_groupWidgets.insert(QStringLiteral("gyro"), tr("Gyroscope"));
  m_groupWidgets.insert(QStringLiteral("multiplot"), tr("Multiple Plot"));
  m_groupWidgets.insert(QStringLiteral("accelerometer"), tr("Accelerometer"));
  m_groupWidgets.insert(QStringLiteral(""), tr("None"));

  // Initialize dataset-level widgets
  m_datasetWidgets.clear();
  m_datasetWidgets.insert(QStringLiteral(""), tr("None"));
  m_datasetWidgets.insert(QStringLiteral("bar"), tr("Bar"));
  m_datasetWidgets.insert(QStringLiteral("gauge"), tr("Gauge"));
  m_datasetWidgets.insert(QStringLiteral("compass"), tr("Compass"));

  // Initialize EOL options
  m_eolSequences.clear();
  m_eolSequences.insert(QStringLiteral(""), tr("None"));
  m_eolSequences.insert(QStringLiteral("\n"), tr("New Line (\\n)"));
  m_eolSequences.insert(QStringLiteral("\r"), tr("Carriage Return (\\r)"));
  m_eolSequences.insert(QStringLiteral("\r\n"), tr("CRLF (\\r\\n)"));

  // Initialize plot options
  m_plotOptions.clear();
  m_plotOptions.insert(qMakePair(false, false), tr("No"));
  m_plotOptions.insert(qMakePair(true, false), tr("Linear Plot"));
  // m_plotOptions.insert(qMakePair(true, true), tr("Logarithmic Plot"));
}

//------------------------------------------------------------------------------
// Document status modification
//------------------------------------------------------------------------------

/**
 * @brief Sets the modification status of the project.
 *
 * This function updates the modified flag of the project and emits the
 * `modifiedChanged()` signal to notify the user interface of the change.
 *
 * @param modified A boolean indicating whether the project has been modified (
 *                 true) or not (false).
 */
void JSON::ProjectModel::setModified(const bool modified)
{
  m_modified = modified;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Sets the current view of the project.
 *
 * This function updates the current view mode of the project and emits the
 * `currentViewChanged()` signal to notify the user interface of the change.
 *
 * @param currentView The new view mode, selected from the `CurrentView` enum.
 */
void JSON::ProjectModel::setCurrentView(const CurrentView currentView)
{
  m_currentView = currentView;
  Q_EMIT currentViewChanged();
}

//------------------------------------------------------------------------------
// Model modification functions
//------------------------------------------------------------------------------

/**
 * @brief Handles changes made to a group item in the group model.
 *
 * This function processes changes to group items such as the group title or
 * widget type. It validates the modified item, updates the group title or
 * widget accordingly, and rebuilds the tree model to reflect the changes.
 *
 * If the group was modified, it sets the modified flag. Finally, it reselects
 * the group in the user interface.
 *
 * @param item A pointer to the modified `QStandardItem` representing the
 *             changed group property.
 */
void JSON::ProjectModel::onGroupItemChanged(QStandardItem *item)
{
  // Validate item pointer
  if (!item)
    return;

  // Construct lists with key values for QMap-based comboboxes
  static QStringList widgets;
  if (widgets.isEmpty())
    for (auto i = m_groupWidgets.begin(); i != m_groupWidgets.end(); ++i)
      widgets.append(i.key());

  // Obtain which item was modified & its new value
  const auto id = static_cast<GroupItem>(item->data(ParameterType).toInt());
  const auto value = item->data(EditableValue);

  // Obtain group ID
  bool modified = false;
  const auto groupId = m_selectedGroup.groupId();

  // Change group title
  if (id == kGroupView_Title)
  {
    modified = m_selectedGroup.m_title != value.toString();
    m_selectedGroup.m_title = value.toString();
    m_groups.replace(groupId, m_selectedGroup);
  }

  // Change group widget
  else if (id == kGroupView_Widget)
  {
    // Obtain widget enum from string
    WC::GroupWidget widget;
    const auto widgetStr = widgets.at(value.toInt());
    if (widgetStr == "accelerometer")
      widget = WC::Accelerometer;
    else if (widgetStr == "multiplot")
      widget = WC::MultiPlot;
    else if (widgetStr == "gyro")
      widget = WC::Gyroscope;
    else if (widgetStr == "map")
      widget = WC::GPS;
    else if (widgetStr == "datagrid")
      widget = WC::DataGrid;
    else
      widget = WC::NoGroupWidget;

    // Update group
    modified = setGroupWidget(groupId, widget);
    if (modified)
      m_selectedGroup.m_widget = widgetStr;

    // User canceled the operation, reload model GUI to restore previous value
    else
    {
      buildTreeModel();
      for (auto g = m_groupItems.begin(); g != m_groupItems.end(); ++g)
      {
        if (g.value().groupId() == m_selectedGroup.groupId())
        {
          m_selectionModel->setCurrentIndex(
              g.key()->index(), QItemSelectionModel::ClearAndSelect);
          break;
        }
      }

      return;
    }
  }

  // Re-build tree model
  buildTreeModel();
  if (modified)
    setModified(true);

  // Ensure toolbar items are synched
  Q_EMIT editableOptionsChanged();
}

/**
 * @brief Handles changes made to a action item in the action model.
 *
 * This function processes changes to action items such as the title, data or
 * icon, it updates the action data and rebuilds the tree model to reflect the
 * changes.
 *
 * If the action was modified, it sets the modified flag. Finally, it reselects
 * the action in the user interface.
 *
 * @param item A pointer to the modified `QStandardItem` representing the
 *             changed action property.
 */
void JSON::ProjectModel::onActionItemChanged(QStandardItem *item)
{
  // Validate item pointer
  if (!item)
    return;

  // Construct lists with key values for QMap-based comboboxes
  static QStringList eolSequences;
  if (eolSequences.isEmpty())
    for (auto i = m_eolSequences.begin(); i != m_eolSequences.end(); ++i)
      eolSequences.append(i.key());

  // Obtain which item was modified & its new value
  const auto id = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  // Update internal members of the class accordingly
  switch (static_cast<ActionItem>(id.toInt()))
  {
    case kActionView_Title:
      m_selectedAction.m_title = value.toString();
      break;
    case kActionView_Data:
      m_selectedAction.m_txData = value.toString();
      break;
    case kActionView_EOL:
      m_selectedAction.m_eolSequence = eolSequences.at(value.toInt());
      break;
    case kActionView_Icon:
      m_selectedAction.m_icon = value.toString();
      Q_EMIT actionModelChanged();
      break;
    default:
      break;
  }

  // Replace action data
  const auto actionId = m_selectedAction.actionId();
  m_actions.replace(actionId, m_selectedAction);
  buildTreeModel();

  // Mark document as modified
  setModified(true);
}

/**
 * @brief Handles changes made to a project item in the project model.
 *
 * This function processes changes to project items such as the title,
 * separator, frame sequences, decoder method, and Thunderforest API key.
 *
 * It updates the relevant internal members and emits signals to notify the
 * user interface of changes. After updating the internal state, it marks the
 * document as modified.
 *
 * @param item A pointer to the modified `QStandardItem` representing the
 *             changed project property.
 */
void JSON::ProjectModel::onProjectItemChanged(QStandardItem *item)
{
  // Validate item pointer
  if (!item)
    return;

  // Obtain which item was modified & its new value
  const auto id = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  // Update internal members of the class accordingly
  switch (static_cast<ProjectItem>(id.toInt()))
  {
    case kProjectView_Title:
      m_title = value.toString();
      Q_EMIT titleChanged();
      break;
    case kProjectView_ItemSeparator:
      m_separator = value.toString();
      break;
    case kProjectView_FrameEndSequence:
      m_frameEndSequence = value.toString();
      break;
    case kProjectView_FrameStartSequence:
      m_frameStartSequence = value.toString();
      break;
    case kProjectView_FrameDecoder:
      m_frameDecoder = static_cast<WC::DecoderMethod>(value.toInt());
      break;
    case kProjectView_FrameDetection:
      m_frameDetection = static_cast<WC::FrameDetection>(value.toInt());
      buildProjectModel();
      break;
    case kProjectView_ThunderforestApiKey:
      m_thunderforestApiKey = value.toString();
      Q_EMIT gpsApiKeysChanged();
      break;
    case kProjectView_MapTilerApiKey:
      m_mapTilerApiKey = value.toString();
      Q_EMIT gpsApiKeysChanged();
      break;
    default:
      break;
  }

  // Mark document as modified
  setModified(true);
}

/**
 * @brief Handles changes made to a dataset item in the dataset model.
 *
 * This function processes changes to dataset items such as the title, index,
 * units, widget type, FFT settings, LED settings, plotting mode, and
 * min/max/alarm values. It updates the relevant parameters of the selected
 * dataset, replaces the dataset in its parent group, and rebuilds the tree
 * model. After updating the dataset, it marks the document as modified.
 *
 * @param item A pointer to the modified `QStandardItem` representing the
 *             changed dataset property.
 */
void JSON::ProjectModel::onDatasetItemChanged(QStandardItem *item)
{
  // Validate item pointer
  if (!item)
    return;

  // Construct lists with key values for QMap-based comboboxes
  static QStringList widgets;
  static QList<QPair<bool, bool>> plotOptions;

  // Construct widget list
  if (widgets.isEmpty())
    for (auto i = m_datasetWidgets.begin(); i != m_datasetWidgets.end(); ++i)
      widgets.append(i.key());

  // Construct plot options list
  if (plotOptions.isEmpty())
    for (auto i = m_plotOptions.begin(); i != m_plotOptions.end(); ++i)
      plotOptions.append(i.key());

  // Obtain which item was modified & its new value
  const auto id = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  // Update dataset parameters accordingly
  switch (static_cast<DatasetItem>(id.toInt()))
  {
    case kDatasetView_Title:
      m_selectedDataset.m_title = value.toString();
      break;
    case kDatasetView_Index:
      m_selectedDataset.m_index = value.toInt();
      break;
    case kDatasetView_Units:
      m_selectedDataset.m_units = value.toString();
      break;
    case kDatasetView_Widget:
      m_selectedDataset.m_widget = widgets.at(value.toInt());
      break;
    case kDatasetView_FFT:
      m_selectedDataset.m_fft = value.toBool();
      break;
    case kDatasetView_LED:
      m_selectedDataset.m_led = value.toBool();
      break;
    case kDatasetView_LED_High:
      m_selectedDataset.m_ledHigh = value.toFloat();
      break;
    case kDatasetView_Plot:
      m_selectedDataset.m_graph = plotOptions.at(value.toInt()).first;
      m_selectedDataset.m_log = plotOptions.at(value.toInt()).second;
      break;
    case kDatasetView_Min:
      m_selectedDataset.m_min = value.toFloat();
      break;
    case kDatasetView_Max:
      m_selectedDataset.m_max = value.toFloat();
      break;
    case kDatasetView_Alarm:
      m_selectedDataset.m_alarm = value.toFloat();
      break;
    case kDatasetView_FFT_Samples:
      m_selectedDataset.m_fftSamples = m_fftSamples.at(value.toInt()).toInt();
      break;
    case kDatasetView_FFT_SamplingRate:
      m_selectedDataset.m_fftSamplingRate = value.toInt();
      break;
    default:
      break;
  }

  // Replace dataset in parent group
  const auto groupId = m_selectedDataset.groupId();
  const auto datasetId = m_selectedDataset.datasetId();
  auto group = m_groups.at(groupId);
  group.m_datasets.replace(datasetId, m_selectedDataset);
  m_groups.replace(groupId, group);
  buildTreeModel();

  // Mark document as modified
  setModified(true);

  // Ensure toolbar items are synched
  Q_EMIT datasetOptionsChanged();
  Q_EMIT editableOptionsChanged();
}

//------------------------------------------------------------------------------
// Change view & select group/dataset when user navigates project structure
//------------------------------------------------------------------------------

/**
 * @brief Handles changes in the current selection within the project structure.
 *
 * This function responds to user navigation within the project structure by
 * checking the type of item selected (group, dataset, or root item).
 *
 * It updates the view accordingly, building the appropriate model (group,
 * dataset, or project) and setting the current view to the corresponding
 * mode (e.g., GroupView, DatasetView, FrameParserView, or ProjectView).
 *
 * @param current The currently selected index in the tree model.
 * @param previous The previously selected index in the tree model
 *                 (unused in this function).
 */
void JSON::ProjectModel::onCurrentSelectionChanged(const QModelIndex &current,
                                                   const QModelIndex &previous)
{
  // Ignore previous item, we don't need it
  (void)previous;

  // Obtain standard item pointer from the index
  auto item = m_treeModel->itemFromIndex(current);

  // Check if the user selected a group
  if (m_groupItems.contains(item))
  {
    const auto group = m_groupItems.value(item);
    setCurrentView(GroupView);
    buildGroupModel(group);
  }

  // Check if the user selected a dataset
  else if (m_datasetItems.contains(item))
  {
    const auto dataset = m_datasetItems.value(item);
    setCurrentView(DatasetView);
    buildDatasetModel(dataset);
  }

  // Check if user selected an action
  else if (m_actionItems.contains(item))
  {
    const auto action = m_actionItems.value(item);
    setCurrentView(ActionView);
    buildActionModel(action);
  }

  // Finally, check if the user selected one of the root items
  else if (m_rootItems.contains(item))
  {
    const auto id = m_rootItems.value(item);
    if (id == kFrameParser)
      setCurrentView(FrameParserView);

    else
    {
      setCurrentView(ProjectView);
      buildProjectModel();
    }
  }
}

//------------------------------------------------------------------------------
// Function to automatically set dataset indexes
//------------------------------------------------------------------------------

/**
 * @brief Determines the next available dataset index.
 *
 * This function iterates through all datasets in the project to find the
 * maximum dataset index. It then returns the next available index, which is
 * one greater than the current maximum.
 *
 * @return The next available dataset index.
 */
int JSON::ProjectModel::nextDatasetIndex()
{
  int maxIndex = 1;
  for (auto i = 0; i < m_groups.count(); ++i)
  {
    for (auto j = 0; j < m_groups.at(i).datasetCount(); ++j)
    {
      const auto dataset = m_groups.at(i).datasets().at(j);
      if (dataset.m_index >= maxIndex)
        maxIndex = dataset.m_index + 1;
    }
  }

  return maxIndex;
}

//------------------------------------------------------------------------------
// Save & restore expanded items of project structure upon modification
//------------------------------------------------------------------------------

/**
 * @brief Recursively saves the expanded state of tree view items.
 *
 * This function stores the expanded state of the given `QStandardItem` and its
 * child items in a `QHash`, with the item's text serving as the key.
 *
 * The expanded state is saved recursively for all child items.
 *
 * @param item Pointer to the `QStandardItem` whose state is being saved.
 * @param map A reference to a `QHash` that stores the state of each item.
 * @param title The current item's title, used as the key in the hash.
 */
void JSON::ProjectModel::saveExpandedStateMap(QStandardItem *item,
                                              QHash<QString, bool> &map,
                                              const QString &title)
{
  // Validate item
  if (!item)
    return;

  // Obtain expanded state for current item
  map[title] = item->data(TreeViewExpanded).toBool();

  // Recursively save the expanded state of child items
  for (auto i = 0; i < item->rowCount(); ++i)
  {
    QStandardItem *child = item->child(i);
    auto childT = title.isEmpty() ? child->text() : title + "/" + child->text();
    saveExpandedStateMap(child, map, childT);
  }
}

/**
 * @brief Restores the expanded state of tree view items.
 *
 * This function restores the expanded state of the given `QStandardItem` and
 * based on the saved states in a `QHash`.
 *
 * If an item is not found in the hash, it is expanded  by default.
 *
 * @param item A pointer to the `QStandardItem` whose state is being restored.
 * @param map A reference to a `QHash` containing the saved expanded states.
 * @param title The current item's title, used to look up its state in the hash.
 */
void JSON::ProjectModel::restoreExpandedStateMap(QStandardItem *item,
                                                 QHash<QString, bool> &map,
                                                 const QString &title)
{
  // Validate item
  if (!item)
    return;

  // Restore expanded state for registered items
  if (map.contains(title))
    item->setData(map[title], TreeViewExpanded);

  // Expand unregistered items
  else
    item->setData(true, TreeViewExpanded);
}
