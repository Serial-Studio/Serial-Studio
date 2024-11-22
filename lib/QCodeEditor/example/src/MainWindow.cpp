// Demo
#include <MainWindow.hpp>

// QCodeEditor
#include <QCodeEditor>
#include <QGLSLCompleter>
#include <QLuaCompleter>
#include <QPythonCompleter>
#include <QSyntaxStyle>
#include <QCXXHighlighter>
#include <QGLSLHighlighter>
#include <QXMLHighlighter>
#include <QJSONHighlighter>
#include <QLuaHighlighter>
#include <QPythonHighlighter>

// Qt
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_setupLayout(nullptr)
  , m_codeSampleCombobox(nullptr)
  , m_highlighterCombobox(nullptr)
  , m_completerCombobox(nullptr)
  , m_styleCombobox(nullptr)
  , m_readOnlyCheckBox(nullptr)
  , m_wordWrapCheckBox(nullptr)
  , m_parenthesesEnabledCheckbox(nullptr)
  , m_tabReplaceEnabledCheckbox(nullptr)
  , m_tabReplaceNumberSpinbox(nullptr)
  , m_autoIndentationCheckbox(nullptr)
  , m_codeEditor(nullptr)
  , m_completers()
  , m_highlighters()
  , m_styles()
{
  initData();
  createWidgets();
  setupWidgets();
  performConnections();
}

void MainWindow::initData()
{
  m_codeSamples = {{"C++", loadCode(":/code_samples/cxx.cpp")},
                   {"GLSL", loadCode(":/code_samples/shader.glsl")},
                   {"XML", loadCode(":/code_samples/xml.xml")},
                   {"JSON", loadCode(":/code_samples/json.json")},
                   {"LUA", loadCode(":/code_samples/lua.lua")},
                   {"Python", loadCode(":/code_samples/python.py")}};

  m_completers = {
      {"None", nullptr},
      {"GLSL", new QGLSLCompleter(this)},
      {"LUA", new QLuaCompleter(this)},
      {"Python", new QPythonCompleter(this)},
  };

  m_highlighters = {
      {"None", nullptr},
      {"C++", new QCXXHighlighter},
      {"GLSL", new QGLSLHighlighter},
      {"XML", new QXMLHighlighter},
      {"JSON", new QJSONHighlighter},
      {"LUA", new QLuaHighlighter},
      {"Python", new QPythonHighlighter},
  };

  m_styles = {{"Default", QSyntaxStyle::defaultStyle()}};

  // Loading styles
  loadStyle(":/styles/drakula.xml");
}

QString MainWindow::loadCode(QString path)
{
  QFile fl(path);

  if (!fl.open(QIODevice::ReadOnly))
  {
    return QString();
  }

  return fl.readAll();
}

void MainWindow::loadStyle(QString path)
{
  QFile fl(path);

  if (!fl.open(QIODevice::ReadOnly))
  {
    return;
  }

  auto style = new QSyntaxStyle(this);

  if (!style->load(fl.readAll()))
  {
    delete style;
    return;
  }

  m_styles.append({style->name(), style});
}

void MainWindow::createWidgets()
{
  // Layout
  auto container = new QWidget(this);
  setCentralWidget(container);

  auto hBox = new QHBoxLayout(container);

  auto setupGroup = new QGroupBox("Setup", container);
  hBox->addWidget(setupGroup);

  m_setupLayout = new QVBoxLayout(setupGroup);
  setupGroup->setLayout(m_setupLayout);
  setupGroup->setMaximumWidth(300);

  // CodeEditor
  m_codeEditor = new QCodeEditor(this);
  hBox->addWidget(m_codeEditor);

  m_codeSampleCombobox = new QComboBox(setupGroup);
  m_highlighterCombobox = new QComboBox(setupGroup);
  m_completerCombobox = new QComboBox(setupGroup);
  m_styleCombobox = new QComboBox(setupGroup);

  m_readOnlyCheckBox = new QCheckBox("Read Only", setupGroup);
  m_wordWrapCheckBox = new QCheckBox("Word Wrap", setupGroup);
  m_parenthesesEnabledCheckbox = new QCheckBox("Auto Parentheses", setupGroup);
  m_tabReplaceEnabledCheckbox = new QCheckBox("Tab Replace", setupGroup);
  m_tabReplaceNumberSpinbox = new QSpinBox(setupGroup);
  m_autoIndentationCheckbox = new QCheckBox("Auto Indentation", setupGroup);

  // Adding widgets
  m_setupLayout->addWidget(new QLabel(tr("Code sample"), setupGroup));
  m_setupLayout->addWidget(m_codeSampleCombobox);
  m_setupLayout->addWidget(new QLabel(tr("Completer"), setupGroup));
  m_setupLayout->addWidget(m_completerCombobox);
  m_setupLayout->addWidget(new QLabel(tr("Highlighter"), setupGroup));
  m_setupLayout->addWidget(m_highlighterCombobox);
  m_setupLayout->addWidget(new QLabel(tr("Style"), setupGroup));
  m_setupLayout->addWidget(m_styleCombobox);
  m_setupLayout->addWidget(m_readOnlyCheckBox);
  m_setupLayout->addWidget(m_wordWrapCheckBox);
  m_setupLayout->addWidget(m_parenthesesEnabledCheckbox);
  m_setupLayout->addWidget(m_tabReplaceEnabledCheckbox);
  m_setupLayout->addWidget(m_tabReplaceNumberSpinbox);
  m_setupLayout->addWidget(m_autoIndentationCheckbox);
  m_setupLayout->addSpacerItem(
      new QSpacerItem(1, 2, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void MainWindow::setupWidgets()
{
  setWindowTitle("QCodeEditor Demo");

  // CodeEditor
  m_codeEditor->setPlainText(m_codeSamples[0].second);
  m_codeEditor->setSyntaxStyle(m_styles[0].second);
  m_codeEditor->setCompleter(m_completers[0].second);
  m_codeEditor->setHighlighter(m_highlighters[0].second);

  QStringList list;
  // Code samples
  for (auto &&el : m_codeSamples)
  {
    list << el.first;
  }

  m_codeSampleCombobox->addItems(list);
  list.clear();

  // Highlighter
  for (auto &&el : m_highlighters)
  {
    list << el.first;
  }

  m_highlighterCombobox->addItems(list);
  list.clear();

  // Completer
  for (auto &&el : m_completers)
  {
    list << el.first;
  }

  m_completerCombobox->addItems(list);
  list.clear();

  // Styles
  for (auto &&el : m_styles)
  {
    list << el.first;
  }

  m_styleCombobox->addItems(list);
  list.clear();

  m_parenthesesEnabledCheckbox->setChecked(m_codeEditor->autoParentheses());
  m_tabReplaceEnabledCheckbox->setChecked(m_codeEditor->tabReplace());
  m_tabReplaceNumberSpinbox->setValue(m_codeEditor->tabReplaceSize());
  m_tabReplaceNumberSpinbox->setSuffix(tr(" spaces"));
  m_autoIndentationCheckbox->setChecked(m_codeEditor->autoIndentation());

  m_wordWrapCheckBox->setChecked(m_codeEditor->wordWrapMode()
                                 != QTextOption::NoWrap);
}

void MainWindow::performConnections()
{
  connect(m_codeSampleCombobox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) {
            m_codeEditor->setPlainText(m_codeSamples[index].second);
          });

  connect(m_highlighterCombobox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) {
            m_codeEditor->setHighlighter(m_highlighters[index].second);
          });

  connect(m_completerCombobox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) {
            m_codeEditor->setCompleter(m_completers[index].second);
          });

  connect(m_styleCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) {
            m_codeEditor->setSyntaxStyle(m_styles[index].second);
          });

  connect(m_readOnlyCheckBox, &QCheckBox::stateChanged,
          [this](int state) { m_codeEditor->setReadOnly(state != 0); });

  connect(m_wordWrapCheckBox, &QCheckBox::stateChanged, [this](int state) {
    if (state != 0)
    {
      m_codeEditor->setWordWrapMode(QTextOption::WordWrap);
    }
    else
    {
      m_codeEditor->setWordWrapMode(QTextOption::NoWrap);
    }
  });

  connect(m_parenthesesEnabledCheckbox, &QCheckBox::stateChanged,
          [this](int state) { m_codeEditor->setAutoParentheses(state != 0); });

  connect(m_tabReplaceEnabledCheckbox, &QCheckBox::stateChanged,
          [this](int state) { m_codeEditor->setTabReplace(state != 0); });

  connect(m_tabReplaceNumberSpinbox,
          QOverload<int>::of(&QSpinBox::valueChanged),
          [this](int value) { m_codeEditor->setTabReplaceSize(value); });

  connect(m_autoIndentationCheckbox, &QCheckBox::stateChanged,
          [this](int state) { m_codeEditor->setAutoIndentation(state != 0); });
}
