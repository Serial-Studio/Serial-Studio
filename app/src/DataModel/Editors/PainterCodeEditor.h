/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "DataModel/Editors/EmbeddedCodeEditorItem.h"
#  include "DataModel/Editors/ScriptTemplateCatalog.h"

namespace Misc {
class Translator;
}  // namespace Misc

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief QML-embeddable code editor for painter-widget paint() / onFrame() scripts.
 */
class PainterCodeEditor : public EmbeddedCodeEditorItem {
  Q_OBJECT
  Q_PROPERTY(bool isModified READ isModified NOTIFY modifiedChanged)
  Q_PROPERTY(bool undoAvailable READ undoAvailable NOTIFY modifiedChanged)
  Q_PROPERTY(bool redoAvailable READ redoAvailable NOTIFY modifiedChanged)
  Q_PROPERTY(QString text READ text NOTIFY textChanged)

signals:
  void textChanged();
  void modifiedChanged();

public:
  explicit PainterCodeEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] QString text() const;

public slots:
  void cut();
  void undo();
  void redo();
  void copy();
  void paste();
  void commit();
  void importFile();
  void selectAll();
  void readCode();
  void formatDocument();
  void formatSelection();
  void selectTemplate();
  void reload(bool guiTrigger = false);

public:
  [[nodiscard]] static QString defaultTemplate();

private:
  void loadTemplates();

private:
  bool m_readingCode;
  Misc::Translator& m_translator;
  DataModel::ProjectEditor& m_projectEditor;
  DataModel::ProjectModel& m_projectModel;
  ScriptTemplateCatalog m_templates;
};

}  // namespace DataModel

#endif  // BUILD_COMMERCIAL
