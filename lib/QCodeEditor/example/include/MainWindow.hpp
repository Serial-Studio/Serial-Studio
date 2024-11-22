#pragma once

// Qt
#include <QMainWindow> // Required for inheritance
#include <QString>
#include <QVector>
#include <QPair>

class QVBoxLayout;
class QSyntaxStyle;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QCompleter;
class QStyleSyntaxHighlighter;
class QCodeEditor;

/**
 * @brief Class, that describes demo main window.
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent Pointer to parent widget.
   */
  explicit MainWindow(QWidget *parent = nullptr);

private:
  void loadStyle(QString path);

  QString loadCode(QString path);

  void initData();

  void createWidgets();

  void setupWidgets();

  void performConnections();

  QVBoxLayout *m_setupLayout;

  QComboBox *m_codeSampleCombobox;
  QComboBox *m_highlighterCombobox;
  QComboBox *m_completerCombobox;
  QComboBox *m_styleCombobox;

  QCheckBox *m_readOnlyCheckBox;
  QCheckBox *m_wordWrapCheckBox;
  QCheckBox *m_parenthesesEnabledCheckbox;
  QCheckBox *m_tabReplaceEnabledCheckbox;
  QSpinBox *m_tabReplaceNumberSpinbox;
  QCheckBox *m_autoIndentationCheckbox;

  QCodeEditor *m_codeEditor;

  QVector<QPair<QString, QString>> m_codeSamples;
  QVector<QPair<QString, QCompleter *>> m_completers;
  QVector<QPair<QString, QStyleSyntaxHighlighter *>> m_highlighters;
  QVector<QPair<QString, QSyntaxStyle *>> m_styles;
};
