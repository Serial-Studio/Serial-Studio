/*
Copyright (C) 2005-2014 Sergey A. Tachenov

This file is part of QuaZip.

QuaZip is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

QuaZip is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QuaZip.  If not, see <http://www.gnu.org/licenses/>.

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant and contributors,
see quazip/(un)zip.h files for details. Basically it's the zlib license.
*/

#include "quazipdir.h"
#include "quazip_qt_compat.h"

#include <QtCore/QSet>
#include <QtCore/QSharedData>

/// \cond internal
class QuaZipDirPrivate: public QSharedData {
    friend class QuaZipDir;
private:
    QuaZipDirPrivate(QuaZip *zip, const QString &dir = QString()):
        zip(zip), dir(dir) {}
    QuaZip *zip;
    QString dir;
    QuaZip::CaseSensitivity caseSensitivity{QuaZip::csDefault};
    QDir::Filters filter{QDir::NoFilter};
    QStringList nameFilters;
    QDir::SortFlags sorting{QDir::NoSort};
    template<typename TFileInfoList>
    bool entryInfoList(QStringList nameFilters, QDir::Filters filter,
        QDir::SortFlags sort, TFileInfoList &result) const;
    inline QString simplePath() const {return QDir::cleanPath(dir);}
};
/// \endcond

QuaZipDir::QuaZipDir(QuaZip *zip, const QString &dir):
    d(new QuaZipDirPrivate(zip, dir))
{
    if (d->dir.startsWith(QLatin1String("/")))
        d->dir = d->dir.mid(1);
}

QuaZipDir::QuaZipDir(const QuaZipDir &that) = default;
QuaZipDir::~QuaZipDir() = default;
QuaZipDir& QuaZipDir::operator=(const QuaZipDir &that) = default;

bool QuaZipDir::operator==(const QuaZipDir &that)
{
    return d->zip == that.d->zip && d->dir == that.d->dir;
}

QString QuaZipDir::operator[](int pos) const
{
    return entryList().at(pos);
}

QuaZip::CaseSensitivity QuaZipDir::caseSensitivity() const
{
    return d->caseSensitivity;
}

bool QuaZipDir::cd(const QString &directoryName)
{
    if (directoryName == QLatin1String("/")) {
        d->dir = QLatin1String("");
        return true;
    }
    QString dirName = directoryName;
    if (dirName.endsWith(QLatin1String("/"))) {
        dirName.chop(1);
    }
    if (dirName.contains(QLatin1String("/"))) {
        QuaZipDir dir(*this);
        if (dirName.startsWith(QLatin1String("/"))) {
#ifdef QUAZIP_QUAZIPDIR_DEBUG
            qDebug("QuaZipDir::cd(%s): going to /",
                    dirName.toUtf8().constData());
#endif
            if (!dir.cd(QLatin1String("/")))
                return false;
        }
        QStringList path = dirName.split(QLatin1String("/"), SkipEmptyParts);
        for (const auto& step : path) {
#ifdef QUAZIP_QUAZIPDIR_DEBUG
            qDebug("QuaZipDir::cd(%s): going to %s",
                    dirName.toUtf8().constData(),
                    step.toUtf8().constData());
#endif
            if (!dir.cd(step))
                return false;
        }
        d->dir = dir.path();
        return true;
    }
    if (dirName == QLatin1String(".")) {
        return true;
    }
    if (dirName == QLatin1String("..")) {
        if (isRoot())
            return false;
        int slashPos = d->dir.lastIndexOf(QLatin1String("/"));
        if (slashPos == -1)
            d->dir = QLatin1String("");
        else
            d->dir = d->dir.left(slashPos);
        return true;
    }
    if (exists(dirName)) {
        if (isRoot())
            d->dir = dirName;
        else
            d->dir += QLatin1String("/") + dirName;
        return true;
    }
    return false;
}

bool QuaZipDir::cdUp()
{
    return cd(QLatin1String(".."));
}

uint QuaZipDir::count() const
{
    return entryList().count();
}

QString QuaZipDir::dirName() const
{
    return QDir(d->dir).dirName();
}

QuaZipFileInfo64 QuaZipDir_getFileInfo(QuaZip *zip, bool *ok,
                                  const QString &relativeName,
                                  bool isReal)
{
    QuaZipFileInfo64 info;
    if (isReal) {
        *ok = zip->getCurrentFileInfo(&info);
    } else {
        *ok = true;
        info.compressedSize = 0;
        info.crc = 0;
        info.diskNumberStart = 0;
        info.externalAttr = 0;
        info.flags = 0;
        info.internalAttr = 0;
        info.method = 0;
        info.uncompressedSize = 0;
        info.versionCreated = info.versionNeeded = 0;
    }
    info.name = relativeName;
    return info;
}

static void QuaZipDir_convertInfoList(const QList<QuaZipFileInfo64> &from,
                                      QList<QuaZipFileInfo64> &to)
{
    to = from;
}

static void QuaZipDir_convertInfoList(const QList<QuaZipFileInfo64> &from,
                                      QStringList &to)
{
    to.clear();
    for (const auto& file : from) {
        to.append(file.name);
    }
}

static void QuaZipDir_convertInfoList(const QList<QuaZipFileInfo64> &from,
                                      QList<QuaZipFileInfo> &to)
{
    to.clear();
    for (const auto& file : from) {
        QuaZipFileInfo info32;
        file.toQuaZipFileInfo(info32);
        to.append(info32);
    }
}

/// \cond internal
/**
  An utility class to restore the current file.
  */
class QuaZipDirRestoreCurrent {
public:
    inline QuaZipDirRestoreCurrent(QuaZip *zip):
        zip(zip), currentFile(zip->getCurrentFileName()) {}
    inline ~QuaZipDirRestoreCurrent()
    {
        zip->setCurrentFile(currentFile);
    }
private:
    QuaZip *zip;
    QString currentFile;
};
/// \endcond

/// \cond internal
class QuaZipDirComparator
{
    private:
        QDir::SortFlags sort;
        static QString getExtension(const QString &name);
        int compareStrings(const QString &string1, const QString &string2);
    public:
        inline QuaZipDirComparator(QDir::SortFlags sort): sort(sort) {}
        bool operator()(const QuaZipFileInfo64 &info1, const QuaZipFileInfo64 &info2);
};

QString QuaZipDirComparator::getExtension(const QString &name)
{
        if (name.endsWith(QLatin1String(".")) || name.indexOf(QLatin1String("."), 1) == -1)
            return QLatin1String("");
        return name.mid(name.lastIndexOf(QLatin1String(".")) + 1);
}

int QuaZipDirComparator::compareStrings(const QString &string1,
        const QString &string2)
{
    if (sort & QDir::LocaleAware) {
        if (sort & QDir::IgnoreCase) {
            return string1.toLower().localeAwareCompare(string2.toLower());
        }
        return string1.localeAwareCompare(string2);
    }
    return string1.compare(string2, (sort & QDir::IgnoreCase) ? Qt::CaseInsensitive : Qt::CaseSensitive);
}

bool QuaZipDirComparator::operator()(const QuaZipFileInfo64 &info1,
        const QuaZipFileInfo64 &info2)
{
    QDir::SortFlags order = sort
        & (QDir::Name | QDir::Time | QDir::Size | QDir::Type);
    if ((sort & QDir::DirsFirst) == QDir::DirsFirst || (sort & QDir::DirsLast) == QDir::DirsLast) {
        if (info1.name.endsWith(QLatin1String("/")) && !info2.name.endsWith(QLatin1String("/"))) {
            return (sort & QDir::DirsFirst) == QDir::DirsFirst;
        }
        if (!info1.name.endsWith(QLatin1String("/")) && info2.name.endsWith(QLatin1String("/"))) {
            return (sort & QDir::DirsLast) == QDir::DirsLast;
        }
    }
    bool result;
    int extDiff;
    switch (order) {
        case QDir::Name:
            result = compareStrings(info1.name, info2.name) < 0;
            break;
        case QDir::Type:
            extDiff = compareStrings(getExtension(info1.name),
                    getExtension(info2.name));
            if (extDiff == 0) {
                result = compareStrings(info1.name, info2.name) < 0;
            } else {
                result = extDiff < 0;
            }
            break;
        case QDir::Size:
            if (info1.uncompressedSize == info2.uncompressedSize) {
                result = compareStrings(info1.name, info2.name) < 0;
            } else {
                result = info1.uncompressedSize < info2.uncompressedSize;
            }
            break;
        case QDir::Time:
            if (info1.dateTime == info2.dateTime) {
                result = compareStrings(info1.name, info2.name) < 0;
            } else {
                result = info1.dateTime < info2.dateTime;
            }
            break;
        default:
            qWarning("QuaZipDirComparator(): Invalid sort mode 0x%2X",
                    static_cast<unsigned>(sort));
            return false;
    }
    return (sort & QDir::Reversed) ? !result : result;
}

template<typename TFileInfoList>
bool QuaZipDirPrivate::entryInfoList(QStringList nameFilters,
    QDir::Filters filter, QDir::SortFlags sort, TFileInfoList &result) const
{
    QString basePath = simplePath();
    if (!basePath.isEmpty())
        basePath += QLatin1String("/");
    int baseLength = basePath.length();
    result.clear();
    QuaZipDirRestoreCurrent saveCurrent(zip);
    if (!zip->goToFirstFile()) {
        return zip->getZipError() == UNZ_OK;
    }
    QDir::Filters fltr = filter;
    if (fltr == QDir::NoFilter)
        fltr = this->filter;
    if (fltr == QDir::NoFilter)
        fltr = QDir::AllEntries;
    QStringList nmfltr = nameFilters;
    if (nmfltr.isEmpty())
        nmfltr = this->nameFilters;
    QSet<QString> dirsFound;
    QList<QuaZipFileInfo64> list;
    do {
        QString name = zip->getCurrentFileName();
        if (!name.startsWith(basePath))
            continue;
        QString relativeName = name.mid(baseLength);
        if (relativeName.isEmpty())
            continue;
        bool isDir = false;
        bool isReal = true;
        if (relativeName.contains(QLatin1String("/"))) {
            int indexOfSlash = relativeName.indexOf(QLatin1String("/"));
            // something like "subdir/"
            isReal = indexOfSlash == relativeName.length() - 1;
            relativeName = relativeName.left(indexOfSlash + 1);
            if (dirsFound.contains(relativeName))
                continue;
            isDir = true;
        }
        dirsFound.insert(relativeName);
        if ((fltr & QDir::Dirs) == 0 && isDir)
            continue;
        if ((fltr & QDir::Files) == 0 && !isDir)
            continue;
        if (!nmfltr.isEmpty() && !QDir::match(nmfltr, relativeName))
            continue;
        bool ok;
        QuaZipFileInfo64 info = QuaZipDir_getFileInfo(zip, &ok, relativeName,
            isReal);
        if (!ok) {
            return false;
        }
        list.append(info);
    } while (zip->goToNextFile());
    QDir::SortFlags srt = sort;
    if (srt == QDir::NoSort)
        srt = sorting;
#ifdef QUAZIP_QUAZIPDIR_DEBUG
    qDebug("QuaZipDirPrivate::entryInfoList(): before sort:");
    foreach (QuaZipFileInfo64 info, list) {
        qDebug("%s\t%s", info.name.toUtf8().constData(),
                info.dateTime.toString(Qt::ISODate).toUtf8().constData());
    }
#endif
    if (srt != QDir::NoSort && (srt & QDir::Unsorted) != QDir::Unsorted) {
        if (QuaZip::convertCaseSensitivity(caseSensitivity)
                == Qt::CaseInsensitive)
            srt |= QDir::IgnoreCase;
        QuaZipDirComparator lessThan(srt);
        quazip_sort(list.begin(), list.end(), lessThan);
    }
    QuaZipDir_convertInfoList(list, result);
    return true;
}

/// \endcond

QList<QuaZipFileInfo> QuaZipDir::entryInfoList(const QStringList &nameFilters,
    QDir::Filters filters, QDir::SortFlags sort) const
{
    QList<QuaZipFileInfo> result;
    if (!d->entryInfoList(nameFilters, filters, sort, result))
        return QList<QuaZipFileInfo>();
    return result;
}

QList<QuaZipFileInfo> QuaZipDir::entryInfoList(QDir::Filters filters,
    QDir::SortFlags sort) const
{
    return entryInfoList(QStringList(), filters, sort);
}

QList<QuaZipFileInfo64> QuaZipDir::entryInfoList64(const QStringList &nameFilters,
    QDir::Filters filters, QDir::SortFlags sort) const
{
    QList<QuaZipFileInfo64> result;
    if (!d->entryInfoList(nameFilters, filters, sort, result))
        return QList<QuaZipFileInfo64>();
    return result;
}

QList<QuaZipFileInfo64> QuaZipDir::entryInfoList64(QDir::Filters filters,
    QDir::SortFlags sort) const
{
    return entryInfoList64(QStringList(), filters, sort);
}

QStringList QuaZipDir::entryList(const QStringList &nameFilters,
    QDir::Filters filters, QDir::SortFlags sort) const
{
    QStringList result;
    if (!d->entryInfoList(nameFilters, filters, sort, result))
        return QStringList();
    return result;
}

QStringList QuaZipDir::entryList(QDir::Filters filters,
    QDir::SortFlags sort) const
{
    return entryList(QStringList(), filters, sort);
}

bool QuaZipDir::exists(const QString &filePath) const
{
    if (filePath == QLatin1String("/") || filePath.isEmpty())
        return true;
    QString fileName = filePath;
    if (fileName.endsWith(QLatin1String("/")))
        fileName.chop(1);
    if (fileName.contains(QLatin1String("/"))) {
        QFileInfo fileInfo(fileName);
#ifdef QUAZIP_QUAZIPDIR_DEBUG
        qDebug("QuaZipDir::exists(): fileName=%s, fileInfo.fileName()=%s, "
                "fileInfo.path()=%s", fileName.toUtf8().constData(),
                fileInfo.fileName().toUtf8().constData(),
                fileInfo.path().toUtf8().constData());
#endif
        QuaZipDir dir(*this);
        return dir.cd(fileInfo.path()) && dir.exists(fileInfo.fileName());
    }
    if (fileName == QLatin1String("..")) {
        return !isRoot();
    }
    if (fileName == QLatin1String(".")) {
        return true;
    }
    QStringList entries = entryList(QDir::AllEntries, QDir::NoSort);
#ifdef QUAZIP_QUAZIPDIR_DEBUG
            qDebug("QuaZipDir::exists(): looking for %s",
                    fileName.toUtf8().constData());
            for (const auto& entry : entries) {
                qDebug("QuaZipDir::exists(): entry: %s",
                        entry.toUtf8().constData());
            }
#endif
            Qt::CaseSensitivity cs = QuaZip::convertCaseSensitivity(
                    d->caseSensitivity);
            if (filePath.endsWith(QLatin1String("/"))) {
                return entries.contains(filePath, cs);
            }
            return entries.contains(fileName, cs) ||
                   entries.contains(fileName + QLatin1String("/"), cs);
}

bool QuaZipDir::exists() const
{
    return QuaZipDir(d->zip).exists(d->dir);
}

QString QuaZipDir::filePath(const QString &fileName) const
{
    return QDir(d->dir).filePath(fileName);
}

QDir::Filters QuaZipDir::filter()
{
    return d->filter;
}

bool QuaZipDir::isRoot() const
{
    return d->simplePath().isEmpty();
}

QStringList QuaZipDir::nameFilters() const
{
    return d->nameFilters;
}

QString QuaZipDir::path() const
{
    return d->dir;
}

QString QuaZipDir::relativeFilePath(const QString &fileName) const
{
    return QDir(QLatin1String("/") + d->dir).relativeFilePath(fileName);
}

void QuaZipDir::setCaseSensitivity(QuaZip::CaseSensitivity caseSensitivity)
{
    d->caseSensitivity = caseSensitivity;
}

void QuaZipDir::setFilter(QDir::Filters filters)
{
    d->filter = filters;
}

void QuaZipDir::setNameFilters(const QStringList &nameFilters)
{
    d->nameFilters = nameFilters;
}

void QuaZipDir::setPath(const QString &path)
{
    QString newDir = path;
    if (newDir == QLatin1String("/")) {
        d->dir = QLatin1String("");
    } else {
        if (newDir.endsWith(QLatin1String("/")))
            newDir.chop(1);
        if (newDir.startsWith(QLatin1String("/")))
            newDir = newDir.mid(1);
        d->dir = newDir;
    }
}

void QuaZipDir::setSorting(QDir::SortFlags sort)
{
    d->sorting = sort;
}

QDir::SortFlags QuaZipDir::sorting() const
{
    return d->sorting;
}
