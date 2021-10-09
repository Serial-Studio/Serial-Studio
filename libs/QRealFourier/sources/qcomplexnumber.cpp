/***********************************************************************

qcomplexnumber.cpp - Source file for QComplexNumber

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

#include "qcomplexnumber.h"

template <typename T>
QComplexNumber<T>::QComplexNumber()
{
	mReal = 0;
	mImaginary = 0;
}

template <typename T>
QComplexNumber<T>::QComplexNumber(T real, T imaginary)
{
	mReal = real;
	mImaginary = imaginary;
}

template <typename T>
void QComplexNumber<T>::setReal(T real)
{
	mReal = real;
}

template <typename T>
void QComplexNumber<T>::setImaginary(T imaginary)
{
	mImaginary = imaginary;
}

template <typename T>
void QComplexNumber<T>::set(T real, T imaginary)
{
	mReal = real;
	mImaginary = imaginary;
}

template <typename T>
void QComplexNumber<T>::set(QComplexNumber complex)
{
	mReal = complex.real();
	mImaginary = complex.imaginary();
}

template <typename T>
T QComplexNumber<T>::real()
{
	return mReal;
}

template <typename T>
T QComplexNumber<T>::imaginary()
{
	return mImaginary;
}

template <typename T>
QComplexNumber<T>& QComplexNumber<T>::operator+=(const QComplexNumber<T> &number)
{
	mReal += number.mReal;
	mImaginary += number.mImaginary;
	return *this;
}

template <typename T>
QComplexNumber<T>& QComplexNumber<T>::operator-=(const QComplexNumber<T> &number)
{
	mReal -= number.mReal;
	mImaginary -= number.mImaginary;
	return *this;
}

template <typename T>
QComplexNumber<T>& QComplexNumber<T>::operator/=(const QComplexNumber<T> &number)
{
	mReal /= number.mReal;
	mImaginary /= number.mImaginary;
	return *this;
}

template <typename T>
QComplexNumber<T>& QComplexNumber<T>::operator*=(const QComplexNumber<T> &number)
{
	mReal *= number.mReal;
	mImaginary *= number.mImaginary;
	return *this;
}

template <typename T>
QComplexNumber<T>& QComplexNumber<T>::operator/=(const T &value)
{
	mReal /= value;
	mImaginary /= value;
	return *this;
}

template <typename T>
QComplexNumber<T>& QComplexNumber<T>::operator*=(const T &value)
{
	mReal *= value;
	mImaginary *= value;
	return *this;
}

template class QComplexNumber<short>;
template class QComplexNumber<int>;
template class QComplexNumber<long>;
template class QComplexNumber<float>;
template class QComplexNumber<double>;
