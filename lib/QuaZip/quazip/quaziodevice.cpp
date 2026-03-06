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

#include "quaziodevice.h"

#define QUAZIO_INBUFSIZE 4096
#define QUAZIO_OUTBUFSIZE 4096

/// \cond internal
class QuaZIODevicePrivate {
    friend class QuaZIODevice;
    QuaZIODevicePrivate(QIODevice *io, QuaZIODevice *q);
    ~QuaZIODevicePrivate();
    QIODevice *io;
    QuaZIODevice *q;
    z_stream zins;
    z_stream zouts;
    char *inBuf{nullptr};
    int inBufPos{0};
    int inBufSize{0};
    char *outBuf{nullptr};
    int outBufPos{0};
    int outBufSize{0};
    bool zBufError{false};
    bool atEnd{false};
    bool flush(int sync);
    int doFlush(QString &error);
};

QuaZIODevicePrivate::QuaZIODevicePrivate(QIODevice *io, QuaZIODevice *q):
  io(io),
  q(q)
{
  zins.zalloc = (alloc_func) nullptr;
  zins.zfree = (free_func) nullptr;
  zins.opaque = nullptr;
  zouts.zalloc = (alloc_func) nullptr;
  zouts.zfree = (free_func) nullptr;
  zouts.opaque = nullptr;
  inBuf = new char[QUAZIO_INBUFSIZE];
  outBuf = new char[QUAZIO_OUTBUFSIZE];
#ifdef QUAZIP_ZIODEVICE_DEBUG_OUTPUT
  debug.setFileName("debug.out");
  debug.open(QIODevice::WriteOnly);
#endif
#ifdef QUAZIP_ZIODEVICE_DEBUG_INPUT
  indebug.setFileName("debug.in");
  indebug.open(QIODevice::WriteOnly);
#endif
}

QuaZIODevicePrivate::~QuaZIODevicePrivate()
{
#ifdef QUAZIP_ZIODEVICE_DEBUG_OUTPUT
  debug.close();
#endif
#ifdef QUAZIP_ZIODEVICE_DEBUG_INPUT
  indebug.close();
#endif
  delete[] inBuf;
  delete[] outBuf;
}

bool QuaZIODevicePrivate::flush(int sync)
{
    QString error;
    if (doFlush(error) < 0) {
        q->setErrorString(error);
        return false;
    }
    // can't flush buffer, some data is still waiting
    if (outBufPos < outBufSize)
        return true;
    Bytef c = 0;
    zouts.next_in = &c; // fake input buffer
    zouts.avail_in = 0; // of zero size
    do {
        zouts.next_out = reinterpret_cast<Bytef *>(outBuf);
        zouts.avail_out = QUAZIO_OUTBUFSIZE;
        int result = deflate(&zouts, sync);
        switch (result) {
        case Z_OK:
        case Z_STREAM_END:
          outBufSize = reinterpret_cast<char *>(zouts.next_out) - outBuf;
          if (doFlush(error) < 0) {
              q->setErrorString(error);
              return false;
          }
          if (outBufPos < outBufSize)
              return true;
          break;
        case Z_BUF_ERROR: // nothing to write?
          return true;
        default:
          q->setErrorString(QString::fromLocal8Bit(zouts.msg));
          return false;
        }
    } while (zouts.avail_out == 0);
    return true;
}

int QuaZIODevicePrivate::doFlush(QString &error)
{
  int flushed = 0;
  while (outBufPos < outBufSize) {
    int more = io->write(outBuf + outBufPos, outBufSize - outBufPos);
    if (more == -1) {
      error = io->errorString();
      return -1;
    }
    if (more == 0)
      break;
    outBufPos += more;
    flushed += more;
  }
  if (outBufPos == outBufSize) {
    outBufPos = outBufSize = 0;
  }
  return flushed;
}

/// \endcond

// #define QUAZIP_ZIODEVICE_DEBUG_OUTPUT
// #define QUAZIP_ZIODEVICE_DEBUG_INPUT
#ifdef QUAZIP_ZIODEVICE_DEBUG_OUTPUT
#include <QtCore/QFile>
static QFile debug;
#endif
#ifdef QUAZIP_ZIODEVICE_DEBUG_INPUT
#include <QtCore/QFile>
static QFile indebug;
#endif

QuaZIODevice::QuaZIODevice(QIODevice *io, QObject *parent):
    QIODevice(parent),
    d(new QuaZIODevicePrivate(io, this))
{
  connect(io, SIGNAL(readyRead()), SIGNAL(readyRead()));
}

QuaZIODevice::~QuaZIODevice()
{
    if (isOpen())
        close();
    delete d;
}

QIODevice *QuaZIODevice::getIoDevice() const
{
    return d->io;
}

bool QuaZIODevice::open(QIODevice::OpenMode mode)
{
    if ((mode & QIODevice::Append) != 0) {
        setErrorString(tr("QIODevice::Append is not supported for"
                    " QuaZIODevice"));
        return false;
    }
    if ((mode & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        setErrorString(tr("QIODevice::ReadWrite is not supported for"
                    " QuaZIODevice"));
        return false;
    }
    if ((mode & QIODevice::ReadOnly) != 0) {
        if (inflateInit(&d->zins) != Z_OK) {
            setErrorString(QString::fromLocal8Bit(d->zins.msg));
            return false;
        }
    }
    if ((mode & QIODevice::WriteOnly) != 0) {
        if (deflateInit(&d->zouts, Z_DEFAULT_COMPRESSION) != Z_OK) {
            setErrorString(QString::fromLocal8Bit(d->zouts.msg));
            return false;
        }
    }
    return QIODevice::open(mode);
}

void QuaZIODevice::close()
{
    if ((openMode() & QIODevice::ReadOnly) != 0) {
        if (inflateEnd(&d->zins) != Z_OK) {
            setErrorString(QString::fromLocal8Bit(d->zins.msg));
        }
    }
    if ((openMode() & QIODevice::WriteOnly) != 0) {
        d->flush(Z_FINISH);
        if (deflateEnd(&d->zouts) != Z_OK) {
            setErrorString(QString::fromLocal8Bit(d->zouts.msg));
        }
    }
    QIODevice::close();
}

qint64 QuaZIODevice::readData(char *data, qint64 maxSize)
{
  int read = 0;
  while (read < maxSize) {
    if (d->inBufPos == d->inBufSize) {
      d->inBufPos = 0;
      d->inBufSize = d->io->read(d->inBuf, QUAZIO_INBUFSIZE);
      if (d->inBufSize == -1) {
        d->inBufSize = 0;
        setErrorString(d->io->errorString());
        return -1;
      }
      if (d->inBufSize == 0)
        break;
    }
    while (read < maxSize && d->inBufPos < d->inBufSize) {
      d->zins.next_in = reinterpret_cast<Bytef *>(d->inBuf + d->inBufPos);
      d->zins.avail_in = d->inBufSize - d->inBufPos;
      d->zins.next_out = reinterpret_cast<Bytef *>(data + read);
      d->zins.avail_out = static_cast<uInt>(maxSize - read); // hope it's less than 2GB
      int more = 0;
      switch (inflate(&d->zins, Z_SYNC_FLUSH)) {
      case Z_OK:
        read = reinterpret_cast<char *>(d->zins.next_out) - data;
        d->inBufPos = reinterpret_cast<z_const char *>(d->zins.next_in) - d->inBuf;
        break;
      case Z_STREAM_END:
        read = reinterpret_cast<char *>(d->zins.next_out) - data;
        d->inBufPos = reinterpret_cast<z_const char *>(d->zins.next_in) - d->inBuf;
        d->atEnd = true;
        return read;
      case Z_BUF_ERROR: // this should never happen, but just in case
        if (!d->zBufError) {
          qWarning("Z_BUF_ERROR detected with %d/%d in/out, weird",
              d->zins.avail_in, d->zins.avail_out);
          d->zBufError = true;
        }
        memmove(d->inBuf, d->inBuf + d->inBufPos, d->inBufSize - d->inBufPos);
        d->inBufSize -= d->inBufPos;
        d->inBufPos = 0;
        more = d->io->read(d->inBuf + d->inBufSize, QUAZIO_INBUFSIZE - d->inBufSize);
        if (more == -1) {
          setErrorString(d->io->errorString());
          return -1;
        }
        if (more == 0)
          return read;
        d->inBufSize += more;
        break;
      default:
        setErrorString(QString::fromLocal8Bit(d->zins.msg));
        return -1;
      }
    }
  }
#ifdef QUAZIP_ZIODEVICE_DEBUG_INPUT
  indebug.write(data, read);
#endif
  return read;
}

qint64 QuaZIODevice::writeData(const char *data, qint64 maxSize)
{
  int written = 0;
  QString error;
  if (d->doFlush(error) == -1) {
    setErrorString(error);
    return -1;
  }
  while (written < maxSize) {
      // there is some data waiting in the output buffer
    if (d->outBufPos < d->outBufSize)
      return written;
    d->zouts.next_in = (Bytef *) (data + written);
    d->zouts.avail_in = static_cast<uInt>(maxSize - written); // hope it's less than 2GB
    d->zouts.next_out = reinterpret_cast<Bytef *>(d->outBuf);
    d->zouts.avail_out = QUAZIO_OUTBUFSIZE;
    switch (deflate(&d->zouts, Z_NO_FLUSH)) {
    case Z_OK:
      written = reinterpret_cast<z_const char *>(d->zouts.next_in) - data;
      d->outBufSize = reinterpret_cast<char *>(d->zouts.next_out) - d->outBuf;
      break;
    default:
      setErrorString(QString::fromLocal8Bit(d->zouts.msg));
      return -1;
    }
    if (d->doFlush(error) == -1) {
      setErrorString(error);
      return -1;
    }
  }
#ifdef QUAZIP_ZIODEVICE_DEBUG_OUTPUT
  debug.write(data, written);
#endif
  return written;
}

bool QuaZIODevice::flush()
{
    return d->flush(Z_SYNC_FLUSH);
}

bool QuaZIODevice::isSequential() const
{
    return true;
}

bool QuaZIODevice::atEnd() const
{
    // Here we MUST check QIODevice::bytesAvailable() because WE
    // might have reached the end, but QIODevice didn't--
    // it could have simply pre-buffered all remaining data.
    return (openMode() == NotOpen) || (QIODevice::bytesAvailable() == 0 && d->atEnd);
}

qint64 QuaZIODevice::bytesAvailable() const
{
    // If we haven't recevied Z_STREAM_END, it means that
    // we have at least one more input byte available.
    // Plus whatever QIODevice has buffered.
    return (d->atEnd ? 0 : 1) + QIODevice::bytesAvailable();
}
