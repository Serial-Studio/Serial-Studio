#ifndef QUAADLER32_H
#define QUAADLER32_H

/*
Copyright (C) 2010 Adam Walczak
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

#include <QtCore/QByteArray>

#include "quachecksum32.h"

/// Adler32 checksum
/** \class QuaAdler32 quaadler32.h <quazip/quaadler32.h>
 * This class wrappers the adler32 function with the QuaChecksum32 interface.
 * See QuaChecksum32 for more info.
 */
class QUAZIP_EXPORT QuaAdler32 : public QuaChecksum32
{

public:
	QuaAdler32();

	quint32 calculate(const QByteArray &data) override;

	void reset() override;
	void update(const QByteArray &buf) override;
	quint32 value() override;

private:
	quint32 checksum;
};

#endif //QUAADLER32_H
