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

#include <QFile>
#include <QTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QFileDialog>
#include <QJsonObject>
#include <QDirIterator>
#include <QJsonDocument>

#include "AppInfo.h"
#include "IO/Checksum.h"
#include "Misc/Utilities.h"
#include "Misc/Translator.h"
#include "Misc/WorkspaceManager.h"

#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"
#include "JSON/FrameBuilder.h"

//------------------------------------------------------------------------------
// ProjectModel.cpp - Core data model for Serial Studio projects
//
// 4000+ lines of conditionally stable logic supporting project structures,
// group hierarchies, dataset bindings, and action editing, all through the
// reluctant and occasionally baffling magic of QStandardItemModel.
//
// It's purpose? (Hopefully) making project editing and generation less
// painful for end users.
//
// Warning: The JSON::Frame structures and hotpaths are innocent.
//          The QStandardItemModel is not. Every change risks unleashing UI
//          behavior so cursed, even the debugger won't follow you in.
//          Handle with precision, patience, and possibly a therapist.
//------------------------------------------------------------------------------

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
  kProjectView_Title,               /**< Represents the project title item */
  kProjectView_FrameStartSequence,  /**< Represents the frame start sequence */
  kProjectView_FrameEndSequence,    /**< Represents the frame end sequence */
  kProjectView_FrameDecoder,        /**< Represents the frame decoder item */
  kProjectView_HexadecimalSequence, /**< Represents the frame sequence format */
  kProjectView_FrameDetection,      /**< Represents the frame detection item */
  kProjectView_ChecksumFunction     /**< Represents the frame checksum item */
} ProjectItem;
// clang-format on

/**
 * @brief Enum representing items in the dataset view.
 */
// clang-format off
typedef enum
{
  kDatasetView_Title,            /**< Dataset title item. */
  kDatasetView_Index,            /**< Dataset frame index item. */
  kDatasetView_Units,            /**< Dataset units item. */
  kDatasetView_Widget,           /**< Dataset widget item. */
  kDatasetView_FFT,              /**< FFT plot checkbox item. */
  kDatasetView_LED,              /**< LED panel checkbox item. */
  kDatasetView_LED_High,         /**< LED high (on) value item. */
  kDatasetView_Plot,             /**< Dataset plot mode item. */
  kDatasetView_FFTMin,           /**< Dataset minimum FFT value item. */
  kDatasetView_FFTMax,           /**< Dataset maximum FFT value item. */
  kDatasetView_PltMin,           /**< Dataset minimum plot value item. */
  kDatasetView_PltMax,           /**< Dataset maximum plot value item. */
  kDatasetView_WgtMin,           /**< Dataset minimum widget value item. */
  kDatasetView_WgtMax,           /**< Dataset maximum widget value item. */
  kDatasetView_AlarmLow,         /**< Dataset alarm low value item. */
  kDatasetView_AlarmHigh,        /**< Dataset alarm high value item. */
  kDatasetView_FFT_Samples,      /**< FFT window size item. */
  kDatasetView_AlarmEnabled,     /**< Alarm enabled status item. */
  kDatasetView_FFT_SamplingRate, /**< FFT sampling rate item. */
  kDatasetView_xAxis,            /**< Plot X axis item. */
  kDatasetView_Overview          /**< Display in Overview workspace. */
} DatasetItem;
// clang-format on

/**
 * @brief Enum representing items in the action view.
 */
// clang-format off
typedef enum
{
  kActionView_Title,        /**< The action title item. */
  kActionView_Icon,         /**< The icon item. */
  kActionView_EOL,          /**< The EOL (end of line) item. */
  kActionView_Data,         /**< The TX data item. */
  kActionView_Binary,       /**< The binary data transmission status. */
  kActionView_AutoExecute,  /**< Whether the action executes on connect. */
  kActionView_TimerMode,    /**< The timer behavior mode. */
  kActionView_TimerInterval /**< The timer interval in milliseconds. */
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
  , m_frameParserCode("")
  , m_frameEndSequence("")
  , m_frameStartSequence("")
  , m_hexadecimalDelimiters(false)
  , m_currentView(ProjectView)
  , m_frameDecoder(SerialStudio::PlainText)
  , m_frameDetection(SerialStudio::EndDelimiterOnly)
  , m_modified(false)
  , m_filePath("")
  , m_treeModel(nullptr)
  , m_selectionModel(nullptr)
  , m_groupModel(nullptr)
  , m_projectModel(nullptr)
  , m_datasetModel(nullptr)
{
  // Generate data sources for project model
  generateComboBoxModels();

  // Clear selection model when JSON file is changed
  connect(this, &JSON::ProjectModel::jsonFileChanged, this, [=, this] {
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
  bool parserModified = false;
  if (JSON::FrameBuilder::instance().frameParser())
    parserModified = JSON::FrameBuilder::instance().frameParser()->isModified();

  return m_modified || parserModified;
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
SerialStudio::DecoderMethod JSON::ProjectModel::decoderMethod() const
{
#ifdef BUILD_COMMERCIAL
  if (SerialStudio::activated())
    return m_frameDecoder;
#endif

  return SerialStudio::PlainText;
}

/**
 * @brief Retrieves the current strategy used for detecting data frames.
 *
 * This function returns the detection method currently set for identifying data
 * frames.
 *
 * @return The current decoder method as a value from the `FrameDetection` enum.
 */
SerialStudio::FrameDetection JSON::ProjectModel::frameDetection() const
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
  return Misc::WorkspaceManager::instance().path("JSON Projects");
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
 * @brief Retrieves a list of available X-axis data sources.
 *
 * This function returns a list of X-axis data source names. It includes a
 * default entry ("Samples") and all registered datasets from the project model.
 * Each dataset is identified by its title and the title of the group it belongs
 * to.
 *
 * @return A `QStringList` containing the names of X-axis data sources.
 */
QStringList JSON::ProjectModel::xDataSources() const
{
  QStringList list;
  list.append(tr("Samples"));

  QMap<int, QString> datasets;
  for (const auto &group : m_groups)
  {
    for (const auto &dataset : group.datasets)
    {
      const auto index = dataset.index;
      if (!datasets.contains(index))
      {
        const auto t = QString("%1 (%2)").arg(dataset.title, group.title);
        datasets.insert(index, t);
      }
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
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
  return m_selectedAction.icon;
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
    const auto widget = m_selectedGroup.widget;
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
    const auto groupId = m_selectedDataset.groupId;
    if (m_groups.size() > static_cast<size_t>(groupId))
    {
      const auto widget = m_groups[groupId].widget;
      if (widget != "" && widget != "multiplot" && widget != "datagrid")
        return false;
    }
  }

  return true;
}

/**
 * Returns @c true if the project contains features that should only be enabled
 * for commercial users with a valid license, such as the 3D plot widget.
 */
bool JSON::ProjectModel::containsCommercialFeatures() const
{
  return SerialStudio::commercialCfg(m_groups)
         || m_frameDecoder != SerialStudio::PlainText;
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
  return static_cast<int>(m_groups.size());
}

/**
 * @brief Retrieves the number of datasets in the project.
 *
 * This function returns the total count of datasets currently present in the
 * project.
 *
 * @return The number of datasets.
 */
int JSON::ProjectModel::datasetCount() const
{
  int count = 0;
  for (const auto &group : m_groups)
    count += group.datasets.size();

  return count;
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
  quint8 option = SerialStudio::DatasetGeneric;

  if (m_selectedDataset.plt)
    option |= SerialStudio::DatasetPlot;

  if (m_selectedDataset.fft)
    option |= SerialStudio::DatasetFFT;

  if (m_selectedDataset.led)
    option |= SerialStudio::DatasetLED;

  if (m_selectedDataset.widget == QStringLiteral("bar"))
    option |= SerialStudio::DatasetBar;

  else if (m_selectedDataset.widget == QStringLiteral("gauge"))
    option |= SerialStudio::DatasetGauge;

  else if (m_selectedDataset.widget == QStringLiteral("compass"))
    option |= SerialStudio::DatasetCompass;

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
const std::vector<JSON::Group> &JSON::ProjectModel::groups() const
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
      tr("You have unsaved modifications in this project!"),
      QMessageBox::Question, APP_NAME,
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Cancel)
    return false;

  if (ret == QMessageBox::Discard)
  {
    if (jsonFilePath().isEmpty())
      newJsonFile();
    else
      openJsonFile(jsonFilePath());

    return true;
  }

  return saveJsonFile(false);
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
bool JSON::ProjectModel::saveJsonFile(const bool askPath)
{
  // Validate project title
  if (m_title.isEmpty())
  {
    Misc::Utilities::showMessageBox(tr("Project error"),
                                    tr("Project title cannot be empty!"),
                                    QMessageBox::Warning);
    return false;
  }

  // Save and validate javascript code
  auto *parser = JSON::FrameBuilder::instance().frameParser();
  if (parser && parser->isModified())
  {
    if (!parser->save(true))
      return false;
  }

  // Get file save path
  if (jsonFilePath().isEmpty() || askPath)
  {
    auto *dialog
        = new QFileDialog(nullptr, tr("Save Serial Studio Project"),
                          jsonProjectsPath() + "/" + title() + ".ssproj",
                          tr("Serial Studio Project Files (*.ssproj)"));

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setOption(QFileDialog::DontUseNativeDialog);

    connect(dialog, &QFileDialog::fileSelected, this,
            [this, dialog](const QString &path) {
              dialog->deleteLater();

              if (path.isEmpty())
                return;

              m_filePath = path;
              (void)finalizeProjectSave();
            });

    dialog->open();
    return false;
  }

  // File already on disk, just write new data to it
  (void)finalizeProjectSave();
  return false;
}

//------------------------------------------------------------------------------
// Signal/slot handling
//------------------------------------------------------------------------------

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void JSON::ProjectModel::setupExternalConnections()
{
  // Re-load JSON map file into C++ model
  connect(&JSON::FrameBuilder::instance(),
          &JSON::FrameBuilder::jsonFileMapChanged, this,
          &JSON::ProjectModel::onJsonLoaded);

  // Generate combo-boxes again when app is translated
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, [=, this] {
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
}

//------------------------------------------------------------------------------
// Document initialization
//------------------------------------------------------------------------------

/**
 * @brief Initializes a new JSON project.
 *
 * This function clears the current groups, resets project properties
 * (such as the title, frame decoder, and sequences), and sets default values
 * for the project.
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
  m_frameDecoder = SerialStudio::PlainText;
  m_frameDetection = SerialStudio::EndDelimiterOnly;
  m_frameEndSequence = "\\n";
  m_checksumAlgorithm = "";
  m_frameStartSequence = "$";
  m_hexadecimalDelimiters = false;
  m_title = tr("Untitled Project");
  m_frameParserCode = JSON::FrameParser::defaultCode();

  // Load frame parser code into code editor
  if (JSON::FrameBuilder::instance().frameParser())
    JSON::FrameBuilder::instance().frameParser()->readCode();

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
  Q_EMIT frameDetectionChanged();
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
  auto *dialog
      = new QFileDialog(nullptr, tr("Select Project File"), jsonProjectsPath(),
                        tr("Project Files (*.json *.ssproj)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);
  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            if (!path.isEmpty())
              openJsonFile(path);

            dialog->deleteLater();
          });

  dialog->open();
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

  // Reset C++ model
  newJsonFile();

  // Update current JSON document
  m_filePath = path;

  // Read data from JSON document
  auto json = document.object();
  m_title = json.value("title").toString();
  m_frameEndSequence = json.value("frameEnd").toString();
  m_checksumAlgorithm = json.value("checksum").toString();
  m_frameParserCode = json.value("frameParser").toString();
  m_frameStartSequence = json.value("frameStart").toString();
  m_hexadecimalDelimiters = json.value("hexadecimalDelimiters").toBool();
  m_frameDecoder
      = static_cast<SerialStudio::DecoderMethod>(json.value("decoder").toInt());
  m_frameDetection = static_cast<SerialStudio::FrameDetection>(
      json.value("frameDetection").toInt());

  // Preserve compatibility with previous projects
  if (!json.contains("frameDetection"))
    m_frameDetection = SerialStudio::StartAndEndDelimiter;

  // Read groups from JSON document
  auto groups = json.value("groups").toArray();
  for (int g = 0; g < groups.count(); ++g)
  {
    JSON::Group group;
    group.groupId = g;
    if (JSON::read(group, groups.at(g).toObject()))
      m_groups.push_back(group);
  }

  // Read actions from JSON document
  auto actions = json.value("actions").toArray();
  for (int a = 0; a < actions.count(); ++a)
  {
    JSON::Action action;
    action.actionId = a;
    if (JSON::read(action, actions.at(a).toObject()))
      m_actions.push_back(action);
  }

  // Regenerate the tree model
  buildProjectModel();
  buildTreeModel();

  // Show project view
  setCurrentView(ProjectView);

  // Reset modified flag
  setModified(false);

  // Detect legacy frame parser function
  if (json.contains("separator"))
  {
    // Obtain separator value from project file
    const auto separator = json.value("separator").toString();

    // Detect if it's a simple legacy default function
    static QRegularExpression legacyRegex(
        R"(function\s+parse\s*\(\s*frame\s*,\s*separator\s*\)\s*\{\s*return\s+frame\.split\(separator\);\s*\})");
    if (legacyRegex.match(m_frameParserCode).hasMatch())
    {
      // Migrate to new format
      if (separator.length() > 1)
        m_frameParserCode
            = QStringLiteral(
                  "/**\n * Automatically migrated frame parser function.\n "
                  "*/\nfunction parse(frame) {\n    return "
                  "frame.split(\"%1\");\n}")
                  .arg(separator);
      else
        m_frameParserCode
            = QStringLiteral(
                  "/**\n * Automatically migrated frame parser function.\n "
                  "*/\nfunction parse(frame) {\n    return "
                  "frame.split(\'%1\');\n}")
                  .arg(separator);

      // Notify user about the change
      Misc::Utilities::showMessageBox(
          tr("Legacy frame parser function updated"),
          tr("Your project used a legacy frame parser function with a "
             "'separator' argument. It has been automatically migrated to "
             "the new format."),
          QMessageBox::Information);
      saveJsonFile(false);
      return;
    }
  }

  // Let the generator use the given JSON file
  if (JSON::FrameBuilder::instance().jsonMapFilepath() != path)
    JSON::FrameBuilder::instance().loadJsonMap(path);

  // Update UI
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();
}

/**
 * @brief Ensures Serial Studio is in Project File Mode.
 *
 * Prompts the user to switch to Project File Mode if the current operation
 * mode is different.
 */
void JSON::ProjectModel::enableProjectMode()
{
  const auto opMode = JSON::FrameBuilder::instance().operationMode();
  if (opMode != SerialStudio::ProjectFile)
  {
    auto answ = Misc::Utilities::showMessageBox(
        tr("Switch Serial Studio to Project Mode?"),
        tr("This operation mode is required to load and display dashboards "
           "from project files."),
        QMessageBox::Question, qApp->applicationDisplayName(),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (answ == QMessageBox::Yes)
      JSON::FrameBuilder::instance().setOperationMode(
          SerialStudio::ProjectFile);
  }
}

//------------------------------------------------------------------------------
// Group/dataset operations
//------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected group.
 *
 * This function prompts the user for confirmation before deleting the
 * currently selected group. If the user confirms, the group is removed from
 * the project, and group and dataset IDs areregenerated. The tree model is
 * rebuilt, the modified flag is set, and the project item is selected in the
 * UI.
 */
void JSON::ProjectModel::deleteCurrentGroup()
{
  // Ask the user for confirmation
  const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete group \"%1\"?").arg(m_selectedGroup.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question, APP_NAME, QMessageBox::Yes | QMessageBox::No);

  // Validate the user input
  if (ret != QMessageBox::Yes)
    return;

  // Delete the group
  m_groups.erase(m_groups.begin() + m_selectedGroup.groupId);

  // Regenerate group IDs
  int id = 0;
  for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id)
  {
    g->groupId = id;
    for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
      d->groupId = id;
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
 * This function prompts the user for confirmation before deleting the
 * currently selected action. If the user confirms, the action is removed from
 * the project, and action IDs areregenerated. The tree model is rebuilt, the
 * modified flag is set, and the project item is selected in the UI.
 */
void JSON::ProjectModel::deleteCurrentAction()
{
  // Ask the user for confirmation
  const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete action \"%1\"?").arg(m_selectedAction.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question, APP_NAME, QMessageBox::Yes | QMessageBox::No);

  // Validate the user input
  if (ret != QMessageBox::Yes)
    return;

  // Delete the action
  m_actions.erase(m_actions.begin() + m_selectedAction.actionId);

  // Regenerate action IDs
  int id = 0;
  for (auto a = m_actions.begin(); a != m_actions.end(); ++a, ++id)
    a->actionId = id;

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
 * This function prompts the user for confirmation before deleting the
 * currently selected dataset. If the user confirms, the dataset is removed
 * from the associated group, and dataset IDs within the group are reassigned.
 * The tree model is rebuilt, the modified flag is set, and the parent group
 * is selected in the UI.
 */
void JSON::ProjectModel::deleteCurrentDataset()
{
  // Ask the user for confirmation
  const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete dataset \"%1\"?").arg(m_selectedDataset.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question, APP_NAME, QMessageBox::Yes | QMessageBox::No);

  // Validate the user input
  if (ret != QMessageBox::Yes)
    return;

  // Get group ID & dataset ID
  const auto groupId = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  // Remove dataset
  m_groups[groupId].datasets.erase(m_groups[groupId].datasets.begin()
                                   + datasetId);

  // Reassign dataset IDs
  int id = 0;
  auto end = m_groups[groupId].datasets.end();
  auto begin = m_groups[groupId].datasets.begin();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->datasetId = id;

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select parent group
  for (auto i = m_groupItems.constBegin(); i != m_groupItems.constEnd(); ++i)
  {
    if (i.value().groupId == groupId)
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
  JSON::Group group;
  group.groupId = m_groups.size();
  group.widget = m_selectedGroup.widget;
  group.title = tr("%1 (Copy)").arg(m_selectedGroup.title);
  for (size_t i = 0; i < m_selectedGroup.datasets.size(); ++i)
  {
    auto &dataset = m_selectedGroup.datasets[i];
    dataset.groupId = group.groupId;
    dataset.index = nextDatasetIndex() + i;
    group.datasets.push_back(dataset);
  }

  // Register the group
  m_groups.push_back(group);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select the group
  for (auto i = m_groupItems.constBegin(); i != m_groupItems.constEnd(); ++i)
  {
    if (i.value().groupId == group.groupId)
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
  JSON::Action action;
  action.actionId = m_actions.size();
  action.icon = m_selectedAction.icon;
  action.txData = m_selectedAction.txData;
  action.timerMode = m_selectedAction.timerMode;
  action.eolSequence = m_selectedAction.eolSequence;
  action.timerIntervalMs = m_selectedAction.timerIntervalMs;
  action.title = tr("%1 (Copy)").arg(m_selectedAction.title);
  action.autoExecuteOnConnect = m_selectedAction.autoExecuteOnConnect;

  // Register the group
  m_actions.push_back(action);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select the action
  for (auto i = m_actionItems.constBegin(); i != m_actionItems.constEnd(); ++i)
  {
    if (i.value().actionId == action.actionId)
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
  dataset.index = nextDatasetIndex();
  dataset.title = tr("%1 (Copy)").arg(dataset.title);
  dataset.datasetId = m_groups[dataset.groupId].datasets.size();

  // Register the dataset to the group
  m_groups[dataset.groupId].datasets.push_back(dataset);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select dataset
  for (auto i = m_datasetItems.begin(); i != m_datasetItems.end(); ++i)
  {
    if (i.value().groupId == dataset.groupId
        && i.value().datasetId == dataset.datasetId)
    {
      m_selectionModel->setCurrentIndex(i.key()->index(),
                                        QItemSelectionModel::ClearAndSelect);
      break;
    }
  }
}

/**
 * @brief Ensures a valid group is selected for dataset insertion.
 *
 * If no groups exist, a default group is created. If a compatible group
 * (MultiPlot, DataGrid, or NoGroupWidget) is already selected in the tree,
 * it is used. If a dataset is selected, its parent group is resolved and
 * validated. Otherwise, the first compatible group in the project is selected.
 *
 * Updates m_selectedGroup with the resolved group.
 *
 * @see addGroup()
 */
void JSON::ProjectModel::ensureValidGroup()
{
  // Add a group if needed
  if (m_groups.empty())
    addGroup(tr("Group"), SerialStudio::NoGroupWidget);

  // Lambda to check if a group is compatible
  const auto isValidGroup = [](const QString &widgetId) -> bool {
    switch (SerialStudio::groupWidgetFromId(widgetId))
    {
      case SerialStudio::MultiPlot:
      case SerialStudio::DataGrid:
      case SerialStudio::NoGroupWidget:
        return true;
      default:
        return false;
    }
  };

  // Check if a valid group is currently selected
  bool hasGroupSelected = false;
  if (m_selectionModel && m_selectionModel->currentIndex().isValid())
  {
    auto index = m_selectionModel->currentIndex();
    auto *item = m_treeModel->itemFromIndex(index);
    if (!item && index.parent().isValid())
      item = m_treeModel->itemFromIndex(index.parent());

    if (item && m_groupItems.contains(item))
    {
      if (isValidGroup(m_groupItems.value(item).widget))
        hasGroupSelected = true;
    }

    else if (item && m_datasetItems.contains(item))
    {
      auto groupId = m_selectedDataset.groupId;
      for (const auto &group : std::as_const(m_groups))
      {
        if (group.groupId == groupId)
        {
          if (isValidGroup(group.widget))
          {
            m_selectedGroup = group;
            hasGroupSelected = true;
            break;
          }
        }
      }
    }
  }

  // Select the first compatible group if none is selected
  if (!hasGroupSelected)
  {
    for (const auto &group : std::as_const(m_groups))
    {
      if (isValidGroup(group.widget))
      {
        hasGroupSelected = true;
        m_selectedGroup = group;
        break;
      }
    }

    if (!hasGroupSelected)
      addGroup(tr("Group"), SerialStudio::NoGroupWidget);
  }
}

/**
 * @brief Adds a new dataset to the currently selected group.
 *
 * This function creates a new dataset based on the provided option, assigns
 * it a unique title, ID, and frame index, and adds it to the selected group.
 * The tree model is rebuilt, the modification flag is set, and the group is
 * reselected to rebuild the dataset model.
 *
 * @param option The dataset option that defines the type of dataset to add
 *               (e.g., Plot, FFT, Bar, Gauge, Compass).
 */
void JSON::ProjectModel::addDataset(const SerialStudio::DatasetOption option)
{
  // Initialize a new dataset
  ensureValidGroup();
  const auto groupId = m_selectedGroup.groupId;
  JSON::Dataset dataset;
  dataset.groupId = groupId;

  // Configure dataset options
  QString title;
  switch (option)
  {
    case SerialStudio::DatasetGeneric:
      title = tr("New Dataset");
      break;
    case SerialStudio::DatasetPlot:
      title = tr("New Plot");
      dataset.plt = true;
      break;
    case SerialStudio::DatasetFFT:
      title = tr("New FFT Plot");
      dataset.fft = true;
      break;
    case SerialStudio::DatasetBar:
      title = tr("New Level Indicator");
      dataset.widget = QStringLiteral("bar");
      break;
    case SerialStudio::DatasetGauge:
      title = tr("New Gauge");
      dataset.widget = QStringLiteral("gauge");
      break;
    case SerialStudio::DatasetCompass:
      title = tr("New Compass");
      dataset.wgtMin = 0;
      dataset.wgtMax = 360;
      dataset.alarmLow = 0;
      dataset.alarmHigh = 0;
      dataset.widget = QStringLiteral("compass");
      break;
    case SerialStudio::DatasetLED:
      title = tr("New LED Indicator");
      dataset.led = true;
      break;
    default:
      break;
  }

  // Check if any existing dataset has the same title
  int count = 1;
  QString newTitle = title;
  for (const auto &d : std::as_const(m_groups[groupId].datasets))
  {
    if (d.title == newTitle)
    {
      count++;
      newTitle = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  // If the title was modified, ensure it is unique
  while (count > 1)
  {
    bool titleExists = false;
    for (const auto &d : std::as_const(m_groups[groupId].datasets))
    {
      if (d.title == newTitle)
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
  dataset.title = newTitle;
  dataset.index = nextDatasetIndex();
  dataset.datasetId = m_groups[groupId].datasets.size();

  // Add dataset to group
  m_groups[groupId].datasets.push_back(dataset);

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select newly added dataset item
  for (auto i = m_datasetItems.begin(); i != m_datasetItems.end(); ++i)
  {
    if (i.value().datasetId == dataset.datasetId
        && i.value().groupId == dataset.groupId)
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
void JSON::ProjectModel::changeDatasetOption(
    const SerialStudio::DatasetOption option, const bool checked)
{
  // Modify dataset options
  switch (option)
  {
    case SerialStudio::DatasetPlot:
      m_selectedDataset.plt = checked;
      break;
    case SerialStudio::DatasetFFT:
      m_selectedDataset.fft = checked;
      break;
    case SerialStudio::DatasetBar:
      m_selectedDataset.widget = checked ? QStringLiteral("bar") : "";
      break;
    case SerialStudio::DatasetGauge:
      m_selectedDataset.widget = checked ? QStringLiteral("gauge") : "";
      break;
    case SerialStudio::DatasetCompass:
      m_selectedDataset.widget = checked ? QStringLiteral("compass") : "";
      break;
    case SerialStudio::DatasetLED:
      m_selectedDataset.led = checked;
      break;
    default:
      break;
  }

  // Replace dataset
  const auto groupId = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;
  m_groups[groupId].datasets[datasetId] = m_selectedDataset;

  // Build tree model & set modification flag
  buildTreeModel();
  setModified(true);

  // Select dataset item again to rebuild dataset model
  for (auto i = m_datasetItems.begin(); i != m_datasetItems.end(); ++i)
  {
    if (i.value().datasetId == datasetId && i.value().groupId == groupId)
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
 * This function creates a new action, ensuring the title is unique by
 * appending a number if necessary. It then registers the action, updates the
 * tree model, and sets the modified flag. The newly added action is selected
 * in the user interface.
 */
void JSON::ProjectModel::addAction()
{
  // Check if any existing group has the same title
  int count = 1;
  QString title = tr("New Action");
  for (const auto &action : std::as_const(m_actions))
  {
    if (action.title == title)
    {
      count++;
      title = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  // If the title was modified, ensure it is unique
  while (count > 1)
  {
    bool titleExists = false;
    for (const auto &action : std::as_const(m_actions))
    {
      if (action.title == title)
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
  JSON::Action action;
  action.title = title;
  action.actionId = m_actions.size();

  // Register the action
  m_actions.push_back(action);

  // Update the user interface
  buildTreeModel();
  setModified(true);

  // Select action
  for (auto i = m_actionItems.constBegin(); i != m_actionItems.constEnd(); ++i)
  {
    if (i.value().actionId == action.actionId)
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
 * This function creates a new group, ensuring the title is unique by
 * appending a number if necessary. It then registers the group, sets its
 * widget type, updates the tree model, and sets the modified flag. The newly
 * added group is selected in the user interface.
 *
 * @param title The desired title for the new group.
 * @param widget The widget type associated with the group.
 */
void JSON::ProjectModel::addGroup(const QString &title,
                                  const SerialStudio::GroupWidget widget)
{
  // Check if any existing group has the same title
  int count = 1;
  QString newTitle = title;
  for (const auto &group : std::as_const(m_groups))
  {
    if (group.title == newTitle)
    {
      count++;
      newTitle = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  // If the title was modified, ensure it is unique
  while (count > 1)
  {
    bool titleExists = false;
    for (const auto &group : std::as_const(m_groups))
    {
      if (group.title == newTitle)
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
  JSON::Group group;
  group.title = newTitle;
  group.groupId = m_groups.size();

  // Register the group & add the widget
  m_groups.push_back(group);
  setGroupWidget(m_groups.size() - 1, widget);

  // Update the user interface
  buildTreeModel();
  setModified(true);

  // Select group
  for (auto i = m_groupItems.constBegin(); i != m_groupItems.constEnd(); ++i)
  {
    if (i.value().groupId == group.groupId)
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
                                        const SerialStudio::GroupWidget widget)
{
  // Get group data
  auto &grp = m_groups[group];
  const auto groupId = grp.groupId;

  // Warn user if group contains existing datasets
  if (!grp.datasets.empty())
  {
    if ((widget == SerialStudio::DataGrid || widget == SerialStudio::MultiPlot
         || widget == SerialStudio::NoGroupWidget)
        && (grp.widget == "multiplot" || grp.widget == "datagrid"
            || grp.widget == ""))
      grp.widget = "";

    else
    {
      auto ret = Misc::Utilities::showMessageBox(
          tr("Are you sure you want to change the group-level widget?"),
          tr("Existing datasets for this group will be deleted"),
          QMessageBox::Question, APP_NAME, QMessageBox::Yes | QMessageBox::No);
      if (ret == QMessageBox::No)
        return false;
      else
        grp.datasets.clear();
    }
  }

  // No widget
  if (widget == SerialStudio::NoGroupWidget)
    grp.widget = "";

  // Data grid widget
  if (widget == SerialStudio::DataGrid)
    grp.widget = "datagrid";

  // Multiplot widget
  else if (widget == SerialStudio::MultiPlot)
    grp.widget = "multiplot";

  // Accelerometer widget
  else if (widget == SerialStudio::Accelerometer)
  {
    // Set widget type
    grp.widget = "accelerometer";

    // Create datasets
    JSON::Dataset x, y, z;

    // Set dataset IDs
    x.datasetId = 0;
    y.datasetId = 1;
    z.datasetId = 2;

    // Register parent group for the datasets
    x.groupId = groupId;
    y.groupId = groupId;
    z.groupId = groupId;

    // Set dataset indexes
    x.index = nextDatasetIndex();
    y.index = nextDatasetIndex() + 1;
    z.index = nextDatasetIndex() + 2;

    // Set measurement units
    x.units = "m/s²";
    y.units = "m/s²";
    z.units = "m/s²";

    // Set dataset properties
    x.wgtMin = 0;
    x.wgtMax = 0;
    y.wgtMin = 0;
    y.wgtMax = 0;
    z.wgtMin = 0;
    z.wgtMax = 0;
    x.plt = true;
    y.plt = true;
    z.plt = true;
    x.widget = "x";
    y.widget = "y";
    z.widget = "z";
    x.alarmLow = 0;
    y.alarmLow = 0;
    z.alarmLow = 0;
    x.alarmHigh = 0;
    y.alarmHigh = 0;
    z.alarmHigh = 0;
    x.title = tr("Accelerometer %1").arg("X");
    y.title = tr("Accelerometer %1").arg("Y");
    z.title = tr("Accelerometer %1").arg("Z");

    // Add datasets to group
    grp.datasets.push_back(x);
    grp.datasets.push_back(y);
    grp.datasets.push_back(z);
  }

  // Gyroscope widget
  else if (widget == SerialStudio::Gyroscope)
  {
    // Set widget type
    grp.widget = "gyro";

    // Create datasets
    JSON::Dataset x, y, z;

    // Set dataset IDs
    x.datasetId = 0;
    y.datasetId = 1;
    z.datasetId = 2;

    // Register parent group for the datasets
    x.groupId = groupId;
    y.groupId = groupId;
    z.groupId = groupId;

    // Set dataset indexes
    x.index = nextDatasetIndex();
    y.index = nextDatasetIndex() + 1;
    z.index = nextDatasetIndex() + 2;

    // Set measurement units
    x.units = "deg/s";
    y.units = "deg/s";
    z.units = "deg/s";

    // Set dataset properties
    x.wgtMin = 0;
    x.wgtMax = 0;
    y.wgtMin = 0;
    y.wgtMax = 0;
    z.wgtMin = 0;
    z.wgtMax = 0;
    x.plt = true;
    y.plt = true;
    z.plt = true;
    x.widget = "x";
    y.widget = "y";
    z.widget = "z";
    x.alarmLow = 0;
    y.alarmLow = 0;
    z.alarmLow = 0;
    x.alarmHigh = 0;
    y.alarmHigh = 0;
    z.alarmHigh = 0;
    x.title = tr("Gyro %1").arg("X");
    y.title = tr("Gyro %1").arg("Y");
    z.title = tr("Gyro %1").arg("Z");

    // Add datasets to group
    grp.datasets.push_back(x);
    grp.datasets.push_back(y);
    grp.datasets.push_back(z);
  }

  // Map widget
  else if (widget == SerialStudio::GPS)
  {
    // Set widget type
    grp.widget = "map";

    // Create datasets
    JSON::Dataset lat, lon, alt;

    // Set dataset IDs
    lat.datasetId = 0;
    lon.datasetId = 1;
    alt.datasetId = 2;

    // Register parent group for the datasets
    lat.groupId = groupId;
    lon.groupId = groupId;
    alt.groupId = groupId;

    // Set dataset indexes
    lat.index = nextDatasetIndex();
    lon.index = nextDatasetIndex() + 1;
    alt.index = nextDatasetIndex() + 2;

    // Set measurement units
    lat.units = "°";
    lon.units = "°";
    alt.units = "m";

    // Set dataset properties
    lat.widget = "lat";
    lon.widget = "lon";
    alt.widget = "alt";
    lat.alarmLow = 0;
    lon.alarmLow = 0;
    alt.alarmLow = 0;
    lat.alarmHigh = 0;
    lon.alarmHigh = 0;
    alt.alarmHigh = 0;
    lat.wgtMax = 90.0;
    lat.wgtMin = -90.0;
    lon.wgtMax = 180.0;
    lon.wgtMin = -180.0;
    alt.wgtMin = -500.0;
    alt.wgtMax = 1000000.0;
    lat.title = tr("Latitude");
    lon.title = tr("Longitude");
    alt.title = tr("Altitude");

    // Add datasets to group
    grp.datasets.push_back(lat);
    grp.datasets.push_back(lon);
    grp.datasets.push_back(alt);
  }

  // 3D plot widget
  else if (widget == SerialStudio::Plot3D)
  {
    // Set widget type
    grp.widget = "plot3d";

    // Create datasets
    JSON::Dataset x, y, z;

    // Set dataset IDs
    x.datasetId = 0;
    y.datasetId = 1;
    z.datasetId = 2;

    // Register parent group for the datasets
    x.groupId = groupId;
    y.groupId = groupId;
    z.groupId = groupId;

    // Set dataset indexes
    x.index = nextDatasetIndex();
    y.index = nextDatasetIndex() + 1;
    z.index = nextDatasetIndex() + 2;

    // Set dataset properties
    x.wgtMin = 0;
    x.wgtMax = 0;
    y.wgtMin = 0;
    y.wgtMax = 0;
    z.wgtMin = 0;
    z.wgtMax = 0;
    x.widget = "x";
    y.widget = "y";
    z.widget = "z";
    x.alarmLow = 0;
    y.alarmLow = 0;
    z.alarmLow = 0;
    x.alarmHigh = 0;
    y.alarmHigh = 0;
    z.alarmHigh = 0;
    x.title = tr("X");
    y.title = tr("Y");
    z.title = tr("Z");

    // Add datasets to group
    grp.datasets.push_back(x);
    grp.datasets.push_back(y);
    grp.datasets.push_back(z);
  }

  // Replace previous group with new group
  m_groups[group] = grp;

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

/**
 * @brief Forces the Project Editor to show the frame parser code editor
 */
void JSON::ProjectModel::displayFrameParserView()
{
  for (auto it = m_rootItems.begin(); it != m_rootItems.end(); ++it)
  {
    if (it.value() == kFrameParser)
    {
      QTimer::singleShot(100, this, [=, this] {
        selectionModel()->setCurrentIndex(it.key()->index(),
                                          QItemSelectionModel::ClearAndSelect);
      });

      break;
    }
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
  auto *frameParsingCode = new QStandardItem(tr("Frame Parser Code"));

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
  for (const auto &action : m_actions)
  {
    // Create action item
    auto *actionItem = new QStandardItem(action.title);

    // Configure action item
    const auto icon = "qrc:/rcc/icons/project-editor/treeview/action.svg";
    actionItem->setData(-1, TreeViewFrameIndex);
    actionItem->setData(icon, TreeViewIcon);
    actionItem->setData(action.title, TreeViewText);

    // Register action item
    root->appendRow(actionItem);
    m_actionItems.insert(actionItem, action);
  }

  // Iterate through the groups and add them to the model
  for (const auto &group : m_groups)
  {
    // Get group item
    auto *groupItem = new QStandardItem(group.title);

    // Get which icon to use for the group
    auto widget = SerialStudio::getDashboardWidget(group);
    auto icon = SerialStudio::dashboardWidgetIcon(widget, false);

    // Set metadata for the group item
    groupItem->setData(icon, TreeViewIcon);
    groupItem->setData(-1, TreeViewFrameIndex);
    groupItem->setData(group.title, TreeViewText);

    // Iterate through the datasets within this group and add them as children
    for (const auto &dataset : group.datasets)
    {
      // Create dataset item
      auto *datasetItem = new QStandardItem(dataset.title);

      // Get which icon to use for the DATASET
      auto widgets = SerialStudio::getDashboardWidgets(dataset);
      QString dIcon = "qrc:/rcc/icons/project-editor/treeview/dataset.svg";
      if (widgets.count() > 0)
        dIcon = SerialStudio::dashboardWidgetIcon(widgets.first(), false);

      // Set metadata for the dataset item
      datasetItem->setData(dIcon, TreeViewIcon);
      datasetItem->setData(dataset.title, TreeViewText);
      datasetItem->setData(dataset.index, TreeViewFrameIndex);

      // Add dataset item as child for group item
      groupItem->appendRow(datasetItem);

      // Create relationship with dataset item & actual dataset
      m_datasetItems.insert(datasetItem, dataset);
    }

    // Restore expanded states for the group
    restoreExpandedStateMap(groupItem, expandedStates,
                            root->text() + "/" + group.title);

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
 * @brief Builds the project model that contains project configuration
 * settings.
 *
 * This function creates a new `CustomModel` for managing and displaying
 * project-level settings such as the title, frame start and end delimiters,
 * data conversion method, and the Thunderforest API key.
 *
 * Each item in the model is editable and has associated metadata like
 * placeholders and descriptions. The function also sets up a signal to handle
 * changes made to the model items and emits the `projectModelChanged()`
 * signal to update the user interface.
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

  //----------------------------------------------------------------------------
  // General information section
  //----------------------------------------------------------------------------

  // Add section header
  auto *sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Project Information"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/project.svg",
                      ParameterIcon);
  m_projectModel->appendRow(sectionHdr);

  // Add project title
  auto *title = new QStandardItem();
  title->setEditable(true);
  title->setData(true, Active);
  title->setData(TextField, WidgetType);
  title->setData(m_title, EditableValue);
  title->setData(kProjectView_Title, ParameterType);
  title->setData(tr("Project Title"), ParameterName);
  title->setData(tr("Untitled Project"), PlaceholderValue);
  title->setData(tr("Name or description of the project"),
                 ParameterDescription);
  m_projectModel->appendRow(title);

  //----------------------------------------------------------------------------
  // Frame detection
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Frame Detection"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/frame-detection.svg",
                      ParameterIcon);
  m_projectModel->appendRow(sectionHdr);

  // Add frame detection method
  auto *frameDetection = new QStandardItem();
  frameDetection->setEditable(true);
  frameDetection->setData(true, Active);
  frameDetection->setData(ComboBox, WidgetType);
  frameDetection->setData(m_frameDetectionMethods, ComboBoxData);
  frameDetection->setData(
      m_frameDetectionMethodsValues.indexOf(m_frameDetection), EditableValue);
  frameDetection->setData(kProjectView_FrameDetection, ParameterType);
  frameDetection->setData(tr("Frame Detection Method"), ParameterName);
  frameDetection->setData(tr("Select how incoming data frames are identified"),
                          ParameterDescription);
  m_projectModel->appendRow(frameDetection);

  // Add hexadecimal frame sequence
  auto *sequence = new QStandardItem();
  sequence->setEditable(m_frameDetection != SerialStudio::NoDelimiters);
  sequence->setData(sequence->isEditable(), Active);
  sequence->setData(CheckBox, WidgetType);
  sequence->setData(m_hexadecimalDelimiters, EditableValue);
  sequence->setData(tr("Hexadecimal Delimiters"), ParameterName);
  sequence->setData(kProjectView_HexadecimalSequence, ParameterType);
  sequence->setData(tr("Use hex values to define frame boundaries"),
                    ParameterDescription);
  m_projectModel->appendRow(sequence);

  // Get frame type data
  auto delimWidget = m_hexadecimalDelimiters ? HexTextField : TextField;

  // Add frame start sequence
  auto *frameStart = new QStandardItem();
  frameStart->setEditable(m_frameDetection == SerialStudio::StartAndEndDelimiter
                          || m_frameDetection
                                 == SerialStudio::StartDelimiterOnly);
  frameStart->setData(frameStart->isEditable(), Active);
  frameStart->setData(delimWidget, WidgetType);
  frameStart->setData(m_frameStartSequence, EditableValue);
  frameStart->setData(tr("Start Sequence"), ParameterName);
  frameStart->setData(QStringLiteral("/*"), PlaceholderValue);
  frameStart->setData(kProjectView_FrameStartSequence, ParameterType);
  frameStart->setData(tr("Marks the beginning of each data frame"),
                      ParameterDescription);
  m_projectModel->appendRow(frameStart);

  // Add frame end sequence
  auto *frameEnd = new QStandardItem();
  frameEnd->setEditable(m_frameDetection == SerialStudio::StartAndEndDelimiter
                        || m_frameDetection == SerialStudio::EndDelimiterOnly);
  frameEnd->setData(frameEnd->isEditable(), Active);
  frameEnd->setData(delimWidget, WidgetType);
  frameEnd->setData(m_frameEndSequence, EditableValue);
  frameEnd->setData(tr("End Sequence"), ParameterName);
  frameEnd->setData(QStringLiteral("*/"), PlaceholderValue);
  frameEnd->setData(kProjectView_FrameEndSequence, ParameterType);
  frameEnd->setData(tr("Marks the end of each data frame"),
                    ParameterDescription);
  m_projectModel->appendRow(frameEnd);

  //----------------------------------------------------------------------------
  // Data conversion & integrity checks
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Payload Processing & Validation"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/data-conversion.svg",
                      ParameterIcon);
  m_projectModel->appendRow(sectionHdr);

  // Add decoding
  auto *decoding = new QStandardItem();
  decoding->setEditable(true);
  decoding->setData(true, Active);
  decoding->setData(ComboBox, WidgetType);
  decoding->setData(m_frameDecoder, EditableValue);
  decoding->setData(m_decoderOptions, ComboBoxData);
  decoding->setData(tr("Data Format"), ParameterName);
  decoding->setData(kProjectView_FrameDecoder, ParameterType);
  decoding->setData(tr("Format of raw data used for decoding each frame"),
                    ParameterDescription);
  m_projectModel->appendRow(decoding);

  // Add checksum
  auto *checksum = new QStandardItem();
  auto checksumAlgo = IO::availableChecksums().indexOf(m_checksumAlgorithm);
  checksum->setEditable(true);
  checksum->setData(true, Active);
  checksum->setData(ComboBox, WidgetType);
  checksum->setData(checksumAlgo, EditableValue);
  checksum->setData(m_checksumMethods, ComboBoxData);
  checksum->setData(tr("Checksum Algorithm"), ParameterName);
  checksum->setData(kProjectView_ChecksumFunction, ParameterType);
  checksum->setData(tr("Method used to validate frame integrity"),
                    ParameterDescription);
  m_projectModel->appendRow(checksum);

  //----------------------------------------------------------------------------
  // Signals/slots finalization
  //----------------------------------------------------------------------------

  // Handle edits
  connect(m_projectModel, &CustomModel::itemChanged, this,
          &JSON::ProjectModel::onProjectItemChanged);

  // Update user interface
  Q_EMIT projectModelChanged();
}

/**
 * @brief Builds the group model for managing the settings of a specific
 * group.
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

  // Add section header
  auto *sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Group Information"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/group.svg",
                      ParameterIcon);
  m_groupModel->appendRow(sectionHdr);

  // Add group title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(true, Active);
  title->setData(TextField, WidgetType);
  title->setData(group.title, EditableValue);
  title->setData(kGroupView_Title, ParameterType);
  title->setData(tr("Group Title"), ParameterName);
  title->setData(tr("Untitled Group"), PlaceholderValue);
  title->setData(tr("Title or description of this dataset group"),
                 ParameterDescription);
  m_groupModel->appendRow(title);

  // Get appropiate widget index for current group
  int index = 0;
  bool found = false;
  for (auto it = m_groupWidgets.begin(); it != m_groupWidgets.end();
       ++it, ++index)
  {
    if (it.key() == group.widget)
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
  widget->setData(true, Active);
  widget->setData(ComboBox, WidgetType);
  widget->setData(m_groupWidgets.values(), ComboBoxData);
  widget->setData(index, EditableValue);
  widget->setData(kGroupView_Widget, ParameterType);
  widget->setData(tr("Composite Widget"), ParameterName);
  widget->setData(
      tr("Select how this group of datasets should be visualized (optional)"),
      ParameterDescription);
  m_groupModel->appendRow(widget);

  // Handle edits
  connect(m_groupModel, &CustomModel::itemChanged, this,
          &JSON::ProjectModel::onGroupItemChanged);

  // Update user interface
  emit groupModelChanged();
}

/**
 * @brief Builds the UI model for editing an action configuration.
 *
 * This method initializes and populates the `m_actionModel` with editable
 * UI fields representing a single action in the project. Actions define
 * user-triggered commands that can transmit data through the serial interface,
 * optionally using timers or automatic triggers.
 *
 * The model includes the following configuration sections:
 * - General Information: title and icon
 * - Data Payload: binary/text data and optional EOL sequences
 * - Execution Behavior: automatic execution on device connection
 * - Timer Behavior: optional timer-based repetition of the action
 *
 * The populated model is connected to change tracking and triggers
 * `actionModelChanged()` when edits are made.
 *
 * @param action The Action object whose data will populate the editing model.
 */
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

  //----------------------------------------------------------------------------
  // General information section
  //----------------------------------------------------------------------------

  // Add section header
  auto *sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("General Information"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/action.svg",
                      ParameterIcon);
  m_actionModel->appendRow(sectionHdr);

  // Add action title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(true, Active);
  title->setData(TextField, WidgetType);
  title->setData(action.title, EditableValue);
  title->setData(tr("Action Title"), ParameterName);
  title->setData(kActionView_Title, ParameterType);
  title->setData(tr("Untitled Action"), PlaceholderValue);
  title->setData(tr("Name or description of this action"),
                 ParameterDescription);
  m_actionModel->appendRow(title);

  // Add action icon
  auto icon = new QStandardItem();
  icon->setEditable(true);
  icon->setData(true, Active);
  icon->setData(IconPicker, WidgetType);
  icon->setData(action.icon, EditableValue);
  icon->setData(kActionView_Icon, ParameterType);
  icon->setData(tr("Action Icon"), ParameterName);
  icon->setData(tr("Default Icon"), PlaceholderValue);
  icon->setData(tr("Icon displayed for this action in the dashboard"),
                ParameterDescription);
  m_actionModel->appendRow(icon);

  //----------------------------------------------------------------------------
  // Data format
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Data Payload"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/tx-data.svg",
                      ParameterIcon);
  m_actionModel->appendRow(sectionHdr);

  // Add binary selector checkbox
  auto binaryData = new QStandardItem();
  binaryData->setEditable(true);
  binaryData->setData(true, Active);
  binaryData->setData(CheckBox, WidgetType);
  binaryData->setData(0, PlaceholderValue);
  binaryData->setData(action.binaryData, EditableValue);
  binaryData->setData(kActionView_Binary, ParameterType);
  binaryData->setData(tr("Send as Binary"), ParameterName);
  binaryData->setData(tr("Send raw binary data when this action is triggered"),
                      ParameterDescription);
  m_actionModel->appendRow(binaryData);

  // Add binary action data
  if (action.binaryData)
  {
    auto data = new QStandardItem();
    data->setEditable(true);
    data->setData(true, Active);
    data->setData(HexTextField, WidgetType);
    data->setData(action.txData, EditableValue);
    data->setData(kActionView_Data, ParameterType);
    data->setData(tr("Command"), PlaceholderValue);
    data->setData(tr("Transmit Data (Hex)"), ParameterName);
    data->setData(
        tr("Hexadecimal payload to send when the action is triggered"),
        ParameterDescription);
    m_actionModel->appendRow(data);
  }

  // Add action data (non-binary)
  else
  {
    auto data = new QStandardItem();
    data->setEditable(true);
    data->setData(true, Active);
    data->setData(TextField, WidgetType);
    data->setData(action.txData, EditableValue);
    data->setData(kActionView_Data, ParameterType);
    data->setData(tr("Command"), PlaceholderValue);
    data->setData(tr("Transmit Data"), ParameterName);
    data->setData(tr("Text payload to send when the action is triggered"),
                  ParameterDescription);
    m_actionModel->appendRow(data);
  }

  // Get appropiate end of line index for current action
  int eolIndex = 0;
  bool found = false;
  for (auto it = m_eolSequences.begin(); it != m_eolSequences.end();
       ++it, ++eolIndex)
  {
    if (it.key() == action.eolSequence)
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
  eol->setData(ComboBox, WidgetType);
  eol->setEditable(!action.binaryData);
  eol->setData(eolIndex, EditableValue);
  eol->setData(!action.binaryData, Active);
  eol->setData(kActionView_EOL, ParameterType);
  eol->setData(m_eolSequences.values(), ComboBoxData);
  eol->setData(tr("End-of-Line Sequence"), ParameterName);
  eol->setData(tr("EOL characters to append to the message (e.g. \\n, \\r\\n)"),
               ParameterDescription);
  m_actionModel->appendRow(eol);

  //----------------------------------------------------------------------------
  // Action behavior
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Execution Behavior"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/action-behavior.svg",
                      ParameterIcon);
  m_actionModel->appendRow(sectionHdr);

  // Auto-execute on connect
  auto autoExecute = new QStandardItem();
  autoExecute->setEditable(true);
  autoExecute->setData(true, Active);
  autoExecute->setData(0, PlaceholderValue);
  autoExecute->setData(CheckBox, WidgetType);
  autoExecute->setData(kActionView_AutoExecute, ParameterType);
  autoExecute->setData(action.autoExecuteOnConnect, EditableValue);
  autoExecute->setData(tr("Auto-Execute on Connect"), ParameterName);
  autoExecute->setData(
      tr("Automatically trigger this action when the device connects"),
      ParameterDescription);
  m_actionModel->appendRow(autoExecute);

  //----------------------------------------------------------------------------
  // Action timers
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Timer Behavior"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/timer.svg",
                      ParameterIcon);
  m_actionModel->appendRow(sectionHdr);

  // Timer mode
  auto timerMode = new QStandardItem();
  timerMode->setEditable(true);
  timerMode->setData(true, Active);
  timerMode->setData(ComboBox, WidgetType);
  timerMode->setData(m_timerModes, ComboBoxData);
  timerMode->setData(tr("Timer Mode"), ParameterName);
  timerMode->setData(kActionView_TimerMode, ParameterType);
  timerMode->setData(static_cast<int>(action.timerMode), EditableValue);
  timerMode->setData(
      tr("Choose when and how this action should repeat automatically"),
      ParameterDescription);
  m_actionModel->appendRow(timerMode);

  // Timer interval
  auto timerInterval = new QStandardItem();
  timerInterval->setData(IntField, WidgetType);
  timerInterval->setEditable(action.timerMode != JSON::TimerMode::Off);
  timerInterval->setData(tr("Interval (ms)"), ParameterName);
  timerInterval->setData(timerInterval->isEditable(), Active);
  timerInterval->setData(action.timerIntervalMs, EditableValue);
  timerInterval->setData(kActionView_TimerInterval, ParameterType);
  timerInterval->setData(tr("Timer Interval (ms)"), PlaceholderValue);
  timerInterval->setData(
      tr("Milliseconds between each repeated trigger of this action"),
      ParameterDescription);
  m_actionModel->appendRow(timerInterval);

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

  // Get which optional parameters should be displayed
  const bool showWidget = currentDatasetIsEditable();

  //----------------------------------------------------------------------------
  // General information section
  //----------------------------------------------------------------------------

  // Add section header
  auto *sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("General Information"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/dataset.svg",
                      ParameterIcon);
  m_datasetModel->appendRow(sectionHdr);

  // Add dataset title
  auto title = new QStandardItem();
  title->setEditable(true);
  title->setData(true, Active);
  title->setData(TextField, WidgetType);
  title->setData(dataset.title, EditableValue);
  title->setData(kDatasetView_Title, ParameterType);
  title->setData(tr("Untitled Dataset"), PlaceholderValue);
  title->setData(tr("Dataset Title"), ParameterName);
  title->setData(
      tr("Name of the dataset, used for labeling and identification"),
      ParameterDescription);
  m_datasetModel->appendRow(title);

  // Add dataset index
  auto index = new QStandardItem();
  index->setEditable(true);
  index->setData(true, Active);
  index->setData(IntField, WidgetType);
  index->setData(dataset.index, EditableValue);
  index->setData(kDatasetView_Index, ParameterType);
  index->setData(nextDatasetIndex(), PlaceholderValue);
  index->setData(tr("Frame Index"), ParameterName);
  index->setData(tr("Frame position used for aligning datasets in time"),
                 ParameterDescription);
  m_datasetModel->appendRow(index);

  // Add units
  auto units = new QStandardItem();
  units->setEditable(true);
  units->setData(true, Active);
  units->setData(TextField, WidgetType);
  units->setData(dataset.units, EditableValue);
  units->setData(kDatasetView_Units, ParameterType);
  units->setData(tr("Measurement Unit"), ParameterName);
  units->setData(tr("Volts, Amps, etc."), PlaceholderValue);
  units->setData(tr("Unit of measurement, such as volts or amps (optional)"),
                 ParameterDescription);
  m_datasetModel->appendRow(units);

  //----------------------------------------------------------------------------
  // Plot section
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Plot Settings"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/plot.svg",
                      ParameterIcon);
  m_datasetModel->appendRow(sectionHdr);

  // Get appropiate plotting mode index for current dataset
  int plotIndex = 0;
  bool found = false;
  const auto currentPair = qMakePair(dataset.plt, dataset.log);
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
  plot->setData(plotIndex, EditableValue);
  plot->setData(plot->isEditable(), Active);
  plot->setData(m_plotOptions.values(), ComboBoxData);
  plot->setData(tr("Enable Plot Widget"), ParameterName);
  plot->setData(kDatasetView_Plot, ParameterType);
  plot->setData(tr("Plot data in real-time"), ParameterDescription);
  m_datasetModel->appendRow(plot);

  // Ensure X-axis ID is reset to "Samples" when an invalid index is set
  int xAxisIdx = 0;
  for (const auto &group : std::as_const(m_groups))
  {
    for (const auto &d : group.datasets)
    {
      const auto idx = d.index;
      if (idx == m_selectedDataset.xAxisId)
      {
        xAxisIdx = idx;
        break;
      }
    }

    if (xAxisIdx != 0)
      break;
  }

  // Construct item
  auto xAxis = new QStandardItem();
  xAxis->setEditable(dataset.plt);
  xAxis->setData(ComboBox, WidgetType);
  xAxis->setData(xAxisIdx, EditableValue);
  xAxis->setData(xAxis->isEditable(), Active);
  xAxis->setData(xDataSources(), ComboBoxData);
  xAxis->setData(kDatasetView_xAxis, ParameterType);
  xAxis->setData(tr("X-Axis Source"), ParameterName);
  xAxis->setData(tr("Choose which dataset to use for the X-Axis in plots"),
                 ParameterDescription);
  m_datasetModel->appendRow(xAxis);

  // Add minimum value
  auto pltMin = new QStandardItem();
  pltMin->setEditable(dataset.plt);
  pltMin->setData(0, PlaceholderValue);
  pltMin->setData(FloatField, WidgetType);
  pltMin->setData(pltMin->isEditable(), Active);
  pltMin->setData(dataset.pltMin, EditableValue);
  pltMin->setData(kDatasetView_PltMin, ParameterType);
  pltMin->setData(tr("Minimum Plot Value (optional)"), ParameterName);
  pltMin->setData(tr("Lower bound for plot display range"),
                  ParameterDescription);
  m_datasetModel->appendRow(pltMin);

  // Add maximum value
  auto pltMax = new QStandardItem();
  pltMax->setEditable(dataset.plt);
  pltMax->setData(0, PlaceholderValue);
  pltMax->setData(FloatField, WidgetType);
  pltMax->setData(pltMax->isEditable(), Active);
  pltMax->setData(dataset.pltMax, EditableValue);
  pltMax->setData(kDatasetView_PltMax, ParameterType);
  pltMax->setData(tr("Maximum Plot Value (optional)"), ParameterName);
  pltMax->setData(tr("Upper bound for plot display range"),
                  ParameterDescription);
  m_datasetModel->appendRow(pltMax);

  //----------------------------------------------------------------------------
  // FFT Section
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("FFT Configuration"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/fft.svg",
                      ParameterIcon);
  m_datasetModel->appendRow(sectionHdr);

  // Add FFT checkbox
  auto fft = new QStandardItem();
  fft->setEditable(true);
  fft->setData(0, PlaceholderValue);
  fft->setData(CheckBox, WidgetType);
  fft->setData(fft->isEditable(), Active);
  fft->setData(dataset.fft, EditableValue);
  fft->setData(kDatasetView_FFT, ParameterType);
  fft->setData(tr("Enable FFT Analysis"), ParameterName);
  fft->setData(tr("Perform frequency-domain analysis of the dataset"),
               ParameterDescription);
  m_datasetModel->appendRow(fft);

  // Get FFT window size index
  const auto windowSize = QString::number(dataset.fftSamples);
  int windowIndex = m_fftSamples.indexOf(windowSize);
  if (windowIndex < 0)
    windowIndex = 7;

  // Add FFT window size
  auto fftWindow = new QStandardItem();
  fftWindow->setEditable(dataset.fft);
  fftWindow->setData(ComboBox, WidgetType);
  fftWindow->setData(m_fftSamples, ComboBoxData);
  fftWindow->setData(windowIndex, EditableValue);
  fftWindow->setData(fftWindow->isEditable(), Active);
  fftWindow->setData(kDatasetView_FFT_Samples, ParameterType);
  fftWindow->setData(tr("FFT Window Size"), ParameterName);
  fftWindow->setData(
      tr("Number of samples used for each FFT calculation window"),
      ParameterDescription);
  m_datasetModel->appendRow(fftWindow);

  // Add FFT sampling rate
  auto fftSamplingRate = new QStandardItem();
  fftSamplingRate->setEditable(dataset.fft);
  fftSamplingRate->setData(IntField, WidgetType);
  fftSamplingRate->setData(100, PlaceholderValue);
  fftSamplingRate->setData(fftSamplingRate->isEditable(), Active);
  fftSamplingRate->setData(dataset.fftSamplingRate, EditableValue);
  fftSamplingRate->setData(kDatasetView_FFT_SamplingRate, ParameterType);
  fftSamplingRate->setData(tr("FFT Sampling Rate (Hz, required)"),
                           ParameterName);
  fftSamplingRate->setData(tr("Sampling frequency used for FFT (in Hz)"),
                           ParameterDescription);
  m_datasetModel->appendRow(fftSamplingRate);

  // Add minimum value
  auto fftMin = new QStandardItem();
  fftMin->setEditable(dataset.fft);
  fftMin->setData(0, PlaceholderValue);
  fftMin->setData(FloatField, WidgetType);
  fftMin->setData(fftMin->isEditable(), Active);
  fftMin->setData(dataset.fftMin, EditableValue);
  fftMin->setData(kDatasetView_FFTMin, ParameterType);
  fftMin->setData(tr("Minimum Value (recommended)"), ParameterName);
  fftMin->setData(tr("Lower bound for data normalization"),
                  ParameterDescription);
  m_datasetModel->appendRow(fftMin);

  // Add maximum value
  auto fftMax = new QStandardItem();
  fftMax->setEditable(dataset.fft);
  fftMax->setData(0, PlaceholderValue);
  fftMax->setData(FloatField, WidgetType);
  fftMax->setData(fftMax->isEditable(), Active);
  fftMax->setData(dataset.fftMax, EditableValue);
  fftMax->setData(kDatasetView_FFTMax, ParameterType);
  fftMax->setData(tr("Maximum Value (recommended)"), ParameterName);
  fftMax->setData(tr("Upper bound for data normalization"),
                  ParameterDescription);
  m_datasetModel->appendRow(fftMax);

  //----------------------------------------------------------------------------
  // Widget section
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Widget Settings"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/widget.svg",
                      ParameterIcon);
  m_datasetModel->appendRow(sectionHdr);

  // Get appropiate widget index for current dataset
  found = false;
  int widgetIndex = 0;
  for (auto it = m_datasetWidgets.begin(); it != m_datasetWidgets.end();
       ++it, ++widgetIndex)
  {
    if (it.key() == dataset.widget)
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
  widget->setEditable(showWidget);
  widget->setData(showWidget, Active);
  widget->setData(ComboBox, WidgetType);
  widget->setData(widgetIndex, EditableValue);
  widget->setData(kDatasetView_Widget, ParameterType);
  widget->setData(m_datasetWidgets.values(), ComboBoxData);
  widget->setData(tr("Widget"), ParameterName);
  widget->setData(tr("Select the visual widget used to display this dataset"),
                  ParameterDescription);
  m_datasetModel->appendRow(widget);

  // Add show in overview method
  auto overview = new QStandardItem();
  overview->setData(CheckBox, WidgetType);
  overview->setEditable(showWidget && m_groups.size() > 1);
  overview->setData(overview->isEditable(), Active);
  overview->setData(dataset.overviewDisplay, EditableValue);
  overview->setData(kDatasetView_Overview, ParameterType);
  overview->setData(tr("Show in Overview"), ParameterName);
  overview->setData(
      tr("Display this widget in the dashboard overview (if enabled)"),
      ParameterDescription);
  m_datasetModel->appendRow(overview);

  // Check if range widget options should be enabled
  const bool rangeEnabled
      = showWidget && (dataset.widget == "bar" || dataset.widget == "gauge");

  // Add minimum value
  auto wgtMin = new QStandardItem();
  wgtMin->setEditable(rangeEnabled);
  wgtMin->setData(0, PlaceholderValue);
  wgtMin->setData(FloatField, WidgetType);
  wgtMin->setData(wgtMin->isEditable(), Active);
  wgtMin->setData(dataset.wgtMin, EditableValue);
  wgtMin->setData(kDatasetView_WgtMin, ParameterType);
  wgtMin->setData(tr("Minimum Display Value (required)"), ParameterName);
  wgtMin->setData(tr("Lower bound of the gauge or bar display range"),
                  ParameterDescription);
  m_datasetModel->appendRow(wgtMin);

  // Add maximum value
  auto wgtMax = new QStandardItem();
  wgtMax->setEditable(rangeEnabled);
  wgtMax->setData(0, PlaceholderValue);
  wgtMax->setData(FloatField, WidgetType);
  wgtMax->setData(wgtMax->isEditable(), Active);
  wgtMax->setData(dataset.wgtMax, EditableValue);
  wgtMax->setData(kDatasetView_WgtMax, ParameterType);
  wgtMax->setData(tr("Maximum Display Value (required)"), ParameterName);
  wgtMax->setData(tr("Upper bound of the gauge or bar display range"),
                  ParameterDescription);
  m_datasetModel->appendRow(wgtMax);

  //----------------------------------------------------------------------------
  // Alarm section
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("Alarm Settings"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/alarm.svg",
                      ParameterIcon);
  m_datasetModel->appendRow(sectionHdr);

  // Add low alarm threshold
  auto alarmEnabled = new QStandardItem();
  alarmEnabled->setEditable(rangeEnabled);
  alarmEnabled->setData(0, PlaceholderValue);
  alarmEnabled->setData(CheckBox, WidgetType);
  alarmEnabled->setData(alarmEnabled->isEditable(), Active);
  alarmEnabled->setData(dataset.alarmEnabled, EditableValue);
  alarmEnabled->setData(kDatasetView_AlarmEnabled, ParameterType);
  alarmEnabled->setData(tr("Enable Alarms"), ParameterName);
  alarmEnabled->setData(
      tr("Triggers a visual alarm when the value exceeds alarm thresholds"),
      ParameterDescription);
  m_datasetModel->appendRow(alarmEnabled);

  // Add low alarm threshold
  auto alarmLow = new QStandardItem();
  alarmLow->setEditable(rangeEnabled && dataset.alarmEnabled);
  alarmLow->setData(0, PlaceholderValue);
  alarmLow->setData(FloatField, WidgetType);
  alarmLow->setData(alarmLow->isEditable(), Active);
  alarmLow->setData(dataset.alarmLow, EditableValue);
  alarmLow->setData(kDatasetView_AlarmLow, ParameterType);
  alarmLow->setData(tr("Low Threshold"), ParameterName);
  alarmLow->setData(
      tr("Triggers a visual alarm when the value drops below this threshold"),
      ParameterDescription);
  m_datasetModel->appendRow(alarmLow);

  // Add high alarm threshold
  auto alarmHigh = new QStandardItem();
  alarmHigh->setEditable(rangeEnabled && dataset.alarmEnabled);
  alarmHigh->setData(0, PlaceholderValue);
  alarmHigh->setData(FloatField, WidgetType);
  alarmHigh->setData(alarmHigh->isEditable(), Active);
  alarmHigh->setData(dataset.alarmHigh, EditableValue);
  alarmHigh->setData(kDatasetView_AlarmHigh, ParameterType);
  alarmHigh->setData(tr("High Threshold"), ParameterName);
  alarmHigh->setData(
      tr("Triggers a visual alarm when the value exceeds this threshold"),
      ParameterDescription);
  m_datasetModel->appendRow(alarmHigh);

  //----------------------------------------------------------------------------
  // LED section
  //----------------------------------------------------------------------------

  // Add section header
  sectionHdr = new QStandardItem();
  sectionHdr->setData(SectionHeader, WidgetType);
  sectionHdr->setData(tr("LED Display Settings"), PlaceholderValue);
  sectionHdr->setData("qrc:/rcc/icons/project-editor/model/led.svg",
                      ParameterIcon);
  m_datasetModel->appendRow(sectionHdr);

  // Add LED panel checkbox
  auto led = new QStandardItem();
  led->setEditable(true);
  led->setData(0, PlaceholderValue);
  led->setData(CheckBox, WidgetType);
  led->setData(led->isEditable(), Active);
  led->setData(dataset.led, EditableValue);
  led->setData(kDatasetView_LED, ParameterType);
  led->setData(tr("Show in LED Panel"), ParameterName);
  led->setData(tr("Enable visual status monitoring using an LED display"),
               ParameterDescription);
  m_datasetModel->appendRow(led);

  // Add LED High value
  auto ledHigh = new QStandardItem();
  ledHigh->setEditable(dataset.led);
  ledHigh->setData(0, PlaceholderValue);
  ledHigh->setData(FloatField, WidgetType);
  ledHigh->setData(ledHigh->isEditable(), Active);
  ledHigh->setData(dataset.ledHigh, EditableValue);
  ledHigh->setData(kDatasetView_LED_High, ParameterType);
  ledHigh->setData(tr("LED On Threshold (required)"), ParameterName);
  ledHigh->setData(
      tr("LED lights up when value meets or exceeds this threshold"),
      ParameterDescription);
  m_datasetModel->appendRow(ledHigh);

  //----------------------------------------------------------------------------
  // Connections & signals
  //----------------------------------------------------------------------------

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
 * These sources are re-generated when the language is changed to ensure
 * proper localization.
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

  // Initialize timer modes
  m_timerModes.clear();
  m_timerModes.append(tr("Off"));
  m_timerModes.append(tr("Auto Start"));
  m_timerModes.append(tr("Start on Trigger"));
  m_timerModes.append(tr("Toggle on Trigger"));

  // Initialize decoder options
  m_decoderOptions.clear();
  m_decoderOptions.append(tr("Plain Text (UTF8)"));
  m_decoderOptions.append(tr("Hexadecimal"));
  m_decoderOptions.append(tr("Base64"));
  m_decoderOptions.append(tr("Binary (Direct)"));

  // Initialize checksum options
  m_checksumMethods.clear();
  m_checksumMethods = IO::availableChecksums();
  const int index = m_checksumMethods.indexOf(QLatin1String(""));
  if (index >= 0)
    m_checksumMethods[index] = tr("No Checksum");

  // Initialize frame detection methods
  m_frameDetectionMethods.clear();
  m_frameDetectionMethodsValues.clear();
  m_frameDetectionMethods.append(tr("End Delimiter Only"));
  m_frameDetectionMethods.append(tr("Start Delimiter Only"));
  m_frameDetectionMethods.append(tr("Start + End Delimiter"));
  m_frameDetectionMethods.append(tr("No Delimiters"));
  m_frameDetectionMethodsValues.append(SerialStudio::EndDelimiterOnly);
  m_frameDetectionMethodsValues.append(SerialStudio::StartDelimiterOnly);
  m_frameDetectionMethodsValues.append(SerialStudio::StartAndEndDelimiter);
  m_frameDetectionMethodsValues.append(SerialStudio::NoDelimiters);

  // Initialize group-level widgets
  m_groupWidgets.clear();
  m_groupWidgets.insert(QStringLiteral("datagrid"), tr("Data Grid"));
  m_groupWidgets.insert(QStringLiteral("map"), tr("GPS Map"));
  m_groupWidgets.insert(QStringLiteral("gyro"), tr("Gyroscope"));
  m_groupWidgets.insert(QStringLiteral("multiplot"), tr("Multiple Plot"));
  m_groupWidgets.insert(QStringLiteral("accelerometer"), tr("Accelerometer"));
  m_groupWidgets.insert(QStringLiteral("plot3d"), tr("3D Plot"));
  m_groupWidgets.insert(QLatin1String(""), tr("None"));

  // Initialize dataset-level widgets
  m_datasetWidgets.clear();
  m_datasetWidgets.insert(QLatin1String(""), tr("None"));
  m_datasetWidgets.insert(QStringLiteral("bar"), tr("Bar"));
  m_datasetWidgets.insert(QStringLiteral("gauge"), tr("Gauge"));
  m_datasetWidgets.insert(QStringLiteral("compass"), tr("Compass"));

  // Initialize EOL options
  m_eolSequences.clear();
  m_eolSequences.insert(QLatin1String(""), tr("None"));
  m_eolSequences.insert(QStringLiteral("\n"), tr("New Line (\\n)"));
  m_eolSequences.insert(QStringLiteral("\r"), tr("Carriage Return (\\r)"));
  m_eolSequences.insert(QStringLiteral("\r\n"), tr("CRLF (\\r\\n)"));

  // Initialize plot options
  m_plotOptions.clear();
  m_plotOptions.insert(qMakePair(false, false), tr("No"));
  m_plotOptions.insert(qMakePair(true, false), tr("Yes"));
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
 * @param modified A boolean indicating whether the project has been modified
 * ( true) or not (false).
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
  auto *parser = JSON::FrameBuilder::instance().frameParser();
  if (parser && m_currentView == FrameParserView
      && m_currentView != currentView)
  {
    if (parser->isModified())
    {
      bool changeView = false;
      const auto ret = Misc::Utilities::showMessageBox(
          tr("Save changes to frame parser code?"),
          tr("Select 'Save' to keep your changes, 'Discard' to lose them "
             "permanently, or 'Cancel' to return."),
          QMessageBox::Question, tr("Save Changes"),
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

      if (ret == QMessageBox::Save)
        changeView = parser->save(true);

      else if (ret == QMessageBox::Discard)
      {
        changeView = true;
        parser->readCode();
      }

      else if (ret == QMessageBox::Cancel)
        changeView = false;

      if (!changeView)
        displayFrameParserView();
    }
  }

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
  const auto groupId = m_selectedGroup.groupId;

  // Change group title
  if (id == kGroupView_Title)
  {
    modified = m_selectedGroup.title != value.toString();
    m_selectedGroup.title = value.toString();
    m_groups[groupId] = m_selectedGroup;
  }

  // Change group widget
  else if (id == kGroupView_Widget)
  {
    // Obtain widget enum from string
    SerialStudio::GroupWidget widget;
    const auto widgetStr = widgets.at(value.toInt());
    if (widgetStr == "accelerometer")
      widget = SerialStudio::Accelerometer;
    else if (widgetStr == "multiplot")
      widget = SerialStudio::MultiPlot;
    else if (widgetStr == "gyro")
      widget = SerialStudio::Gyroscope;
    else if (widgetStr == "map")
      widget = SerialStudio::GPS;
    else if (widgetStr == "datagrid")
      widget = SerialStudio::DataGrid;
    else if (widgetStr == "plot3d")
      widget = SerialStudio::Plot3D;
    else
      widget = SerialStudio::NoGroupWidget;

    // Update group
    modified = setGroupWidget(groupId, widget);
    if (modified)
      m_selectedGroup.widget = widgetStr;

    // User canceled the operation, reload model GUI to restore previous value
    else
    {
      buildTreeModel();
      for (auto g = m_groupItems.begin(); g != m_groupItems.end(); ++g)
      {
        if (g.value().groupId == m_selectedGroup.groupId)
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
 * If the action was modified, it sets the modified flag. Finally, it
 * reselects the action in the user interface.
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
      m_selectedAction.title = value.toString();
      break;
    case kActionView_Data:
      m_selectedAction.txData = value.toString();
      break;
    case kActionView_EOL:
      m_selectedAction.eolSequence = eolSequences.at(value.toInt());
      break;
    case kActionView_Icon:
      m_selectedAction.icon = value.toString();
      Q_EMIT actionModelChanged();
      break;
    case kActionView_Binary:
      m_selectedAction.binaryData = value.toBool();
      buildActionModel(m_selectedAction);
      break;
    case kActionView_AutoExecute:
      m_selectedAction.autoExecuteOnConnect = value.toBool();
      break;
    case kActionView_TimerMode:
      m_selectedAction.timerMode = static_cast<JSON::TimerMode>(value.toInt());
      buildActionModel(m_selectedAction);
      break;
    case kActionView_TimerInterval:
      m_selectedAction.timerIntervalMs = value.toInt();
      break;
    default:
      break;
  }

  // Replace action data
  m_actions[m_selectedAction.actionId] = m_selectedAction;
  buildTreeModel();

  // Mark document as modified
  setModified(true);
}

/**
 * @brief Handles changes made to a project item in the project model.
 *
 * This function processes changes to project items such as the title, frame
 * start/end sequences, decoder method, and Thunderforest API key.
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
    case kProjectView_FrameEndSequence:
      m_frameEndSequence = value.toString();
      break;
    case kProjectView_FrameStartSequence:
      m_frameStartSequence = value.toString();
      break;
    case kProjectView_FrameDecoder:
      m_frameDecoder = static_cast<SerialStudio::DecoderMethod>(value.toInt());
      break;
    case kProjectView_ChecksumFunction:
      m_checksumAlgorithm = IO::availableChecksums()[value.toInt()];
      break;
    case kProjectView_HexadecimalSequence: {
      bool changed = m_hexadecimalDelimiters != value.toBool();
      m_hexadecimalDelimiters = value.toBool();
      if (changed && m_hexadecimalDelimiters)
      {
        m_frameEndSequence = SerialStudio::stringToHex(m_frameEndSequence);
        m_frameStartSequence = SerialStudio::stringToHex(m_frameEndSequence);
      }

      else if (changed)
      {
        m_frameEndSequence = SerialStudio::hexToString(m_frameEndSequence);
        m_frameStartSequence = SerialStudio::hexToString(m_frameEndSequence);
      }

      buildProjectModel();
    }
    break;
    case kProjectView_FrameDetection:
      m_frameDetection = m_frameDetectionMethodsValues.at(value.toInt());
      Q_EMIT frameDetectionChanged();
      buildProjectModel();
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
 * pltMin/pltMax/alarm values. It updates the relevant parameters of the
 * selected dataset, replaces the dataset in its parent group, and rebuilds
 * the tree model. After updating the dataset, it marks the document as
 * modified.
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
      m_selectedDataset.title = value.toString();
      break;
    case kDatasetView_Index:
      m_selectedDataset.index = value.toInt();
      break;
    case kDatasetView_Units:
      m_selectedDataset.units = value.toString();
      break;
    case kDatasetView_Widget:
      m_selectedDataset.widget = widgets.at(value.toInt());
      if (m_selectedDataset.widget == "compass")
      {
        m_selectedDataset.wgtMin = 0;
        m_selectedDataset.wgtMax = 360;
        m_selectedDataset.alarmLow = 0;
        m_selectedDataset.alarmHigh = 0;
      }

      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_FFT:
      m_selectedDataset.fft = value.toBool();
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_LED:
      m_selectedDataset.led = value.toBool();
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_LED_High:
      m_selectedDataset.ledHigh = value.toDouble();
      break;
    case kDatasetView_Overview:
      m_selectedDataset.overviewDisplay = value.toBool();
      break;
    case kDatasetView_Plot:
      m_selectedDataset.plt = plotOptions.at(value.toInt()).first;
      m_selectedDataset.log = plotOptions.at(value.toInt()).second;
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_xAxis:
      m_selectedDataset.xAxisId = value.toInt();
      break;
    case kDatasetView_FFTMin:
      m_selectedDataset.fftMin = value.toDouble();
      break;
    case kDatasetView_FFTMax:
      m_selectedDataset.fftMax = value.toDouble();
      break;
    case kDatasetView_PltMin:
      m_selectedDataset.pltMin = value.toDouble();
      break;
    case kDatasetView_PltMax:
      m_selectedDataset.pltMax = value.toDouble();
      break;
    case kDatasetView_WgtMin:
      m_selectedDataset.wgtMin = value.toDouble();
      break;
    case kDatasetView_WgtMax:
      m_selectedDataset.wgtMax = value.toDouble();
      break;
    case kDatasetView_AlarmLow:
      m_selectedDataset.alarmLow = value.toDouble();
      break;
    case kDatasetView_AlarmHigh:
      m_selectedDataset.alarmHigh = value.toDouble();
      break;
    case kDatasetView_AlarmEnabled:
      m_selectedDataset.alarmEnabled = value.toBool();
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_FFT_Samples:
      m_selectedDataset.fftSamples = m_fftSamples.at(value.toInt()).toInt();
      break;
    case kDatasetView_FFT_SamplingRate:
      m_selectedDataset.fftSamplingRate = value.toInt();
      break;
    default:
      break;
  }

  // Replace dataset in parent group
  const auto groupId = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;
  m_groups[groupId].datasets[datasetId] = m_selectedDataset;
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
 * @brief Handles changes in the current selection within the project
 * structure.
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
  for (size_t i = 0; i < m_groups.size(); ++i)
  {
    const auto &group = m_groups[i];
    for (size_t j = 0; j < group.datasets.size(); ++j)
    {
      const auto &dataset = group.datasets[j];
      if (dataset.index >= maxIndex)
        maxIndex = dataset.index + 1;
    }
  }

  return maxIndex;
}

//------------------------------------------------------------------------------
// Project writting
//------------------------------------------------------------------------------

/**
 * @brief Finalizes the process of saving a Serial Studio project to disk.
 *
 * This method writes the current project configuration into a JSON file,
 * including metadata, group definitions, and action definitions. It performs
 * the following steps:
 *
 * - Opens the project file for writing
 * - Serializes internal data into a JSON structure
 * - Writes the JSON data to the selected file
 * - Enables project mode in the application
 * - Loads the saved file back into the runtime environment
 *
 * If the file cannot be opened, an error message is displayed and the
 * operation is aborted.
 *
 * @return true if the file was saved successfully, false otherwise
 */
bool JSON::ProjectModel::finalizeProjectSave()
{
  // Open file for writing
  QFile file(m_filePath);
  if (!file.open(QFile::WriteOnly))
  {
    Misc::Utilities::showMessageBox(tr("File open error"), file.errorString(),
                                    QMessageBox::Critical);
    return false;
  }

  // Create JSON document & add properties
  QJsonObject json;
  json.insert("title", m_title);
  json.insert("decoder", m_frameDecoder);
  json.insert("frameEnd", m_frameEndSequence);
  json.insert("frameParser", m_frameParserCode);
  json.insert("checksum", m_checksumAlgorithm);
  json.insert("frameDetection", m_frameDetection);
  json.insert("frameStart", m_frameStartSequence);
  json.insert("hexadecimalDelimiters", m_hexadecimalDelimiters);

  // Create group array
  QJsonArray groupArray;
  for (const auto &group : std::as_const(m_groups))
    groupArray.append(JSON::serialize(group));

  // Add groups array to JSON
  json.insert("groups", groupArray);

  // Create actions array
  QJsonArray actionsArray;
  for (const auto &action : std::as_const(m_actions))
    actionsArray.append(JSON::serialize(action));

  // Insert actions array to JSON
  json.insert("actions", actionsArray);

  // Write JSON data to file
  file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
  file.close();

  // Ask user to switch to project mode
  enableProjectMode();

  // Load JSON file to Serial Studio
  openJsonFile(file.fileName());
  JSON::FrameBuilder::instance().loadJsonMap(file.fileName());
  return true;
}

//------------------------------------------------------------------------------
// Save & restore expanded items of project structure upon modification
//------------------------------------------------------------------------------

/**
 * @brief Recursively saves the expanded state of tree view items.
 *
 * This function stores the expanded state of the given `QStandardItem` and
 * its child items in a `QHash`, with the item's text serving as the key.
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
 * @param title The current item's title, used to look up its state in the
 * hash.
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
