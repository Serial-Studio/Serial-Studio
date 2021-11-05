/***********************************************************************

qfourierfixedcalculator.cpp - Source file for QFourierFixedCalculator

Template class for calculating FFts of a fixed size specified by the
template parameter.

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

#include "qfourierfixedcalculator.h"

template <int T>
QFourierFixedCalculator<T>::QFourierFixedCalculator()
	: QFourierCalculator()
{
	setSize(mFourierTransform.get_length());
}

template <int T>
void QFourierFixedCalculator<T>::setSize(int size)
{
    (void) size;
}

template <int T>
void QFourierFixedCalculator<T>::forward()
{
	QFourierFixedCalculator<T>::mFourierTransform.do_fft(QFourierCalculator::mOutput, QFourierCalculator::mInput);
}

template <int T>
void QFourierFixedCalculator<T>::inverse()
{
	QFourierFixedCalculator<T>::mFourierTransform.do_ifft(QFourierCalculator::mInput, QFourierCalculator::mOutput);
}

template <int T>
void QFourierFixedCalculator<T>::rescale()
{
	QFourierFixedCalculator<T>::mFourierTransform.rescale(QFourierCalculator::mInput);
}

template class QFourierFixedCalculator<3>;
template class QFourierFixedCalculator<4>;
template class QFourierFixedCalculator<5>;
template class QFourierFixedCalculator<6>;
template class QFourierFixedCalculator<7>;
template class QFourierFixedCalculator<8>;
template class QFourierFixedCalculator<9>;
template class QFourierFixedCalculator<10>;
template class QFourierFixedCalculator<11>;
template class QFourierFixedCalculator<12>;
template class QFourierFixedCalculator<13>;
template class QFourierFixedCalculator<14>;
