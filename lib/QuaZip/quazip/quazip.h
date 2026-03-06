#ifndef QUA_ZIP_H
#define QUA_ZIP_H

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

Original ZIP package is copyrighted by Gilles Vollant, see
quazip/(un)zip.h files for details, basically it's zlib license.
 **/

#include <QtCore/QString>
#include <QtCore/QStringList>
#include "quazip_qt_compat.h"

#include "zip.h"
#include "unzip.h"

#include "quazip_global.h"
#include "quazipfileinfo.h"

// just in case it will be defined in the later versions of the ZIP/UNZIP
#ifndef UNZ_OPENERROR
// define additional error code
#define UNZ_OPENERROR -1000
#endif

class QuaZipPrivate;

/// ZIP archive.
/** \class QuaZip quazip.h <quazip/quazip.h>
 * This class implements basic interface to the ZIP archive. It can be
 * used to read table contents of the ZIP archive and retreiving
 * information about the files inside it.
 *
 * You can also use this class to open files inside archive by passing
 * pointer to the instance of this class to the constructor of the
 * QuaZipFile class. But see QuaZipFile::QuaZipFile(QuaZip*, QObject*)
 * for the possible pitfalls.
 *
 * This class is indended to provide interface to the ZIP subpackage of
 * the ZIP/UNZIP package as well as to the UNZIP subpackage. But
 * currently it supports only UNZIP.
 *
 * The use of this class is simple - just create instance using
 * constructor, then set ZIP archive file name using setFile() function
 * (if you did not passed the name to the constructor), then open() and
 * then use different functions to work with it! Well, if you are
 * paranoid, you may also wish to call close before destructing the
 * instance, to check for errors on close.
 *
 * You may also use getUnzFile() and getZipFile() functions to get the
 * ZIP archive handle and use it with ZIP/UNZIP package API directly.
 *
 * This class supports localized file names inside ZIP archive, but you
 * have to set up proper codec with setCodec() function. By default,
 * locale codec will be used, which is probably ok for UNIX systems, but
 * will almost certainly fail with ZIP archives created in Windows. This
 * is because Windows ZIP programs have strange habit of using DOS
 * encoding for file names in ZIP archives. For example, ZIP archive
 * with cyrillic names created in Windows will have file names in \c
 * IBM866 encoding instead of \c WINDOWS-1251. I think that calling one
 * function is not much trouble, but for true platform independency it
 * would be nice to have some mechanism for file name encoding auto
 * detection using locale information. Does anyone know a good way to do
 * it?
 **/
class QUAZIP_EXPORT QuaZip {
  friend class QuaZipPrivate;
  public:
    /// Useful constants.
    enum Constants {
      MAX_FILE_NAME_LENGTH=256 /**< Maximum file name length. Taken from
                                 \c UNZ_MAXFILENAMEINZIP constant in
                                 unzip.c. */
    };
    /// Open mode of the ZIP file.
    enum Mode {
      mdNotOpen, ///< ZIP file is not open. This is the initial mode.
      mdUnzip, ///< ZIP file is open for reading files inside it.
      mdCreate, ///< ZIP file was created with open() call.
      mdAppend, /**< ZIP file was opened in append mode. This refers to
                  * \c APPEND_STATUS_CREATEAFTER mode in ZIP/UNZIP package
                  * and means that zip is appended to some existing file
                  * what is useful when that file contains
                  * self-extractor code. This is obviously \em not what
                  * you whant to use to add files to the existing ZIP
                  * archive.
                  **/
      mdAdd ///< ZIP file was opened for adding files in the archive.
    };
    /// Case sensitivity for the file names.
    /** This is what you specify when accessing files in the archive.
     * Works perfectly fine with any characters thanks to Qt's great
     * unicode support. This is different from ZIP/UNZIP API, where
     * only US-ASCII characters was supported.
     **/
    enum CaseSensitivity {
      csDefault=0, ///< Default for platform. Case sensitive for UNIX, not for Windows.
      csSensitive=1, ///< Case sensitive.
      csInsensitive=2 ///< Case insensitive.
    };
    /// Returns the actual case sensitivity for the specified QuaZip one.
    /**
      \param cs The value to convert.
      \returns If CaseSensitivity::csDefault, then returns the default
      file name case sensitivity for the platform. Otherwise, just
      returns the appropriate value from the Qt::CaseSensitivity enum.
      */
    static Qt::CaseSensitivity convertCaseSensitivity(
            CaseSensitivity cs);
  private:
    QuaZipPrivate *p;
  public:
    /// Constructs QuaZip object.
    /** Call setName() before opening constructed object. */
    QuaZip();
    /// Constructs QuaZip object associated with ZIP file \a zipName.
    QuaZip(const QString& zipName);
    /// Constructs QuaZip object associated with ZIP file represented by \a ioDevice.
    /** The IO device must be seekable, otherwise an error will occur when opening. */
    QuaZip(QIODevice *ioDevice);
    // not (and will not be) implemented
    QuaZip(const QuaZip& that) = delete;
    // not (and will not be) implemented
    QuaZip& operator=(const QuaZip& that) = delete;
    /// Destroys QuaZip object.
    /** Calls close() if necessary. */
    ~QuaZip();
    /// Opens ZIP file.
    /**
     * Argument \a mode specifies open mode of the ZIP archive. See Mode
     * for details. Note that there is zipOpen2() function in the
     * ZIP/UNZIP API which accepts \a globalcomment argument, but it
     * does not use it anywhere, so this open() function does not have this
     * argument. See setComment() if you need to set global comment.
     *
     * If the ZIP file is accessed via explicitly set QIODevice, then
     * this device is opened in the necessary mode. If the device was
     * already opened by some other means, then QuaZip checks if the
     * open mode is compatible to the mode needed for the requested operation.
     * If necessary, seeking is performed to position the device properly.
     *
     * \return \c true if successful, \c false otherwise.
     *
     * \note ZIP/UNZIP API open calls do not return error code - they
     * just return \c null indicating an error. But to make things
     * easier, quazip.h header defines additional error code \c
     * UNZ_ERROROPEN and getZipError() will return it if the open call
     * of the ZIP/UNZIP API returns \c null.
     *
     * Argument \a ioApi specifies IO function set for ZIP/UNZIP
     * package to use. See unzip.h, zip.h and ioapi.h for details. Note
     * that IO API for QuaZip is different from the original package.
     * The file path argument was changed to be of type \c voidpf, and
     * QuaZip passes a QIODevice pointer there. This QIODevice is either
     * set explicitly via setIoDevice() or the QuaZip(QIODevice*)
     * constructor, or it is created internally when opening the archive
     * by its file name. The default API (qioapi.cpp) just delegates
     * everything to the QIODevice API. Not only this allows to use a
     * QIODevice instead of file name, but also has a nice side effect
     * of raising the file size limit from 2G to 4G (in non-zip64 archives).
     *
     * \note If the zip64 support is needed, the ioApi argument \em must be null
     * because due to the backwards compatibility issues it can be used to
     * provide a 32-bit API only.
     *
     * \note If the \ref QuaZip::setAutoClose() "no-auto-close" feature is used,
     * then the \a ioApi argument \em should be null because the old API
     * doesn't support the 'fake close' operation, causing slight memory leaks
     * and other possible troubles (like closing the output device in case
     * when an error occurs during opening).
     *
     * In short: just forget about the \a ioApi argument and you'll be
     * fine.
     **/
    bool open(Mode mode, zlib_filefunc_def *ioApi =nullptr);
    /// Closes ZIP file.
    /** Call getZipError() to determine if the close was successful.
     *
     * If the file was opened by name, then the underlying QIODevice is closed
     * and deleted.
     *
     * If the underlying QIODevice was set explicitly using setIoDevice() or
     * the appropriate constructor, then it is closed if the auto-close flag
     * is set (which it is by default). Call setAutoClose() to clear the
     * auto-close flag if this behavior is undesirable.
     *
     * Since Qt 5.1, the QSaveFile was introduced. It breaks the QIODevice API
     * by making close() private and crashing the application if it is called
     * from the base class where it is public. It is an excellent example
     * of poor design that illustrates why you should never ever break
     * an is-a relationship between the base class and a subclass. QuaZip
     * works around this bug by checking if the QIODevice is an instance
     * of QSaveFile, using qobject_cast<>, and if it is, calls
     * QSaveFile::commit() instead of close(). It is a really ugly hack,
     * but at least it makes your programs work instead of crashing. Note that
     * if the auto-close flag is cleared, then this is a non-issue, and
     * commit() isn't called.
     *
     * Closing an already closed (or never opened) instance is safe,
     * regardless of whether the first close attempt was successful.
     * This second close does nothing, but is considered a success,
     * as far as getZipError() is concerned.
      */
    void close();
    /// Sets the codec used to encode/decode file names inside archive.
    /** This is necessary to access files in the ZIP archive created
     * under Windows with non-latin characters in file names. For
     * example, file names with cyrillic letters will be in \c IBM866
     * encoding.
     **/
    void setFileNameCodec(QTextCodec *fileNameCodec);
    /// Sets the codec used to encode/decode file names inside archive.
    /** \overload
     * Equivalent to calling setFileNameCodec(QTextCodec::codecForName(codecName));
     **/
    void setFileNameCodec(const char *fileNameCodecName);
    /// Sets the OS code (highest 8 bits of the “version made by” field) for new files.
    /** There is currently no way to specify this for each file individually,
        except by calling this function before opening each file. If this function is not called,
        then the default OS code will be used. The default code is set by calling
        setDefaultOsCode(). The default value at the moment of QuaZip creation will be used. */
    void setOsCode(uint osCode);
    /// Returns the OS code for new files.
    uint getOsCode() const;
    /// Returns the codec used to encode/decode comments inside archive.
    QTextCodec* getFileNameCodec() const;
    /// Sets the codec used to encode/decode comments inside archive.
    /** This codec defaults to locale codec, which is probably ok.
     **/
    void setCommentCodec(QTextCodec *commentCodec);
    /// Sets the codec used to encode/decode comments inside archive.
    /** \overload
     * Equivalent to calling setCommentCodec(QTextCodec::codecForName(codecName));
     **/
    void setCommentCodec(const char *commentCodecName);
    /// Returns the codec used to encode/decode comments inside archive.
    QTextCodec* getCommentCodec() const;
    /// Returns the name of the ZIP file.
    /** Returns null string if no ZIP file name has been set, for
     * example when the QuaZip instance is set up to use a QIODevice
     * instead.
     * \sa setZipName(), setIoDevice(), getIoDevice()
     **/
    QString getZipName() const;
    /// Sets the name of the ZIP file.
    /** Does nothing if the ZIP file is open.
     *
     * Does not reset error code returned by getZipError().
     * \sa setIoDevice(), getIoDevice(), getZipName()
     **/
    void setZipName(const QString& zipName);
    /// Returns the device representing this ZIP file.
    /** Returns null string if no device has been set explicitly, for
     * example when opening a ZIP file by name.
     * \sa setIoDevice(), getZipName(), setZipName()
     **/
    QIODevice *getIoDevice() const;
    /// Sets the device representing the ZIP file.
    /** Does nothing if the ZIP file is open.
     *
     * Does not reset error code returned by getZipError().
     * \sa getIoDevice(), getZipName(), setZipName()
     **/
    void setIoDevice(QIODevice *ioDevice);
    /// Returns the mode in which ZIP file was opened.
    Mode getMode() const;
    /// Returns \c true if ZIP file is open, \c false otherwise.
    bool isOpen() const;
    /// Returns the error code of the last operation.
    /** Returns \c UNZ_OK if the last operation was successful.
     *
     * Error code resets to \c UNZ_OK every time you call any function
     * that accesses something inside ZIP archive, even if it is \c
     * const (like getEntriesCount()). open() and close() calls reset
     * error code too. See documentation for the specific functions for
     * details on error detection.
     **/
    int getZipError() const;
    /// Returns number of the entries in the ZIP central directory.
    /** Returns negative error code in the case of error. The same error
     * code will be returned by subsequent getZipError() call.
     **/
    int getEntriesCount() const;
    /// Returns global comment in the ZIP file.
    QString getComment() const;
    /// Sets the global comment in the ZIP file.
    /** The comment will be written to the archive on close operation.
     * QuaZip makes a distinction between a null QByteArray() comment
     * and an empty &quot;&quot; comment in the QuaZip::mdAdd mode.
     * A null comment is the default and it means &quot;don't change
     * the comment&quot;. An empty comment removes the original comment.
     *
     * \sa open()
     **/
    void setComment(const QString& comment);
    /// Sets the current file to the first file in the archive.
    /** Returns \c true on success, \c false otherwise. Call
     * getZipError() to get the error code.
     **/
    bool goToFirstFile();
    /// Sets the current file to the next file in the archive.
    /** Returns \c true on success, \c false otherwise. Call
     * getZipError() to determine if there was an error.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     *
     * \note If the end of file was reached, getZipError() will return
     * \c UNZ_OK instead of \c UNZ_END_OF_LIST_OF_FILE. This is to make
     * things like this easier:
     * \code
     * for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
     *   // do something
     * }
     * if(zip.getZipError()==UNZ_OK) {
     *   // ok, there was no error
     * }
     * \endcode
     **/
    bool goToNextFile();
    /// Sets current file by its name.
    /** Returns \c true if successful, \c false otherwise. Argument \a
     * cs specifies case sensitivity of the file name. Call
     * getZipError() in the case of a failure to get error code.
     *
     * This is not a wrapper to unzLocateFile() function. That is
     * because I had to implement locale-specific case-insensitive
     * comparison.
     *
     * Here are the differences from the original implementation:
     *
     * - If the file was not found, error code is \c UNZ_OK, not \c
     *   UNZ_END_OF_LIST_OF_FILE (see also goToNextFile()).
     * - If this function fails, it unsets the current file rather than
     *   resetting it back to what it was before the call.
     *
     * If \a fileName is null string then this function unsets the
     * current file and return \c true. Note that you should close the
     * file first if it is open! See
     * QuaZipFile::QuaZipFile(QuaZip*,QObject*) for the details.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     *
     * \sa setFileNameCodec(), CaseSensitivity
     **/
    bool setCurrentFile(const QString& fileName, CaseSensitivity cs =csDefault);
    /// Returns \c true if the current file has been set.
    bool hasCurrentFile() const;
    /// Retrieves information about the current file.
    /** Fills the structure pointed by \a info. Returns \c true on
     * success, \c false otherwise. In the latter case structure pointed
     * by \a info remains untouched. If there was an error,
     * getZipError() returns error code.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     *
     * Does nothing and returns \c false in any of the following cases.
     * - ZIP is not open;
     * - ZIP does not have current file.
     *
     * In both cases getZipError() returns \c UNZ_OK since there
     * is no ZIP/UNZIP API call.
     *
     * This overload doesn't support zip64, but will work OK on zip64 archives
     * except that if one of the sizes (compressed or uncompressed) is greater
     * than 0xFFFFFFFFu, it will be set to exactly 0xFFFFFFFFu.
     *
     * \sa getCurrentFileInfo(QuaZipFileInfo64* info)const
     * \sa QuaZipFileInfo64::toQuaZipFileInfo(QuaZipFileInfo&)const
     **/
    bool getCurrentFileInfo(QuaZipFileInfo* info)const;
    /// Retrieves information about the current file.
    /** \overload
     *
     * This function supports zip64. If the archive doesn't use zip64, it is
     * completely equivalent to getCurrentFileInfo(QuaZipFileInfo* info)
     * except for the argument type.
     *
     * \sa
     **/
    bool getCurrentFileInfo(QuaZipFileInfo64* info)const;
    /// Returns the current file name.
    /** Equivalent to calling getCurrentFileInfo() and then getting \c
     * name field of the QuaZipFileInfo structure, but faster and more
     * convenient.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     **/
    QString getCurrentFileName()const;
    /// Returns \c unzFile handle.
    /** You can use this handle to directly call UNZIP part of the
     * ZIP/UNZIP package functions (see unzip.h).
     *
     * \warning When using the handle returned by this function, please
     * keep in mind that QuaZip class is unable to detect any changes
     * you make in the ZIP file state (e. g. changing current file, or
     * closing the handle). So please do not do anything with this
     * handle that is possible to do with the functions of this class.
     * Or at least return the handle in the original state before
     * calling some another function of this class (including implicit
     * destructor calls and calls from the QuaZipFile objects that refer
     * to this QuaZip instance!). So if you have changed the current
     * file in the ZIP archive - then change it back or you may
     * experience some strange behavior or even crashes.
     **/
    unzFile getUnzFile();
    /// Returns \c zipFile handle.
    /** You can use this handle to directly call ZIP part of the
     * ZIP/UNZIP package functions (see zip.h). Warnings about the
     * getUnzFile() function also apply to this function.
     **/
    zipFile getZipFile();
    /// Changes the data descriptor writing mode.
    /**
      According to the ZIP format specification, a file inside archive
      may have a data descriptor immediately following the file
      data. This is reflected by a special flag in the local file header
      and in the central directory. By default, QuaZip sets this flag
      and writes the data descriptor unless both method and level were
      set to 0, in which case it operates in 1.0-compatible mode and
      never writes data descriptors.

      By setting this flag to false, it is possible to disable data
      descriptor writing, thus increasing compatibility with archive
      readers that don't understand this feature of the ZIP file format.

      Setting this flag affects all the QuaZipFile instances that are
      opened after this flag is set.

      The data descriptor writing mode is enabled by default.

      Note that if the ZIP archive is written into a QIODevice for which
      QIODevice::isSequential() returns \c true, then the data descriptor
      is mandatory and will be written even if this flag is set to false.

      \param enabled If \c true, enable local descriptor writing,
      disable it otherwise.

      \sa QuaZipFile::isDataDescriptorWritingEnabled()
      */
    void setDataDescriptorWritingEnabled(bool enabled);
    /// Returns the data descriptor default writing mode.
    /**
      \sa setDataDescriptorWritingEnabled()
      */
    bool isDataDescriptorWritingEnabled() const;
    /// Returns a list of files inside the archive.
    /**
      \return A list of file names or an empty list if there
      was an error or if the archive is empty (call getZipError() to
      figure out which).
      \sa getFileInfoList()
      */
    QStringList getFileNameList() const;
    /// Returns information list about all files inside the archive.
    /**
      \return A list of QuaZipFileInfo objects or an empty list if there
      was an error or if the archive is empty (call getZipError() to
      figure out which).

      This function doesn't support zip64, but will still work with zip64
      archives, converting results using QuaZipFileInfo64::toQuaZipFileInfo().
      If all file sizes are below 4 GB, it will work just fine.

      \sa getFileNameList()
      \sa getFileInfoList64()
      */
    QList<QuaZipFileInfo> getFileInfoList() const;
    /// Returns information list about all files inside the archive.
    /**
      \overload

      This function supports zip64.

      \sa getFileNameList()
      \sa getFileInfoList()
      */
    QList<QuaZipFileInfo64> getFileInfoList64() const;
    /// Enables the zip64 mode.
    /**
     * @param zip64 If \c true, the zip64 mode is enabled, disabled otherwise.
     *
     * Once this is enabled, all new files (until the mode is disabled again)
     * will be created in the zip64 mode, thus enabling the ability to write
     * files larger than 4 GB. By default, the zip64 mode is off due to
     * compatibility reasons.
     *
     * Note that this does not affect the ability to read zip64 archives in any
     * way.
     *
     * \sa isZip64Enabled()
     */
    void setZip64Enabled(bool zip64);
    /// Returns whether the zip64 mode is enabled.
    /**
     * @return \c true if and only if the zip64 mode is enabled.
     *
     * \sa setZip64Enabled()
     */
    bool isZip64Enabled() const;
    /// Enables the use of UTF-8 encoding for file names and comments text.
    /**
     * @param utf8 If \c true, the UTF-8 mode is enabled, disabled otherwise.
     *
     * Once this is enabled, the names of all new files and comments text (until
     * the mode is disabled again) will be encoded in UTF-8 encoding, and the
     * version to extract will be set to 6.3 (63) in ZIP header. By default,
     * the UTF-8 mode is off due to compatibility reasons.
     *
     * Note that when extracting ZIP archives, the UTF-8 mode is determined from
     * ZIP file header, not from this flag.
     *
     * \sa isUtf8Enabled()
     */
    void setUtf8Enabled(bool utf8);
    /// Returns whether the UTF-8 encoding mode is enabled.
    /**
     * @return \c true if and only if the UTF-8 mode is enabled.
     *
     * \sa setUtf8Enabled()
     */
    bool isUtf8Enabled() const;
    /// Returns the auto-close flag.
    /**
      @sa setAutoClose()
      */
    bool isAutoClose() const;
    /// Sets or unsets the auto-close flag.
    /**
      By default, QuaZip opens the underlying QIODevice when open() is called,
      and closes it when close() is called. In some cases, when the device
      is set explicitly using setIoDevice(), it may be desirable to
      leave the device open. If the auto-close flag is unset using this method,
      then the device isn't closed automatically if it was set explicitly.

      If it is needed to clear this flag, it is recommended to do so before
      opening the archive because otherwise QuaZip may close the device
      during the open() call if an error is encountered after the device
      is opened.

      If the device was not set explicitly, but rather the setZipName() or
      the appropriate constructor was used to set the ZIP file name instead,
      then the auto-close flag has no effect, and the internal device
      is closed nevertheless because there is no other way to close it.

      @sa isAutoClose()
      @sa setIoDevice()
      */
    void setAutoClose(bool autoClose) const;
    /// Sets the default file name codec to use.
    /**
     * The default codec is used by the constructors, so calling this function
     * won't affect the QuaZip instances already created at that moment.
     *
     * The codec specified here can be overriden by calling setFileNameCodec().
     * If neither function is called, QTextCodec::codecForLocale() will be used
     * to decode or encode file names. Use this function with caution if
     * the application uses other libraries that depend on QuaZip. Those
     * libraries can either call this function by themselves, thus overriding
     * your setting or can rely on the default encoding, thus failing
     * mysteriously if you change it. For these reasons, it isn't recommended
     * to use this function if you are developing a library, not an application.
     * Instead, ask your library users to call it in case they need specific
     * encoding.
     *
     * In most cases, using setFileNameCodec() instead is the right choice.
     * However, if you depend on third-party code that uses QuaZip, then the
     * reasons stated above can actually become a reason to use this function
     * in case the third-party code in question fails because it doesn't
     * understand the encoding you need and doesn't provide a way to specify it.
     * This applies to the JlCompress class as well, as it was contributed and
     * doesn't support explicit encoding parameters.
     *
     * In short: use setFileNameCodec() when you can, resort to
     * setDefaultFileNameCodec() when you don't have access to the QuaZip
     * instance.
     *
     * @param codec The codec to use by default. If null, resets to default.
     */
    static void setDefaultFileNameCodec(QTextCodec *codec);
    /**
     * @overload
     * Equivalent to calling
     * setDefaultFileNameCodec(QTextCodec::codecForName(codecName)).
     */
    static void setDefaultFileNameCodec(const char *codecName);
    /// Sets default OS code.
    /**
     * @sa setOsCode()
     */
    static void setDefaultOsCode(uint osCode);
    /// Returns default OS code.
    /**
     * @sa getOsCode()
     */
    static uint getDefaultOsCode();
};

#endif
