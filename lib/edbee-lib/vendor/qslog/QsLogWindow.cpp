// Copyright (c) 2015, Axel Gembe <axel@gembe.net>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "QsLogWindow.h"
#include "QsLogWindowModel.h"
#include "QsLog.h"
#include "QsLogMessage.h"
#include "ui_QsLogWindow.h"

#include <QIcon>
#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QKeyEvent>
#include <QFileDialog>
#include <QtGlobal>
#include <cstddef>

static const QIcon& pauseIcon()
{
    static QIcon icon(QString::fromLatin1(":/QsLogWindow/images/icon-pause-16.png"));
    return icon;
}

static const QIcon& playIcon()
{
    static QIcon icon(QString::fromLatin1(":/QsLogWindow/images/icon-resume-16.png"));
    return icon;
}


class QsLogging::WindowLogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    WindowLogFilterProxyModel(Level level, QObject* parent = 0)
        : QSortFilterProxyModel(parent)
        , mLevel(level)
        , mLastVisibleRow(0)
    {
    }

    Level level() const
    {
        return mLevel;
    }

    void setLevel(const Level level)
    {
        mLevel = level;
        invalidateFilter();
    }

    void setPaused(bool paused)
    {
        mLastVisibleRow = paused ? rowCount() : 0;
        if (!paused) {
            invalidateFilter();
        }
    }

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        Q_UNUSED(source_parent);
        if (!mLastVisibleRow) {
            LogWindowModel* model = dynamic_cast<LogWindowModel*>(sourceModel());
            const LogMessage& d = model->at(source_row);
            return d.level >= mLevel;
        }

        return source_row <= mLastVisibleRow;
    }

private:
    Level mLevel;
    int mLastVisibleRow;
};

QsLogging::Window::Window(QWidget* parent)
    : QDialog(parent)
    , mUi(nullptr)
    , mProxyModel(nullptr)
    , mIsPaused(false)
    , mHasAutoScroll(true)
{
    mUi.reset(new Ui::LogWindow());
    mUi->setupUi(this);

    connect(mUi->toolButtonPause, SIGNAL(clicked()), SLOT(OnPauseClicked()));
    connect(mUi->toolButtonSave, SIGNAL(clicked()), SLOT(OnSaveClicked()));
    connect(mUi->toolButtonClear, SIGNAL(clicked()), SLOT(OnClearClicked()));
    connect(mUi->toolButtonCopy, SIGNAL(clicked()), SLOT(OnCopyClicked()));
    connect(mUi->comboBoxLevel, SIGNAL(currentIndexChanged(int)), SLOT(OnLevelChanged(int)));
    connect(mUi->checkBoxAutoScroll, SIGNAL(toggled(bool)), SLOT(OnAutoScrollChanged(bool)));
    connect(&mModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            SLOT(ModelRowsInserted(const QModelIndex&, int, int)));

    // Install the sort / filter model
    mProxyModel.reset(new WindowLogFilterProxyModel(InfoLevel, this));
    mProxyModel->setSourceModel(&mModel);
    mUi->tableViewMessages->setModel(mProxyModel.get());

    mUi->tableViewMessages->installEventFilter(this);

    mUi->tableViewMessages->setSelectionBehavior(QAbstractItemView::SelectRows);
#if QT_VERSION >= 0x050000
    mUi->tableViewMessages->horizontalHeader()->setSectionResizeMode(LogWindowModel::TimeColumn, QHeaderView::ResizeToContents);
    mUi->tableViewMessages->horizontalHeader()->setSectionResizeMode(LogWindowModel::LevelNameColumn, QHeaderView::ResizeToContents);
    mUi->tableViewMessages->horizontalHeader()->setSectionResizeMode(LogWindowModel::MessageColumn, QHeaderView::Stretch);
    mUi->tableViewMessages->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    mUi->tableViewMessages->horizontalHeader()->setResizeMode(LogWindowModel::TimeColumn, QHeaderView::ResizeToContents);
    mUi->tableViewMessages->horizontalHeader()->setResizeMode(LogWindowModel::LevelNameColumn, QHeaderView::ResizeToContents);
    mUi->tableViewMessages->horizontalHeader()->setResizeMode(LogWindowModel::MessageColumn, QHeaderView::Stretch);
    mUi->tableViewMessages->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif

    // Initialize log level selection
    for (int l = TraceLevel; l < OffLevel; l++) {
        const QString ln = LocalizedLevelName(static_cast<Level>(l));
        mUi->comboBoxLevel->addItem(ln, l);
    }
    mUi->comboBoxLevel->setCurrentIndex(InfoLevel);
}

QsLogging::Window::~Window() noexcept = default;

bool QsLogging::Window::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == mUi->tableViewMessages) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_C && (keyEvent->modifiers() & Qt::ControlModifier)) {
                copySelection();
                return true;
            }
        }

        return false;
    } else {
        return QDialog::eventFilter(obj, event);
    }
}

void QsLogging::Window::addLogMessage(const QsLogging::LogMessage &m)
{
    mModel.addEntry(m);
}

void QsLogging::Window::OnPauseClicked()
{
    mUi->toolButtonPause->setIcon(mIsPaused ? pauseIcon() : playIcon());
    mUi->toolButtonPause->setText(mIsPaused ? tr("&Pause") : tr("&Resume"));

    mIsPaused = !mIsPaused;

    mProxyModel->setPaused(mIsPaused);
}

void QsLogging::Window::OnSaveClicked()
{
    saveSelection();
}

void QsLogging::Window::OnClearClicked()
{
    mModel.clear();
}

void QsLogging::Window::OnCopyClicked()
{
    copySelection();
}

void QsLogging::Window::OnLevelChanged(int value)
{
    mProxyModel->setLevel(static_cast<Level>(value));
}

void QsLogging::Window::OnAutoScrollChanged(bool checked)
{
    mHasAutoScroll = checked;
}

void QsLogging::Window::ModelRowsInserted(const QModelIndex& parent, int start, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(last);
    if (mHasAutoScroll && !mIsPaused) {
        mUi->tableViewMessages->scrollToBottom();
    }
}

void QsLogging::Window::copySelection() const
{
    const QString text = getSelectionText();
    if (text.isEmpty()) {
        return;
    }

    QApplication::clipboard()->setText(text);
}

void QsLogging::Window::saveSelection()
{
    const QString text = getSelectionText();
    if (text.isEmpty()) {
        return;
    }

    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Save log"));
    dialog.setNameFilter(tr("Log file (*.log)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("log");
    dialog.exec();

    const QStringList sel = dialog.selectedFiles();
    if (sel.size() < 1) {
        return;
    }

    QFile file(sel.at(0));
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << text;
        file.close();
    }
}

QString QsLogging::Window::getSelectionText() const
{
    QModelIndexList rows = mUi->tableViewMessages->selectionModel()->selectedRows();
    std::sort(rows.begin(), rows.end());

    QString text;

    if (rows.count() == 0) {
        for (int i = 0; i < mProxyModel->rowCount(); i++) {
            const int srow = mProxyModel->mapToSource(mProxyModel->index(i, 0)).row();
            text += mModel.at(srow).formatted + "\n";
        }
    } else {
        for (QModelIndexList::const_iterator i = rows.begin();i != rows.end();++i) {
            const int srow = mProxyModel->mapToSource(*i).row();
            text += mModel.at(srow).formatted + "\n";
        }
    }

    return text;
}

#include "QsLogWindow.moc"
