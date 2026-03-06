#ifndef QUAZIP_QT_COMPAT_H
#define QUAZIP_QT_COMPAT_H

/*
 * For some reason, Qt 5.14 and 5.15 introduced a whole mess of seemingly random
 * moves and deprecations. To avoid populating code with #ifs,
 * we handle this stuff here, as well as some other compatibility issues.
 *
 * Some includes are repeated just in case we want to split this file later.
 */

#include <QtCore/Qt>
#include <QtCore/QtGlobal>

// Legacy encodings are still everywhere, but the Qt team decided we
// don't need them anymore and moved them out of Core in Qt 6.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QtCore5Compat/QTextCodec>
#else
#  include <QtCore/QTextCodec>
#endif

// QSaveFile terribly breaks the is-a idiom (Liskov substitution principle):
// QSaveFile is-a QIODevice, but it makes close() private and aborts
// if you call it through the base class. Hence this ugly hack:
#if (QT_VERSION >= 0x050100)
#include <QtCore/QSaveFile>
inline bool quazip_close(QIODevice *device) {
    QSaveFile *file = qobject_cast<QSaveFile*>(device);
    if (file != nullptr) {
        // We have to call the ugly commit() instead:
        return file->commit();
    }
    device->close();
    return true;
}
#else
inline bool quazip_close(QIODevice *device) {
    device->close();
    return true;
}
#endif

// this is yet another stupid move and deprecation
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
using Qt::SkipEmptyParts;
#else
#include <QtCore/QString>
const auto SkipEmptyParts = QString::SplitBehavior::SkipEmptyParts;
#endif

// and yet another... (why didn't they just make qSort delegate to std::sort?)
#include <QtCore/QList>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
#include <algorithm>
template<typename T, typename C>
inline void quazip_sort(T begin, T end, C comparator) {
    std::sort(begin, end, comparator);
}
#else
#include <QtCore/QtAlgorithms>
template<typename T, typename C>
inline void quazip_sort(T begin, T end, C comparator) {
    qSort(begin, end, comparator);
}
#endif

// this is a stupid rename...
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
inline QDateTime quazip_ctime(const QFileInfo &fi) {
    return fi.birthTime();
}
#else
inline QDateTime quazip_ctime(const QFileInfo &fi) {
    return fi.created();
}
#endif

// this is just a slightly better alternative
#include <QtCore/QFileInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
inline bool quazip_is_symlink(const QFileInfo &fi) {
    return fi.isSymbolicLink();
}
#else
inline bool quazip_is_symlink(const QFileInfo &fi) {
    // also detects *.lnk on Windows, but better than nothing
    return fi.isSymLink();
}
#endif

// I'm not even sure what this one is, but nevertheless
#include <QtCore/QFileInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
inline QString quazip_symlink_target(const QFileInfo &fi) {
    return fi.symLinkTarget();
}
#else
inline QString quazip_symlink_target(const QFileInfo &fi) {
    return fi.readLink(); // What's the difference? I've no idea.
}
#endif

// deprecation
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QtCore/QTimeZone>
inline QDateTime quazip_since_epoch() {
    return QDateTime(QDate(1970, 1, 1), QTime(0, 0), QTimeZone::utc());
}

inline QDateTime quazip_since_epoch_ntfs() {
    return QDateTime(QDate(1601, 1, 1), QTime(0, 0), QTimeZone::utc());
}
#else
inline QDateTime quazip_since_epoch() {
    return QDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC);
}

inline QDateTime quazip_since_epoch_ntfs() {
    return QDateTime(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
}
#endif

// this is not a deprecation but an improvement, for a change
#include <QtCore/QDateTime>
#if (QT_VERSION >= 0x040700)
inline quint64 quazip_ntfs_ticks(const QDateTime &time, int fineTicks) {
    QDateTime base = quazip_since_epoch_ntfs();
    return base.msecsTo(time) * 10000 + fineTicks;
}
#else
inline quint64 quazip_ntfs_ticks(const QDateTime &time, int fineTicks) {
    QDateTime base = quazip_since_epoch_ntfs();
    QDateTime utc = time.toUTC();
    return (static_cast<qint64>(base.date().daysTo(utc.date()))
            * Q_INT64_C(86400000)
            + static_cast<qint64>(base.time().msecsTo(utc.time())))
        * Q_INT64_C(10000) + fineTicks;
}
#endif

// yet another improvement...
#include <QtCore/QDateTime>
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0) // Yay! Finally a way to get time as qint64!
inline qint64 quazip_to_time64_t(const QDateTime &time) {
    return time.toSecsSinceEpoch();
}
#else
inline qint64 quazip_to_time64_t(const QDateTime &time) {
    return static_cast<qint64>(time.toTime_t()); // 32 bits only, but better than nothing
}
#endif

#include <QtCore/QTextStream>
// and another stupid move
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
const auto quazip_endl = Qt::endl;
#else
const auto quazip_endl = endl;
#endif

#endif // QUAZIP_QT_COMPAT_H
