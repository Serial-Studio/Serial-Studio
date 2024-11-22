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

#include "QsLogWindowModel.h"
#include "QsLog.h"

#include <QColor>

QsLogging::LogWindowModel::LogWindowModel(size_t max_items) :
    mMaxItems(max_items)
{
}

QsLogging::LogWindowModel::~LogWindowModel() noexcept = default;

void QsLogging::LogWindowModel::addEntry(const LogMessage& message)
{
    const int next_idx = static_cast<int>(mLogMessages.size());
    beginInsertRows(QModelIndex(), next_idx, next_idx);
    {
        QWriteLocker lock(&mMessagesLock);
        mLogMessages.push_back(message);
    }
    endInsertRows();

    if (mMaxItems < std::numeric_limits<size_t>::max() && mLogMessages.size() > mMaxItems) {
        {
            QWriteLocker lock(&mMessagesLock);
            mLogMessages.pop_front();
        }
        // Every item changed
        const QModelIndex idx1 = index(0, 0);
        const QModelIndex idx2 = index(static_cast<int>(mLogMessages.size()), rowCount());
        emit dataChanged(idx1, idx2);
    }
}

void QsLogging::LogWindowModel::clear()
{
    beginResetModel();
    {
        QWriteLocker lock(&mMessagesLock);
        mLogMessages.clear();
    }
    endResetModel();
}

QsLogging::LogMessage QsLogging::LogWindowModel::at(size_t index) const
{
    return mLogMessages[index];
}

int QsLogging::LogWindowModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

int QsLogging::LogWindowModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    QReadLocker lock(&mMessagesLock);

    return static_cast<int>(mLogMessages.size());
}

QVariant QsLogging::LogWindowModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        QReadLocker lock(&mMessagesLock);

        const LogMessage& item = mLogMessages.at(index.row());

        switch (index.column()) {
        case TimeColumn:
            return item.time.toLocalTime().toString();
        case LevelNameColumn:
            return LocalizedLevelName(item.level);
        case MessageColumn:
            return item.message;
        case FormattedMessageColumn:
            return item.formatted;
        default:
            return QVariant();
        }

        return QString();
    }

    if (role == Qt::BackgroundColorRole) {
        QReadLocker lock(&mMessagesLock);

        const LogMessage& item = mLogMessages.at(index.row());

        switch (item.level)
        {
        case QsLogging::WarnLevel:
            return QVariant(QColor(255, 255, 128));
        case QsLogging::ErrorLevel:
            return QVariant(QColor(255, 128, 128));
        case QsLogging::FatalLevel:
            return QVariant(QColor(255, 0, 0));
        default:
            break;
        }
    }

    return QVariant();
}

QVariant QsLogging::LogWindowModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case TimeColumn:
                return tr("Time");
            case LevelNameColumn:
                return tr("Level");
            case MessageColumn:
                return tr("Message");
            default:
                break;
        }
    }

    return QVariant();
}
