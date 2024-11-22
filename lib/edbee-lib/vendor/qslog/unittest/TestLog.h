#ifndef TESTLOG_H
#define TESTLOG_H

#include "QsLogDest.h"
#include "QsLogDestFile.h"
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QDir>

// A destination that tracks log messages
class MockDestination : public QsLogging::Destination
{
public:
    struct Message
    {
        Message() : level(QsLogging::TraceLevel) {}
        QString text;
        QsLogging::Level level;
    };

public:
    explicit MockDestination(const QLatin1String& myType = QLatin1String("mock"))
        : mMyType(myType)
    {
    }

    void write(const QsLogging::LogMessage& message) override
    {
        Message m;
        m.text = message.formatted;
        m.level = message.level;
        mMessages.push_back(m);
        ++mCountByLevel[message.level];
    }

    bool isValid() override
    {
        return true;
    }

    QString type() const override
    {
        return mMyType;
    }

    void clear()
    {
        mMessages.clear();
        mCountByLevel.clear();
    }

    int messageCount() const
    {
        return mMessages.count();
    }

    int messageCountForLevel(QsLogging::Level level) const
    {
        return mCountByLevel.value(level);
    }

    bool hasMessage(const QString &messageContent, QsLogging::Level level) const
    {
        Q_FOREACH (const Message &m, mMessages) {
            if (m.level == level && m.text.contains(messageContent))
                return true;
        }

        return false;
    }

    const Message& messageAt(int index)
    {
        Q_ASSERT(index >= 0 && index < messageCount());
        return mMessages.at(index);
    }

private:
    QHash<QsLogging::Level,int> mCountByLevel;
    QList<Message> mMessages;
    QLatin1String mMyType;
};

// A rotation strategy that simulates file rotations.
// Stores a "file system" inside a string list and can perform some operations on it.
class MockSizeRotationStrategy : public QsLogging::SizeRotationStrategy
{
public:
    MockSizeRotationStrategy()
        : fakePath("FAKEPATH")
        , logName("QsLog.txt")
    {
        setInitialInfo(QDir(fakePath).filePath(logName), 0);
        files.append(logName);
    }

    const QStringList& filesList() {
        return files;
    }

    void rotate() override {
        SizeRotationStrategy::rotate();
        files.append(logName);
    }

protected:
    bool removeFileAtPath(const QString& path) override {
        QString editedPath = path;
        if (editedPath.startsWith(fakePath)) {
            editedPath.remove(0, fakePath.size() + 1);
            const int indexOfPath = files.indexOf(editedPath);
            if (indexOfPath != -1) {
                files.removeAt(indexOfPath);
                return true;
            }
        }

        return false;
    }

    bool fileExistsAtPath(const QString& path) override {
        QString editedPath = path;
        if (editedPath.startsWith(fakePath)) {
            editedPath.remove(0, fakePath.size() + 1);
            return files.indexOf(editedPath) != -1;
        }
        return false;
    }

    bool renameFileFromTo(const QString& from, const QString& to) override {
        QString editedFromPath = from;
        QString editedToPath = to;

        if (editedFromPath.startsWith(fakePath) && editedToPath.startsWith(fakePath)) {
            editedFromPath.remove(0, fakePath.size() + 1);
            editedToPath.remove(0, fakePath.size() + 1);
            const int ind = files.indexOf(editedFromPath);
            if (ind != -1) {
                files.removeAll(editedFromPath);
                files.append(editedToPath);
                return true;
            }
        }

        return false;
    }

private:
    QStringList files;
    const QString fakePath;
    const QString logName;
};


#endif // TESTLOG_H

