/***********************************************************************

qcomplexnumber.h - Header file for QComplexNumber

Template class for handling complex numbers with a real and imaginary
part. 

************************************************************************

This file is part of QRealFourier.

QRealFourier is free software: you can redistribute it and/or modify it
under the terms of the Lesser GNU General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Foobar is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser GNU General Public
License for more details.

You should have received a copy of the Lesser GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************

Copyright © 2012 - 2013 Christoph Stallmann, University of Pretoria

Developer: Christoph Stallmann
University of Pretoria
Department of Computer Science

http://www.visore.org
http://sourceforge.net/projects/qrealfourier
http://github.com/visore/QRealFourier

qrealfourier@visore.org
qrealfourier@gmail.com

***********************************************************************/

#ifndef QCOMPLEXNUMBER_H
#define QCOMPLEXNUMBER_H

template <typename T>
class QComplexNumber
{

	public:

		QComplexNumber();
		QComplexNumber(T real, T imaginary);

		void setReal(T real);
		void setImaginary(T imaginary);
		void set(T real, T imaginary);
		void set(QComplexNumber complex);

		T real();
		T imaginary();

		QComplexNumber<T>& operator+=(const QComplexNumber<T> &number);
		QComplexNumber<T>& operator-=(const QComplexNumber<T> &number);
		QComplexNumber<T>& operator/=(const QComplexNumber<T> &number);
		QComplexNumber<T>& operator*=(const QComplexNumber<T> &number);

		QComplexNumber<T>& operator/=(const T &value);
		QComplexNumber<T>& operator*=(const T &value);

	private:

		T mReal;
		T mImaginary;

};

typedef QComplexNumber<float> QComplexFloat;
typedef QComplexNumber<double> QComplexDouble;
typedef QComplexNumber<int> QComplexInteger;

#endif
